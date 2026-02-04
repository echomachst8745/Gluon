#include "search.h"
#include "board.h"
#include "evaluate.h"
#include "movegenerator.h"

#include <limits>

namespace Gluon::Search {

namespace { // Anonymous namespace for internal helper functions

double Search(Board& board, int depth, double alpha, double beta)
{
    if (board.GetCurrentPositionHistoryCount() >= 3)
    {
        return 0.0; // Threefold repetition is a draw
    }

    if (board.GetHalfmoveClock() >= 50)
    {
        return 0.0; // Fifty-move rule is a draw
    }

    if (depth == 0)
    {
        return Evaluate::Evaluate(board);
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

    for (int i = 0; i < moves.moveCount; ++i)
    {
        const auto& move = moves.moves[i];

        board.MakeMove(move);
        double evaluation = -Search(board, depth - 1, -beta, -alpha);
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

}

MoveGenerator::Move FindBestMove(Board& board, int depth)
{
    auto moves = MoveGenerator::GenerateMoves(board);

    MoveGenerator::Move bestMove = moves.moves[0];
    double bestEvaluation = -std::numeric_limits<double>::infinity();
    
    const double alpha = -std::numeric_limits<double>::infinity();
    const double beta = std::numeric_limits<double>::infinity();

    for (int i = 0; i < moves.moveCount; ++i)
    {
        const auto& move = moves.moves[i];

        board.MakeMove(move);
        double evaluation = -Search(board, depth - 1, alpha, beta);
        board.UnmakeLastMove();

        if (evaluation > bestEvaluation)
        {
            bestEvaluation = evaluation;
            bestMove = move;
        }
    }

    return bestMove;
}

} // namespace Gluon::Search