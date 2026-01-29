#include "search.h"
#include "evaluate.h"
#include "board.h"
#include "movegenerator.h"

#include <limits>

namespace Gluon::Search {

SearchResult Search(Board& board, int depth)
{
    auto moves = MoveGenerator::GenerateMoves(board);
    double bestEvaluation = std::numeric_limits<double>::lowest();

    if (moves.empty())
    {
        return SearchResult{ MoveGenerator::Move{}, bestEvaluation };
    }

    MoveGenerator::Move bestMove = moves[0];

    if (depth == 0)
    {
        return SearchResult{ bestMove, Evaluate::Evaluate(board) };
    }

    for (const auto& move : moves)
    {
        board.MakeMove(move);
        double evaluation = -Search(board, depth - 1).evaluation;
        if (evaluation > bestEvaluation)
        {
            bestEvaluation = evaluation;
            bestMove = move;
        }
        board.UnmakeLastMove();
    }

    return SearchResult{ bestMove, bestEvaluation };
}

} // namespace Gluon::Search