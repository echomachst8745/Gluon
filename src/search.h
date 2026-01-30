#pragma once

#include "board.h"
#include "movegenerator.h"

namespace Gluon::Search {

struct SearchResult
{
    MoveGenerator::Move bestMove;
    double evaluation;
};

SearchResult AlphaBetaMaxSearch(Board& board, int depth, double alpha, double beta);

} // namespace Gluon::Search