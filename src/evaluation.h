#pragma once

#include "position.h"

namespace Gluon {

// [ Scores ]
constexpr int DRAW_SCORE = 0;
constexpr int MATE_SCORE = 32000;
constexpr int INFINITE_SCORE = 32001;

// Furthest a mate score can be pushed away from MATE_SCORE by the distance to the mate.
constexpr int MAX_MATE_PLIES = 256;

// [ Game phase ]
enum Phase : uint8_t
{
    MIDGAME,
    ENDGAME,

    NUM_PHASES
};

constexpr int MAX_PHASE = 24;

// [ Piece indexing ]
constexpr size_t PieceTypeToIndex(PieceType pieceType)
{
    switch (pieceType)
    {
        case PAWN:   return 0;
        case KNIGHT: return 1;
        case BISHOP: return 2;
        case ROOK:   return 3;
        case QUEEN:  return 4;
        case KING:   return 5;
        default:     return 0;
    }
}

constexpr bool IsMateScore(int score)
{
    return score >= MATE_SCORE - MAX_MATE_PLIES || score <= -MATE_SCORE + MAX_MATE_PLIES;
}

// !EXPLAIN!
constexpr int MateScoreToMoves(int score)
{
    const int plies = MATE_SCORE - (score < 0 ? -score : score);
    const int moves = (plies + 1) / 2;

    return score < 0 ? -moves : moves;
}

// Scores the position from the point of view of the side to move.
int Evaluate(const Position& position);

} // namespace Gluon
