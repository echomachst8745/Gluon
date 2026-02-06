#pragma once

#include "move.h"
#include "board.h"

#include <vector>

namespace Gluon {

std::vector<Move> GeneratePseudoLegalMoves(const Board& board);

} // namespace Gluon