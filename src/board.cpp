#include "board.h"

#include "piece.h"
#include "boardhelpers.h"
#include "move.h"

#include <string>
#include <sstream>
#include <stdexcept>


namespace Gluon {

Board::Board(const std::string& fen)
{
    SetupWithFEN(fen);
}

void Board::Clear()
{
	squares.fill(PieceType::NONE);
	piecePlacementMap.Clear();
	isWhitesMove = true;
	castlingRights = NO_CASTLING_RIGHTS;
	enPassantSquare = NO_EN_PASSANT;
	halfMoveClock = 0;
	fullMoveNumber = 1;
}

void Board::SetupWithFEN(const std::string& fen)
{
    // Split FEN string into its components
    std::istringstream fenStream(fen);
    std::string piecePlacement, sideToMove, stringCastlingRights, stringEnPassantSquare, halfmoveClock, fullmoveNumber;
    fenStream >> piecePlacement >> sideToMove >> stringCastlingRights >> stringEnPassantSquare >> halfmoveClock >> fullmoveNumber;

    if (!fenStream)
    {
        throw std::runtime_error("Could not parse FEN string: " + fen);
    }

    // Clear the board
    Clear();

    // Place pieces
    int currentSquare = FileRankToSquare(FILE_A, RANK_8); // Start from a8
    for (char c : piecePlacement)
    {
        if (c == '/')
        {
            currentSquare -= 2 * RANK_FILE_SIZE; // Skip a rank
        }
        else if (std::isdigit(c))
        {
            currentSquare += std::stoi(std::string(1, c)); // Skip empty squares
        }
        else
        {
			Piece piece = CharToPiece(c);
            squares[currentSquare] = piece;
            piecePlacementMap.AddPiece(currentSquare, piece);
            currentSquare++;
        }
    }

    // Set side to move
    if (sideToMove != "w" && sideToMove != "b")
    {
        throw std::invalid_argument("Invalid side to move in FEN: " + sideToMove);
    }
    isWhitesMove = (sideToMove == "w");

    // Set castling rights
    castlingRights = NO_CASTLING_RIGHTS;
    if (stringCastlingRights != "-")
    {
        castlingRights = NO_CASTLING_RIGHTS;
        for (char c : stringCastlingRights)
        {
            switch (c)
            {
                case 'K': castlingRights |= WHITE_KING_SIDE_CASTLE; break;
                case 'Q': castlingRights |= WHITE_QUEEN_SIDE_CASTLE; break;
                case 'k': castlingRights |= BLACK_KING_SIDE_CASTLE; break;
                case 'q': castlingRights |= BLACK_QUEEN_SIDE_CASTLE; break;

                default:
                {
                    throw std::invalid_argument("Invalid castling right in FEN: " + std::string(1, c));
                }
            }
        }
    }

    // Set en passant square
    if (stringEnPassantSquare == "-")
    {
        enPassantSquare = NO_EN_PASSANT;
    }
    else
    {
        enPassantSquare = CoordToSquare(stringEnPassantSquare);
    }

    // Set halfmove clock
    halfMoveClock = std::stoi(halfmoveClock);

    // Set fullmove number
    fullMoveNumber = std::stoi(fullmoveNumber);
}

const std::string Board::GetBoardString(bool whitePOV) const
{
    static const std::string rankSpacing = "+---+---+---+---+---+---+---+---+\n";

    static const std::string whitePOVFileLabels = "  a   b   c   d   e   f   g   h  \n";
    static const std::string blackPOVFileLabels = "  h   g   f   e   d   c   b   a  \n";

    std::string boardString = rankSpacing;

    const int startRank = whitePOV ? RANK_8 : RANK_1;
    const int endRank = whitePOV ? RANK_1 : RANK_8;
    const int rankStep = whitePOV ? -1 : 1;

    const int startFile = whitePOV ? FILE_A : FILE_H;
    const int endFile = whitePOV ? FILE_H : FILE_A;
    const int fileStep = whitePOV ? 1 : -1;

    for (int rank = startRank;; rank += rankStep)
    {
        boardString += "|";

        for (int file = startFile;; file += fileStep)
        {
            int square = FileRankToSquare(file, rank);
            Piece piece = squares[square];
            boardString += " " + std::string(1, PieceToChar(piece)) + " |";
            if (file == endFile)
            {
                boardString += " " + std::to_string(rank + 1) + "\n"; // Print rank number at the end of the row
                break;
            }
        }

        boardString += rankSpacing;

        if (rank == endRank) { break; }
    }

    boardString += whitePOV ? whitePOVFileLabels : blackPOVFileLabels; // Print file letters at the bottom

    return boardString;
}

bool Board::IsWhitesMove() const
{
    return isWhitesMove;
}

int Board::GetEnPassantSquare() const
{
    return enPassantSquare;
}

Piece Board::GetPieceAtSquare(int square) const
{
    return squares[square];
}

bool Board::HasCastlingRight(std::uint8_t castlingRight) const
{
    return (castlingRights & castlingRight) != 0;
}

const std::vector<int>& Board::GetPawnPlacements(bool forWhite) const
{
    return piecePlacementMap.GetPawnPlacements(forWhite);
}

const std::vector<int>& Board::GetKnightPlacements(bool forWhite) const
{
    return piecePlacementMap.GetKnightPlacements(forWhite);
}

const std::vector<int>& Board::GetBishopPlacements(bool forWhite) const
{
    return piecePlacementMap.GetBishopPlacements(forWhite);
}

const std::vector<int>& Board::GetRookPlacements(bool forWhite) const
{
    return piecePlacementMap.GetRookPlacements(forWhite);
}

const std::vector<int>& Board::GetQueenPlacements(bool forWhite) const
{
    return piecePlacementMap.GetQueenPlacements(forWhite);
}

int Board::GetKingPlacement(bool forWhite) const
{
    return piecePlacementMap.GetKingPlacements(forWhite)[0];
}

void Board::MakeMove(const Move& move)
{
	const int moveFromSquare = move.GetFromSquare();
	const int moveToSquare = move.GetToSquare();
	const Piece movingPiece = squares[moveFromSquare];
	const PieceType movingPieceType = GetPieceType(movingPiece);
	const Piece capturedPiece = squares[moveToSquare];

	// Updated castling rights
    if (movingPieceType == PieceType::KING)
    {
        castlingRights &= ~(isWhitesMove ? (WHITE_KING_SIDE_CASTLE | WHITE_QUEEN_SIDE_CASTLE)
                                         : (BLACK_KING_SIDE_CASTLE | BLACK_QUEEN_SIDE_CASTLE));
    }
    else if (movingPieceType == PieceType::ROOK)
    {
        if (moveFromSquare == FileRankToSquare(FILE_H, RANK_1))
        {
            castlingRights &= ~WHITE_QUEEN_SIDE_CASTLE;
        }
        else if (moveFromSquare == FileRankToSquare(FILE_A, RANK_1))
        {
            castlingRights &= ~WHITE_KING_SIDE_CASTLE;
        }
        else if (moveFromSquare == FileRankToSquare(FILE_H, RANK_8))
        {
            castlingRights &= ~BLACK_QUEEN_SIDE_CASTLE;
        }
        else if (moveFromSquare == FileRankToSquare(FILE_A, RANK_8))
        {
            castlingRights &= ~BLACK_KING_SIDE_CASTLE;
        }
	}

    if (capturedPiece != NONE_PIECE && GetPieceType(capturedPiece) == PieceType::ROOK)
    {
        if (moveToSquare == FileRankToSquare(FILE_H, RANK_1))
        {
            castlingRights &= ~WHITE_QUEEN_SIDE_CASTLE;
        }
        else if (moveToSquare == FileRankToSquare(FILE_A, RANK_1))
        {
            castlingRights &= ~WHITE_KING_SIDE_CASTLE;
        }
        else if (moveToSquare == FileRankToSquare(FILE_H, RANK_8))
        {
            castlingRights &= ~BLACK_QUEEN_SIDE_CASTLE;
        }
        else if (moveToSquare == FileRankToSquare(FILE_A, RANK_8))
        {
            castlingRights &= ~BLACK_KING_SIDE_CASTLE;
		}
    }

    // Update half move clock
    if (movingPieceType == PieceType::PAWN || move.IsCapture())
    {
        halfMoveClock = 0;
    }
    else
    {
        halfMoveClock++;
    }

	// Handle special Moves
	enPassantSquare = NO_EN_PASSANT; // Reset en passant square by default
    if (move.HasFlag(Move::MoveFlag::DOUBLE_PAWN_PUSH))
    {
		enPassantSquare = moveToSquare + (isWhitesMove ? SOUTH_DIRECTION : NORTH_DIRECTION);

		squares[moveFromSquare] = NONE_PIECE;
		squares[moveToSquare] = movingPiece;
		piecePlacementMap.MovePiece(moveFromSquare, moveToSquare);
    }
    else if (move.HasFlag(Move::MoveFlag::KING_SIDE_CASTLE))
    {
        const int rookFromSquare = moveToSquare + EAST_DIRECTION;
        const int rookToSquare = moveToSquare + WEST_DIRECTION;

        squares[rookFromSquare] = NONE_PIECE;
        squares[rookToSquare] = squares[rookFromSquare];
		piecePlacementMap.MovePiece(rookFromSquare, rookToSquare);
    }
    else if (move.HasFlag(Move::MoveFlag::QUEEN_SIDE_CASTLE))
    {
        const int rookFromSquare = moveToSquare + WEST_DIRECTION * 2;
        const int rookToSquare = moveToSquare + EAST_DIRECTION;

        squares[rookFromSquare] = NONE_PIECE;
        squares[rookToSquare] = squares[rookFromSquare];
        piecePlacementMap.MovePiece(rookFromSquare, rookToSquare);
    }
    else if (move.HasFlag(Move::MoveFlag::EN_PASSANT_CAPTURE))
    {
        const int capturedPawnSquare = moveToSquare + (isWhitesMove ? SOUTH_DIRECTION : NORTH_DIRECTION);

        squares[moveFromSquare] = NONE_PIECE;
        squares[moveToSquare] = movingPiece;
        piecePlacementMap.MovePiece(moveFromSquare, moveToSquare);

		squares[capturedPawnSquare] = NONE_PIECE;
        piecePlacementMap.RemovePiece(capturedPawnSquare);
    }
    else if (move.IsPromotion())
    {
		PieceType promotionPieceType = PieceType::QUEEN; // Queen by default
        if (move.HasFlag(Move::MoveFlag::KNIGHT_PROMOTION) || move.HasFlag(Move::MoveFlag::KNIGHT_PROMOTION_CAPTURE))
        {
            promotionPieceType = PieceType::KNIGHT;

        }
        else if (move.HasFlag(Move::MoveFlag::BISHOP_PROMOTION) || move.HasFlag(Move::MoveFlag::BISHOP_PROMOTION_CAPTURE))
        {
            promotionPieceType = PieceType::BISHOP;
        }
        else if (move.HasFlag(Move::MoveFlag::ROOK_PROMOTION) || move.HasFlag(Move::MoveFlag::ROOK_PROMOTION_CAPTURE))
        {
            promotionPieceType = PieceType::ROOK;
        }

        if (move.IsCapture())
        {
			piecePlacementMap.RemovePiece(moveToSquare);
        }

		squares[moveFromSquare] = NONE_PIECE;
        squares[moveToSquare] = MakePiece(promotionPieceType, isWhitesMove ? PieceColour::WHITE : PieceColour::BLACK);
        piecePlacementMap.MoveAndChangePiece(moveFromSquare, moveToSquare, squares[moveToSquare]);
    }
    else
    {
        // Regular move or capture
        if (move.IsCapture() && !move.HasFlag(Move::EN_PASSANT_CAPTURE))
        {
            piecePlacementMap.RemovePiece(moveToSquare);
        }

        squares[moveFromSquare] = NONE_PIECE;
        squares[moveToSquare] = movingPiece;
        piecePlacementMap.MovePiece(moveFromSquare, moveToSquare);
    }

	// Update side to move and full move number
    if (!isWhitesMove)
    {
		fullMoveNumber++;
    }
	isWhitesMove = !isWhitesMove;
}

} // namespace Gluon