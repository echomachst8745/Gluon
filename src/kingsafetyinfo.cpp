#include "kingsafetyinfo.h"

#include "precomputedmovedata.h"
#include "board.h"
#include "bitboardhelpers.h"

#include <cstdint>

namespace Gluon {

std::uint64_t GenerateBishopAttacksBitboard(int square, std::uint64_t occupiedSquares)
{
	std::uint64_t attacksBitboard = 0;

	for (int directionIndex = 4; directionIndex < 8; ++directionIndex)
	{
		for (int step = 1; step <= NumberOfSquaresToEdgePerSquare[square][directionIndex]; ++step)
		{
			const int targetSquare = square + step * DIRECTION_OFFSETS[directionIndex];

			attacksBitboard |= (1ULL << targetSquare);

			if (occupiedSquares & (1ULL << targetSquare))
			{
				break; // Stop sliding in this direction after hitting a piece
			}
		}
	}

	return attacksBitboard;
}

std::uint64_t GenerateRookAttacksBitboard(int square, std::uint64_t occupiedSquares)
{
	std::uint64_t attacksBitboard = 0;

	for (int directionIndex = 0; directionIndex < 4; ++directionIndex)
	{
		for (int step = 1; step <= NumberOfSquaresToEdgePerSquare[square][directionIndex]; ++step)
		{
			const int targetSquare = square + step * DIRECTION_OFFSETS[directionIndex];

			attacksBitboard |= (1ULL << targetSquare);

			if (occupiedSquares & (1ULL << targetSquare))
			{
				break; // Stop sliding in this direction after hitting a piece
			}
		}
	}

	return attacksBitboard;
}

std::uint64_t GenerateCheckersBitboard(const Board& board)
{
	const bool isWhitesMove = board.IsWhitesMove();
	const int friendlyKingSquare = board.GetKingPlacement(isWhitesMove);

	std::uint64_t checkersBitboard = 0;

	// Add pawn checkers
	checkersBitboard |= (isWhitesMove ? WhitePawnAttacksBitboardPerSquare[friendlyKingSquare] :
										BlackPawnAttacksBitboardPerSquare[friendlyKingSquare]) & board.GetPawnPlacementBitboard(!isWhitesMove);

	// Add knight checkers
	checkersBitboard |= KnightAttacksBitboardPerSquare[friendlyKingSquare] & board.GetKnightPlacementBitboard(!isWhitesMove);

	const std::uint64_t occupiedSquaresBitboard = board.GetAllOccupiedSquaresBitboard();

	const std::uint64_t rookAttackRay = GenerateRookAttacksBitboard(friendlyKingSquare, occupiedSquaresBitboard);
	const std::uint64_t bishopAttackRay = GenerateBishopAttacksBitboard(friendlyKingSquare, occupiedSquaresBitboard);

	const std::uint64_t enemyQueenPlacementBitboard = board.GetQueenPlacementBitboard(!isWhitesMove);

	// Add rook and queen checkers
	checkersBitboard |= rookAttackRay & (board.GetRookPlacementBitboard(!isWhitesMove) | enemyQueenPlacementBitboard);

	// Add bishop and queen checkers
	checkersBitboard |= bishopAttackRay & (board.GetBishopPlacementBitboard(!isWhitesMove) | enemyQueenPlacementBitboard);

	return checkersBitboard;
}

std::uint64_t GenerateCheckMaskBitboard(const Board& board, std::uint64_t checkersBitboard)
{
	if (checkersBitboard == 0)
	{
		return ~0ULL; // No checkers, so no check mask needed
	}

	int checkerSquare = GetLSBIndex(checkersBitboard);
	
	const std::uint64_t squaresBetween = SquaresBetweenBitboard[board.GetKingPlacement(board.IsWhitesMove())][checkerSquare];

	return squaresBetween | (1ULL << checkerSquare); // Include the checker square itself in the check mask
}

std::uint64_t GeneratePinnedPiecesBitboard(const Board& board)
{
	const bool isWhitesMove = board.IsWhitesMove();
	
	const std::uint64_t enemyRooksQueensBitboard = board.GetRookPlacementBitboard(!isWhitesMove) | board.GetQueenPlacementBitboard(!isWhitesMove);
	const std::uint64_t enemyBishopsQueensBitboard = board.GetBishopPlacementBitboard(!isWhitesMove) | board.GetQueenPlacementBitboard(!isWhitesMove);

	const std::uint64_t occupiedSquaresBitboard = board.GetAllOccupiedSquaresBitboard();
	const std::uint64_t friendlyOccupiedSquaresBitboard = board.GetOccupiedSquaresBitboardForColour(isWhitesMove);
	const int friendlyKingSquare = board.GetKingPlacement(isWhitesMove);

	std::uint64_t pinnedPiecesBitboard = 0;

	{
		// Check for pinned pieces along rook attack rays
		const std::uint64_t rookAttackRay = GenerateRookAttacksBitboard(friendlyKingSquare, occupiedSquaresBitboard);
		std::uint64_t potentialPinnedPieces = rookAttackRay & friendlyOccupiedSquaresBitboard;

		while (potentialPinnedPieces)
		{
			int potentialPinnedSquare = PopLSB(potentialPinnedPieces);

			std::uint64_t occupiedSquaresExcludingPotentialPinned = occupiedSquaresBitboard & ~(1ULL << potentialPinnedSquare);
			std::uint64_t newRookAttackRay = GenerateRookAttacksBitboard(friendlyKingSquare, occupiedSquaresExcludingPotentialPinned) & ~rookAttackRay;

			if (newRookAttackRay & enemyRooksQueensBitboard)
			{
				pinnedPiecesBitboard |= (1ULL << potentialPinnedSquare);
			}
		}
	}

	{
		// Check for pinned pieces along bishop attack rays
		const std::uint64_t bishopAttackRay = GenerateBishopAttacksBitboard(friendlyKingSquare, occupiedSquaresBitboard);
		std::uint64_t potentialPinnedPieces = bishopAttackRay & friendlyOccupiedSquaresBitboard;

		while (potentialPinnedPieces)
		{
			int potentialPinnedSquare = PopLSB(potentialPinnedPieces);

			std::uint64_t occupiedSquaresExcludingPotentialPinned = occupiedSquaresBitboard & ~(1ULL << potentialPinnedSquare);
			std::uint64_t newBishopAttackRay = GenerateBishopAttacksBitboard(friendlyKingSquare, occupiedSquaresExcludingPotentialPinned) & ~bishopAttackRay;

			if (newBishopAttackRay & enemyBishopsQueensBitboard)
			{
				pinnedPiecesBitboard |= (1ULL << potentialPinnedSquare);
			}
		}
	}

	return pinnedPiecesBitboard;
}

bool IsSquareAttacked(const Board& board, int square, bool byWhite, std::uint64_t whitePawnPlacementBitboard,
																	std::uint64_t blackPawnPlacementBitboard,
																	std::uint64_t knightPlacementBitboard,
																	std::uint64_t bishopPlacementBitboard,
																	std::uint64_t rookPlacementBitboard,
																	std::uint64_t queenPlacementBitboard,
																	std::uint64_t occupiedSquaresBitboard)
{
	const int kingSquare = board.GetKingPlacement(byWhite);

	// Pawn attacks
	if (byWhite)
	{
		if (BlackPawnAttacksBitboardPerSquare[square] & whitePawnPlacementBitboard)
		{
			return true;
		}
	}
	else
	{
		if (WhitePawnAttacksBitboardPerSquare[square] & blackPawnPlacementBitboard)
		{
			return true;
		}
	}

	// Knight attacks
	if (KnightAttacksBitboardPerSquare[square] & knightPlacementBitboard)
	{
		return true;
	}

	// Bishop/Queen attacks
	const std::uint64_t bishopAttackRay = GenerateBishopAttacksBitboard(square, occupiedSquaresBitboard);
		
	if (bishopAttackRay & (bishopPlacementBitboard | queenPlacementBitboard))
	{
		return true;
	}

	// Rook/Queen attacks
	const std::uint64_t rookAttackRay = GenerateRookAttacksBitboard(square, occupiedSquaresBitboard);
	if (rookAttackRay & (rookPlacementBitboard | queenPlacementBitboard))
	{
		return true;
	}

	// King attacks
	if (KingAttacksBitboardPerSquare[square] & (1ULL << kingSquare))
	{
		return true;
	}

	return false;
}

KingSafetyInfo ComputeKingSafetyInfo(const Board& board)
{
	KingSafetyInfo kingSafetyInfo{};
	
	kingSafetyInfo.checkersBitboard = GenerateCheckersBitboard(board);
	kingSafetyInfo.kingInDoubleCheck = (kingSafetyInfo.checkersBitboard & (kingSafetyInfo.checkersBitboard - 1)) != 0; // More than one bit set in checkers bitboard indicates double check
	kingSafetyInfo.checkMaskBitboard = kingSafetyInfo.kingInDoubleCheck ? 0ULL : GenerateCheckMaskBitboard(board, kingSafetyInfo.checkersBitboard);

	kingSafetyInfo.pinnedPiecesBitboard = GeneratePinnedPiecesBitboard(board);

	return kingSafetyInfo;
}

} // namespace Gluon