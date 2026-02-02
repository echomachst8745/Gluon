#include "transpositiontable.h"
#include "move.h"

#include <cstdint>

namespace Gluon::Search::TranspositionTable {

bool TableEntry::IsValidEntry() const noexcept
{
    return depth != -1;
}

TranspositionTable::TranspositionTable()
{
    tableSize = 0;
}

void TranspositionTable::StoreEntry(std::uint64_t zobristHash, double evaluation, int depth, EntryType entryType, const MoveGenerator::Move& bestMove)
{
    auto& entry = table[zobristHash % TableSizeEntries];

    if (entry.zobristHash != zobristHash || depth >= entry.depth)
    {
        entry.zobristHash = zobristHash;
        entry.evaluation  = evaluation;
        entry.depth       = depth;
        entry.entryType   = entryType;
        entry.bestMove    = bestMove;

        tableSize++;
    }
}

TableEntry& TranspositionTable::RetrieveEntry(std::uint64_t zobristHash)
{
    return table[zobristHash % TableSizeEntries];
}

void TranspositionTable::Clear()
{
    table.fill(TableEntry{});
    tableSize = 0;
}

} // namespace Gluon::Search::TranspositionTable