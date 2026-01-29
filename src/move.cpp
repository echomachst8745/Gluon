#include "move.h"
#include "boardhelpers.h"
#include "board.h"

namespace Gluon::MoveGenerator {

using namespace MoveValues;

Move::Move()
{
    move = INVALID_MOVE;
}

Move::Move(int fromSquare, int toSquare, std::uint16_t flags)
{
    move = (static_cast<std::uint16_t>(fromSquare) << FROM_SQUARE_SHIFT) |
           (static_cast<std::uint16_t>(toSquare) << TO_SQUARE_SHIFT)     |
           (flags & FLAG_MASK);
}

bool Move::IsValidMove() const noexcept
{
    return move != INVALID_MOVE;
}

int Move::GetFromSquare() const noexcept
{
    return (int)((move & FROM_SQUARE_MASK) >> FROM_SQUARE_SHIFT);
}

int Move::GetToSquare() const noexcept
{
    return (int)((move & TO_SQUARE_MASK) >> TO_SQUARE_SHIFT);
}

bool Move::HasFlag(std::uint16_t flag) const noexcept
{
    return (move & FLAG_MASK) == flag;
}

bool Move::IsCapture() const noexcept
{
    switch (move & FLAG_MASK)
    {
        case CAPTURE:
        case EN_PASSANT:
        case KNIGHT_PROMOTION_CAPTURE:
        case BISHOP_PROMOTION_CAPTURE:
        case ROOK_PROMOTION_CAPTURE:
        case QUEEN_PROMOTION_CAPTURE:
            return true;

        default:
            return false;
    }
}

bool Move::IsPromotion() const noexcept
{
    switch (move & FLAG_MASK)
    {
        case KNIGHT_PROMOTION:
        case BISHOP_PROMOTION:
        case ROOK_PROMOTION:
        case QUEEN_PROMOTION:
        case KNIGHT_PROMOTION_CAPTURE:
        case BISHOP_PROMOTION_CAPTURE:
        case ROOK_PROMOTION_CAPTURE:
        case QUEEN_PROMOTION_CAPTURE:
            return true;

        default:
            return false;
    }
}

const std::string Move::ToUCIString() const noexcept
    {
        char promotionChar = '\0';
        switch (move & FLAG_MASK)
        {
            case KNIGHT_PROMOTION:
            case KNIGHT_PROMOTION_CAPTURE:
                promotionChar = 'n';
                break;
            case BISHOP_PROMOTION:
            case BISHOP_PROMOTION_CAPTURE:
                promotionChar = 'b';
                break;
            case ROOK_PROMOTION:
            case ROOK_PROMOTION_CAPTURE:
                promotionChar = 'r';
                break;
            case QUEEN_PROMOTION:
            case QUEEN_PROMOTION_CAPTURE:
                promotionChar = 'q';
                break;

            default:
                break;
        }

        return BoardHelpers::SquareToCoord(GetFromSquare()) + BoardHelpers::SquareToCoord(GetToSquare()) + (promotionChar ? std::string(1, promotionChar) : "");
    }

} // namespace Gluon::MoveGenerator