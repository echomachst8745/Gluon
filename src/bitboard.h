#pragma once

#include "types.h"

#include <bit>

namespace Gluon::BB {

constexpr void SetSquare(Bitboard& bitboard, Square square)
{
    bitboard |= (1ULL << square);
}

constexpr Bitboard Shift(Bitboard bitboard, Direction direction)
{
    return direction > 0 ? (bitboard << int(direction)) : (bitboard >> -int(direction));
}

constexpr int GetLSB(Bitboard bitboard)
{
    return std::countr_zero(bitboard);
}

constexpr int GetMSB(Bitboard bitboard)
{
    return (NUM_SQUARES - 1) - std::countl_zero(bitboard);
}

// Gives the index of the least significant bit and clears it from the bitboard.
constexpr int PopLSB(Bitboard& bitboard)
{
    int index = std::countr_zero(bitboard);
    bitboard &= bitboard - 1ULL;

    return index;
}

constexpr int CountBits(Bitboard bitboard)
{
    return std::popcount(bitboard);
}

} // namespace Gluon::BB