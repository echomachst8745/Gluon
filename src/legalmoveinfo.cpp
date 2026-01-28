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

void LegalMoveInfo::AddSlidingChecker(int square, std::uint64_t removeBlockCheckBitboard) noexcept
{
    AddChecker(square);
    this->removeBlockCheckBitboard = removeBlockCheckBitboard;
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

bool LegalMoveInfo::MoveIsIllegal(const Board& board, const MoveGenerator::Move& move) const noexcept
{
    const int fromSquare = move.GetFromSquare();
    const int toSquare = move.GetToSquare();

    // If in double check, only king moves are legal
    if (KingInDoubleCheck())
    {
        Piece::Piece movingPiece = board.GetPieceAtSquare(fromSquare);
        if (Piece::GetType(movingPiece) != Piece::KING)
        {
            return true; // Illegal move
        }
        return false; // King moves are handled elsewhere
    }

    // If the piece is pinned, check if the move is legal for the pinned piece
    if (pinnedPiecesBitboard & (1ULL << fromSquare))
    {
        if (!(pinnedPieceLegalMovesBitboards[fromSquare] & (1ULL << toSquare)))
        {
            return true; // Illegal move
        }
    }

    // If in single check, only moves that capture the checker or block the check are legal
    if (KingInCheck())
    {
        if (!(removeBlockCheckBitboard & (1ULL << toSquare)))
        {
            return true; // Illegal move
        }
    }

    return false; // Move is legal
}

} // namespace Gluon