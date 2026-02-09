#pragma once

#include "move.h"

#include <array>

namespace Gluon {

struct MoveList
{
private:
	std::array<Move, 256> moves;
	size_t size = 0;

public:
	inline void AddMove(const Move& move) { moves[size++] = move; };

	inline Move* begin() { return moves.data(); }
	inline Move* end() { return moves.data() + size; }

	inline const Move* begin() const { return moves.data(); }
	inline const Move* end() const { return moves.data() + size; }

	inline size_t Size() const { return size; }
};

} // namespace Gluon