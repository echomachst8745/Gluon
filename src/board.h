#pragma once

#include "boardhelpers.h"
#include "piece.h"
#include "pieceplacementmap.h"

#include <array>

#include <cstdint>
#include <string>

namespace Gluon {

// A class to hold the state of the chess board.
class Board
{
private:
	std::array<Piece, NUM_SQUARES> squares;
	PiecePlacementMap piecePlacementMap;

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

	bool IsWhitesMove() const;

	int GetEnPassantSquare() const;

	Piece GetPieceAtSquare(int square) const;

	bool HasCastlingRight(std::uint8_t castlingRight) const;

	const std::vector<int>& GetPawnPlacements(bool forWhite) const;
	const std::vector<int>& GetKnightPlacements(bool forWhite) const;
	const std::vector<int>& GetBishopPlacements(bool forWhite) const;
	const std::vector<int>& GetRookPlacements(bool forWhite) const;
	const std::vector<int>& GetQueenPlacements(bool forWhite) const;
	int GetKingPlacement(bool forWhite) const;
};

} // namespace Gluon