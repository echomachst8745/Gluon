#include "evaluate.h"
#include "board.h"
#include "piece.h"

#include <cmath>

namespace Gluon::Evaluate {

namespace { // anonymous namespace for helper functions

double GetPieceCentralisationBonus(int square, double maxBonus)
{
    int rank = BoardHelpers::GetRankFromSquare(square);
    int file = BoardHelpers::GetFileFromSquare(square);

    double distanceFromCenter = std::abs(file - 3.5) + std::abs(rank - 3.5);
    double bonus = (7.0 - distanceFromCenter) / 7.0;

    return bonus * maxBonus;
}

double CountMaterial(const Board& board, bool forWhite)
{
    double material = 0.0;

    material += board.GetPawnPlacements(forWhite).size() * PAWN_VALUE;
    material += board.GetKnightPlacements(forWhite).size() * KNIGHT_VALUE;
    material += board.GetBishopPlacements(forWhite).size() * BISHOP_VALUE;
    material += board.GetRookPlacements(forWhite).size() * ROOK_VALUE;
    material += board.GetQueenPlacements(forWhite).size() * QUEEN_VALUE;

    return material;
}

double EvaluatePiecePlacement(const Board& board, bool forWhite)
{
    double placementScore = 0.0;

    const int fullmoveNumber = board.GetFullmoveNumber();
    const double opening = std::max(0.0, 1.0 - fullmoveNumber / 50.0);
    const double lateGame = 1.0 - opening;

    const double knightWeight = 0.3 * opening + 1.0 * lateGame;
    const double bishopWeight = 0.2 * opening + 0.8 * lateGame;

    for (int square : board.GetKnightPlacements(forWhite))
    {
        placementScore += GetPieceCentralisationBonus(square, 12) * knightWeight;
    }
    for (int square : board.GetBishopPlacements(forWhite))
    {
        placementScore += GetPieceCentralisationBonus(square, 10) * bishopWeight;
    }

    const double pawnWeight = 0.2 * opening;

    for (int square : board.GetPawnPlacements(forWhite))
    {
        placementScore += GetPieceCentralisationBonus(square, 4) * pawnWeight;
    }

    return placementScore;
}

} // anonymous namespace for helper functions

double Evaluate(const Board& board)
{
    double whiteEval = CountMaterial(board, true) + EvaluatePiecePlacement(board, true);
    double blackEval = CountMaterial(board, false) + EvaluatePiecePlacement(board, false);

    double perspective = board.IsWhiteToMove() ? 1.0 : -1.0;

    return (whiteEval - blackEval) * perspective;
}

} // namespace Gluon::Evaluate