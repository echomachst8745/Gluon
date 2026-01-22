#pragma once

#include "board.h"

#include <cstdint>
#include <string>

namespace Gluon::Debug {

std::uint64_t Perft(int depth, Board& board);

std::uint64_t RunPerftOnPosition(const std::string& fen, int depth, bool verbose = false);

} // namespace Gluon::Debug