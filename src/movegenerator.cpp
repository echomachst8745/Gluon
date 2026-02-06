#include "movegenerator.h"

#include "precomputedmovedata.h"
#include "move.h"
#include "piece.h"

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
			outMoveVector.emplace_back(pawnSquare, pawnSinglePushTarget, Move::MoveFlag::QUIET_MOVE);

			if (pawnDoublePushTarget != -1 && board.GetPieceAtSquare(pawnDoublePushTarget) == PieceType::NONE)
			{
				outMoveVector.emplace_back(pawnSquare, pawnDoublePushTarget, Move::MoveFlag::DOUBLE_PAWN_PUSH);
			}
		}

		if (pawnCaptureWestTarget != -1)
		{
			const Piece targetPiece = board.GetPieceAtSquare(pawnCaptureWestTarget);
			if (targetPiece != NONE_PIECE && PieceIsWhite(targetPiece) != isWhitesMove)
			{
				outMoveVector.emplace_back(pawnSquare, pawnCaptureWestTarget, Move::MoveFlag::CAPTURE);
			}
		}

		if (pawnCaptureEastTarget != -1)
		{
			const Piece targetPiece = board.GetPieceAtSquare(pawnCaptureEastTarget);
			if (targetPiece != NONE_PIECE && PieceIsWhite(targetPiece) != isWhitesMove)
			{
				outMoveVector.emplace_back(pawnSquare, pawnCaptureEastTarget, Move::MoveFlag::CAPTURE);
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
				board.GetPieceAtSquare(FileRankToSquare(FILE_C, RANK_1)) == NONE_PIECE)
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
				board.GetPieceAtSquare(FileRankToSquare(FILE_C, RANK_8)) == NONE_PIECE)
			{
				outMoveVector.emplace_back(kingSquare, FileRankToSquare(FILE_C, RANK_8), Move::MoveFlag::QUEEN_SIDE_CASTLE);
			}
		}
	}
}

std::vector<Move> GeneratePseudoLegalMoves(const Board& board)
{
	std::vector<Move> moves;
	moves.reserve(256); // Reserve space for 256 moves to avoid frequent reallocations

	GeneratePawnMoves(board, moves);
	GenerateKnightMoves(board, moves);
	GenerateBishopMoves(board, moves);
	GenerateRookMoves(board, moves);
	GenerateQueenMoves(board, moves);
	GenerateKingMoves(board, moves);

	return moves;
}

} // namespace Gluon