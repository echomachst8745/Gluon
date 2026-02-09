#pragma once

#include "boardhelpers.h"
#include "board.h"

#include <cstdint>
#include <array>

namespace Gluon {

struct KingSafetyInfo
{
	std::uint64_t checkersBitboard;
	std::uint64_t pinnedPiecesBitboard;
	std::uint64_t checkMaskBitboard;

	bool kingInDoubleCheck;
};

bool IsSquareAttacked(const Board& board, int square, bool byWhite, std::uint64_t whitePawnPlacementBitboard,
																	std::uint64_t blackPawnPlacementBitboard,
																	std::uint64_t knightPlacementBitboard,
																	std::uint64_t bishopPlacementBitboard,
																	std::uint64_t rookPlacementBitboard,
																	std::uint64_t queenPlacementBitboard,
																	std::uint64_t occupiedSquaresBitboard);

KingSafetyInfo ComputeKingSafetyInfo(const Board& board);

} // namespace Gluon