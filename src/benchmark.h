#pragma once

#include "position.h"

#include <cstdint>

namespace Gluon {

constexpr int DEFAULT_BENCHMARK_DEPTH = 4;

constexpr int DEFAULT_SEARCH_BENCHMARK_DEPTH = 5;

uint64_t Perft(Position& position, int depth);

// Runs every perft position up to maxDepth and returns the number of failures.
size_t RunBenchmark(int maxDepth = DEFAULT_BENCHMARK_DEPTH);

// Scores every evaluation position and returns the number whose score does not survive a mirror.
size_t RunEvaluationTest();

// Searches every evaluation position to a fixed depth and reports nodes and speed.
void RunSearchBenchmark(int depth = DEFAULT_SEARCH_BENCHMARK_DEPTH);

} // namespace Gluon
