#pragma once

#include "board.h"
#include "move.h"

#include <vector>
#include <array>

namespace Gluon::MoveGenerator {

constexpr std::array<int, 8> DIRECTION_OFFSETS = { 8, -8, 1, -1, 7, -7, 9, -9 }; // N, S, E, W, NW, SE, NE, SW
constexpr int NORTH_DIRECTION                  = DIRECTION_OFFSETS[0];
constexpr int SOUTH_DIRECTION                  = DIRECTION_OFFSETS[1];
constexpr int EAST_DIRECTION                   = DIRECTION_OFFSETS[2];
constexpr int WEST_DIRECTION                   = DIRECTION_OFFSETS[3];
constexpr int NORTH_WEST_DIRECTION             = DIRECTION_OFFSETS[4];
constexpr int SOUTH_EAST_DIRECTION             = DIRECTION_OFFSETS[5];
constexpr int NORTH_EAST_DIRECTION             = DIRECTION_OFFSETS[6];
constexpr int SOUTH_WEST_DIRECTION             = DIRECTION_OFFSETS[7];

MoveList GeneratePseudoLegalMoves(const Board& board);

MoveList GenerateMoves(Board& board, bool onlyCaptures = false);

} // namespace Gluon::MoveGenerator