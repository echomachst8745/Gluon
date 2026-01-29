#include "debug.h"
#include "board.h"
#include "movegenerator.h"

#include "legalmoveinfo.h"

#include <cstdint>
#include <string>
#include <iostream>

namespace Gluon::Debug {

namespace { // Anonymous namespace for helper functions

std::uint64_t PerftDivide(int depth, Board& board, bool bulkCount)
{
    auto moves = MoveGenerator::GenerateMoves(board);
    std::uint64_t totalNodes = 0;

    for (const auto& move : moves)
    {
        board.MakeMove(move);
        std::uint64_t nodes = (depth == 1) ? 1 : (bulkCount ? PerftBulk(depth - 1, board) : Perft(depth - 1, board));
        board.UnmakeLastMove();

        std::cout << move.ToUCIString() << ": " << nodes << std::endl;
        totalNodes += nodes;
    }

    std::cout << std::endl << "Nodes searched: " << totalNodes << std::endl;

    return totalNodes;
}

} // Anonymous namespace for helper functions

std::uint64_t Perft(int depth, Board& board)
{
    if (depth == 0)
    {
        return 1ULL;
    }

    auto moves = MoveGenerator::GenerateMoves(board);
    std::uint64_t nodes = 0;

    for (const auto& move : moves)
    {
        board.MakeMove(move);
        nodes += Perft(depth - 1, board);
        board.UnmakeLastMove();
    }

    return nodes;
}

std::uint64_t PerftBulk(int depth, Board& board)
{
    auto moves = MoveGenerator::GenerateMoves(board);
    auto pseudoLegalMoves = MoveGenerator::GeneratePseudoLegalMoves(board);
    std::uint64_t nodes = 0;

    if (depth == 1)
    {
        return static_cast<std::uint64_t>(moves.size());
    }

    for (const auto& move : moves)
    {
        board.MakeMove(move);
        nodes += PerftBulk(depth - 1, board);
        board.UnmakeLastMove();
    }

    return nodes;
}

std::uint64_t RunPerftOnPosition(const std::string& fen, int depth, bool bulkCount, bool verbose)
{
    Board board(fen);
    return verbose ?  PerftDivide(depth, board, bulkCount) : (bulkCount ? PerftBulk(depth, board) : Perft(depth, board));
}

} // namespace Gluon::Debug