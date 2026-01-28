#include "legalmoveinfo.h"
#include "move.h"
#include "board.h"

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

    removeBlockCheckBitboard = (1ULL << square);
}

void LegalMoveInfo::AddSlidingChecker(int square, std::uint64_t blockCheckBitboard) noexcept
{
    AddChecker(square);
    removeBlockCheckBitboard &= blockCheckBitboard;
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

bool LegalMoveInfo::KingInCheck() const noexcept
{
    return checkerSquares[0] != -1 || checkerSquares[1] != -1;
}

bool LegalMoveInfo::KingInDoubleCheck() const noexcept 
{
    return checkerSquares[0] != -1 && checkerSquares[1] != -1;
}

bool LegalMoveInfo::MoveIsLegal(const Board& board, const MoveGenerator::Move& move) const noexcept
{
    const int fromSquare = move.GetFromSquare();
    const int toSquare   = move.GetToSquare();
    const std::uint64_t toSquareBitboard = (1ULL << toSquare);
    const Piece::Piece movingPiece = board.GetPieceAtSquare(fromSquare);
    const bool isKingMove = (Piece::GetType(movingPiece) == Piece::KING);

    // Castling is not allowed when in check
    if (KingInCheck() && 
        (move.HasFlag(MoveGenerator::MoveValues::KING_CASTLE) || 
         move.HasFlag(MoveGenerator::MoveValues::QUEEN_CASTLE)))
    {
        return false;
    }

    // King moves: must not move into attacked squares
    if (isKingMove)
    {
        if ((attackedSquaresBitboard & toSquareBitboard) != 0)
        {
            return false;
        }
        return true;
    }

    // Non-king piece moves
    if (KingInCheck())
    {
        // In check: move must capture checker or block the check
        if ((toSquareBitboard & removeBlockCheckBitboard) == 0)
        {
            return false;
        }

        // If piece is also pinned, it must respect the pin
        if (pinnedPiecesBitboard & (1ULL << fromSquare))
        {
            if ((pinnedPieceLegalMovesBitboards[fromSquare] & toSquareBitboard) == 0)
            {
                return false;
            }
        }

        return true;
    }

    // Not in check
    if (pinnedPiecesBitboard & (1ULL << fromSquare))
    {
        // Pinned piece must stay on the pin line
        if ((pinnedPieceLegalMovesBitboards[fromSquare] & toSquareBitboard) == 0)
        {
            return false;
        }
    }

    return true;
}

} // namespace Gluon