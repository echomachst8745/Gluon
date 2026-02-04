#pragma once

#include "move.h"

#include <cstdint>
#include <vector>

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
    int depth;
    EntryType entryType;
    MoveGenerator::Move bestMove;

    bool isValid = false;

    bool IsValidEntry() const noexcept;
};

class TranspositionTable
{
public:
    TranspositionTable(int maxSizeMB);
    ~TranspositionTable() = default;

    void StoreEntry(std::uint64_t zobristHash, double evaluation, int depth, EntryType entryType, const MoveGenerator::Move& bestMove);
    TableEntry& RetrieveEntry(std::uint64_t zobristHash);

    void Resize(int maxSizeMB);

    size_t GetMaxTableSizeMB() const noexcept;
    size_t GetMaxTableSizeEntries() const noexcept;

    size_t SizeMB() const noexcept;
    size_t Size() const noexcept;

    void Clear();

private:
    size_t maxTableSizeMB;
    size_t maxTableSizeEntries;
    size_t currentSizeEntries;
    std::vector<TableEntry> table;
};

} // namespace TranspositionTable

} // namespace Gluon::Search