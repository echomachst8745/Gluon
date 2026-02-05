#pragma once

#include "move.h"

#include <array>

namespace Gluon {

class MoveList
{
public:
    MoveList()
        : size(0) {}

    // !EXPLAIN!
    inline void AddMove(const Move& move)
    {
        moves[size++] = move;
    }

    inline const Move& operator[](size_t index) const
    {
        return moves[index];
    }

    inline size_t Size() const
    {
        return size;
    }

private:
    static constexpr size_t MAX_MOVES = 256;

    std::array<Move, MAX_MOVES> moves;

    size_t size;
};

} // namespace Gluon