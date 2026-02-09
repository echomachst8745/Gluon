#pragma once

#include <cstdint>

namespace Gluon {

int GetLSBIndex(std::uint64_t bitboard);

int PopLSB(std::uint64_t& bitboard);

} // namespace Gluon