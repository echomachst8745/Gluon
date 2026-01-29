#pragma once

#include "board.h"

namespace Gluon::Evaluate {

constexpr double PAWN_VALUE = 1.0;
constexpr double KNIGHT_VALUE = 3.0;
constexpr double BISHOP_VALUE = 3.0;
constexpr double ROOK_VALUE = 5.0;
constexpr double QUEEN_VALUE = 9.0;

double Evaluate(const Board& board);

} // namespace Gluon::Evaluate