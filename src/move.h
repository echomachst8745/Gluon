#pragma once

#include <cstdint>
#include <string>
#include <array>

namespace Gluon::MoveGenerator::MoveValues {

constexpr std::uint16_t INVALID_MOVE             = 0xFFFF;
constexpr std::uint16_t QUIET_MOVE               = 0;
constexpr std::uint16_t DOUBLE_PAWN_PUSH         = 1;
constexpr std::uint16_t KING_CASTLE              = 2;
constexpr std::uint16_t QUEEN_CASTLE             = 3;
constexpr std::uint16_t CAPTURE                  = 4;
constexpr std::uint16_t EN_PASSANT               = 5;
constexpr std::uint16_t KNIGHT_PROMOTION         = 8;
constexpr std::uint16_t BISHOP_PROMOTION         = 9;
constexpr std::uint16_t ROOK_PROMOTION           = 10;
constexpr std::uint16_t QUEEN_PROMOTION          = 11;
constexpr std::uint16_t KNIGHT_PROMOTION_CAPTURE = 12;
constexpr std::uint16_t BISHOP_PROMOTION_CAPTURE = 13;
constexpr std::uint16_t ROOK_PROMOTION_CAPTURE   = 14;
constexpr std::uint16_t QUEEN_PROMOTION_CAPTURE  = 15;

constexpr std::uint16_t FLAG_MASK                = 0x000F;
constexpr std::uint16_t FROM_SQUARE_MASK         = 0xFC00;
constexpr std::uint16_t TO_SQUARE_MASK           = 0x03F0;

constexpr int FROM_SQUARE_SHIFT = 10;
constexpr int TO_SQUARE_SHIFT   = 4;

} // namespace Gluon::MoveGenerator::MoveValues

namespace Gluon::MoveGenerator {

constexpr int MAX_MOVES_PER_POSITION = 256;

struct Move
{
    std::uint16_t move;
    bool isCheckingMove = false;

    Move();

    Move(int fromSquare, int toSquare, std::uint16_t flags);

    bool IsValidMove() const noexcept;

    int GetFromSquare() const noexcept;

    int GetToSquare() const noexcept;

    bool HasFlag(std::uint16_t flag) const noexcept;

    bool IsCapture() const noexcept;

    bool IsPromotion() const noexcept;

    bool IsCheckingMove() const noexcept;

    const std::string ToUCIString() const noexcept;
};

struct MoveList
{
    std::array<Move, MAX_MOVES_PER_POSITION> moves;

    int moveCount = 0;

    void Clear();
    void AddMove(const Move& move);

    bool IsEmpty() const noexcept;
};

} // namespace Gluon::MoveGenerator