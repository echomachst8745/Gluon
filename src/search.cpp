#include "search.h"

#include "evaluation.h"
#include "movegenerator.h"
#include "movelist.h"

#include <algorithm>

namespace Gluon {

// [ Search tuning ]
static constexpr int MAX_QUIESCENCE_PLY = 32;

static constexpr int HALF_MOVE_CLOCK_DRAW_LIMIT = 100;

static constexpr uint64_t TIME_CHECK_NODE_INTERVAL = 2048;

static constexpr int64_t MOVE_OVERHEAD_MILLISECONDS = 50;

static constexpr int64_t DEFAULT_MOVES_TO_GO = 30;

// !EXPLAIN!
static int64_t CalculateTimeBudget(const SearchLimits& limits, Colour activeColour)
{
    if (limits.infinite)
    {
        return NO_TIME_LIMIT;
    }

    if (limits.moveTimeMilliseconds != NO_TIME_LIMIT)
    {
        return limits.moveTimeMilliseconds;
    }

    const int64_t remainingTime = limits.remainingTimeMilliseconds[activeColour];

    if (remainingTime == NO_TIME_LIMIT)
    {
        return NO_TIME_LIMIT;
    }

    const int64_t movesToGo = limits.movesToGo > 0 ? int64_t(limits.movesToGo) : DEFAULT_MOVES_TO_GO;
    const int64_t budget = remainingTime / movesToGo + limits.incrementMilliseconds[activeColour] / 2;

    return std::max(int64_t(1), std::min(budget, remainingTime - MOVE_OVERHEAD_MILLISECONDS));
}

// [ Constructors ]
Searcher::Searcher()
    : stopRequested(false),
      stopped(false),
      nodes(0),
      timeBudgetMilliseconds(NO_TIME_LIMIT),
      startTime(std::chrono::steady_clock::now()) {}

// [ Public methods ]
SearchResult Searcher::Run(Position& position, const SearchLimits& limits, SearchReportCallback report)
{
    searchLimits = limits;
    stopRequested = false;
    stopped = false;
    nodes = 0;
    rootBestMove = Move();
    startTime = std::chrono::steady_clock::now();
    timeBudgetMilliseconds = CalculateTimeBudget(limits, position.GetActiveColour());

    SearchResult result;

    const MoveList rootMoves = GenerateLegalMoves(position);

    if (rootMoves.Size() == 0)
    {
        return result;
    }

    // Always have something to play, even if the first iteration is cut short.
    result.bestMove = rootMoves[0];

    const int maxDepth = std::min(searchLimits.depth, MAX_SEARCH_DEPTH);

    for (int depth = 1; depth <= maxDepth; ++depth)
    {
        const int score = Negamax(position, depth, 0, -INFINITE_SCORE, INFINITE_SCORE);

        // !EXPLAIN!
        if (stopped)
        {
            break;
        }

        result.bestMove = rootBestMove;
        result.score = score;
        result.depth = depth;
        result.nodes = nodes;
        result.timeMilliseconds = GetElapsedMilliseconds();
        result.hashFull = transpositionTable.GetHashFull();

        if (report != nullptr)
        {
            report(result);
        }
    }

    result.nodes = nodes;
    result.timeMilliseconds = GetElapsedMilliseconds();

    return result;
}

void Searcher::RequestStop()
{
    stopRequested.store(true, std::memory_order_relaxed);
}

void Searcher::ResizeTranspositionTable(size_t megabytes)
{
    transpositionTable.Resize(megabytes);
}

void Searcher::ClearTranspositionTable()
{
    transpositionTable.Clear();
}

// [ Private methods ]
// The stored move first, then captures and promotions, as they are the most likely to cause a cut-off.
MoveList Searcher::OrderMoves(const MoveList& moves, Move hashMove)
{
    MoveList orderedMoves;

    // !EXPLAIN!
    for (size_t moveIndex = 0; moveIndex < moves.Size(); ++moveIndex)
    {
        if (moves[moveIndex] == hashMove)
        {
            orderedMoves.AddMove(moves[moveIndex]);
        }
    }

    for (size_t moveIndex = 0; moveIndex < moves.Size(); ++moveIndex)
    {
        if ((moves[moveIndex].IsCapture() || moves[moveIndex].IsPromotion()) &&
            !(moves[moveIndex] == hashMove))
        {
            orderedMoves.AddMove(moves[moveIndex]);
        }
    }

    for (size_t moveIndex = 0; moveIndex < moves.Size(); ++moveIndex)
    {
        if (!moves[moveIndex].IsCapture() && !moves[moveIndex].IsPromotion() &&
            !(moves[moveIndex] == hashMove))
        {
            orderedMoves.AddMove(moves[moveIndex]);
        }
    }

    return orderedMoves;
}

int Searcher::Negamax(Position& position, int depth, int ply, int alpha, int beta)
{
    if (ShouldStopSearch())
    {
        stopped = true;

        return DRAW_SCORE;
    }

    // !EXPLAIN!
    if (ply > 0 && (position.IsRepetition(ply) ||
                    (position.GetHalfMoveClock() >= HALF_MOVE_CLOCK_DRAW_LIMIT &&
                     (!position.IsInCheck() || GenerateLegalMoves(position).Size() > 0))))
    {
        return DRAW_SCORE;
    }

    if (depth <= 0)
    {
        return Quiescence(position, ply, alpha, beta);
    }

    ++nodes;

    const HashKey hashKey = position.GetHashKey();
    const int originalAlpha = alpha;

    Move hashMove;
    int storedScore = 0;

    // !EXPLAIN!
    if (ply > 0 && transpositionTable.Probe(hashKey, depth, ply, alpha, beta, storedScore, hashMove))
    {
        return storedScore;
    }

    const MoveList moves = OrderMoves(GenerateLegalMoves(position), hashMove);

    // !EXPLAIN!
    if (moves.Size() == 0)
    {
        return position.IsInCheck() ? -MATE_SCORE + ply : DRAW_SCORE;
    }

    int bestScore = -INFINITE_SCORE;
    Move bestMove;

    for (size_t moveIndex = 0; moveIndex < moves.Size(); ++moveIndex)
    {
        PositionState state;

        position.MakeMove(moves[moveIndex], state);

        const int score = -Negamax(position, depth - 1, ply + 1, -beta, -alpha);

        position.UnmakeMove(moves[moveIndex], state);

        if (stopped)
        {
            return bestScore;
        }

        if (score > bestScore)
        {
            bestScore = score;
            bestMove = moves[moveIndex];

            if (ply == 0)
            {
                rootBestMove = moves[moveIndex];
            }
        }

        // !EXPLAIN!
        if (score > alpha)
        {
            alpha = score;
        }

        if (alpha >= beta)
        {
            break;
        }
    }

    // !EXPLAIN!
    const BoundType boundType = bestScore >= beta         ? LOWER_BOUND
                              : bestScore > originalAlpha ? EXACT_BOUND
                                                          : UPPER_BOUND;

    // !EXPLAIN!
    if (bestScore != DRAW_SCORE)
    {
        transpositionTable.Store(hashKey, depth, ply, bestScore, boundType, bestMove);
    }

    return bestScore;
}

// !EXPLAIN!
int Searcher::Quiescence(Position& position, int ply, int alpha, int beta)
{
    if (ShouldStopSearch())
    {
        stopped = true;

        return DRAW_SCORE;
    }

    ++nodes;

    const int standPatScore = Evaluate(position);

    if (standPatScore >= beta || ply >= MAX_QUIESCENCE_PLY)
    {
        return standPatScore;
    }

    if (standPatScore > alpha)
    {
        alpha = standPatScore;
    }

    const MoveList moves = GenerateLegalMoves(position, true);

    int bestScore = standPatScore;

    for (size_t moveIndex = 0; moveIndex < moves.Size(); ++moveIndex)
    {
        PositionState state;

        position.MakeMove(moves[moveIndex], state);

        const int score = -Quiescence(position, ply + 1, -beta, -alpha);

        position.UnmakeMove(moves[moveIndex], state);

        if (stopped)
        {
            return bestScore;
        }

        if (score > bestScore)
        {
            bestScore = score;
        }

        if (score > alpha)
        {
            alpha = score;
        }

        if (alpha >= beta)
        {
            break;
        }
    }

    return bestScore;
}

int64_t Searcher::GetElapsedMilliseconds() const
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(
               std::chrono::steady_clock::now() - startTime).count();
}

bool Searcher::ShouldStopSearch()
{
    if (stopped || stopRequested.load(std::memory_order_relaxed))
    {
        return true;
    }

    // !EXPLAIN!
    return timeBudgetMilliseconds != NO_TIME_LIMIT &&
           (nodes % TIME_CHECK_NODE_INTERVAL) == 0 &&
           GetElapsedMilliseconds() >= timeBudgetMilliseconds;
}

} // namespace Gluon
