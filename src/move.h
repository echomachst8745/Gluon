#pragma once

#include <cstdint>
#include <string>

namespace Gluon {

struct Move
{
private:
	std::uint16_t moveData;
	bool isCheckingMove;

	static const int FLAG_MASK        = 0x000F; // 4 bits for move flags
	static const int FROM_SQUARE_MASK = 0xFC00; // 6 bits for from square
	static const int TO_SQUARE_MASK   = 0x03F0; // 6 bits for to square

	static const int FROM_SQUARE_SHIFT = 10;
	static const int TO_SQUARE_SHIFT   = 4;

public:
	enum MoveFlag : std::uint16_t
	{
		INVALID_MOVE             = 0xFFFF,
		QUIET_MOVE               = 0,
		DOUBLE_PAWN_PUSH         = 1,
		KING_SIDE_CASTLE         = 2,
		QUEEN_SIDE_CASTLE        = 3,
		CAPTURE                  = 4,
		EN_PASSANT_CAPTURE       = 5,
		KNIGHT_PROMOTION         = 8,
		BISHOP_PROMOTION         = 9,
		ROOK_PROMOTION           = 10,
		QUEEN_PROMOTION          = 11,
		KNIGHT_PROMOTION_CAPTURE = 12,
		BISHOP_PROMOTION_CAPTURE = 13,
		ROOK_PROMOTION_CAPTURE   = 14,
		QUEEN_PROMOTION_CAPTURE  = 15
	};

	Move();
	Move(int fromSquare, int toSquare, MoveFlag flag = QUIET_MOVE);

	bool IsValid() const;

	int GetFromSquare() const;
	int GetToSquare() const;
	bool HasFlag(MoveFlag flag) const;

	bool IsCapture() const;
	bool IsPromotion() const;
	bool IsCheckingMove() const;

	const std::string ToUCIString() const;
};

} // namespace Gluon