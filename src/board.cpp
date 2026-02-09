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

std::uint64_t Board::GetPawnPlacementBitboard(bool forWhite) const
{
    return piecePlacementMap.GetPawnBitboard(forWhite);
}

std::uint64_t Board::GetKnightPlacementBitboard(bool forWhite) const
{
    return piecePlacementMap.GetKnightBitboard(forWhite);
}

std::uint64_t Board::GetBishopPlacementBitboard(bool forWhite) const
{
    return piecePlacementMap.GetBishopBitboard(forWhite);
}

std::uint64_t Board::GetRookPlacementBitboard(bool forWhite) const
{
    return piecePlacementMap.GetRookBitboard(forWhite);
}

std::uint64_t Board::GetQueenPlacementBitboard(bool forWhite) const
{
    return piecePlacementMap.GetQueenBitboard(forWhite);
}

std::uint64_t Board::GetOccupiedSquaresBitboardForColour(bool forWhite) const
{
    return piecePlacementMap.GetPawnBitboard(forWhite)   | piecePlacementMap.GetKnightBitboard(forWhite) |
           piecePlacementMap.GetBishopBitboard(forWhite) | piecePlacementMap.GetRookBitboard(forWhite)   |
           piecePlacementMap.GetQueenBitboard(forWhite)  | (1ULL << GetKingPlacement(forWhite));
}

std::uint64_t Board::GetAllOccupiedSquaresBitboard() const
{
    return GetOccupiedSquaresBitboardForColour(true) | GetOccupiedSquaresBitboardForColour(false);
}

void Board::MakeMove(const Move& move)
{
    const bool usWhite = isWhitesMove;
    const int fromSquare = move.GetFromSquare();
    const int toSquare = move.GetToSquare();
    const Piece movingPiece = squares[fromSquare];
    const PieceType movingPieceType = GetPieceType(movingPiece);
    const Piece capturedPiece = move.HasFlag(Move::EN_PASSANT_CAPTURE) ? MakePiece(PieceType::PAWN, usWhite ? PieceColour::BLACK : PieceColour::WHITE) : squares[toSquare];

    // Save board state
    boardStateHistory.push(BoardState{
        move,
        enPassantSquare,
        castlingRights,
        halfMoveClock,
        fullMoveNumber,
        movingPiece,
        capturedPiece
    });

    // Handle captures
    if (move.IsCapture())
    {
        int captureSquare = toSquare;

        if (move.HasFlag(Move::EN_PASSANT_CAPTURE))
        {
            captureSquare += usWhite ? SOUTH_DIRECTION : NORTH_DIRECTION;
        }

        squares[captureSquare] = NONE_PIECE;
        piecePlacementMap.RemovePiece(captureSquare);
    }

    // Handle promotions and normal moves
    squares[fromSquare] = NONE_PIECE;
    if (move.IsPromotion())
    {
        const PieceType promotionPieceType = move.GetPromotionPieceType();
        const Piece promotionPiece = MakePiece(promotionPieceType, usWhite ? PieceColour::WHITE : PieceColour::BLACK);

        squares[toSquare] = promotionPiece;
        piecePlacementMap.MoveAndChangePiece(fromSquare, toSquare, promotionPiece);
    }
    else
    {
        // Move the piece normally otherwise
        squares[toSquare] = movingPiece;
        piecePlacementMap.MovePiece(fromSquare, toSquare);
    }

    // Handle castling rook moves
    if (move.HasFlag(Move::KING_SIDE_CASTLE) || move.HasFlag(Move::QUEEN_SIDE_CASTLE))
    {
        const bool isKingSide = move.HasFlag(Move::KING_SIDE_CASTLE);

        const int rookFromFile = isKingSide ? FILE_H : FILE_A;
        const int rookFromRank = usWhite ? RANK_1 : RANK_8;
        const int rookFromSquare = FileRankToSquare(rookFromFile, rookFromRank);

        const int rookToFile = isKingSide ? FILE_F : FILE_D;
        const int rookToRank = rookFromRank;
        const int rookToSquare = FileRankToSquare(rookToFile, rookToRank);

        squares[rookToSquare] = squares[rookFromSquare];
        squares[rookFromSquare] = NONE_PIECE;
        piecePlacementMap.MovePiece(rookFromSquare, rookToSquare);
    }

    // Update castling rights
    if (movingPieceType == PieceType::KING)
    {
        castlingRights &= ~(usWhite ? (WHITE_KING_SIDE_CASTLE | WHITE_QUEEN_SIDE_CASTLE) :
                                      (BLACK_KING_SIDE_CASTLE | BLACK_QUEEN_SIDE_CASTLE));
    }
    else if (movingPieceType == PieceType::ROOK)
    {
        if (usWhite)
        {
            if (fromSquare == FileRankToSquare(FILE_H, RANK_1))
            {
                castlingRights &= ~WHITE_KING_SIDE_CASTLE;
            }
            else if (fromSquare == FileRankToSquare(FILE_A, RANK_1))
            {
                castlingRights &= ~WHITE_QUEEN_SIDE_CASTLE;
            }
        }
        else
        {
            if (fromSquare == FileRankToSquare(FILE_H, RANK_8))
            {
                castlingRights &= ~BLACK_KING_SIDE_CASTLE;
            }
            else if (fromSquare == FileRankToSquare(FILE_A, RANK_8))
            {
                castlingRights &= ~BLACK_QUEEN_SIDE_CASTLE;
            }
        }
    }
    if (GetPieceType(capturedPiece) == PieceType::ROOK)
    {
        if (!usWhite)
        {
            if (toSquare == FileRankToSquare(FILE_H, RANK_1))
            {
                castlingRights &= ~WHITE_KING_SIDE_CASTLE;
            }
            else if (toSquare == FileRankToSquare(FILE_A, RANK_1))
            {
                castlingRights &= ~WHITE_QUEEN_SIDE_CASTLE;
            }
        }
        else
        {
            if (toSquare == FileRankToSquare(FILE_H, RANK_8))
            {
                castlingRights &= ~BLACK_KING_SIDE_CASTLE;
            }
            else if (toSquare == FileRankToSquare(FILE_A, RANK_8))
            {
                castlingRights &= ~BLACK_QUEEN_SIDE_CASTLE;
            }
        }
    }

    // Update en passant square
    if (move.HasFlag(Move::DOUBLE_PAWN_PUSH))
    {
        enPassantSquare = toSquare + (usWhite ? SOUTH_DIRECTION : NORTH_DIRECTION);
    }
    else
    {
        enPassantSquare = NO_EN_PASSANT;
    }

    // Update clocks
    if (movingPieceType == PieceType::PAWN || move.IsCapture())
    {
        halfMoveClock = 0;
    }
    else
    {
        halfMoveClock++;
    }

    if (!usWhite)
    {
        fullMoveNumber++;
    }

    // Switch side to move
    isWhitesMove = !isWhitesMove;
}

void Board::UndoLastMove()
{
    if (boardStateHistory.empty())
        throw std::runtime_error("No moves to undo");

    BoardState lastState = boardStateHistory.top();
    boardStateHistory.pop();

    const Move& moveMade = lastState.moveMade;
    const int from = moveMade.GetFromSquare();
    const int to = moveMade.GetToSquare();

    // Undo side-to-move FIRST (so "us" is the mover)
    isWhitesMove = !isWhitesMove;
    const bool usWhite = isWhitesMove;

    const Piece movedPiece = lastState.movedPiece;     // original piece from 'from' (pawn if promotion)
    const Piece capturedPiece = lastState.capturedPiece;

    // Restore saved state (these are from BEFORE the move)
    enPassantSquare = lastState.enPassantSquare;
    castlingRights = lastState.castlingRights;
    halfMoveClock = lastState.halfMoveClock;
    fullMoveNumber = lastState.fullMoveNumber;

    // Compute captured square (EP captures a pawn behind 'to')
    int capturedSquare = to;
    if (moveMade.HasFlag(Move::EN_PASSANT_CAPTURE))
        capturedSquare = to + (usWhite ? SOUTH_DIRECTION : NORTH_DIRECTION);

    // --- PIECE PLACEMENT MAP (must obey invariants) ---

    // 1) Undo castling rook move (rook is currently on F/D, destination H/A must be empty)
    if (moveMade.HasFlag(Move::KING_SIDE_CASTLE) || moveMade.HasFlag(Move::QUEEN_SIDE_CASTLE))
    {
        const bool isKingSide = moveMade.HasFlag(Move::KING_SIDE_CASTLE);

        const int rookFromFile = isKingSide ? FILE_F : FILE_D;
        const int rookRank = usWhite ? RANK_1 : RANK_8;
        const int rookFrom = FileRankToSquare(rookFromFile, rookRank);

        const int rookToFile = isKingSide ? FILE_H : FILE_A;
        const int rookTo = FileRankToSquare(rookToFile, rookRank);

        piecePlacementMap.MovePiece(rookFrom, rookTo);
    }

    // 2) Move the moved piece back (to -> from)
    //    For promotion: remove promoted piece from 'to', add pawn back on 'from'
    if (moveMade.IsPromotion())
    {
        piecePlacementMap.MoveAndChangePiece(to, from, movedPiece);
    }
    else
    {
        piecePlacementMap.MovePiece(to, from);
    }

    // 3) Restore captured piece (square must be empty in the map right now)
    if (moveMade.IsCapture())
    {
        piecePlacementMap.AddPiece(capturedSquare, capturedPiece);
    }

    // --- SQUARE ARRAY (mirror the same final position) ---

    // Undo rook squares for castling
    if (moveMade.HasFlag(Move::KING_SIDE_CASTLE) || moveMade.HasFlag(Move::QUEEN_SIDE_CASTLE))
    {
        const bool isKingSide = moveMade.HasFlag(Move::KING_SIDE_CASTLE);

        const int rookFromFile = isKingSide ? FILE_F : FILE_D;
        const int rookRank = usWhite ? RANK_1 : RANK_8;
        const int rookFrom = FileRankToSquare(rookFromFile, rookRank);

        const int rookToFile = isKingSide ? FILE_H : FILE_A;
        const int rookTo = FileRankToSquare(rookToFile, rookRank);

        squares[rookTo] = squares[rookFrom];
        squares[rookFrom] = NONE_PIECE;
    }

    // Move piece back on board
    squares[from] = movedPiece;
    squares[to] = NONE_PIECE;

    // Restore capture
    if (moveMade.IsCapture())
    {
        squares[capturedSquare] = capturedPiece;
    }
}

} // namespace Gluon