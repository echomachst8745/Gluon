#pragma once

#include "boardhelpers.h"
#include "move.h"
#include "board.h"

#include <cstdint>
#include <array>

namespace Gluon {

struct LegalMoveInfo
{
    std::array<int, 2> checkerSquares = {-1, -1}; // Squares of opponent checking pieces
    
    std::uint64_t removeBlockCheckBitboard = ~0ULL; // Bitboard of toSquares the friendly pieces can move to in order to block a check or capture the checker

    std::uint64_t pinnedPiecesBitboard = 0ULL; // Bitboard of pinned pieces
    std::array<std::uint64_t, BoardHelpers::NUM_SQUARES> pinnedPieceLegalMovesBitboards = {0}; // For each square, bitboard of legal moves for pinned piece on that square

    std::uint64_t attackedSquaresBitboard = 0ULL; // Bitboard of squares attacked by opponent

    void SetSquareAsAttacked(int square) noexcept;

    void AddChecker(int square);

    void AddSlidingChecker(int square, std::uint64_t blockCheckBitboard) noexcept;

    void SetPiecePinned(int square, std::uint64_t legalMovesBitboard) noexcept;

    bool KingInCheck() const noexcept;

    bool KingInDoubleCheck() const noexcept;

    bool MoveIsLegal(const Board& board, const MoveGenerator::Move& move) const noexcept;
};

} // namespace Gluon