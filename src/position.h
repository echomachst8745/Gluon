#pragma once

#include "types.h"
#include "move.h"

#include <array>
#include <string>
#include <ostream>
#include <cassert>
#include <vector>

namespace Gluon {

// State that a move destroys and so cannot be recovered from the move alone.
struct PositionState
{
    Piece capturedPiece;

    CastlingRight castlingRights;

    Square enPassantTargetSquare;

    int halfMoveClock;
};

class Position
{
public:
    // [ Constructors ]
    Position();

    // [ Public methods ]
    void SetupWithFEN(const std::string& fen);

    void MakeMove(Move move, PositionState& state);

    void UnmakeMove(Move move, const PositionState& state);

    bool IsInCheck() const;

    bool IsRepetition(int searchPly) const;

    std::string ToString(bool whitePOV = true) const;

    inline Colour GetActiveColour() const
    {
        return activeColour;
    }

    inline Bitboard GetPieceBitboard(Piece piece) const
    {
        return pieceBitboards[PieceToBitboardIndex(piece)];
    }

    inline Bitboard GetOccupancyBitboard(Colour colour) const
    {
        return colourOccupancyBitboards[colour];
    }

    inline Bitboard GetAllOccupancyBitboard() const
    {
        return allOccupancyBitboard;
    }

    inline Square GetEnPassantTargetSquare() const
    {
        return enPassantTargetSquare;
    }

    inline CastlingRight GetCastlingRights() const
    {
        return castlingRights;
    }

    inline int GetHalfMoveClock() const
    {
        return halfMoveClock;
    }

    inline HashKey GetHashKey() const
    {
        return hashKey;
    }

private:
    // [ Private methods ]
    static constexpr size_t PieceToBitboardIndex(Piece piece)
    {
        switch (piece)
        {
            case WHITE_PAWN:   return 0; case BLACK_PAWN:   return 6;
            case WHITE_KNIGHT: return 1; case BLACK_KNIGHT: return 7;
            case WHITE_BISHOP: return 2; case BLACK_BISHOP: return 8;
            case WHITE_ROOK:   return 3; case BLACK_ROOK:   return 9;
            case WHITE_QUEEN:  return 4; case BLACK_QUEEN:  return 10;
            case WHITE_KING:   return 5; case BLACK_KING:   return 11;
            default:
                assert(false && "Invalid piece");
                return SIZE_MAX;
        }
    }

    void Clear();

    void SetSquare(Square square, Piece piece);

    void ClearSquare(Square square);

    void MovePiece(Square fromSquare, Square toSquare);

    // Folds the state that the piece placement alone does not cover into the hash key.
    void UpdateHashKeyWithState();

    // [ Data members ]
    std::array<Piece, NUM_SQUARES> squares;
    std::array<Bitboard, NUM_PIECES> pieceBitboards;
    std::array<Bitboard, NUM_COLOURS> colourOccupancyBitboards;
    Bitboard allOccupancyBitboard;

    Colour activeColour;

    CastlingRight castlingRights;

    Square enPassantTargetSquare;

    int halfMoveClock;

    int fullMoveNumber;

    HashKey hashKey;

    // !EXPLAIN!
    std::vector<HashKey> hashKeyHistory;
};

inline std::ostream& operator<<(std::ostream& os, const Position& position)
{
    os << position.ToString();

    return os;
}

} // namespace Gluon