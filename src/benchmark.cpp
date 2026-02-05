#include "benchmark.h"

#include "evaluation.h"
#include "movegenerator.h"
#include "movelist.h"
#include "search.h"

#include <array>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace Gluon {

// Positions and expected node counts from https://www.chessprogramming.org/Perft_Results
// A zero expected count means that depth is not published for that position.

static constexpr size_t MAX_BENCHMARK_DEPTH = 6;
static constexpr size_t NUM_BENCHMARK_POSITIONS = 7;

struct BenchmarkPosition
{
    std::string name;

    std::string fen;

    std::array<uint64_t, MAX_BENCHMARK_DEPTH> expectedNodes;
};

static const std::array<BenchmarkPosition, NUM_BENCHMARK_POSITIONS> BENCHMARK_POSITIONS = { {
    {
        "Initial Position",
        "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
        { 20, 400, 8902, 197281, 4865609, 119060324 }
    },
    {
        "Position 2 (Kiwipete)",
        "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1",
        { 48, 2039, 97862, 4085603, 193690690, 8031647685 }
    },
    {
        "Position 3",
        "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1",
        { 14, 191, 2812, 43238, 674624, 11030083 }
    },
    {
        "Position 4",
        "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1",
        { 6, 264, 9467, 422333, 15833292, 706045033 }
    },
    {
        "Position 4 (Mirrored)",
        "r2q1rk1/pP1p2pp/Q4n2/bbp1p3/Np6/1B3NBn/pPPP1PPP/R3K2R b KQ - 0 1",
        { 6, 264, 9467, 422333, 15833292, 706045033 }
    },
    {
        "Position 5",
        "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8",
        { 44, 1486, 62379, 2103487, 89941194, 0 }
    },
    {
        "Position 6",
        "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10",
        { 46, 2079, 89890, 3894594, 164075551, 6923051137 }
    }
} };

// [ Evaluation positions ]

static constexpr size_t NUM_EVALUATION_POSITIONS = 12;

struct EvaluationPosition
{
    std::string name;

    std::string fen;
};

static const std::array<EvaluationPosition, NUM_EVALUATION_POSITIONS> EVALUATION_POSITIONS = { {
    { "Start Position",       "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1" },
    { "Kiwipete",             "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1" },
    { "Open Middlegame",      "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10" },
    { "Closed Centre",        "r1bq1rk1/pp2nppp/2n1p3/2ppP3/3P4/P1P2N2/2P1NPPP/R1BQKB1R w KQ - 0 10" },
    { "Sicilian Middlegame",  "r1bqk2r/pp2bppp/2n1pn2/2pp4/3P1B2/2PBPN2/PP1N1PPP/R2QK2R w KQkq - 0 8" },
    { "Queenless Middlegame", "r3k2r/pp3ppp/2n1bn2/2ppp3/4P3/2PP1N2/PP1N1PPP/R3KB1R w KQkq - 0 10" },
    { "Material Imbalance",   "r4rk1/pp3ppp/2n5/8/8/2N5/PP3PPP/3QK2R w K - 0 20" },
    { "Blocked Pawns",        "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1" },
    { "Rook Endgame",         "8/5pk1/6p1/8/1R6/5PK1/6P1/2r5 w - - 0 40" },
    { "Bishop Pair Endgame",  "8/5pk1/8/8/2B5/5PK1/1B6/2n5 w - - 0 45" },
    { "Pawn Endgame",         "8/5p2/6k1/8/5PK1/8/8/8 w - - 0 50" },
    { "Passed Pawn Race",     "8/1P4k1/8/8/8/8/6p1/1K6 w - - 0 60" }
} };

static char SwapPieceCharColour(char pieceChar)
{
    if (pieceChar >= 'a' && pieceChar <= 'z')
    {
        return char(pieceChar - 'a' + 'A');
    }

    if (pieceChar >= 'A' && pieceChar <= 'Z')
    {
        return char(pieceChar - 'A' + 'a');
    }

    return pieceChar;
}

// !EXPLAIN!
static std::string MirrorFEN(const std::string& fen)
{
    std::istringstream fenStringStream(fen);
    std::string piecePlacementString, activeColourString, castlingRightsString,
                enPassantTargetSquareString, halfMoveClockString, fullMoveNumberString;
    fenStringStream >> piecePlacementString >> activeColourString >> castlingRightsString
                    >> enPassantTargetSquareString >> halfMoveClockString >> fullMoveNumberString;

    std::vector<std::string> rankStrings;
    std::istringstream piecePlacementStream(piecePlacementString);
    std::string rankString;

    while (std::getline(piecePlacementStream, rankString, '/'))
    {
        rankStrings.push_back(rankString);
    }

    std::string mirroredPiecePlacementString;

    for (size_t rankIndex = rankStrings.size(); rankIndex > 0; --rankIndex)
    {
        for (char rankChar : rankStrings[rankIndex - 1])
        {
            mirroredPiecePlacementString += SwapPieceCharColour(rankChar);
        }

        if (rankIndex > 1)
        {
            mirroredPiecePlacementString += '/';
        }
    }

    std::string mirroredCastlingRightsString;

    for (char castlingRightChar : std::string("KQkq"))
    {
        if (castlingRightsString.find(SwapPieceCharColour(castlingRightChar)) != std::string::npos)
        {
            mirroredCastlingRightsString += castlingRightChar;
        }
    }

    if (mirroredCastlingRightsString.empty())
    {
        mirroredCastlingRightsString = "-";
    }

    std::string mirroredEnPassantTargetSquareString = enPassantTargetSquareString;

    if (mirroredEnPassantTargetSquareString != "-")
    {
        mirroredEnPassantTargetSquareString[1] = char('1' + '8' - mirroredEnPassantTargetSquareString[1]);
    }

    return mirroredPiecePlacementString + ' ' +
           (activeColourString == "w" ? "b" : "w") + ' ' +
           mirroredCastlingRightsString + ' ' +
           mirroredEnPassantTargetSquareString + ' ' +
           halfMoveClockString + ' ' +
           fullMoveNumberString;
}

uint64_t Perft(Position& position, int depth)
{
    if (depth == 0)
    {
        return 1;
    }

    MoveList moves = GenerateLegalMoves(position);

    if (depth == 1)
    {
        return moves.Size();
    }

    uint64_t nodes = 0;

    for (size_t moveIndex = 0; moveIndex < moves.Size(); ++moveIndex)
    {
        PositionState state;

        position.MakeMove(moves[moveIndex], state);

        nodes += Perft(position, depth - 1);

        position.UnmakeMove(moves[moveIndex], state);
    }

    return nodes;
}

size_t RunBenchmark(int maxDepth)
{
    static const std::string rowSpacing = std::string(93, '-');

    uint64_t totalNodes = 0;
    double totalSeconds = 0.0;
    size_t failureCount = 0;

    std::cout << std::left  << std::setw(23) << "Position"
              << std::right << std::setw(6)  << "Depth"
                            << std::setw(16) << "Expected"
                            << std::setw(16) << "Nodes"
                            << std::setw(10) << "Time (s)"
                            << std::setw(14) << "Nodes/s"
                            << std::setw(8)  << "Result" << '\n'
              << rowSpacing << '\n';

    for (const BenchmarkPosition& benchmarkPosition : BENCHMARK_POSITIONS)
    {
        for (size_t depthIndex = 0; depthIndex < MAX_BENCHMARK_DEPTH; ++depthIndex)
        {
            const int depth = int(depthIndex) + 1;
            const uint64_t expectedNodes = benchmarkPosition.expectedNodes[depthIndex];

            if (depth > maxDepth || expectedNodes == 0)
            {
                continue;
            }

            Position position;
            position.SetupWithFEN(benchmarkPosition.fen);

            const auto startTime = std::chrono::steady_clock::now();
            const uint64_t nodes = Perft(position, depth);
            const double seconds = std::chrono::duration<double>(
                                       std::chrono::steady_clock::now() - startTime).count();

            const bool passed = nodes == expectedNodes;

            totalNodes += nodes;
            totalSeconds += seconds;
            failureCount += passed ? 0 : 1;

            std::cout << std::left  << std::setw(23) << benchmarkPosition.name
                      << std::right << std::setw(6)  << depth
                                    << std::setw(16) << expectedNodes
                                    << std::setw(16) << nodes
                                    << std::setw(10) << std::fixed << std::setprecision(3) << seconds
                                    << std::setw(14) << std::setprecision(0)
                                    << (seconds > 0.0 ? double(nodes) / seconds : 0.0)
                                    << std::setw(8)  << (passed ? "PASS" : "FAIL") << '\n';
        }
    }

    std::cout << rowSpacing << '\n'
              << "Total: " << totalNodes << " nodes in "
              << std::fixed << std::setprecision(3) << totalSeconds << "s ("
              << std::setprecision(0)
              << (totalSeconds > 0.0 ? double(totalNodes) / totalSeconds : 0.0) << " nodes/s), "
              << failureCount << " failure(s)\n";

    return failureCount;
}

size_t RunEvaluationTest()
{
    static const std::string rowSpacing = std::string(62, '-');

    size_t failureCount = 0;

    std::cout << std::left  << std::setw(24) << "Position"
              << std::right << std::setw(14) << "Evaluation"
                            << std::setw(14) << "Mirrored"
                            << std::setw(10) << "Result" << '\n'
              << rowSpacing << '\n';

    for (const EvaluationPosition& evaluationPosition : EVALUATION_POSITIONS)
    {
        Position position;
        position.SetupWithFEN(evaluationPosition.fen);

        Position mirroredPosition;
        mirroredPosition.SetupWithFEN(MirrorFEN(evaluationPosition.fen));

        const int score = Evaluate(position);
        const int mirroredScore = Evaluate(mirroredPosition);

        // !EXPLAIN!
        const bool passed = score == mirroredScore;

        failureCount += passed ? 0 : 1;

        std::cout << std::left  << std::setw(24) << evaluationPosition.name
                  << std::right << std::setw(14) << score
                                << std::setw(14) << mirroredScore
                                << std::setw(10) << (passed ? "PASS" : "FAIL") << '\n';
    }

    std::cout << rowSpacing << '\n'
              << "Total: " << failureCount << " failure(s)\n";

    return failureCount;
}

void RunSearchBenchmark(int depth)
{
    static const std::string rowSpacing = std::string(80, '-');

    uint64_t totalNodes = 0;
    double totalSeconds = 0.0;

    SearchLimits limits;
    limits.depth = depth;

    std::cout << std::left  << std::setw(24) << "Position"
              << std::right << std::setw(6)  << "Depth"
                            << std::setw(10) << "Score"
                            << std::setw(16) << "Nodes"
                            << std::setw(10) << "Time (s)"
                            << std::setw(14) << "Nodes/s" << '\n'
              << rowSpacing << '\n';

    for (const EvaluationPosition& evaluationPosition : EVALUATION_POSITIONS)
    {
        Position position;
        position.SetupWithFEN(evaluationPosition.fen);

        Searcher searcher;

        const auto startTime = std::chrono::steady_clock::now();
        const SearchResult result = searcher.Run(position, limits);
        const double seconds = std::chrono::duration<double>(
                                   std::chrono::steady_clock::now() - startTime).count();

        totalNodes += result.nodes;
        totalSeconds += seconds;

        std::cout << std::left  << std::setw(24) << evaluationPosition.name
                  << std::right << std::setw(6)  << result.depth
                                << std::setw(10) << result.score
                                << std::setw(16) << result.nodes
                                << std::setw(10) << std::fixed << std::setprecision(3) << seconds
                                << std::setw(14) << std::setprecision(0)
                                << (seconds > 0.0 ? double(result.nodes) / seconds : 0.0) << '\n';
    }

    std::cout << rowSpacing << '\n'
              << "Total: " << totalNodes << " nodes in "
              << std::fixed << std::setprecision(3) << totalSeconds << "s ("
              << std::setprecision(0)
              << (totalSeconds > 0.0 ? double(totalNodes) / totalSeconds : 0.0) << " nodes/s)\n";
}

} // namespace Gluon
