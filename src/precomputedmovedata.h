#pragma once

#include "boardhelpers.h"

#include <array>
#include <cstdint>

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

static const std::array<std::uint64_t, NUM_SQUARES> WhitePawnAttacksBitboardPerSquare = []() constexpr
{
	std::array<std::uint64_t, NUM_SQUARES> whitePawnAttacksBitboardPerSquare;

	for (int square = 0; square < NUM_SQUARES; ++square)
	{
		std::uint64_t whitePawnAttacksBitboard = 0;

		int westAttackSquare = PawnMovesPerSquare[square][2];
		int eastAttackSquare = PawnMovesPerSquare[square][3];

		if (westAttackSquare != -1)
		{
			whitePawnAttacksBitboard |= (1ULL << westAttackSquare);
		}
		if (eastAttackSquare != -1)
		{
			whitePawnAttacksBitboard |= (1ULL << eastAttackSquare);
		}

		whitePawnAttacksBitboardPerSquare[square] = whitePawnAttacksBitboard;
	}

	return whitePawnAttacksBitboardPerSquare;
}();

static const std::array<std::uint64_t, NUM_SQUARES> BlackPawnAttacksBitboardPerSquare = []() constexpr
{
	std::array<std::uint64_t, NUM_SQUARES> blackPawnAttacksBitboardPerSquare;

	for (int square = 0; square < NUM_SQUARES; ++square)
	{
		std::uint64_t blackPawnAttacksBitboard = 0;

		int westAttackSquare = PawnMovesPerSquare[square][6];
		int eastAttackSquare = PawnMovesPerSquare[square][7];

		if (westAttackSquare != -1)
		{
			blackPawnAttacksBitboard |= (1ULL << westAttackSquare);
		}
		if (eastAttackSquare != -1)
		{
			blackPawnAttacksBitboard |= (1ULL << eastAttackSquare);
		}

		blackPawnAttacksBitboardPerSquare[square] = blackPawnAttacksBitboard;
	}

	return blackPawnAttacksBitboardPerSquare;
}();

static const std::array<std::uint64_t, NUM_SQUARES> KnightAttacksBitboardPerSquare = []() constexpr
{
	std::array<std::uint64_t, NUM_SQUARES> knightAttacksBitboardPerSquare;

	for (int square = 0; square < NUM_SQUARES; ++square)
	{
		std::uint64_t knightAttacksBitboard = 0;

		for (int targetSquare : KnightMovesPerSquare[square])
		{
			if (targetSquare == -1)
			{
				break;
			}

			knightAttacksBitboard |= (1ULL << targetSquare);
		}

		knightAttacksBitboardPerSquare[square] = knightAttacksBitboard;
	}

	return knightAttacksBitboardPerSquare;
}();

static const std::array<std::uint64_t, NUM_SQUARES> BishopAttacksBitboardPerSquare = []() constexpr
{
	std::array<std::uint64_t, NUM_SQUARES> bishopAttacksBitboardPerSquare;

	for (int square = 0; square < NUM_SQUARES; ++square)
	{
		std::uint64_t bishopAttacksBitboard = 0;

		for (int direction = 4; direction < NUM_DIRECTIONS; ++direction)
		{
			for (int step = 1; step <= NumberOfSquaresToEdgePerSquare[square][direction]; ++step)
			{
				int targetSquare = square + step * DIRECTION_OFFSETS[direction];

				bishopAttacksBitboard |= (1ULL << targetSquare);
			}
		}

		bishopAttacksBitboardPerSquare[square] = bishopAttacksBitboard;
	}

	return bishopAttacksBitboardPerSquare;
}();

static const std::array<std::uint64_t, NUM_SQUARES> RookAttacksBitboardPerSquare = []() constexpr
{
	std::array<std::uint64_t, NUM_SQUARES> rookAttacksBitboardPerSquare;

	for (int square = 0; square < NUM_SQUARES; ++square)
	{
		std::uint64_t rookAttacksBitboard = 0;

		for (int direction = 0; direction < 4; ++direction)
		{
			for (int step = 1; step <= NumberOfSquaresToEdgePerSquare[square][direction]; ++step)
			{
				int targetSquare = square + step * DIRECTION_OFFSETS[direction];

				rookAttacksBitboard |= (1ULL << targetSquare);
			}
		}

		rookAttacksBitboardPerSquare[square] = rookAttacksBitboard;
	}
	return rookAttacksBitboardPerSquare;
}();

static const std::array<std::uint64_t, NUM_SQUARES> KingAttacksBitboardPerSquare = []() constexpr
{
	std::array<std::uint64_t, NUM_SQUARES> kingAttacksBitboardPerSquare;

	for (int square = 0; square < NUM_SQUARES; ++square)
	{
		std::uint64_t kingAttacksBitboard = 0;

		for (int targetSquare : KingMovesPerSquare[square])
		{
			if (targetSquare == -1)
			{
				break;
			}

			kingAttacksBitboard |= (1ULL << targetSquare);
		}

		kingAttacksBitboardPerSquare[square] = kingAttacksBitboard;
	}

	return kingAttacksBitboardPerSquare;
}();

static const std::array<std::array<std::uint64_t, NUM_SQUARES>, NUM_SQUARES> SquaresBetweenBitboard = []() constexpr
{
	std::array<std::array<std::uint64_t, NUM_SQUARES>, NUM_SQUARES> squaresBetweenBitboard;

	for (int fromSquare = 0; fromSquare < NUM_SQUARES; ++fromSquare)
	{
		for (int toSquare = 0; toSquare < NUM_SQUARES; ++toSquare)
		{
			for (int direction = 0; direction < NUM_DIRECTIONS; ++direction)
			{
				std::uint64_t betweenBitboard = 0;

				bool toSquareInDirection = false;
				for (int step = 1; step <= NumberOfSquaresToEdgePerSquare[fromSquare][direction]; ++step)
				{
					int targetSquare = fromSquare + step * DIRECTION_OFFSETS[direction];

					if (targetSquare == toSquare)
					{
						toSquareInDirection = true;
						break;
					}

					betweenBitboard |= (1ULL << targetSquare);
				}

				if (toSquareInDirection)
				{
					squaresBetweenBitboard[fromSquare][toSquare] = betweenBitboard;
					break;
				}
				else
				{
					squaresBetweenBitboard[fromSquare][toSquare] = 0;
				}
			}
		}
	}

	return squaresBetweenBitboard;
}();

static const std::array<std::array<std::uint64_t, NUM_SQUARES>, NUM_SQUARES> SquaresBetweenAndIncludingBitboard = []() constexpr
{
	std::array<std::array<std::uint64_t, NUM_SQUARES>, NUM_SQUARES> squaresBetweenAndIncludingBitboard;

	for (int fromSquare = 0; fromSquare < NUM_SQUARES; ++fromSquare)
	{
		for (int toSquare = 0; toSquare < NUM_SQUARES; ++toSquare)
		{
			if (SquaresBetweenBitboard[fromSquare][toSquare] != 0)
			{
				squaresBetweenAndIncludingBitboard[fromSquare][toSquare] = SquaresBetweenBitboard[fromSquare][toSquare] | (1ULL << fromSquare) | (1ULL << toSquare);
			}
			else
			{
				squaresBetweenAndIncludingBitboard[fromSquare][toSquare] = 0;
			}
		}
	}

	return squaresBetweenAndIncludingBitboard;
}();

static const std::array<std::array<std::uint64_t, NUM_SQUARES>, NUM_SQUARES> AllSquaresAllignedBitboard = []() constexpr
{
	std::array<std::array<std::uint64_t, NUM_SQUARES>, NUM_SQUARES> allSquaresAllignedBitboard;

	for (int fromSquare = 0; fromSquare < NUM_SQUARES; ++fromSquare)
	{
		for (int toSquare = 0; toSquare < NUM_SQUARES; ++toSquare)
		{
			for (int direction = 0; direction < NUM_DIRECTIONS; ++direction)
			{
				std::uint64_t betweenBitboard = 0;

				bool toSquareInDirection = false;
				for (int step = 0; step <= NumberOfSquaresToEdgePerSquare[fromSquare][direction]; ++step)
				{
					int targetSquare = fromSquare + step * DIRECTION_OFFSETS[direction];

					if (targetSquare == toSquare)
					{
						toSquareInDirection = true;
					}

					betweenBitboard |= (1ULL << targetSquare);
				}

				if (toSquareInDirection)
				{
					allSquaresAllignedBitboard[fromSquare][toSquare] = betweenBitboard;
					break;
				}
				else
				{
					allSquaresAllignedBitboard[fromSquare][toSquare] = 0;
				}
			}
		}
	}

	return allSquaresAllignedBitboard;
}();

} // namespace Gluon