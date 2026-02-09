#include "benchmark.h"

#include "board.h"
#include "movegenerator.h"

#include <cstdint>
#include <iostream>

namespace Gluon::Benchmark {

std::uint64_t Gluon::Benchmark::PerftTest(Board& board, int depth)
{
	if (depth == 0)
	{
		return 1ULL;
	}

	auto moves = GenerateMoves(board);

	if (depth == 1)
	{
		return static_cast<std::uint64_t>(moves.size());
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

void Gluon::Benchmark::Perft(Board& board, int depth)
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
}

} // namespace Gluon::Benchmark

