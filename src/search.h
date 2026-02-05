#pragma once

#include "move.h"
#include "movelist.h"
#include "position.h"
#include "transposition.h"
#include "types.h"

#include <array>
#include <atomic>
#include <chrono>
#include <cstdint>

namespace Gluon {

// [ Search bounds ]
constexpr int MAX_SEARCH_DEPTH = 64;

constexpr int64_t NO_TIME_LIMIT = -1;

// Limits on a single search, as given by the UCI "go" command.
struct SearchLimits
{
    int depth = MAX_SEARCH_DEPTH;

    int64_t moveTimeMilliseconds = NO_TIME_LIMIT;

    std::array<int64_t, NUM_COLOURS> remainingTimeMilliseconds = { NO_TIME_LIMIT, NO_TIME_LIMIT };

    std::array<int64_t, NUM_COLOURS> incrementMilliseconds = { 0, 0 };

    int movesToGo = 0;

    bool infinite = false;
};

// Best move found so far, reported after each completed iteration and returned once the search ends.
struct SearchResult
{
    Move bestMove;

    int score = 0;

    int depth = 0;

    uint64_t nodes = 0;

    int64_t timeMilliseconds = 0;

    int hashFull = 0;
};

using SearchReportCallback = void (*)(const SearchResult& result);

class Searcher
{
public:
    // [ Constructors ]
    Searcher();

    // [ Public methods ]
    SearchResult Run(Position& position, const SearchLimits& limits,
                     SearchReportCallback report = nullptr);

    // Asks a running search to finish as soon as it can. May be called from another thread.
    void RequestStop();

    void ResizeTranspositionTable(size_t megabytes);

    void ClearTranspositionTable();

private:
    // [ Private methods ]
    static MoveList OrderMoves(const MoveList& moves, Move hashMove);

    int Negamax(Position& position, int depth, int ply, int alpha, int beta);

    int Quiescence(Position& position, int ply, int alpha, int beta);

    int64_t GetElapsedMilliseconds() const;

    bool ShouldStopSearch();

    // [ Data members ]
    std::atomic<bool> stopRequested;

    bool stopped;

    uint64_t nodes;

    Move rootBestMove;

    SearchLimits searchLimits;

    int64_t timeBudgetMilliseconds;

    std::chrono::steady_clock::time_point startTime;

    TranspositionTable transpositionTable;
};

} // namespace Gluon
