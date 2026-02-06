#pragma once

#include "boardhelpers.h"
#include "piece.h"

#include <array>
#include <cstdint>
#include <string>

namespace Gluon {

// A class to hold the state of the chess board.
class Board
{
private:
	std::array<Piece, NUM_SQUARES> squares;

	bool isWhitesMove;

	std::uint8_t castlingRights;

	int enPassantSquare;

	int halfMoveClock;
	int fullMoveNumber;

public:
	Board(const std::string& fen = STARTING_POSITION_FEN);

	void Clear();

	void SetupWithFEN(const std::string& fen);

	const std::string GetBoardString(bool whitePOV = true) const;
};

} // namespace Gluon