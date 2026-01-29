#pragma once

#include "board.h"

#include <cstdint>
#include <string>

namespace Gluon::Debug {

std::uint64_t Perft(int depth, Board& board, bool bulkCount = false);

std::uint64_t RunPerft(Board& board, int depth, bool bulkCount = true, bool verbose = false);
std::uint64_t RunPerftOnPosition(const std::string& fen, int depth, bool bulkCount = false, bool verbose = false);

} // namespace Gluon::Debug