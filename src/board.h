#pragma once

#include "boardhelpers.h"
#include "piece.h"
#include "pieceplacements.h"
#include "undomovestate.h"
#include "zobrist.h"

#include <string>
#include <cstdint>
#include <array>
#include <vector>
#include <stack>

namespace Gluon {

class Board
{
public:
    Board();
    Board(const std::string& fen);

    bool IsWhiteToMove() const noexcept;

    int GetEnPassantSquare() const noexcept;

    bool HasCastlingRight(std::uint8_t castlingRight) const noexcept;

    const std::array<Piece::Piece, BoardHelpers::NUM_SQUARES>& GetSquares() const noexcept;

    void SetCurrentPlayerInCheck(bool inCheck);
    bool IsCurrentPlayerInCheck() const;

    int GetFullmoveNumber() const noexcept;

    std::uint64_t GetZobristHash() const noexcept;

    const std::vector<int>& GetPawnPlacements(bool forWhite) const noexcept;
    const std::vector<int>& GetKnightPlacements(bool forWhite) const noexcept;
    const std::vector<int>& GetBishopPlacements(bool forWhite) const noexcept;
    const std::vector<int>& GetRookPlacements(bool forWhite) const noexcept;
    const std::vector<int>& GetQueenPlacements(bool forWhite) const noexcept;
    int GetKingSquare(bool forWhite) const;
    
    Piece::Piece GetPieceAtSquare(int square) const;
    Piece::Piece GetPieceAtFileRank(int file, int rank) const;

    void Clear();
    
    void SetupWithFEN(const std::string& fen);
    
    const std::string GetBoardString(bool whitePOV = true) const;

    void MakeMove(const MoveGenerator::Move& move);

    void UnmakeLastMove();

    std::uint64_t ComputeZobristHash();

private:
    std::array<Piece::Piece, BoardHelpers::NUM_SQUARES> squares;
    PiecePlacements piecePlacements;

    bool isWhiteToMove;
    
    std::uint8_t castlingRights;

    int enPassantSquare;

    bool currentPlayerInCheck;

    int halfmoveClock;
    int fullmoveNumber;

    std::uint64_t zobristHash;

    std::stack<UndoMoveState> undoMoveStateStack;
};

} // namespace Gluon