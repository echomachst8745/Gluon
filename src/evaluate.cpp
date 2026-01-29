#include "evaluate.h"
#include "board.h"
#include "piece.h"

namespace Gluon::Evaluate {

namespace { // anonymous namespace for helper functions

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

} // anonymous namespace for helper functions

double Evaluate(const Board& board)
{
    double whiteEval = CountMaterial(board, true);
    double blackEval = CountMaterial(board, false);

    int perspective = board.IsWhiteToMove() ? 1 : -1;

    return (whiteEval - blackEval) * perspective;
}

} // namespace Gluon::Evaluate