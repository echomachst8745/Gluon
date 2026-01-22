#include "board.h"
#include "movegenerator.h"

#include <cstdint>
#include <string>
#include <iostream>

namespace Gluon::Debug {

std::uint64_t Perft(int depth, Board& board)
{
    auto moves = MoveGenerator::GenerateMoves(board);
    std::uint64_t nodes = 0;

    if (depth == 1)
    {
        return static_cast<std::uint64_t>(moves.size());
    }

    for (const auto& move : moves)
    {
        board.MakeMove(move);
        nodes += Perft(depth - 1, board);
        board.UnmakeLastMove();
    }

    return nodes;
}

std::uint64_t PerftDivide(int depth, Board& board)
{
    auto moves = MoveGenerator::GenerateMoves(board);
    std::uint64_t totalNodes = 0;

    for (const auto& move : moves)
    {
        board.MakeMove(move);
        std::uint64_t nodes = (depth == 1) ? 1 : Perft(depth - 1, board);
        board.UnmakeLastMove();

        std::cout << move.ToUCIString() << ": " << nodes << std::endl;
        totalNodes += nodes;
    }

    std::cout << std::endl << "Nodes searched: " << totalNodes << std::endl;

    return totalNodes;
}

std::uint64_t RunPerftOnPosition(const std::string& fen, int depth, bool verbose)
{
    Board board(fen);
    return verbose ? PerftDivide(depth, board) : Perft(depth, board);
}

} // namespace Gluon::Debug