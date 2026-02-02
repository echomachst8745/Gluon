#include "search.h"
#include "evaluate.h"
#include "board.h"
#include "movegenerator.h"
#include "transpositiontable.h"

#include <limits>
#include <cstdlib>
#include <vector>
#include <array>
#include <chrono>
#include <iostream>

namespace Gluon::Search {

std::atomic<bool> SearchStopped = false;
std::chrono::steady_clock::time_point SearchStartTime;
double SearchMaxTimeSeconds = 10.0;
int SearchMaxDepth = 20;

TranspositionTable::TranspositionTable transpositionTable;

namespace { // anonymous namespace for helper functions

SearchResult SearchAllCaptures(Board& board, double alpha, double beta)
{
    double evaluation = Evaluate::Evaluate(board);
    if (evaluation >= beta)
    {
        return SearchResult{ MoveGenerator::Move(), beta }; // Beta cutoff
    }
    if (evaluation > alpha)
    {
        alpha = evaluation;
    }

    if (SearchShouldStop())
    {
        auto result = SearchResult{ MoveGenerator::Move(), evaluation };
        result.searchInterrupted = true;

        return result;
    }

    auto moves = MoveGenerator::GenerateMoves(board, true); // Only captures
    if (moves.IsEmpty())
    {
        return SearchResult{ MoveGenerator::Move(), evaluation };
    }

    MoveGenerator::Move bestMove = moves.moves[0];

    for (int i = 0; i < moves.moveCount; ++i)
    {
        if (SearchShouldStop())
        {
            auto result = SearchResult{ bestMove, alpha };
            result.searchInterrupted = true;

            return result;
        }

        const auto& move = moves.moves[i];

        board.MakeMove(move);
        evaluation = -SearchAllCaptures(board, -beta, -alpha).evaluation;
        board.UnmakeLastMove();

        if (evaluation >= beta)
        {
            return SearchResult{ move, beta }; // Beta cutoff
        }

        if (evaluation > alpha)
        {
            alpha = evaluation;
            bestMove = move;
        }
    }

    return SearchResult{ bestMove, alpha };
}

SearchResult Search(Board& board, int depth, double alpha, double beta)
{
    const double originalAlpha = alpha;
    const std::uint64_t zobristHash = board.GetZobristHash();

    // Probe transposition table
    auto& ttEntry = transpositionTable.RetrieveEntry(zobristHash);
    if (ttEntry.IsValidEntry() && ttEntry.depth >= depth)
    {
        if (ttEntry.entryType == TranspositionTable::EXACT)
        {
            return SearchResult{ ttEntry.bestMove, ttEntry.evaluation };
        }
        else if (ttEntry.entryType == TranspositionTable::LOWERBOUND)
        {
            if (ttEntry.evaluation >= beta)
            {
                return SearchResult{ ttEntry.bestMove, ttEntry.evaluation };
            }

            if (ttEntry.evaluation > alpha)
            {
                alpha = ttEntry.evaluation;
            }
        }
        else if (ttEntry.entryType == TranspositionTable::UPPERBOUND)
        {
            if (ttEntry.evaluation <= alpha)
            {
                return SearchResult{ ttEntry.bestMove, ttEntry.evaluation };
            }

            if (ttEntry.evaluation < beta)
            {
                beta = ttEntry.evaluation;
            }
        }
    }

    if (depth == 0)
    {
        return SearchAllCaptures(board, alpha, beta);
    }

    auto moves = MoveGenerator::GenerateMoves(board);
    if (moves.IsEmpty())
    {
        if (board.IsCurrentPlayerInCheck())
        {
            return SearchResult{ MoveGenerator::Move(), -std::numeric_limits<double>::infinity() }; // Checkmate
        }

        return SearchResult{ MoveGenerator::Move(), 0.0 }; // Stalemate
    }

    MoveGenerator::Move bestMove = moves.moves[0];

    for (int i = 0; i < moves.moveCount; ++i)
    {
        if (SearchShouldStop())
        {
            auto result = SearchResult{ bestMove, alpha };
            result.searchInterrupted = true;

            return result;
        }

        const auto& move = moves.moves[i];

        board.MakeMove(move);
        double evaluation = -Search(board, depth - 1, -beta, -alpha).evaluation;
        board.UnmakeLastMove();

        if (evaluation >= beta)
        {
            transpositionTable.StoreEntry(zobristHash, beta, depth, TranspositionTable::LOWERBOUND, move);
            return SearchResult{ move, beta }; // Beta cutoff
        }
        if (evaluation > alpha)
        {
            alpha = evaluation;
            bestMove = move;
        }
    }

    TranspositionTable::EntryType entryType = (alpha > originalAlpha) ? TranspositionTable::EXACT : TranspositionTable::UPPERBOUND;
    transpositionTable.StoreEntry(zobristHash, alpha, depth, entryType, bestMove);

    return SearchResult{ bestMove, alpha };
}

} // anonymous namespace for helper functions

bool SearchShouldStop()
{
    if (SearchStopped)
    {
        return true;
    }

    auto currentTime = std::chrono::steady_clock::now();
    std::chrono::duration<double> elapsedSeconds = currentTime - SearchStartTime;

    if (elapsedSeconds.count() >= SearchMaxTimeSeconds)
    {
        return true;
    }

    return false;
}

SearchResult StartSearch(Board& board, bool fixedDepth)
{
    transpositionTable.Clear();

    SearchStartTime = std::chrono::steady_clock::now();
    SearchStopped = false;

    SearchResult finalResult = { MoveGenerator::Move(), -std::numeric_limits<double>::infinity() };
    for (int depth = (fixedDepth ? SearchMaxDepth : 1); depth <= SearchMaxDepth; ++depth)
    {
        double alpha = -std::numeric_limits<double>::infinity();
        double beta = std::numeric_limits<double>::infinity();

        auto depthResult = Search(board, depth, alpha, beta);

        if (depthResult.evaluation > finalResult.evaluation && !depthResult.searchInterrupted)
        {
            finalResult = depthResult;
        }

        if (SearchShouldStop())
        {
            break;
        }
    }

    return finalResult;
}

} // namespace Gluon::Search