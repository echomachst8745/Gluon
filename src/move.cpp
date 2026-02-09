#include "move.h"

#include "boardhelpers.h"
#include "piece.h"

namespace Gluon {

Move::Move()
{
	moveData = MoveFlag::INVALID_MOVE;

	isCheckingMove = false;
}

Move::Move(int fromSquare, int toSquare, MoveFlag flag)
{
	moveData = (static_cast<std::uint16_t>(fromSquare) << FROM_SQUARE_SHIFT) |
		       (static_cast<std::uint16_t>(toSquare)   << TO_SQUARE_SHIFT)   |
		       (static_cast<std::uint16_t>(flag)        & FLAG_MASK);

	isCheckingMove = false;
}

bool Move::IsValid() const
{
	return moveData != MoveFlag::INVALID_MOVE;
}

int Move::GetFromSquare() const
{
	return (moveData & FROM_SQUARE_MASK) >> FROM_SQUARE_SHIFT;
}

int Move::GetToSquare() const
{
	return (moveData & TO_SQUARE_MASK) >> TO_SQUARE_SHIFT;
}

bool Move::HasFlag(MoveFlag flag) const
{
	return (moveData & FLAG_MASK) == flag;
}

bool Move::IsCapture() const
{
	return HasFlag(CAPTURE)                  ||
		   HasFlag(EN_PASSANT_CAPTURE)       ||
		   HasFlag(KNIGHT_PROMOTION_CAPTURE) ||
		   HasFlag(BISHOP_PROMOTION_CAPTURE) ||
		   HasFlag(ROOK_PROMOTION_CAPTURE)   ||
		   HasFlag(QUEEN_PROMOTION_CAPTURE);
}

bool Move::IsPromotion() const
{
	return HasFlag(KNIGHT_PROMOTION)         ||
		   HasFlag(BISHOP_PROMOTION)         ||
		   HasFlag(ROOK_PROMOTION)           ||
		   HasFlag(QUEEN_PROMOTION)          ||
		   HasFlag(KNIGHT_PROMOTION_CAPTURE) ||
		   HasFlag(BISHOP_PROMOTION_CAPTURE) ||
		   HasFlag(ROOK_PROMOTION_CAPTURE)   ||
		   HasFlag(QUEEN_PROMOTION_CAPTURE);
}

bool Move::IsCheckingMove() const
{
	return isCheckingMove;
}

PieceType Move::GetPromotionPieceType() const
{
	switch (moveData & FLAG_MASK)
	{
		case KNIGHT_PROMOTION:
		case KNIGHT_PROMOTION_CAPTURE:
			return PieceType::KNIGHT;
		case BISHOP_PROMOTION:
		case BISHOP_PROMOTION_CAPTURE:
			return PieceType::BISHOP;
		case ROOK_PROMOTION:
		case ROOK_PROMOTION_CAPTURE:
			return PieceType::ROOK;
		case QUEEN_PROMOTION:
		case QUEEN_PROMOTION_CAPTURE:
			return PieceType::QUEEN;
		default:
			return PieceType::NONE;
	}
}

const std::string Move::ToUCIString() const
{
	char promotionChar = '\0';
	switch (moveData & FLAG_MASK)
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

	return SquareToCoord(GetFromSquare()) + SquareToCoord(GetToSquare()) + (promotionChar ? std::string(1, promotionChar) : "");
}

} // namespace Gluon