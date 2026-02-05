#include "transposition.h"

#include "evaluation.h"

#include <algorithm>

namespace Gluon {

// !EXPLAIN!
static int ScoreToStoredScore(int score, int ply)
{
    if (score >= MATE_SCORE - MAX_MATE_PLIES)
    {
        return score + ply;
    }

    if (score <= -MATE_SCORE + MAX_MATE_PLIES)
    {
        return score - ply;
    }

    return score;
}

static int StoredScoreToScore(int storedScore, int ply)
{
    if (storedScore >= MATE_SCORE - MAX_MATE_PLIES)
    {
        return storedScore - ply;
    }

    if (storedScore <= -MATE_SCORE + MAX_MATE_PLIES)
    {
        return storedScore + ply;
    }

    return storedScore;
}

// [ Constructors ]
TranspositionTable::TranspositionTable()
{
    Resize(DEFAULT_HASH_SIZE_MEGABYTES);
}

// [ Public methods ]
void TranspositionTable::Resize(size_t megabytes)
{
    const size_t clampedMegabytes = std::clamp(megabytes, MIN_HASH_SIZE_MEGABYTES,
                                               MAX_HASH_SIZE_MEGABYTES);
    const size_t requestedEntryCount = clampedMegabytes * BYTES_PER_MEGABYTE /
                                       sizeof(TranspositionEntry);

    // !EXPLAIN!
    size_t entryCount = 1U;
    while (entryCount * 2U <= requestedEntryCount)
    {
        entryCount *= 2U;
    }

    entries.assign(entryCount, TranspositionEntry());
}

void TranspositionTable::Clear()
{
    std::fill(entries.begin(), entries.end(), TranspositionEntry());
}

bool TranspositionTable::Probe(HashKey key, int depth, int ply, int alpha, int beta, int& score,
                               Move& hashMove) const
{
    const TranspositionEntry& entry = entries[GetIndex(key)];

    if (entry.boundType == NO_BOUND || entry.key != key)
    {
        return false;
    }

    hashMove = entry.move;

    if (int(entry.depth) < depth)
    {
        return false;
    }

    const int storedScore = StoredScoreToScore(int(entry.score), ply);

    // !EXPLAIN!
    if (entry.boundType == EXACT_BOUND ||
        (entry.boundType == LOWER_BOUND && storedScore >= beta) ||
        (entry.boundType == UPPER_BOUND && storedScore <= alpha))
    {
        score = storedScore;

        return true;
    }

    return false;
}

void TranspositionTable::Store(HashKey key, int depth, int ply, int score, BoundType boundType,
                               Move move)
{
    TranspositionEntry& entry = entries[GetIndex(key)];

    // Keep the deeper search of a position that is already stored.
    if (entry.key == key && int(entry.depth) > depth && boundType != EXACT_BOUND)
    {
        return;
    }

    entry.key = key;
    entry.move = move;
    entry.score = int16_t(ScoreToStoredScore(score, ply));
    entry.depth = uint8_t(depth);
    entry.boundType = boundType;
}

int TranspositionTable::GetHashFull() const
{
    const size_t sampleSize = std::min(HASH_FULL_SAMPLE_SIZE, entries.size());

    size_t usedEntryCount = 0;

    for (size_t entryIndex = 0; entryIndex < sampleSize; ++entryIndex)
    {
        if (entries[entryIndex].boundType != NO_BOUND)
        {
            ++usedEntryCount;
        }
    }

    return sampleSize > 0 ? int(usedEntryCount * HASH_FULL_SAMPLE_SIZE / sampleSize) : 0;
}

} // namespace Gluon
