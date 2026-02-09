#include "benchmark.h"

#include "board.h"
#include "movegenerator.h"

#include <cstdint>
#include <iostream>

namespace Gluon::Benchmark {

std::uint64_t PerftTest(Board& board, int depth)
{
	if (depth == 0)
	{
		return 1ULL;
	}

	auto moves = GenerateMoves(board);

	if (depth == 1)
	{
		return static_cast<std::uint64_t>(moves.Size());
	}

	std::uint64_t nodes = 0;
	for (const Move& move : moves)
	{
		board.MakeMove(move);
		nodes += PerftTest(board, depth - 1);
		board.UndoLastMove();
	}

	return nodes;
}

std::uint64_t Perft(Board& board, int depth)
{
	auto moves = GenerateMoves(board);
	std::uint64_t totalNodes = 0;

	for (const Move& move : moves)
	{
		board.MakeMove(move);
		std::uint64_t nodes = PerftTest(board, depth - 1);
		board.UndoLastMove();

		totalNodes += nodes;
		std::cout << move.ToUCIString() << ": " << nodes << std::endl;
	}

	std::cout << std::endl << "Nodes searched: " << totalNodes << std::endl;

	return totalNodes;
}

void RunPerftTestPositions()
{
	static const std::string testFens[] = { "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1",
											"8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1",
											"r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1",
											"rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8",
											"r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10" };
	static const int totalTestsNum = 5;

	static const int testFenDepths[] = { 6, 8, 6, 5, 9 };

	static const std::uint64_t testPerftExpectedResults[] = { 8031647685ULL, 3009794393ULL, 706045033ULL, 89941194ULL, 490154852788714ULL };

	for (int i = 0; i < totalTestsNum; ++i)
	{
		std::string fen = testFens[i];
		int depth = testFenDepths[i];

		Board board(fen);
		//std::uint64_t result = PerftTest(board, depth);
		std::uint64_t result = Perft(board, depth);
		bool passed = result == testPerftExpectedResults[i];

		std::cout << "Test #" << (i + 1) << " -- Fen: \"" << testFens[i] << "\" - Depth: " << testFenDepths[i] << " - Expected: " << testPerftExpectedResults[i] << " - Result: " << result << " - [" << (passed ? "PASS" : "FAIL") << "]" << std::endl;
	}
}

} // namespace Gluon::Benchmark

