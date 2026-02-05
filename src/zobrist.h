#pragma once

#include "types.h"

#include <array>

namespace Gluon::Zobrist {

constexpr size_t NUM_CASTLING_RIGHT_COMBINATIONS = ALL_CASTLING_RIGHTS + 1U;

constexpr HashKey RANDOM_SEED = 0xC7E6A15D3B940F82ULL;

// A key for every piece of state that tells two positions apart.
struct ZobristKeys
{
    std::array<std::array<HashKey, NUM_SQUARES>, NUM_PIECES> pieceSquareKeys;

    std::array<HashKey, NUM_CASTLING_RIGHT_COMBINATIONS> castlingRightKeys;

    std::array<HashKey, NUM_FILES> enPassantFileKeys;

    HashKey sideToMoveKey;
};

// !EXPLAIN!
constexpr HashKey NextRandomKey(HashKey& seed)
{
    seed += 0x9E3779B97F4A7C15ULL;

    HashKey key = seed;

    key = (key ^ (key >> 30)) * 0xBF58476D1CE4E5B9ULL;
    key = (key ^ (key >> 27)) * 0x94D049BB133111EBULL;

    return key ^ (key >> 31);
}

constexpr ZobristKeys KEYS = []()
{
    HashKey seed = RANDOM_SEED;

    ZobristKeys keys{};

    for (size_t pieceIndex = 0; pieceIndex < NUM_PIECES; ++pieceIndex)
    {
        for (size_t square = 0; square < NUM_SQUARES; ++square)
        {
            keys.pieceSquareKeys[pieceIndex][square] = NextRandomKey(seed);
        }
    }

    for (size_t castlingRights = 0; castlingRights < NUM_CASTLING_RIGHT_COMBINATIONS; ++castlingRights)
    {
        keys.castlingRightKeys[castlingRights] = NextRandomKey(seed);
    }

    for (size_t file = 0; file < NUM_FILES; ++file)
    {
        keys.enPassantFileKeys[file] = NextRandomKey(seed);
    }

    keys.sideToMoveKey = NextRandomKey(seed);

    return keys;
}();

} // namespace Gluon::Zobrist
