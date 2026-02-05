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

double ForceKingTowardsCornersBonus(const Board& board, bool forWhite, double endgameFactor)
{
    double evaluation = 0.0;

    int opponentKingSquare = board.GetKingSquare(!forWhite);
    int opponentKingRank = BoardHelpers::GetRankFromSquare(opponentKingSquare);
    int opponentKingFile = BoardHelpers::GetFileFromSquare(opponentKingSquare);

    int opponentKingDistanceFromCenterFile = std::max(3 - opponentKingFile, opponentKingFile - 4);
    int opponentKingDistanceFromCenterRank = std::max(3 - opponentKingRank, opponentKingRank - 4);
    int opponentKingDistanceFromCenter = opponentKingDistanceFromCenterFile + opponentKingDistanceFromCenterRank;
    evaluation += opponentKingDistanceFromCenter;

    int friendlyKingSquare = board.GetKingSquare(forWhite);
    int friendlyKingRank = BoardHelpers::GetRankFromSquare(friendlyKingSquare);
    int friendlyKingFile = BoardHelpers::GetFileFromSquare(friendlyKingSquare);

    int distanceBetweenKingsFile = std::abs(friendlyKingFile - opponentKingFile);
    int distanceBetweenKingsRank = std::abs(friendlyKingRank - opponentKingRank);
    int distanceBetweenKings = distanceBetweenKingsFile + distanceBetweenKingsRank;
    evaluation += 14 - distanceBetweenKings;

    return evaluation * 10.0 * endgameFactor;
}

} // anonymous namespace for helper functions

double Evaluate(const Board& board)
{
    double whiteMaterial = CountMaterial(board, true);
    double blackMaterial = CountMaterial(board, false);

    double whitePiecePlacementBonus = EvaluatePiecePlacement(board, true);
    double blackPiecePlacementBonus = EvaluatePiecePlacement(board, false);

    double endgameFactor = (whiteMaterial + blackMaterial) / 7800.0;
    bool queensOnBoard = (!board.GetQueenPlacements(true).empty()) || (!board.GetQueenPlacements(false).empty());
    bool isEndgame = (endgameFactor < 0.5) || (!queensOnBoard && endgameFactor < 0.7);

    double whiteEval = whiteMaterial + whitePiecePlacementBonus;
    double blackEval = blackMaterial + blackPiecePlacementBonus;

    if (isEndgame)
    {
        double whiteKingCornerBonus = ForceKingTowardsCornersBonus(board, true, 1.0 - endgameFactor);
        double blackKingCornerBonus = ForceKingTowardsCornersBonus(board, false, 1.0 - endgameFactor);

        whiteEval += whiteKingCornerBonus;
        blackEval += blackKingCornerBonus;
    }

    double perspective = board.IsWhiteToMove() ? 1.0 : -1.0;

    return (whiteEval - blackEval) * perspective;
}

double GetPieceValue(Piece::Piece piece)
{
    switch (Piece::GetType(piece))
    {
        case Piece::PAWN:   return PAWN_VALUE;
        case Piece::KNIGHT: return KNIGHT_VALUE;
        case Piece::BISHOP: return BISHOP_VALUE;
        case Piece::ROOK:   return ROOK_VALUE;
        case Piece::QUEEN:  return QUEEN_VALUE;
        case Piece::KING:   return 0.0;

        default:
            throw std::invalid_argument("Invalid piece type");
    }
}

} // namespace Gluon::Evaluate