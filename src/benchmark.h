#pragma once

#include "board.h"

#include <cstdint>

namespace Gluon::Benchmark {

std::uint64_t PerftTest(Board& board, int depth);

std::uint64_t Perft(Board& board, int depth);

void RunPerftTestPositions();

} // namespace Gluon::Benchmark
