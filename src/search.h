#pragma once

#include "board.h"
#include "movegenerator.h"

#include <atomic>
#include <chrono>

namespace Gluon::Search {

extern std::atomic<bool> SearchStopped;
extern std::chrono::steady_clock::time_point SearchStartTime;
extern double SearchMaxTimeSeconds;
extern int SearchMaxDepth;

struct SearchResult
{
    MoveGenerator::Move bestMove;
    double evaluation;

    bool searchInterrupted = false;
};

bool SearchShouldStop();

SearchResult StartSearch(Board& board);

} // namespace Gluon::Search