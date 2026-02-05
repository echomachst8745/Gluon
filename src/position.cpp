#include "position.h"

#include "bitboard.h"
#include "movetables.h"
#include "zobrist.h"

#include <algorithm>
#include <sstream>

namespace Gluon {

// [ Constructors ]
Position::Position()
{
    Clear();
}

// [ Public methods ]
void Position::SetupWithFEN(const std::string& fen)
{
    std::istringstream fenStringStream(fen);
    std::string piecePlacementString, activeColourString, castlingRightsString,
                enPassantTargetSquareString, halfMoveClockString, fullMoveNumberString;
    fenStringStream >> piecePlacementString >> activeColourString >> castlingRightsString
                    >> enPassantTargetSquareString >> halfMoveClockString >> fullMoveNumberString;

    // Clear the position
    Clear();

    // Set pieces
    // !EXPLAIN!
    Square currentSquare = SQUARE_A8;
    for (char pieceChar : piecePlacementString)
    {
        if (pieceChar == '/')
        {
            currentSquare = currentSquare + SOUTH + SOUTH;
        }
        else if (std::isdigit(pieceChar))
        {
            currentSquare = Square(currentSquare + (pieceChar - '0'));
        }
        else
        {
            Piece piece = CharToPiece(pieceChar);
            SetSquare(currentSquare, piece);
            currentSquare = currentSquare + EAST;
        }
    }

    // Set active colour
    activeColour = activeColourString == "w" ? WHITE : BLACK;

    // Set castling rights
    for (char castlingRightChar : castlingRightsString)
    {
        switch (castlingRightChar)
        {
            case 'K': castlingRights |= WHITE_OO;  break;
            case 'Q': castlingRights |= WHITE_OOO; break;
            case 'k': castlingRights |= BLACK_OO;  break;
            case 'q': castlingRights |= BLACK_OOO; break;
        }
    }

    // Set en passant target square
    enPassantTargetSquare = (enPassantTargetSquareString == "-")
                            ? NO_SQUARE
                            : CoordStringToSquare(enPassantTargetSquareString);

    // Set half move clock
    halfMoveClock = halfMoveClockString.length() > 0 ? std::stoi(halfMoveClockString) : 0;

    // Set full move number
    fullMoveNumber = fullMoveNumberString.length() > 0 ? std::stoi(fullMoveNumberString) : 1;

    // Set the hash key, the pieces having already been folded in by SetSquare
    UpdateHashKeyWithState();
}

void Position::MakeMove(Move move, PositionState& state)
{
    Square fromSquare = move.GetFromSquare();
    Square toSquare = move.GetToSquare();
    Move::MoveFlag moveFlag = move.GetFlag();

    Piece movedPiece = squares[fromSquare];
    Direction pushDirection = activeColour == WHITE ? NORTH : SOUTH;

    hashKeyHistory.push_back(hashKey);

    // !EXPLAIN!
    UpdateHashKeyWithState();

    // Save the state the move destroys
    state.capturedPiece = moveFlag == Move::EN_PASSANT_CAPTURE ? squares[toSquare - pushDirection]
                                                               : squares[toSquare];
    state.castlingRights = castlingRights;
    state.enPassantTargetSquare = enPassantTargetSquare;
    state.halfMoveClock = halfMoveClock;

    // Remove the captured piece
    if (moveFlag == Move::EN_PASSANT_CAPTURE)
    {
        ClearSquare(toSquare - pushDirection);
    }
    else if (move.IsCapture())
    {
        ClearSquare(toSquare);
    }

    // Move the piece
    MovePiece(fromSquare, toSquare);

    // Replace a promoting pawn
    if (move.IsPromotion())
    {
        ClearSquare(toSquare);
        SetSquare(toSquare, MakePiece(activeColour, move.GetPromotionPieceType()));
    }

    // !EXPLAIN!
    // Move the castling rook
    if (moveFlag == Move::KING_CASTLE)
    {
        MovePiece(toSquare + EAST, toSquare + WEST);
    }
    else if (moveFlag == Move::QUEEN_CASTLE)
    {
        MovePiece(toSquare + WEST + WEST, toSquare + EAST);
    }

    // !EXPLAIN!
    // Update castling rights
    castlingRights = CastlingRight(castlingRights &
                                   MoveTables::CASTLING_RIGHT_MASK_TABLE[fromSquare] &
                                   MoveTables::CASTLING_RIGHT_MASK_TABLE[toSquare]);

    // Update en passant target square
    enPassantTargetSquare = moveFlag == Move::DOUBLE_PAWN_PUSH ? toSquare - pushDirection : NO_SQUARE;

    // Update move counters
    halfMoveClock = (GetType(movedPiece) == PAWN || move.IsCapture()) ? 0 : halfMoveClock + 1;

    if (activeColour == BLACK)
    {
        ++fullMoveNumber;
    }

    activeColour = ~activeColour;

    UpdateHashKeyWithState();
}

void Position::UnmakeMove(Move move, const PositionState& state)
{
    Square fromSquare = move.GetFromSquare();
    Square toSquare = move.GetToSquare();
    Move::MoveFlag moveFlag = move.GetFlag();

    // !EXPLAIN!
    UpdateHashKeyWithState();

    activeColour = ~activeColour;

    Direction pushDirection = activeColour == WHITE ? NORTH : SOUTH;

    // Restore a promoting pawn
    if (move.IsPromotion())
    {
        ClearSquare(toSquare);
        SetSquare(toSquare, MakePiece(activeColour, PAWN));
    }

    // Move the piece back
    MovePiece(toSquare, fromSquare);

    // Restore the captured piece
    if (moveFlag == Move::EN_PASSANT_CAPTURE)
    {
        SetSquare(toSquare - pushDirection, state.capturedPiece);
    }
    else if (move.IsCapture())
    {
        SetSquare(toSquare, state.capturedPiece);
    }

    // Move the castling rook back
    if (moveFlag == Move::KING_CASTLE)
    {
        MovePiece(toSquare + WEST, toSquare + EAST);
    }
    else if (moveFlag == Move::QUEEN_CASTLE)
    {
        MovePiece(toSquare + EAST, toSquare + WEST + WEST);
    }

    // Restore the saved state
    castlingRights = state.castlingRights;
    enPassantTargetSquare = state.enPassantTargetSquare;
    halfMoveClock = state.halfMoveClock;

    if (activeColour == BLACK)
    {
        --fullMoveNumber;
    }

    UpdateHashKeyWithState();

    hashKeyHistory.pop_back();
}

// !EXPLAIN!
bool Position::IsInCheck() const
{
    Colour enemyColour = ~activeColour;

    Square kingSquare = Square(BB::GetLSB(GetPieceBitboard(MakePiece(activeColour, KING))));

    Bitboard enemyBishopsAndQueensBitboard = GetPieceBitboard(MakePiece(enemyColour, BISHOP)) |
                                             GetPieceBitboard(MakePiece(enemyColour, QUEEN));
    Bitboard enemyRooksAndQueensBitboard   = GetPieceBitboard(MakePiece(enemyColour, ROOK)) |
                                             GetPieceBitboard(MakePiece(enemyColour, QUEEN));

    return (MoveTables::PAWN_ATTACK_TABLE[activeColour][kingSquare] &
            GetPieceBitboard(MakePiece(enemyColour, PAWN)))                                          ||
           (MoveTables::KNIGHT_MOVE_TABLE[kingSquare] & GetPieceBitboard(MakePiece(enemyColour, KNIGHT))) ||
           (MoveTables::KING_MOVE_TABLE[kingSquare] & GetPieceBitboard(MakePiece(enemyColour, KING)))     ||
           (MoveTables::GetBishopMoves(kingSquare, allOccupancyBitboard) & enemyBishopsAndQueensBitboard) ||
           (MoveTables::GetRookMoves(kingSquare, allOccupancyBitboard) & enemyRooksAndQueensBitboard);
}

// !EXPLAIN!
bool Position::IsRepetition(int searchPly) const
{
    const int maximumLookback = std::min(halfMoveClock, int(hashKeyHistory.size()));

    int repetitionCount = 0;

    for (int lookback = 2; lookback <= maximumLookback; lookback += 2)
    {
        if (hashKeyHistory[hashKeyHistory.size() - size_t(lookback)] != hashKey)
        {
            continue;
        }

        ++repetitionCount;

        if (lookback <= searchPly || repetitionCount == 2)
        {
            return true;
        }
    }

    return false;
}

std::string Position::ToString(bool whitePOV) const
{
    static const std::string rankSpacing = "+---+---+---+---+---+---+---+---+\n";

    static const std::string whitePOVFileLabels = "  a   b   c   d   e   f   g   h  \n";
    static const std::string blackPOVFileLabels = "  h   g   f   e   d   c   b   a  \n";

    const Rank startRank = whitePOV ? RANK_8 : RANK_1;
    const Rank endRank = whitePOV ? RANK_1 : RANK_8;
    const int rankStep = whitePOV ? -1 : 1;

    const File startFile = whitePOV ? FILE_A : FILE_H;
    const File endFile = whitePOV ? FILE_H : FILE_A;
    const int fileStep = whitePOV ? 1 : -1;

    std::string positionString = rankSpacing;

    for (Rank rank = startRank;; rank = Rank(rank + rankStep))
    {
        positionString += "|";

        for (File file = startFile;; file = File(file + fileStep))
        {
            Square square = FileRankToSquare(file, rank);
            Piece piece = squares[square];

            positionString += " " + std::string(1, PieceToChar(piece)) + " |";

            if (file == endFile)
            {
                positionString += " " + std::to_string(rank + 1) + "\n";
                break;
            }
        }

        positionString += rankSpacing;

        if (rank == endRank) { break; }
    }

    positionString += whitePOV ? whitePOVFileLabels : blackPOVFileLabels;

    return positionString;
}

// [ Private methods ]
void Position::Clear()
{
    squares.fill(NO_PIECE);
    pieceBitboards.fill(0ULL);
    colourOccupancyBitboards.fill(0ULL);
    allOccupancyBitboard = 0ULL;
    activeColour = WHITE;
    castlingRights = NO_CASTLING_RIGHTS;
    enPassantTargetSquare = NO_SQUARE;
    halfMoveClock = 0;
    fullMoveNumber = 1;
    hashKey = 0ULL;
    hashKeyHistory.clear();
}

void Position::SetSquare(Square square, Piece piece)
{
    squares[square] = piece;

    BB::SetSquare(pieceBitboards[PieceToBitboardIndex(piece)], square);
    BB::SetSquare(colourOccupancyBitboards[GetColour(piece)], square);
    BB::SetSquare(allOccupancyBitboard, square);

    hashKey ^= Zobrist::KEYS.pieceSquareKeys[PieceToBitboardIndex(piece)][square];
}

void Position::ClearSquare(Square square)
{
    Piece piece = squares[square];
    Bitboard squareBitboard = SquareToBitboard(square);

    pieceBitboards[PieceToBitboardIndex(piece)] &= ~squareBitboard;
    colourOccupancyBitboards[GetColour(piece)] &= ~squareBitboard;
    allOccupancyBitboard &= ~squareBitboard;

    squares[square] = NO_PIECE;

    hashKey ^= Zobrist::KEYS.pieceSquareKeys[PieceToBitboardIndex(piece)][square];
}

// !EXPLAIN!
void Position::MovePiece(Square fromSquare, Square toSquare)
{
    Piece piece = squares[fromSquare];
    Bitboard fromToBitboard = SquareToBitboard(fromSquare) | SquareToBitboard(toSquare);

    pieceBitboards[PieceToBitboardIndex(piece)] ^= fromToBitboard;
    colourOccupancyBitboards[GetColour(piece)] ^= fromToBitboard;
    allOccupancyBitboard ^= fromToBitboard;

    squares[fromSquare] = NO_PIECE;
    squares[toSquare] = piece;

    hashKey ^= Zobrist::KEYS.pieceSquareKeys[PieceToBitboardIndex(piece)][fromSquare] ^
               Zobrist::KEYS.pieceSquareKeys[PieceToBitboardIndex(piece)][toSquare];
}

// !EXPLAIN!
void Position::UpdateHashKeyWithState()
{
    hashKey ^= Zobrist::KEYS.castlingRightKeys[castlingRights];

    if (enPassantTargetSquare != NO_SQUARE)
    {
        hashKey ^= Zobrist::KEYS.enPassantFileKeys[SquareToFile(enPassantTargetSquare)];
    }

    if (activeColour == BLACK)
    {
        hashKey ^= Zobrist::KEYS.sideToMoveKey;
    }
}

} // namespace Gluon