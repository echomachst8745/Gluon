#pragma once

#include "boardhelpers.h"
#include "piece.h"

#include <cstdint>

namespace Gluon::Zobrist {

constexpr std::uint64_t RNG_SEED = 0xF00DCAFEDEADBEEFULL;

constexpr int CASTLING_RIGHTS_HASH_SIZE = 16;
constexpr int EN_PASSANT_FILE_HASH_SIZE = 8;

enum PieceIndex
{
    WHITE_PAWN,
    WHITE_KNIGHT,
    WHITE_BISHOP,
    WHITE_ROOK,
    WHITE_QUEEN,
    WHITE_KING,
    BLACK_PAWN,
    BLACK_KNIGHT,
    BLACK_BISHOP,
    BLACK_ROOK,
    BLACK_QUEEN,
    BLACK_KING,
    NUM_PIECE_TYPES
};

struct ZobristHashTable
{
    std::uint64_t pieceSquareHash[NUM_PIECE_TYPES][BoardHelpers::NUM_SQUARES];
    std::uint64_t castlingRightsHash[CASTLING_RIGHTS_HASH_SIZE];
    std::uint64_t enPassantFileHash[EN_PASSANT_FILE_HASH_SIZE];
    std::uint64_t sideToMoveHash;

    ZobristHashTable();
};

int PieceTypeToIndex(Piece::Piece piece);

std::uint64_t Random_UInt64();

inline const ZobristHashTable GlobalZobristHashTable;

} // namespace Gluon::Zobrist