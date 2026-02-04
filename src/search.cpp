#include "search.h"
#include "board.h"
#include "evaluate.h"
#include "movegenerator.h"

#include <limits>
#include <algorithm>

namespace Gluon::Search {

namespace { // Anonymous namespace for internal helper functions

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

    OrderMoves(board, moves);
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

} // anonymous namespace for internal helper functions

MoveGenerator::Move FindBestMove(Board& board, int depth)
{
    auto moves = MoveGenerator::GenerateMoves(board);
    OrderMoves(board, moves);

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