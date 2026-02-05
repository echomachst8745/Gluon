#pragma once

#include "move.h"
#include "types.h"

#include <cstdint>
#include <vector>

namespace Gluon {

// [ Table sizes ]
constexpr size_t BYTES_PER_MEGABYTE = 1024U * 1024U;

constexpr size_t DEFAULT_HASH_SIZE_MEGABYTES = 16U;
constexpr size_t MIN_HASH_SIZE_MEGABYTES = 1U;
constexpr size_t MAX_HASH_SIZE_MEGABYTES = 1024U;

// Entries sampled to estimate how full the table is, which is reported per mille.
constexpr size_t HASH_FULL_SAMPLE_SIZE = 1000U;

// What a stored score says about the true score of a position.
enum BoundType : uint8_t
{
    NO_BOUND,
    EXACT_BOUND,
    LOWER_BOUND,
    UPPER_BOUND,

    NUM_BOUND_TYPES
};

struct TranspositionEntry
{
    HashKey key = 0ULL;

    Move move;

    int16_t score = 0;

    uint8_t depth = 0U;

    BoundType boundType = NO_BOUND;
};

class TranspositionTable
{
public:
    // [ Constructors ]
    TranspositionTable();

    // [ Public methods ]
    void Resize(size_t megabytes);

    void Clear();

    // Gives the stored move for move ordering, and returns true when the stored score can be used
    // in place of searching the position again.
    bool Probe(HashKey key, int depth, int ply, int alpha, int beta, int& score, Move& hashMove) const;

    void Store(HashKey key, int depth, int ply, int score, BoundType boundType, Move move);

    int GetHashFull() const;

private:
    // [ Private methods ]
    // !EXPLAIN!
    inline size_t GetIndex(HashKey key) const
    {
        return size_t(key) & (entries.size() - 1U);
    }

    // [ Data members ]
    std::vector<TranspositionEntry> entries;
};

} // namespace Gluon
