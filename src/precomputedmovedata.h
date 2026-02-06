#pragma once

#include "boardhelpers.h"

#include <array>

namespace Gluon {

/*
* A precomputed table that, for each square and each direction, gives the valid target squares for a pawn move.Invalid moves are represented with - 1.
* 
* The moves are ordered as follows:
*	- White single push
*   - White double push (only valid from the second rank)
*   - White capture north-west
*   - White capture north-east
*   - Black single push
*   - Black double push (only valid from the seventh rank)
*   - Black capture south-west
*   - Black capture south-east
*/
constexpr std::array<std::array<int, NUM_DIRECTIONS>, NUM_SQUARES> PawnMovesPerSquare = []() constexpr
{
	std::array<std::array<int, NUM_DIRECTIONS>, NUM_SQUARES> pawnMovesPerSquare{};
	for (int square = 0; square < NUM_SQUARES; ++square)
	{
		int fromFile = GetFileFromSquare(square);
		int fromRank = GetRankFromSquare(square);

		if (fromRank < RANK_2 || fromRank > RANK_8)
		{
			pawnMovesPerSquare[square].fill(-1); // Invalid pawn moves for ranks 1 and 8
			continue;
		}

		int whiteSinglePush = FileRankToSquare(fromFile, fromRank + 1);
		int whiteDoublePushRank = (fromRank == RANK_2) ? FileRankToSquare(fromFile, fromRank + 2) : -1;
		int whiteCaptureNorthWest = (fromFile > FILE_A) ? FileRankToSquare(fromFile - 1, fromRank + 1) : -1;
		int whiteCaptureNorthEast = (fromFile < FILE_H) ? FileRankToSquare(fromFile + 1, fromRank + 1) : -1;

		int blackSinglePush = FileRankToSquare(fromFile, fromRank - 1);
		int blackDoublePushRank = (fromRank == RANK_7) ? FileRankToSquare(fromFile, fromRank - 2) : -1;
		int blackCaptureSouthWest = (fromFile > FILE_A) ? FileRankToSquare(fromFile - 1, fromRank - 1) : -1;
		int blackCaptureSouthEast = (fromFile < FILE_H) ? FileRankToSquare(fromFile + 1, fromRank - 1) : -1;

		pawnMovesPerSquare[square] = { whiteSinglePush, whiteDoublePushRank, whiteCaptureNorthWest, whiteCaptureNorthEast,
									   blackSinglePush, blackDoublePushRank, blackCaptureSouthWest, blackCaptureSouthEast };
	}

	return pawnMovesPerSquare;
}();

/*
* A precomputed table that, for each square and each direction, gives the number of squares from that square to the edge of the board in that direction.
* 
* The directions are ordered as follows:
*	- North
*	- East
*	- South
*	- West
*	- North-East
*	- North-West
*	- South-East
*	- South-West
*/ 
constexpr std::array<std::array<int, NUM_DIRECTIONS>, NUM_SQUARES> NumberOfSquaresToEdgePerSquare = []() constexpr
{
	std::array<std::array<int, NUM_DIRECTIONS>, NUM_SQUARES> numberOfSquaresToEdgePerSquare{};

	for (int square = 0; square < NUM_SQUARES; ++square)
	{
		int file = GetFileFromSquare(square);
		int rank = GetRankFromSquare(square);

		int numNorth = RANK_8 - rank;
		int numEast = FILE_H - file;
		int numSouth = rank;
		int numWest = file;

		int numNorthEast = std::min(numNorth, numEast);
		int numNorthWest = std::min(numNorth, numWest);
		int numSouthEast = std::min(numSouth, numEast);
		int numSouthWest = std::min(numSouth, numWest);

		numberOfSquaresToEdgePerSquare[square] = { numNorth, numEast, numSouth, numWest, numNorthEast, numNorthWest, numSouthEast, numSouthWest };
	}

	return numberOfSquaresToEdgePerSquare;
}();

// A precomputed table that, for each square and each direction, gives the valid target squares for a knight. Invalid moves are represented with -1.
constexpr std::array<std::array<int, NUM_DIRECTIONS>, NUM_SQUARES> KnightMovesPerSquare = []() constexpr
{
	std::array<std::array<int, NUM_DIRECTIONS>, NUM_SQUARES> knightMovesPerSquare{};

	const int fileOffsets[NUM_DIRECTIONS] = { 1, 2, 2, 1, -1, -2, -2, -1 };
	const int rankOffsets[NUM_DIRECTIONS] = { 2, 1, -1, -2, -2, -1, 1, 2 };

	for (int square = 0; square < NUM_SQUARES; ++square)
	{
		std::array<int, NUM_DIRECTIONS> knightMoves;
		knightMoves.fill(-1); // Initialize all moves to -1 to indicate invalid moves
		
		int fromFile = GetFileFromSquare(square);
		int fromRank = GetRankFromSquare(square);

		int moveIndex = 0;
		for (int i = 0; i < NUM_DIRECTIONS; ++i)
		{
			int toFile = fromFile + fileOffsets[i];
			int toRank = fromRank + rankOffsets[i];

			if (toFile < FILE_A || toFile > FILE_H || toRank < RANK_1 || toRank > RANK_8)
			{
				continue;
			}

			knightMoves[moveIndex++] = FileRankToSquare(toFile, toRank);
		}

		knightMovesPerSquare[square] = knightMoves;
	}

	return knightMovesPerSquare;
}();

// A precomputed table that, for each square and each direction, gives the valid target squares for a king. Invalid moves are represented with -1.
constexpr std::array<std::array<int, NUM_DIRECTIONS>, NUM_SQUARES> KingMovesPerSquare = []() constexpr
{
	std::array<std::array<int, NUM_DIRECTIONS>, NUM_SQUARES> kingMovesPerSquare{};

	const int validFileOffsets[8] = { -1, 0, 1, -1, 1, -1, 0, 1 };
	const int validRankOffsets[8] = { 1, 1, 1, 0, 0, -1, -1, -1 };

	for (int square = 0; square < NUM_SQUARES; ++square)
	{
		std::array<int, NUM_DIRECTIONS> kingMoves;
		kingMoves.fill(-1); // Initialize all moves to -1 to indicate invalid moves

		int fromFile = GetFileFromSquare(square);
		int fromRank = GetRankFromSquare(square);

		int moveIndex = 0;
		for (int i = 0; i < NUM_DIRECTIONS; ++i)
		{
			int toFile = fromFile + validFileOffsets[i];
			int toRank = fromRank + validRankOffsets[i];

			if (toFile < FILE_A || toFile > FILE_H || toRank < RANK_1 || toRank > RANK_8)
			{
				continue;
			}

			kingMoves[moveIndex++] = FileRankToSquare(toFile, toRank);
		}

		kingMovesPerSquare[square] = kingMoves;
	}

	return kingMovesPerSquare;
}();

} // namespace Gluon