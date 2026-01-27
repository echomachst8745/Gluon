#pragma once

#include "boardhelpers.h"

#include <cstdint>
#include <array>

namespace Gluon {

struct LegalMoveInfo
{
    std::array<int, 2> checkerSquares = {-1, -1}; // Squares of opponent checking pieces
    
    std::uint64_t legalMoveBitboard = ~0ULL; // Bitboard of legal moves

    std::uint64_t pinnedPiecesBitboard = 0ULL; // Bitboard of pinned pieces
    std::array<std::uint64_t, BoardHelpers::NUM_SQUARES> pinnedPieceLegalMovesBitboards = {0}; // For each square, bitboard of legal moves for pinned piece on that square

    std::uint64_t attackedSquaresBitboard = 0ULL; // Bitboard of squares attacked by opponent

    void SetSquareAsAttacked(int square) noexcept;

    void AddChecker(int square);

    void SetPiecePinned(int square, std::uint64_t legalMovesBitboard) noexcept;

    bool KingInCheck() noexcept;

    bool KingInDoubleCheck() noexcept;
};

} // namespace Gluon