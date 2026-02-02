#pragma once

#include "board.h"

namespace Gluon::Evaluate {

constexpr double PAWN_VALUE = 100.0;
constexpr double KNIGHT_VALUE = 300.0;
constexpr double BISHOP_VALUE = 300.0;
constexpr double ROOK_VALUE = 500.0;
constexpr double QUEEN_VALUE = 900.0;

double Evaluate(const Board& board);

} // namespace Gluon::Evaluate