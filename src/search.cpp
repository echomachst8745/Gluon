#include "search.h"
#include "board.h"
#include "evaluate.h"
#include "movegenerator.h"
#include "engine.h"

#include <limits>
#include <algorithm>
#include <chrono>

namespace Gluon::Search {

std::atomic<bool> searchShouldStop = false;
std::chrono::steady_clock::time_point searchStartTime;
double searchMaxTimeSeconds = std::numeric_limits<double>::infinity();

namespace { // Anonymous namespace for internal helper functions

bool SearchShouldStop()
{
    if (searchShouldStop)
    {
        return true;
    }

    double elapsedTime = std::chrono::duration<double>(std::chrono::steady_clock::now() - searchStartTime).count();
    return elapsedTime >= searchMaxTimeSeconds;
}

int ScoreMove(Board& board, const MoveGenerator::Move& move)
{
    int moveScore = 0;
    int movePiece = board.GetPieceAtSquare(move.GetFromSquare());
    int capturedPiece = board.GetPieceAtSquare(move.GetToSquare());

    if (move.IsCheckingMove())
    {
        moveScore += 50; // Moves that give check are scored higher
    }

    if (move.IsCapture())
    {
        moveScore += 10 * (move.HasFlag(MoveGenerator::MoveValues::EN_PASSANT) ? Evaluate::GetPieceValue(Piece::PAWN) : Evaluate::GetPieceValue(capturedPiece)) - Evaluate::GetPieceValue(movePiece); // Capture moves are scored based on the value of the captured piece minus the value of the moving piece
    }

    if (move.IsPromotion())
    {
        int promotionPieceType = move.HasFlag(MoveGenerator::MoveValues::QUEEN_PROMOTION)  ? Piece::QUEEN :
                                 move.HasFlag(MoveGenerator::MoveValues::ROOK_PROMOTION)   ? Piece::ROOK  :
                                 move.HasFlag(MoveGenerator::MoveValues::BISHOP_PROMOTION) ? Piece::BISHOP:
                                                                                             Piece::KNIGHT;

        moveScore += Evaluate::GetPieceValue(promotionPieceType);
    }

    return moveScore;
}

void OrderMoves(Board& board, MoveGenerator::MoveList& moves)
{
    std::sort(moves.moves.data(), moves.moves.data() + moves.moveCount, [&board](const MoveGenerator::Move& a, const MoveGenerator::Move& b) {
        return ScoreMove(board, a) > ScoreMove(board, b); // Higher score moves are ordered first
    });
}

double SearchAllCaptures(Board& board, double alpha, double beta)
{
    if (SearchShouldStop())
    {
        return Evaluate::Evaluate(board);
    }

    double evaluation = Evaluate::Evaluate(board);
    if (evaluation >= beta)
    {
        return beta; // Beta cutoff
    }
    if (evaluation > alpha)
    {
        alpha = evaluation;
    }

    auto moves = MoveGenerator::GenerateMoves(board, true); // Only generate capture moves
    
    OrderMoves(board, moves);
    for (int i = 0; i < moves.moveCount; ++i)
    {
        const auto& move = moves.moves[i];

        board.MakeMove(move);
        double evaluation = -SearchAllCaptures(board, -beta, -alpha);
        board.UnmakeLastMove();

        if (evaluation >= beta)
        {
            return beta; // Beta cutoff
        }
        if (evaluation > alpha)
        {
            alpha = evaluation;
        }
    }

    return alpha;
}

double Search(Board& board, int depth, double alpha, double beta, TranspositionTable::TranspositionTable& transpositionTable)
{
    if (SearchShouldStop())
    {
        return Evaluate::Evaluate(board);
    }

    if (board.GetCurrentPositionHistoryCount() >= 3)
    {
        return 0.0; // Threefold repetition is a draw
    }

    if (board.GetHalfmoveClock() >= 50)
    {
        return 0.0; // Fifty-move rule is a draw
    }

    std::uint64_t zobristHash = board.GetZobristHash();
    auto& ttEntry = transpositionTable.RetrieveEntry(zobristHash);
    if (ttEntry.IsValidEntry() && ttEntry.depth >= depth)
    {
        if (ttEntry.entryType == TranspositionTable::EXACT)
        {
            return ttEntry.evaluation;
        }
        else if (ttEntry.entryType == TranspositionTable::LOWERBOUND)
        {
            alpha = std::max(alpha, ttEntry.evaluation);
        }
        else if (ttEntry.entryType == TranspositionTable::UPPERBOUND)
        {
            beta = std::min(beta, ttEntry.evaluation);
        }
        
        if (alpha >= beta)
        {
            return ttEntry.evaluation;
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
            return -std::numeric_limits<double>::infinity(); // Checkmate
        }

        return 0.0; // Stalemate
    }

    const double startAlpha = alpha;
    MoveGenerator::Move bestMove = moves.moves[0];

    OrderMoves(board, moves);
    for (int i = 0; i < moves.moveCount; ++i)
    {
        const auto& move = moves.moves[i];

        board.MakeMove(move);
        double evaluation = -Search(board, depth - 1, -beta, -alpha, transpositionTable);
        board.UnmakeLastMove();

        if (evaluation >= beta)
        {
            return beta; // Beta cutoff
        }
        if (evaluation > alpha)
        {
            alpha = evaluation;
            bestMove = move;
        }
    }

    // Store the result in the transposition table
    TranspositionTable::EntryType entryType;
    if (alpha > startAlpha)
    {
        entryType = TranspositionTable::EXACT;
    }
    else
    {
        entryType = TranspositionTable::UPPERBOUND;
    }
    
    transpositionTable.StoreEntry(zobristHash, alpha, depth, entryType, bestMove);

    return alpha;
}

} // anonymous namespace for internal helper functions

MoveGenerator::Move FindBestMove(Board& board, int maxDepth, TranspositionTable::TranspositionTable& transpositionTable)
{
    auto moves = MoveGenerator::GenerateMoves(board);
    OrderMoves(board, moves);

    MoveGenerator::Move bestMove = moves.moves[0];
    double bestEvaluation = -std::numeric_limits<double>::infinity();
    
    const double alpha = -std::numeric_limits<double>::infinity();
    const double beta = std::numeric_limits<double>::infinity();

    int startDepth = searchMaxTimeSeconds < std::numeric_limits<double>::infinity() ? 1 : maxDepth;
    for (int depth = startDepth; depth <= maxDepth; ++depth)
    {
        if (SearchShouldStop())
        {
            break;
        }

        MoveGenerator::Move depthBestMove = bestMove;
        double depthBestEvaluation = -std::numeric_limits<double>::infinity();

        for (int i = 0; i < moves.moveCount; ++i)
        {
            if (SearchShouldStop())
            {
                break;
            }

            const auto& move = moves.moves[i];

            board.MakeMove(move);
            double evaluation = -Search(board, depth - 1, alpha, beta, transpositionTable);
            board.UnmakeLastMove();

            if (evaluation > depthBestEvaluation)
            {
                depthBestEvaluation = evaluation;
                depthBestMove = move;
            }
        }

        if (depthBestEvaluation > bestEvaluation)
        {
            bestEvaluation = depthBestEvaluation;
            bestMove = depthBestMove;
        }
    }

    return bestMove;
}

} // namespace Gluon::Search