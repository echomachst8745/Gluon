#pragma once

#include "board.h"
#include "move.h"

namespace Gluon::Search {

MoveGenerator::Move FindBestMove(Board& board, int depth);

} // namespace Gluon::Search