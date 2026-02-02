#pragma once

#include "move.h"

#include <cstdint>
#include <array>

namespace Gluon::Search {

namespace TranspositionTable {

enum EntryType : std::uint8_t
{
    EXACT,
    LOWERBOUND,
    UPPERBOUND
};

struct TableEntry
{
    std::uint64_t zobristHash;
    double evaluation;
    int depth = -1;
    EntryType entryType;
    MoveGenerator::Move bestMove;

    bool IsValidEntry() const noexcept;
};

constexpr int TableSizeMB = 1024; // Default size 1 GB
constexpr int TableSizeEntries = (TableSizeMB * 1024 * 1024) / sizeof(struct TableEntry);

class TranspositionTable
{
public:
    TranspositionTable();
    ~TranspositionTable() = default;

    void StoreEntry(std::uint64_t zobristHash, double evaluation, int depth, EntryType entryType, const MoveGenerator::Move& bestMove);
    TableEntry& RetrieveEntry(std::uint64_t zobristHash);

    void Clear();

private:
    int tableSize;
    std::array<TableEntry, TableSizeEntries> table;
};

} // namespace TranspositionTable

} // namespace Gluon::Search