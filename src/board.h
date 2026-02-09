#pragma once

#include "boardhelpers.h"
#include "piece.h"
#include "pieceplacementmap.h"
#include "move.h"
#include "boardstate.h"

#include <array>

#include <cstdint>
#include <string>
#include <stack>

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

	std::stack<BoardState> boardStateHistory;
	
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

	std::uint64_t GetPawnPlacementBitboard(bool forWhite) const;
	std::uint64_t GetKnightPlacementBitboard(bool forWhite) const;
	std::uint64_t GetBishopPlacementBitboard(bool forWhite) const;
	std::uint64_t GetRookPlacementBitboard(bool forWhite) const;
	std::uint64_t GetQueenPlacementBitboard(bool forWhite) const;

	std::uint64_t GetOccupiedSquaresBitboardForColour(bool forwhite) const;
	std::uint64_t GetAllOccupiedSquaresBitboard() const;

	void MakeMove(const Move& move);

	void UndoLastMove();
};

} // namespace Gluon