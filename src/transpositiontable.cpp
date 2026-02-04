#include "transpositiontable.h"
#include "move.h"

#include <cstdint>

namespace Gluon::Search::TranspositionTable {

bool TableEntry::IsValidEntry() const noexcept
{
    return isValid;
}

TranspositionTable::TranspositionTable(int maxSizeMB)
    : maxTableSizeMB(maxSizeMB)
{
    maxTableSizeEntries = (maxTableSizeMB * 1024 * 1024) / sizeof(TableEntry);

    table.reserve(maxTableSizeEntries);
}

void TranspositionTable::StoreEntry(std::uint64_t zobristHash, double evaluation, int depth, EntryType entryType, const MoveGenerator::Move& bestMove)
{
    TableEntry entry;
    entry.zobristHash = zobristHash;
    entry.evaluation = evaluation;
    entry.depth = depth;
    entry.entryType = entryType;
    entry.bestMove = bestMove;
    entry.isValid = true;

    table[zobristHash] = entry;
}

TableEntry& TranspositionTable::RetrieveEntry(std::uint64_t zobristHash)
{
    return table[zobristHash];
}

void TranspositionTable::Resize(int maxSizeMB)
{
    maxTableSizeMB = maxSizeMB;
    maxTableSizeEntries = (maxTableSizeMB * 1024 * 1024) / sizeof(TableEntry);

    if (maxTableSizeEntries < table.size())
    {
        // If the new size is smaller than the current number of entries, we need to clear the table
        Clear();
    }

    table.reserve(maxTableSizeEntries);
}

size_t TranspositionTable::GetMaxTableSizeMB() const noexcept
{
    return maxTableSizeMB;
}

size_t TranspositionTable::GetMaxTableSizeEntries() const noexcept
{
    return maxTableSizeEntries;
}

size_t TranspositionTable::SizeMB() const noexcept
{
    return (table.size() * sizeof(TableEntry)) / (1024 * 1024);
}

size_t TranspositionTable::Size() const noexcept
{
    return table.size();
}

void TranspositionTable::Clear()
{
    table.clear();
}

} // namespace Gluon::Search::TranspositionTable