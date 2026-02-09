#pragma once

#include "board.h"

#include <cstdint>

namespace Gluon::Benchmark {

std::uint64_t PerftTest(Board& board, int depth);

void Perft(Board& board, int depth);

} // namespace Gluon::Benchmark
