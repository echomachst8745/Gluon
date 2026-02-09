#include "movegenerator.h"

#include "precomputedmovedata.h"
#include "move.h"
#include "piece.h"
#include "kingsafetyinfo.h"

#include <vector>

namespace Gluon {

void GeneratePawnMoves(const Board& board, std::vector<Move>& outMoveVector)
{
	const bool isWhitesMove = board.IsWhitesMove();
	const auto& pawnPlacements = board.GetPawnPlacements(isWhitesMove);

	for (int pawnSquare : pawnPlacements)
	{
		const int pawnSinglePushTarget = PawnMovesPerSquare[pawnSquare][isWhitesMove ? 0 : 4];
		const int pawnDoublePushTarget = PawnMovesPerSquare[pawnSquare][isWhitesMove ? 1 : 5];
		const int pawnCaptureWestTarget = PawnMovesPerSquare[pawnSquare][isWhitesMove ? 2 : 6];
		const int pawnCaptureEastTarget = PawnMovesPerSquare[pawnSquare][isWhitesMove ? 3 : 7];

		if (pawnSinglePushTarget != -1 && board.GetPieceAtSquare(pawnSinglePushTarget) == PieceType::NONE)
		{
			const int singlePushTargetRank = GetRankFromSquare(pawnSinglePushTarget);
			if (singlePushTargetRank == RANK_1 || singlePushTargetRank == RANK_8)
			{
				outMoveVector.emplace_back(pawnSquare, pawnSinglePushTarget, Move::MoveFlag::KNIGHT_PROMOTION);
				outMoveVector.emplace_back(pawnSquare, pawnSinglePushTarget, Move::MoveFlag::BISHOP_PROMOTION);
				outMoveVector.emplace_back(pawnSquare, pawnSinglePushTarget, Move::MoveFlag::ROOK_PROMOTION);
				outMoveVector.emplace_back(pawnSquare, pawnSinglePushTarget, Move::MoveFlag::QUEEN_PROMOTION);
			}
			else
			{
				outMoveVector.emplace_back(pawnSquare, pawnSinglePushTarget, Move::MoveFlag::QUIET_MOVE);
			}

			if (pawnDoublePushTarget != -1 && board.GetPieceAtSquare(pawnDoublePushTarget) == PieceType::NONE)
			{
				outMoveVector.emplace_back(pawnSquare, pawnDoublePushTarget, Move::MoveFlag::DOUBLE_PAWN_PUSH);
			}
		}

		if (pawnCaptureWestTarget != -1)
		{
			const int pawnCaptureWestTargetRank = GetRankFromSquare(pawnCaptureWestTarget);

			const Piece targetPiece = board.GetPieceAtSquare(pawnCaptureWestTarget);
			if (targetPiece != NONE_PIECE && PieceIsWhite(targetPiece) != isWhitesMove)
			{
				if (pawnCaptureWestTargetRank == RANK_1 || pawnCaptureWestTargetRank == RANK_8)
				{
					outMoveVector.emplace_back(pawnSquare, pawnCaptureWestTarget, Move::MoveFlag::KNIGHT_PROMOTION_CAPTURE);
					outMoveVector.emplace_back(pawnSquare, pawnCaptureWestTarget, Move::MoveFlag::BISHOP_PROMOTION_CAPTURE);
					outMoveVector.emplace_back(pawnSquare, pawnCaptureWestTarget, Move::MoveFlag::ROOK_PROMOTION_CAPTURE);
					outMoveVector.emplace_back(pawnSquare, pawnCaptureWestTarget, Move::MoveFlag::QUEEN_PROMOTION_CAPTURE);
				}
				else
				{
					outMoveVector.emplace_back(pawnSquare, pawnCaptureWestTarget, Move::MoveFlag::CAPTURE);
				}
			}
		}

		if (pawnCaptureEastTarget != -1)
		{
			const int pawnCaptureEastTargetRank = GetRankFromSquare(pawnCaptureEastTarget);

			const Piece targetPiece = board.GetPieceAtSquare(pawnCaptureEastTarget);
			if (targetPiece != NONE_PIECE && PieceIsWhite(targetPiece) != isWhitesMove)
			{
				if (pawnCaptureEastTargetRank == RANK_1 || pawnCaptureEastTargetRank == RANK_8)
				{
					outMoveVector.emplace_back(pawnSquare, pawnCaptureEastTarget, Move::MoveFlag::KNIGHT_PROMOTION_CAPTURE);
					outMoveVector.emplace_back(pawnSquare, pawnCaptureEastTarget, Move::MoveFlag::BISHOP_PROMOTION_CAPTURE);
					outMoveVector.emplace_back(pawnSquare, pawnCaptureEastTarget, Move::MoveFlag::ROOK_PROMOTION_CAPTURE);
					outMoveVector.emplace_back(pawnSquare, pawnCaptureEastTarget, Move::MoveFlag::QUEEN_PROMOTION_CAPTURE);
				}
				else
				{
					outMoveVector.emplace_back(pawnSquare, pawnCaptureEastTarget, Move::MoveFlag::CAPTURE);
				}
			}
		}

		// Check en passant captures
		const int enPassantSquare = board.GetEnPassantSquare();
		if (enPassantSquare != NO_EN_PASSANT)
		{
			if (pawnCaptureWestTarget == enPassantSquare)
			{
				outMoveVector.emplace_back(pawnSquare, pawnCaptureWestTarget, Move::MoveFlag::EN_PASSANT_CAPTURE);
			}
			if (pawnCaptureEastTarget == enPassantSquare)
			{
				outMoveVector.emplace_back(pawnSquare, pawnCaptureEastTarget, Move::MoveFlag::EN_PASSANT_CAPTURE);
			}
		}
	}
}

void GenerateKnightMoves(const Board& board, std::vector<Move>& outMoveVector)
{
	const bool isWhitesMove = board.IsWhitesMove();
	const auto& knightPlacements = board.GetKnightPlacements(isWhitesMove);

	for (int knightSquare : knightPlacements)
	{
		for (int targetSquare : KnightMovesPerSquare[knightSquare])
		{
			if (targetSquare == -1)
			{
				break; // Skip invalid moves
			}

			const Piece targetPiece = board.GetPieceAtSquare(targetSquare);
			if (targetPiece == NONE_PIECE)
			{
				outMoveVector.emplace_back(knightSquare, targetSquare, Move::MoveFlag::QUIET_MOVE);
			}
			else if (PieceIsWhite(targetPiece) != isWhitesMove)
			{
				outMoveVector.emplace_back(knightSquare, targetSquare, Move::MoveFlag::CAPTURE);
			}
		}
	}
}

void GenerateSlidingMoves(const Board& board, int fromSquare, int startDirectionIndex, int endDirectionIndex, std::vector<Move>& outMoveVector)
{
	for (int directionIndex = startDirectionIndex; directionIndex < endDirectionIndex; ++directionIndex)
	{
		for (int step = 1; step <= NumberOfSquaresToEdgePerSquare[fromSquare][directionIndex]; ++step)
		{
			const int targetSquare = fromSquare + step * DIRECTION_OFFSETS[directionIndex];
			const Piece targetPiece = board.GetPieceAtSquare(targetSquare);

			if (targetPiece == NONE_PIECE)
			{
				outMoveVector.emplace_back(fromSquare, targetSquare, Move::MoveFlag::QUIET_MOVE);
			}
			else
			{
				if (PieceIsWhite(targetPiece) != board.IsWhitesMove())
				{
					outMoveVector.emplace_back(fromSquare, targetSquare, Move::MoveFlag::CAPTURE);
				}
				break; // Stop sliding in this direction after hitting a piece
			}
		}
	}
}

void GenerateBishopMoves(const Board& board, std::vector<Move>& outMoveVector)
{
	const bool isWhitesMove = board.IsWhitesMove();
	const auto& bishopPlacements = board.GetBishopPlacements(isWhitesMove);

	for (int bishopSquare : bishopPlacements)
	{
		GenerateSlidingMoves(board, bishopSquare, 4, 8, outMoveVector);
	}
}

void GenerateRookMoves(const Board& board, std::vector<Move>& outMoveVector)
{
	const bool isWhitesMove = board.IsWhitesMove();
	const auto& rookPlacements = board.GetRookPlacements(isWhitesMove);

	for (int rookSquare : rookPlacements)
	{
		GenerateSlidingMoves(board, rookSquare, 0, 4, outMoveVector);
	}
}

void GenerateQueenMoves(const Board& board, std::vector<Move>& outMoveVector)
{
	const bool isWhitesMove = board.IsWhitesMove();
	const auto& queenPlacements = board.GetQueenPlacements(isWhitesMove);

	for (int queenSquare : queenPlacements)
	{
		GenerateSlidingMoves(board, queenSquare, 0, 8, outMoveVector);
	}
}

void GenerateKingMoves(const Board& board, std::vector<Move>& outMoveVector)
{
	const bool isWhitesMove = board.IsWhitesMove();
	const int kingSquare = board.GetKingPlacement(isWhitesMove);

	for (int targetSquare : KingMovesPerSquare[kingSquare])
	{
		if (targetSquare == -1)
		{
			break; // Skip invalid moves
		}

		const Piece targetPiece = board.GetPieceAtSquare(targetSquare);
		if (targetPiece == NONE_PIECE)
		{
			outMoveVector.emplace_back(kingSquare, targetSquare, Move::MoveFlag::QUIET_MOVE);
		}
		else if (PieceIsWhite(targetPiece) != isWhitesMove)
		{
			outMoveVector.emplace_back(kingSquare, targetSquare, Move::MoveFlag::CAPTURE);
		}
	}

	// Castling moves
	if (isWhitesMove)
	{
		if (board.HasCastlingRight(WHITE_KING_SIDE_CASTLE))
		{
			if (board.GetPieceAtSquare(FileRankToSquare(FILE_F, RANK_1)) == NONE_PIECE &&
				board.GetPieceAtSquare(FileRankToSquare(FILE_G, RANK_1)) == NONE_PIECE)
			{
				outMoveVector.emplace_back(kingSquare, FileRankToSquare(FILE_G, RANK_1), Move::MoveFlag::KING_SIDE_CASTLE);
			}
		}

		if (board.HasCastlingRight(WHITE_QUEEN_SIDE_CASTLE))
		{
			if (board.GetPieceAtSquare(FileRankToSquare(FILE_D, RANK_1)) == NONE_PIECE &&
				board.GetPieceAtSquare(FileRankToSquare(FILE_C, RANK_1)) == NONE_PIECE &&
				board.GetPieceAtSquare(FileRankToSquare(FILE_B, RANK_1)) == NONE_PIECE)
			{
				outMoveVector.emplace_back(kingSquare, FileRankToSquare(FILE_C, RANK_1), Move::MoveFlag::QUEEN_SIDE_CASTLE);
			}
		}
	}
	else
	{
		if (board.HasCastlingRight(BLACK_KING_SIDE_CASTLE))
		{
			if (board.GetPieceAtSquare(FileRankToSquare(FILE_F, RANK_8)) == NONE_PIECE &&
				board.GetPieceAtSquare(FileRankToSquare(FILE_G, RANK_8)) == NONE_PIECE)
			{
				outMoveVector.emplace_back(kingSquare, FileRankToSquare(FILE_G, RANK_8), Move::MoveFlag::KING_SIDE_CASTLE);
			}
		}

		if (board.HasCastlingRight(BLACK_QUEEN_SIDE_CASTLE))
		{
			if (board.GetPieceAtSquare(FileRankToSquare(FILE_D, RANK_8)) == NONE_PIECE &&
				board.GetPieceAtSquare(FileRankToSquare(FILE_C, RANK_8)) == NONE_PIECE &&
				board.GetPieceAtSquare(FileRankToSquare(FILE_B, RANK_8)) == NONE_PIECE)
			{
				outMoveVector.emplace_back(kingSquare, FileRankToSquare(FILE_C, RANK_8), Move::MoveFlag::QUEEN_SIDE_CASTLE);
			}
		}
	}
}

bool IsMoveLegal(const Board& board, const Move& move, const KingSafetyInfo& kingSafetyInfo, std::uint64_t occupiedSquaresBitboard)
{
	const bool isWhitesMove = board.IsWhitesMove();
	const int fromSquare = move.GetFromSquare();
	const int toSquare = move.GetToSquare();
	const Piece movingPiece = board.GetPieceAtSquare(fromSquare);
	const PieceType movingPieceType = GetPieceType(movingPiece);
	const int friendlyKingSquare = board.GetKingPlacement(isWhitesMove);

	const std::uint64_t moveFromBitboard = 1ULL << fromSquare;
	const std::uint64_t moveToBitboard = 1ULL << toSquare;

	std::uint64_t whitePawnPlacementBitboard = board.GetPawnPlacementBitboard(true);
	std::uint64_t blackPawnPlacementBitboard = board.GetPawnPlacementBitboard(false);
	std::uint64_t enemyKnightPlacementBitboard = board.GetKnightPlacementBitboard(!isWhitesMove);
	std::uint64_t enemyBishopPlacementBitboard = board.GetBishopPlacementBitboard(!isWhitesMove);
	std::uint64_t enemyRookPlacementBitboard = board.GetRookPlacementBitboard(!isWhitesMove);
	std::uint64_t enemyQueenPlacementBitboard = board.GetQueenPlacementBitboard(!isWhitesMove);

	// King moves
	if (movingPieceType == PieceType::KING)
	{
		if (move.HasFlag(Move::KING_SIDE_CASTLE) || move.HasFlag(Move::QUEEN_SIDE_CASTLE))
		{
			const bool kingSideCastle = move.HasFlag(Move::KING_SIDE_CASTLE);

			if (kingSafetyInfo.checkersBitboard)
			{
				return false; // Can't castle out of check
			}

			const int betweenSquare = kingSideCastle ? FileRankToSquare(FILE_F, isWhitesMove ? RANK_1 : RANK_8) :
													   FileRankToSquare(FILE_D, isWhitesMove ? RANK_1 : RANK_8);

			if (IsSquareAttacked(board, betweenSquare, !isWhitesMove, whitePawnPlacementBitboard,
																	  blackPawnPlacementBitboard,
																	  enemyKnightPlacementBitboard,
																	  enemyBishopPlacementBitboard,
																	  enemyRookPlacementBitboard,
																	  enemyQueenPlacementBitboard,
																	  occupiedSquaresBitboard))
			{
				return false;
			}

			if (IsSquareAttacked(board, toSquare, !isWhitesMove, whitePawnPlacementBitboard,
																 blackPawnPlacementBitboard,
																 enemyKnightPlacementBitboard,
																 enemyBishopPlacementBitboard,
															 	 enemyRookPlacementBitboard,
																 enemyQueenPlacementBitboard,
																 occupiedSquaresBitboard))
			{
				return false;
			}

			return true;
		}

		std::uint64_t occupiedWithoutFriendlyKingBitboard = occupiedSquaresBitboard & ~(1ULL << friendlyKingSquare);
		occupiedWithoutFriendlyKingBitboard &= ~moveToBitboard;
		occupiedWithoutFriendlyKingBitboard |= moveToBitboard;

		return !IsSquareAttacked(board, toSquare, !isWhitesMove, whitePawnPlacementBitboard,
															  	 blackPawnPlacementBitboard,
																 enemyKnightPlacementBitboard,
																 enemyBishopPlacementBitboard,
															 	 enemyRookPlacementBitboard,
																 enemyQueenPlacementBitboard,
																 occupiedWithoutFriendlyKingBitboard); // King can't move into check
	}

	// Pieces must block or capture checkers
	if (kingSafetyInfo.checkersBitboard)
	{
		if (move.HasFlag(Move::EN_PASSANT_CAPTURE))
		{
			int captureSquare = toSquare + (isWhitesMove ? SOUTH_DIRECTION : NORTH_DIRECTION);
			std::uint64_t captureSquareBitboard = 1ULL << captureSquare;

			if ((captureSquareBitboard & kingSafetyInfo.checkMaskBitboard) == 0)
			{
				return false; // Move doesn't block or capture the checker(s)
			}
		}
		else if ((moveToBitboard & kingSafetyInfo.checkMaskBitboard) == 0)
		{
			return false; // Move doesn't block or capture the checker(s)
		}
	}

	// Moving a pinned piece must not expose the king to check
	if (kingSafetyInfo.pinnedPiecesBitboard & moveFromBitboard)
	{
		// The move must be along the line between the pinned piece and the king
		const std::uint64_t pinMaskBitboard = AllSquaresAllignedBitboard[friendlyKingSquare][fromSquare];
		if ((moveToBitboard & pinMaskBitboard) == 0)
		{
			return false; // Move exposes king to check
		}
	}

	// Check en passant discovery checks
	if (move.HasFlag(Move::EN_PASSANT_CAPTURE))
	{
		int captureSquare = toSquare + (isWhitesMove ? SOUTH_DIRECTION : NORTH_DIRECTION);

		std::uint64_t occupiedSquaresAfter = occupiedSquaresBitboard;
		occupiedSquaresAfter &= ~moveFromBitboard;
		occupiedSquaresAfter |= moveToBitboard;
		occupiedSquaresAfter &= ~(1ULL << captureSquare);

		std::uint64_t whitePawnsAfter = whitePawnPlacementBitboard;
		std::uint64_t blackPawnsAfter = blackPawnPlacementBitboard;
		if (isWhitesMove)
		{
			whitePawnsAfter &= ~moveFromBitboard;
			whitePawnsAfter |= moveToBitboard;
			blackPawnsAfter &= ~(1ULL << captureSquare);
		}
		else
		{
			blackPawnsAfter &= ~moveFromBitboard;
			blackPawnsAfter |= moveToBitboard;
			whitePawnsAfter &= ~(1ULL << captureSquare);
		}
		
		if (IsSquareAttacked(board, friendlyKingSquare, !isWhitesMove, whitePawnsAfter,
																	   blackPawnsAfter,
																	   enemyKnightPlacementBitboard,
																	   enemyBishopPlacementBitboard,
																	   enemyRookPlacementBitboard,
																	   enemyQueenPlacementBitboard,
																	   occupiedSquaresAfter))
		{
			return false;
		}
	}

	return true;
}

std::vector<Move> GeneratePseudoLegalMoves(const Board& board, PieceType pieceType)
{
	std::vector<Move> moves;
	moves.reserve(256); // Reserve space for 256 moves to avoid frequent reallocations

	switch (pieceType)
	{
		case PieceType::PAWN:
			GeneratePawnMoves(board, moves);
			break;
		case PieceType::KNIGHT:
			GenerateKnightMoves(board, moves);
			break;
		case PieceType::BISHOP:
			GenerateBishopMoves(board, moves);
			break;
		case PieceType::ROOK:
			GenerateRookMoves(board, moves);
			break;
		case PieceType::QUEEN:
			GenerateQueenMoves(board, moves);
			break;
		case PieceType::KING:
			GenerateKingMoves(board, moves);
			break;
		default:
			GeneratePawnMoves(board, moves);
			GenerateKnightMoves(board, moves);
			GenerateBishopMoves(board, moves);
			GenerateRookMoves(board, moves);
			GenerateQueenMoves(board, moves);
			GenerateKingMoves(board, moves);
			break;
	}

	return moves;
}

std::vector<Move> GenerateMoves(const Board& board)
{
	KingSafetyInfo kingSafetyInfo = ComputeKingSafetyInfo(board);

	std::vector<Move> pseudoLegalMoves = kingSafetyInfo.kingInDoubleCheck ? GeneratePseudoLegalMoves(board, PieceType::KING) : GeneratePseudoLegalMoves(board);

	std::vector<Move> legalMoves;
	legalMoves.reserve(pseudoLegalMoves.size()); // Reserve space to avoid frequent reallocations

	const std::uint64_t allOccupiedSquaresBitboard = board.GetAllOccupiedSquaresBitboard();

	for (const Move& move : pseudoLegalMoves)
	{
		int jdsak = move.GetFromSquare();
		jdsak++;
		jdsak--;

		if (IsMoveLegal(board, move, kingSafetyInfo, allOccupiedSquaresBitboard))
		{
			legalMoves.push_back(move);
		}
	}

	return legalMoves;
}

} // namespace Gluon