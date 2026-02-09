#include "bitboardhelpers.h"

#include <intrin.h>

namespace Gluon {

int GetLSBIndex(std::uint64_t bitboard)
{
	unsigned long index;
	_BitScanForward64(&index, bitboard);
	return static_cast<int>(index);
}

int PopLSB(std::uint64_t& bitboard)
{
	int lsbIndex = GetLSBIndex(bitboard);
	bitboard &= (bitboard - 1); // Clear the least significant bit
	return lsbIndex;
}

} // namespace Gluon