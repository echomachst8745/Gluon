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

    currentSizeEntries = 0;
    table.resize(maxTableSizeEntries);
}

void TranspositionTable::StoreEntry(std::uint64_t zobristHash, double evaluation, int depth, EntryType entryType, const MoveGenerator::Move& bestMove)
{
    std::uint64_t index = zobristHash % maxTableSizeEntries;

    auto& entry = table[index];

    if (!entry.isValid || entry.zobristHash != zobristHash)
    {
        if (!entry.isValid && currentSizeEntries < maxTableSizeEntries)
        {
            // Store it
            currentSizeEntries++;
        }
        else if (entry.isValid && entry.depth > depth)
        {
            // Existing entry is deeper, do not overwrite
            return;
        }
    }

    entry.zobristHash = zobristHash;
    entry.evaluation = evaluation;
    entry.depth = depth;
    entry.entryType = entryType;
    entry.bestMove = bestMove;
    entry.isValid = true;
}

TableEntry& TranspositionTable::RetrieveEntry(std::uint64_t zobristHash)
{
    std::uint64_t index = zobristHash % maxTableSizeEntries;

    return table[index];
}

void TranspositionTable::Resize(int maxSizeMB)
{
    maxTableSizeMB = maxSizeMB;
    maxTableSizeEntries = (maxTableSizeMB * 1024 * 1024) / sizeof(TableEntry);

    if (maxTableSizeEntries < table.size())
    {
        // If the new size is smaller than the current number of entries, we need to clear the table
        currentSizeEntries = 0;
        Clear();
    }

    table.resize(maxTableSizeEntries);
    currentSizeEntries = table.size();
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
    return (currentSizeEntries * sizeof(TableEntry)) / (1024 * 1024);
}

size_t TranspositionTable::Size() const noexcept
{
    return currentSizeEntries;
}

void TranspositionTable::Clear()
{
    table.clear();
    currentSizeEntries = 0;
}

} // namespace Gluon::Search::TranspositionTable