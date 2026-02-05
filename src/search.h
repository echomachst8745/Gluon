#pragma once

#include "board.h"
#include "move.h"
#include "transpositiontable.h"

#include <atomic>
#include <chrono>

namespace Gluon::Search {

extern std::atomic<bool> searchShouldStop;
extern std::chrono::steady_clock::time_point searchStartTime;
extern double searchMaxTimeSeconds;

MoveGenerator::Move FindBestMove(Board& board, int maxDepth, TranspositionTable::TranspositionTable& transpositionTable);

} // namespace Gluon::Search