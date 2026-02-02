#include "zobrist.h"
#include "boardhelpers.h"
#include "piece.h"

#include <cstdint>
#include <random>

namespace Gluon::Zobrist {

ZobristHashTable::ZobristHashTable()
{
    for (int pieceIndex = 0; pieceIndex < NUM_PIECE_TYPES; ++pieceIndex)
    {
        for (int square = 0; square < BoardHelpers::NUM_SQUARES; ++square)
        {
            pieceSquareHash[pieceIndex][square] = Random_UInt64();
        }
    }

    for (int i = 0; i < CASTLING_RIGHTS_HASH_SIZE; ++i)
    {
        castlingRightsHash[i] = Random_UInt64();
    }

    for (int i = 0; i < EN_PASSANT_FILE_HASH_SIZE; ++i)
    {
        enPassantFileHash[i] = Random_UInt64();
    }

    sideToMoveHash = Random_UInt64();
}

int PieceTypeToIndex(Piece::Piece piece)
{
    if (Piece::IsWhite(piece))
    {
        switch (Piece::GetType(piece))
        {
            case Piece::PAWN:   return WHITE_PAWN;
            case Piece::KNIGHT: return WHITE_KNIGHT;
            case Piece::BISHOP: return WHITE_BISHOP;
            case Piece::ROOK:   return WHITE_ROOK;
            case Piece::QUEEN:  return WHITE_QUEEN;
            case Piece::KING:   return WHITE_KING;
            default:            return -1; // Invalid piece
        }
    }
    else
    {
        switch (Piece::GetType(piece))
        {
            case Piece::PAWN:   return BLACK_PAWN;
            case Piece::KNIGHT: return BLACK_KNIGHT;
            case Piece::BISHOP: return BLACK_BISHOP;
            case Piece::ROOK:   return BLACK_ROOK;
            case Piece::QUEEN:  return BLACK_QUEEN;
            case Piece::KING:   return BLACK_KING;
            default:            return -1; // Invalid piece
        }
    }
}

std::uint64_t Random_UInt64()
{
    static std::mt19937_64 rng { RNG_SEED };
    static std::uniform_int_distribution<std::uint64_t> dist;

    return dist(rng);
}

} // namespace Gluon::Zobrist