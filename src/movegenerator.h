#pragma once

#include "move.h"
#include "board.h"
#include "movelist.h"

#include <vector>

namespace Gluon {

MoveList GeneratePseudoLegalMoves(const Board& board, PieceType pieceType = PieceType::NONE);

MoveList GenerateMoves(const Board& board);

} // namespace Gluon