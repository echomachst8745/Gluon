#pragma once

#include "piece.h"
#include "move.h"

namespace Gluon {

struct UndoMoveState
{
    MoveGenerator::Move moveMade;
    int previousEnPassantSquare;
    std::uint8_t previousCastlingRights;
    int previousHalfmoveClock;
    int previousFullmoveNumber;
    Piece::Piece movedPiece;
    Piece::Piece capturedPiece;
    int capturedPieceSquare;

    UndoMoveState()
        : previousEnPassantSquare(BoardHelpers::NO_EN_PASSANT),
          previousCastlingRights(BoardHelpers::NO_CASTLING_RIGHTS),
          previousHalfmoveClock(-1),
          previousFullmoveNumber(-1),
          movedPiece(Piece::NONE),
          capturedPiece(Piece::NONE),
          capturedPieceSquare(-1) {}

    UndoMoveState(const MoveGenerator::Move& moveMade,
                  int previousEnPassantSquare,
                  std::uint8_t previousCastlingRights,
                  int previousHalfmoveClock,
                  int previousFullmoveNumber,
                  Piece::Piece movedPiece,
                  Piece::Piece capturedPiece,
                  int capturedPieceSquare)
        : moveMade(moveMade),
          previousEnPassantSquare(previousEnPassantSquare),
          previousCastlingRights(previousCastlingRights),
          previousHalfmoveClock(previousHalfmoveClock),
          previousFullmoveNumber(previousFullmoveNumber),
          movedPiece(movedPiece),
          capturedPiece(capturedPiece),
          capturedPieceSquare(capturedPieceSquare) {}
};

} // namespace Gluon