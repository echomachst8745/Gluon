#include "search.h"
#include "evaluate.h"
#include "board.h"
#include "movegenerator.h"

#include <limits>
#include <cstdlib>
#include <vector>

namespace Gluon::Search {

namespace { // Anonymous namespace for helper functions

SearchResult AlphaBetaMinSearch(Board& board, int depth, double alpha, double beta)
{
    if (depth == 0)
    {
        return SearchResult{ MoveGenerator::Move{}, -Evaluate::Evaluate(board) };
    }

    auto moves = MoveGenerator::GenerateMoves(board);

    if (moves.empty())
    {
        if (board.IsPlayerInCheck())
        {
            return SearchResult{ MoveGenerator::Move{}, std::numeric_limits<double>::max() }; // Checkmate
        }

        return SearchResult{ MoveGenerator::Move{}, 0.0 }; // Stalemate
    }

    MoveGenerator::Move bestMove = moves[rand() % moves.size()];
    double bestEvaluation = std::numeric_limits<double>::max();
    for (const auto& move : moves)
    {
        board.MakeMove(move);
        double evaluation = AlphaBetaMaxSearch(board, depth - 1, alpha, beta).evaluation;
        board.UnmakeLastMove();
        if (evaluation < bestEvaluation)
        {
            bestEvaluation = evaluation;
            bestMove = move;

            if (evaluation < beta)
            {
                beta = evaluation;
            }
        }
        if (evaluation <= alpha)
        {
            return SearchResult{ move, evaluation }; // Alpha cutoff
        }
    }

    return SearchResult{ bestMove, bestEvaluation };
}

} // Anonymous namespace for helper functions

SearchResult AlphaBetaMaxSearch(Board& board, int depth, double alpha, double beta)
{
    if (depth == 0)
    {
        return SearchResult{ MoveGenerator::Move{}, Evaluate::Evaluate(board) };
    }

    auto moves = MoveGenerator::GenerateMoves(board);

    if (moves.empty())
    {
        if (board.IsPlayerInCheck())
        {
            return SearchResult{ MoveGenerator::Move{}, std::numeric_limits<double>::lowest() }; // Checkmate
        }
        
        return SearchResult{ MoveGenerator::Move{}, 0.0 }; // Stalemate
    }

    MoveGenerator::Move bestMove = moves[rand() % moves.size()];
    double bestEvaluation = std::numeric_limits<double>::lowest();
    for (const auto& move : moves)
    {
        board.MakeMove(move);
        double evaluation = AlphaBetaMinSearch(board, depth - 1, alpha, beta).evaluation;
        board.UnmakeLastMove();
        if (evaluation > bestEvaluation)
        {
            bestEvaluation = evaluation;
            bestMove = move;

            if (evaluation > alpha)
            {
                alpha = evaluation;
            }
        }
        if (evaluation >= beta)
        {
            return SearchResult{ move, evaluation }; // Beta cutoff
        }
    }

    return SearchResult{ bestMove, bestEvaluation };
}

} // namespace Gluon::Search