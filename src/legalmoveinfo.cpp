#include "legalmoveinfo.h"

namespace Gluon {

void LegalMoveInfo::SetSquareAsAttacked(int square) noexcept
{
    attackedSquaresBitboard |= (1ULL << square);
}

void LegalMoveInfo::AddChecker(int square)
{
    if (checkerSquares[0] == -1)
    {
        checkerSquares[0] = square;
    }
    else if (checkerSquares[1] == -1)
    {
        checkerSquares[1] = square;
    }
    else
    {
        // More than two checkers
        throw std::runtime_error("More than two checking pieces detected.");
    }
}

void LegalMoveInfo::SetPiecePinned(int square, std::uint64_t legalMovesBitboard) noexcept
{
    if (pinnedPiecesBitboard & (1ULL << square))
    {
        pinnedPieceLegalMovesBitboards[square] &= legalMovesBitboard;
        return;
    }

    pinnedPiecesBitboard |= (1ULL << square);
    pinnedPieceLegalMovesBitboards[square] = legalMovesBitboard;
}

bool LegalMoveInfo::KingInCheck() noexcept
{
    return checkerSquares[0] != -1 || checkerSquares[1] != -1;
}

bool LegalMoveInfo::KingInDoubleCheck() noexcept 
{
    return checkerSquares[0] != -1 && checkerSquares[1] != -1;
}

} // namespace Gluon