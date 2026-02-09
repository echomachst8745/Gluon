#pragma once

#include "move.h"
#include "piece.h"

#include <cstdint>

namespace Gluon {

struct BoardState
{
	Move moveMade;
	int enPassantSquare;
	std::uint8_t castlingRights;
	int halfMoveClock;
	int fullMoveNumber;
	Piece movedPiece;
	Piece capturedPiece;
};

} // namespace Gluon