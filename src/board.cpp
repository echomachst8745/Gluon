#include "board.h"
#include "boardhelpers.h"
#include "piece.h"
#include "move.h"
#include "movegenerator.h"

#include <algorithm>
#include <string>
#include <sstream>
#include <iostream>

namespace Gluon {

Board::Board()
{
    SetupWithFEN(BoardHelpers::STARTING_FEN);
}

Board::Board(const std::string& fen)
{
    SetupWithFEN(fen);
}

bool Board::IsWhiteToMove() const noexcept
{
    return isWhiteToMove;
}

int Board::GetEnPassantSquare() const noexcept
{
    return enPassantSquare;
}

bool Board::HasCastlingRight(std::uint8_t castlingRight) const noexcept
{
    return (castlingRights & castlingRight) == castlingRight;
}

const std::array<Piece::Piece, BoardHelpers::NUM_SQUARES>& Board::GetSquares() const noexcept
{
    return squares;
}

void Board::SetCurrentPlayerInCheck(bool inCheck)
{
    currentPlayerInCheck = inCheck;
}

bool Board::IsCurrentPlayerInCheck() const
{
    return currentPlayerInCheck;
}

int Board::GetFullmoveNumber() const noexcept
{
    return fullmoveNumber;
}

const std::vector<int>& Board::GetPawnPlacements(bool forWhite) const noexcept
{
    return piecePlacements.GetPawnBucket(forWhite);
}

const std::vector<int>& Board::GetKnightPlacements(bool forWhite) const noexcept
{
    return piecePlacements.GetKnightBucket(forWhite);
}

const std::vector<int>& Board::GetBishopPlacements(bool forWhite) const noexcept
{
    return piecePlacements.GetBishopBucket(forWhite);
}

const std::vector<int>& Board::GetRookPlacements(bool forWhite) const noexcept
{
    return piecePlacements.GetRookBucket(forWhite);
}

const std::vector<int>& Board::GetQueenPlacements(bool forWhite) const noexcept
{
    return piecePlacements.GetQueenBucket(forWhite);
}

int Board::GetKingSquare(bool forWhite) const
{
    const auto& kingBucket = piecePlacements.GetKingBucket(forWhite);

    return kingBucket[0];
}

Piece::Piece Board::GetPieceAtSquare(int square) const
{
    return squares[square];
}

Piece::Piece Board::GetPieceAtFileRank(int file, int rank) const
{
    return GetPieceAtSquare(BoardHelpers::FileRankToSquare(file, rank));
}

void Board::Clear()
{
    squares.fill(Piece::NONE);
    piecePlacements.Clear();

    isWhiteToMove   = true;
    castlingRights  = BoardHelpers::NO_CASTLING_RIGHTS;
    enPassantSquare = BoardHelpers::NO_EN_PASSANT;
    halfmoveClock   = 0;
    fullmoveNumber  = 1;
    currentPlayerInCheck = false;
}

void Board::SetupWithFEN(const std::string& fen)
{
    using namespace Piece;

    // Split FEN string into its components
    std::istringstream fenStream(fen);
    std::string piecePlacement, sideToMove, castlingRights, enPassantSquare, halfmoveClock, fullmoveNumber;
    fenStream >> piecePlacement >> sideToMove >> castlingRights >> enPassantSquare >> halfmoveClock >> fullmoveNumber;

    if (!fenStream)
    {
        throw std::runtime_error("Could not parse FEN string: " + fen);
    }

    // Clear the board
    Clear();

    // Place pieces
    int currentSquare = BoardHelpers::FileRankToSquare(BoardHelpers::FILE_A, BoardHelpers::RANK_8); // Start from a8
    for (char c : piecePlacement)
    {
        if (c == '/')
        {
            currentSquare -= 2 * BoardHelpers::RANK_FILE_SIZE; // Skip a rank
        }
        else if (std::isdigit(c))
        {
            currentSquare += std::stoi(std::string(1, c)); // Skip empty squares
        }
        else
        {
            try
            {
                squares[currentSquare] = CharToPiece(c);
                piecePlacements.AddPiece(squares[currentSquare], currentSquare);
            }
            catch (const std::invalid_argument& e)
            {
                throw std::invalid_argument("Invalid piece character in FEN: " + std::string(1, c));
            }

            currentSquare++;
        }
    }

    // Set side to move
    if (sideToMove != "w" && sideToMove != "b")
    {
        throw std::invalid_argument("Invalid side to move in FEN: " + sideToMove);
    }
    isWhiteToMove = (sideToMove == "w");

    // Set castling rights
    if (castlingRights == "-")
    {
        this->castlingRights = BoardHelpers::NO_CASTLING_RIGHTS;
    }
    else
    {
        this->castlingRights = BoardHelpers::NO_CASTLING_RIGHTS;
        for (char c : castlingRights)
        {
            switch (c)
            {
                case 'K': this->castlingRights |= BoardHelpers::WHITE_KING_SIDE_CASTLE; break;
                case 'Q': this->castlingRights |= BoardHelpers::WHITE_QUEEN_SIDE_CASTLE; break;
                case 'k': this->castlingRights |= BoardHelpers::BLACK_KING_SIDE_CASTLE; break;
                case 'q': this->castlingRights |= BoardHelpers::BLACK_QUEEN_SIDE_CASTLE; break;

                default:
                {
                    throw std::invalid_argument("Invalid castling right in FEN: " + std::string(1, c));
                }
            }
        }
    }

    // Set en passant square
    if (enPassantSquare == "-")
    {
        this->enPassantSquare = BoardHelpers::NO_EN_PASSANT;
    }
    else
    {
        try
        {
            this->enPassantSquare = BoardHelpers::CoordToSquare(enPassantSquare);
        }
        catch (const std::invalid_argument& e)
        {
            throw std::invalid_argument("Invalid en passant square in FEN: " + enPassantSquare);
        }
    }

    // Set halfmove clock
    try
    {
        this->halfmoveClock = std::stoi(halfmoveClock);
    }
    catch (const std::exception& e)
    {
        throw std::invalid_argument("Invalid halfmove clock in FEN: " + halfmoveClock);
    }

    // Set fullmove number
    try
    {
        this->fullmoveNumber = std::stoi(fullmoveNumber);
    }
    catch (const std::exception& e)
    {
        throw std::invalid_argument("Invalid fullmove number in FEN: " + fullmoveNumber);
    }
}

const std::string Board::GetBoardString(bool whitePOV) const
{
    using namespace Piece;
    using Piece = Piece::Piece;

    static const std::string rankSpacing = "+---+---+---+---+---+---+---+---+\n";

    static const std::string whitePOVFileLabels = "  a   b   c   d   e   f   g   h  \n";
    static const std::string blackPOVFileLabels = "  h   g   f   e   d   c   b   a  \n";

    std::string boardString = rankSpacing;

    const int startRank = whitePOV ? BoardHelpers::RANK_8 : BoardHelpers::RANK_1;
    const int endRank   = whitePOV ? BoardHelpers::RANK_1 : BoardHelpers::RANK_8;
    const int rankStep  = whitePOV ? -1 : 1;

    const int startFile = whitePOV ? BoardHelpers::FILE_A : BoardHelpers::FILE_H;
    const int endFile   = whitePOV ? BoardHelpers::FILE_H : BoardHelpers::FILE_A;
    const int fileStep  = whitePOV ? 1 : -1;

    for (int rank = startRank;; rank += rankStep)
    {
        boardString += "|";

        for (int file = startFile;; file += fileStep)
        {
            int square = BoardHelpers::FileRankToSquare(file, rank);
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

void Board::MakeMove(const MoveGenerator::Move& move)
{
    const int fromSquare = move.GetFromSquare();
    const int toSquare   = move.GetToSquare();

    const Piece::Piece movingPiece = GetPieceAtSquare(fromSquare);
    const bool movingPieceIsWhite = Piece::IsWhite(movingPiece);

    // Set captured square and piece
    int capturedSquare = -1;
    Piece::Piece capturedPiece = Piece::NONE;

    if (move.HasFlag(MoveGenerator::MoveValues::EN_PASSANT))
    {
        capturedSquare = toSquare + (movingPieceIsWhite ? MoveGenerator::SOUTH_DIRECTION : MoveGenerator::NORTH_DIRECTION);
        capturedPiece  = GetPieceAtSquare(capturedSquare);
    }
    else if (move.IsCapture())
    {
        capturedSquare = toSquare;
        capturedPiece  = GetPieceAtSquare(toSquare);
    }

    // Save undo state
    UndoMoveState undoMoveState = UndoMoveState(
                                  move,
                                  enPassantSquare,
                                  castlingRights,
                                  halfmoveClock,
                                  fullmoveNumber,
                                  movingPiece,
                                  capturedPiece,
                                  capturedSquare);
    undoMoveStateStack.push(undoMoveState);

    // Clear en passant square (will be set later if pawn double push)
    enPassantSquare = BoardHelpers::NO_EN_PASSANT;

    // Remove captured piece
    if (move.IsCapture())
    {
        piecePlacements.RemovePiece(capturedSquare);
        squares[capturedSquare] = Piece::NONE;
    }

    // Set what piece is placed
    Piece::Piece placedPiece = movingPiece;

    // Handle promotions
    if (move.HasFlag(MoveGenerator::MoveValues::KNIGHT_PROMOTION) || move.HasFlag(MoveGenerator::MoveValues::KNIGHT_PROMOTION_CAPTURE))
    {
        placedPiece = Piece::MakePiece(Piece::KNIGHT, movingPieceIsWhite);
    }
    else if (move.HasFlag(MoveGenerator::MoveValues::BISHOP_PROMOTION) || move.HasFlag(MoveGenerator::MoveValues::BISHOP_PROMOTION_CAPTURE))
    {
        placedPiece = Piece::MakePiece(Piece::BISHOP, movingPieceIsWhite);
    }
    else if (move.HasFlag(MoveGenerator::MoveValues::ROOK_PROMOTION) || move.HasFlag(MoveGenerator::MoveValues::ROOK_PROMOTION_CAPTURE))
    {
        placedPiece = Piece::MakePiece(Piece::ROOK, movingPieceIsWhite);
    }
    else if (move.HasFlag(MoveGenerator::MoveValues::QUEEN_PROMOTION) || move.HasFlag(MoveGenerator::MoveValues::QUEEN_PROMOTION_CAPTURE))
    {
        placedPiece = Piece::MakePiece(Piece::QUEEN, movingPieceIsWhite);
    }

    // Remove the piece from its original square
    squares[fromSquare] = Piece::NONE;

    // Place the piece
    if (!move.IsPromotion())
    {
        piecePlacements.MovePiece(fromSquare, toSquare);
    }
    else
    {
        piecePlacements.MoveAndChangePiece(fromSquare, toSquare, placedPiece);
    }
    squares[toSquare] = placedPiece;

    // Set en passant square for double pawn push
    if (move.HasFlag(MoveGenerator::MoveValues::DOUBLE_PAWN_PUSH))
    {
        enPassantSquare = toSquare + (movingPieceIsWhite ? MoveGenerator::SOUTH_DIRECTION : MoveGenerator::NORTH_DIRECTION);
    }

    // Handle rook moves when castling
    if (move.HasFlag(MoveGenerator::MoveValues::KING_CASTLE) || move.HasFlag(MoveGenerator::MoveValues::QUEEN_CASTLE))
    {
        const bool isKingSide = move.HasFlag(MoveGenerator::MoveValues::KING_CASTLE);

        const int rookFromSquare = movingPieceIsWhite
                                    ? BoardHelpers::FileRankToSquare(isKingSide ? BoardHelpers::FILE_H : BoardHelpers::FILE_A, BoardHelpers::RANK_1)
                                    : BoardHelpers::FileRankToSquare(isKingSide ? BoardHelpers::FILE_H : BoardHelpers::FILE_A, BoardHelpers::RANK_8);

        const int rookToSquare = movingPieceIsWhite
                                    ? BoardHelpers::FileRankToSquare(isKingSide ? BoardHelpers::FILE_F : BoardHelpers::FILE_D, BoardHelpers::RANK_1)
                                    : BoardHelpers::FileRankToSquare(isKingSide ? BoardHelpers::FILE_F : BoardHelpers::FILE_D, BoardHelpers::RANK_8);

        squares[rookFromSquare] = Piece::NONE;
        squares[rookToSquare]   = Piece::MakePiece(Piece::ROOK, movingPieceIsWhite);
        piecePlacements.MovePiece(rookFromSquare, rookToSquare);

        castlingRights &= movingPieceIsWhite
                           ? ~(BoardHelpers::WHITE_KING_SIDE_CASTLE | BoardHelpers::WHITE_QUEEN_SIDE_CASTLE)
                           : ~(BoardHelpers::BLACK_KING_SIDE_CASTLE | BoardHelpers::BLACK_QUEEN_SIDE_CASTLE);
    }

    // Update castling rights if king moves or rook moves/captured
    if (Piece::GetType(movingPiece) == Piece::KING)
    {
        castlingRights &= movingPieceIsWhite
                           ? ~(BoardHelpers::WHITE_KING_SIDE_CASTLE | BoardHelpers::WHITE_QUEEN_SIDE_CASTLE)
                           : ~(BoardHelpers::BLACK_KING_SIDE_CASTLE | BoardHelpers::BLACK_QUEEN_SIDE_CASTLE);
    }
    else if (Piece::GetType(movingPiece) == Piece::ROOK)
    {
        if (fromSquare == BoardHelpers::FileRankToSquare(BoardHelpers::FILE_A, BoardHelpers::RANK_1)) { castlingRights &= ~BoardHelpers::WHITE_QUEEN_SIDE_CASTLE; }
        else if (fromSquare == BoardHelpers::FileRankToSquare(BoardHelpers::FILE_H, BoardHelpers::RANK_1)) { castlingRights &= ~BoardHelpers::WHITE_KING_SIDE_CASTLE; }
        else if (fromSquare == BoardHelpers::FileRankToSquare(BoardHelpers::FILE_A, BoardHelpers::RANK_8)) { castlingRights &= ~BoardHelpers::BLACK_QUEEN_SIDE_CASTLE; }
        else if (fromSquare == BoardHelpers::FileRankToSquare(BoardHelpers::FILE_H, BoardHelpers::RANK_8)) { castlingRights &= ~BoardHelpers::BLACK_KING_SIDE_CASTLE; }
    }
    if (move.IsCapture() && Piece::GetType(capturedPiece) == Piece::ROOK)
    {
        if (capturedSquare == BoardHelpers::FileRankToSquare(BoardHelpers::FILE_A, BoardHelpers::RANK_1)) { castlingRights &= ~BoardHelpers::WHITE_QUEEN_SIDE_CASTLE; }
        else if (capturedSquare == BoardHelpers::FileRankToSquare(BoardHelpers::FILE_H, BoardHelpers::RANK_1)) { castlingRights &= ~BoardHelpers::WHITE_KING_SIDE_CASTLE; }
        else if (capturedSquare == BoardHelpers::FileRankToSquare(BoardHelpers::FILE_A, BoardHelpers::RANK_8)) { castlingRights &= ~BoardHelpers::BLACK_QUEEN_SIDE_CASTLE; }
        else if (capturedSquare == BoardHelpers::FileRankToSquare(BoardHelpers::FILE_H, BoardHelpers::RANK_8)) { castlingRights &= ~BoardHelpers::BLACK_KING_SIDE_CASTLE; }
    }

    // Update halfmove clock
    if (Piece::GetType(movingPiece) == Piece::PAWN || move.IsCapture())
    {
        halfmoveClock = 0;
    }
    else
    {
        halfmoveClock++;
    }

    // Update fullmove number
    if (!isWhiteToMove)
    {
        fullmoveNumber++;
    }

    // Switch side to move
    isWhiteToMove = !isWhiteToMove;
}

void Board::UnmakeLastMove()
{
    if (undoMoveStateStack.empty())
    {
        throw std::runtime_error("No move to unmake: undo stack is empty");
    }

    // Retrieve the last undo state
    const UndoMoveState undoMoveState = undoMoveStateStack.top();
    undoMoveStateStack.pop();

    const MoveGenerator::Move& move = undoMoveState.moveMade;

    const int fromSquare = move.GetFromSquare();
    const int toSquare   = move.GetToSquare();

    // Switch side to move back
    isWhiteToMove = !isWhiteToMove;

    // Restore board state
    enPassantSquare = undoMoveState.previousEnPassantSquare;
    castlingRights  = undoMoveState.previousCastlingRights;
    halfmoveClock   = undoMoveState.previousHalfmoveClock;
    fullmoveNumber  = undoMoveState.previousFullmoveNumber;

    const Piece::Piece movedPiece = undoMoveState.movedPiece;
    const bool movingPieceIsWhite = Piece::IsWhite(movedPiece);

    // Undo promotion
    if (move.IsPromotion())
    {
        squares[toSquare]   = Piece::NONE;
        squares[fromSquare] = movedPiece;
        piecePlacements.MoveAndChangePiece(toSquare, fromSquare, movedPiece);
    }
    else if (move.HasFlag(MoveGenerator::MoveValues::KING_CASTLE) || move.HasFlag(MoveGenerator::MoveValues::QUEEN_CASTLE))
    {
        // Undo castling
        const bool isKingSide = move.HasFlag(MoveGenerator::MoveValues::KING_CASTLE);

        const int rookFromSquare = movingPieceIsWhite
                                    ? BoardHelpers::FileRankToSquare(isKingSide ? BoardHelpers::FILE_H : BoardHelpers::FILE_A, BoardHelpers::RANK_1)
                                    : BoardHelpers::FileRankToSquare(isKingSide ? BoardHelpers::FILE_H : BoardHelpers::FILE_A, BoardHelpers::RANK_8);

        const int rookToSquare = movingPieceIsWhite
                                    ? BoardHelpers::FileRankToSquare(isKingSide ? BoardHelpers::FILE_F : BoardHelpers::FILE_D, BoardHelpers::RANK_1)
                                    : BoardHelpers::FileRankToSquare(isKingSide ? BoardHelpers::FILE_F : BoardHelpers::FILE_D, BoardHelpers::RANK_8);

        // Move king back
        squares[toSquare]   = Piece::NONE;
        squares[fromSquare] = movedPiece;
        piecePlacements.MovePiece(toSquare, fromSquare);

        // Move rook back
        squares[rookToSquare]   = Piece::NONE;
        squares[rookFromSquare] = Piece::MakePiece(Piece::ROOK, movingPieceIsWhite);
        piecePlacements.MovePiece(rookToSquare, rookFromSquare);
    }
    else
    {
        // Normal move
        squares[toSquare]   = Piece::NONE;
        squares[fromSquare] = movedPiece;
        piecePlacements.MovePiece(toSquare, fromSquare);
    }

    // Restore captured piece
    if (undoMoveState.capturedPiece != Piece::NONE)
    {
        const int capturedSquare = undoMoveState.capturedPieceSquare;
        squares[capturedSquare] = undoMoveState.capturedPiece;
        piecePlacements.AddPiece(undoMoveState.capturedPiece, capturedSquare);
    }
}

} // namespace Gluon