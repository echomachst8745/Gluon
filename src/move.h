#pragma once

#include "types.h"

#include <cstdint>

namespace Gluon {

class Move
{
public:
    // Flag to indicate the type of move.
    // !EXPLAIN!
    enum MoveFlag : uint16_t
    {
        QUIET_MOVE               = 0b0000U,
        DOUBLE_PAWN_PUSH         = 0b0001U,
        KING_CASTLE              = 0b0010U,
        QUEEN_CASTLE             = 0b0011U,
        CAPTURE                  = 0b0100U,
        EN_PASSANT_CAPTURE       = 0b0101U,
        KNIGHT_PROMOTION         = 0b1000U,
        BISHOP_PROMOTION         = 0b1001U,
        ROOK_PROMOTION           = 0b1010U,
        QUEEN_PROMOTION          = 0b1011U,
        KNIGHT_PROMOTION_CAPTURE = 0b1100U,
        BISHOP_PROMOTION_CAPTURE = 0b1101U,
        ROOK_PROMOTION_CAPTURE   = 0b1110U,
        QUEEN_PROMOTION_CAPTURE  = 0b1111U
    };

    // [ Constructors ]
    Move()
        : moveData(NULL_MOVE) {}

    Move(Square fromSquare, Square toSquare, MoveFlag moveFlag)
        : moveData(static_cast<uint16_t>(((fromSquare & TO_FROM_SQUARE_MASK) << FROM_SQUARE_SHIFT) |
                                         ((toSquare & TO_FROM_SQUARE_MASK) << TO_SQUARE_SHIFT)     |
                                         (moveFlag & FLAG_MASK))) {}

    // [ Public methods ]
    inline Square GetFromSquare() const
    {
        return Square((moveData >> FROM_SQUARE_SHIFT) & TO_FROM_SQUARE_MASK);
    }

    inline Square GetToSquare() const
    {
        return Square((moveData >> TO_SQUARE_SHIFT) & TO_FROM_SQUARE_MASK);
    }

    inline MoveFlag GetFlag() const
    {
        return MoveFlag(moveData & FLAG_MASK);
    }

    inline bool IsNull() const
    {
        return moveData == NULL_MOVE;
    }

    inline bool operator==(const Move& other) const
    {
        return moveData == other.moveData;
    }

    inline bool IsCapture() const
    {
        return moveData & CAPTURE_MASK;
    }

    inline bool IsPromotion() const
    {
        return moveData & PROMOTION_MASK;
    }

    inline PieceType GetPromotionPieceType() const
    {
        switch (GetFlag())
        {
            case KNIGHT_PROMOTION:
            case KNIGHT_PROMOTION_CAPTURE: return KNIGHT;
            case BISHOP_PROMOTION:
            case BISHOP_PROMOTION_CAPTURE: return BISHOP;
            case ROOK_PROMOTION:
            case ROOK_PROMOTION_CAPTURE:   return ROOK;
            case QUEEN_PROMOTION:
            case QUEEN_PROMOTION_CAPTURE:  return QUEEN;
            default:                       return NO_PIECE_TYPE;
        }
    }

    inline std::string ToString() const
    {
        std::string moveString = SquareToCoordString(GetFromSquare()) + SquareToCoordString(GetToSquare());

        if (IsPromotion())
        {
            switch (GetFlag())
            {
                case KNIGHT_PROMOTION:
                case KNIGHT_PROMOTION_CAPTURE: moveString += 'n'; break;
                case BISHOP_PROMOTION:
                case BISHOP_PROMOTION_CAPTURE: moveString += 'b'; break;
                case ROOK_PROMOTION:
                case ROOK_PROMOTION_CAPTURE:   moveString += 'r'; break;
                case QUEEN_PROMOTION:
                case QUEEN_PROMOTION_CAPTURE:  moveString += 'q'; break;
                default: break;
            }
        }

        return moveString;
    }

private:
    // Masks for extracting data from the moveData member.
    // [ Data Masks ]
    static constexpr uint16_t TO_FROM_SQUARE_MASK = 0x3FU;

    static constexpr uint16_t FROM_SQUARE_SHIFT = 4U;
    static constexpr uint16_t TO_SQUARE_SHIFT   = 10U;

    static constexpr uint16_t FLAG_MASK = 0xFU;

    static constexpr uint16_t PROMOTION_MASK = 0x8U;
    static constexpr uint16_t CAPTURE_MASK = 0x4U;

    // [ Null values ]
    // !EXPLAIN!
    static constexpr uint16_t NULL_MOVE = 0U;

    // [ Data members ]
    uint16_t moveData;
};

} // namespace Gluon