#pragma once

#include "board.h"
#include "move.h"
#include "transpositiontable.h"

namespace Gluon::Search {

MoveGenerator::Move FindBestMove(Board& board, int depth, TranspositionTable::TranspositionTable& transpositionTable);

} // namespace Gluon::Search