#pragma once

#include "board.h"
#include "movegenerator.h"

namespace Gluon::Search {

struct SearchResult
{
    MoveGenerator::Move bestMove;
    double evaluation;
};

SearchResult Search(Board& board, int depth);

} // namespace Gluon::Search