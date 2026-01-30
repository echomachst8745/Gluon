#include "search.h"
#include "evaluate.h"
#include "board.h"
#include "movegenerator.h"

#include <limits>
#include <cstdlib>
#include <vector>

namespace Gluon::Search {

SearchResult Search(Board& board, int depth)
{
    auto moves = MoveGenerator::GenerateMoves(board);

    if (moves.empty())
    {
        return SearchResult{ MoveGenerator::Move{}, std::numeric_limits<double>::lowest() };
    }
    
    if (depth == 0)
    {
        return SearchResult{ moves[rand() % moves.size()], Evaluate::Evaluate(board) };
    }

    std::vector<MoveGenerator::Move> bestMoves;
    bestMoves.reserve(moves.size());
    bestMoves.clear();
    double bestEvaluation = std::numeric_limits<double>::lowest();
    for (const auto& move : moves)
    {
        board.MakeMove(move);
        double evaluation = -Search(board, depth - 1).evaluation;
        board.UnmakeLastMove();
        if (evaluation > bestEvaluation)
        {
            bestEvaluation = evaluation;
            bestMoves.clear();
            bestMoves.push_back(move);
        }
        else if (evaluation == bestEvaluation)
        {
            bestMoves.push_back(move);
        }
    }

    return SearchResult{ bestMoves[rand() % bestMoves.size()], bestEvaluation };
}

} // namespace Gluon::Search