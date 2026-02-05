#pragma once

#include "movelist.h"
#include "position.h"

namespace Gluon {

MoveList GeneratePseudoLegalMoves(const Position& position, bool capturesOnly = false);

MoveList GenerateLegalMoves(const Position& position, bool capturesOnly = false);

} // namespace Gluon