#pragma once

#include "types.h"
#include "bitboard.h"

#include <array>

namespace Gluon::MoveTables {

// Knight moves

constexpr std::array<Bitboard, NUM_SQUARES> KNIGHT_MOVE_TABLE = []()
{
    const int knightMoveFileOffsets[NUM_DIRECTIONS] = { 1, 2, 2, 1, -1, -2, -2, -1 };
    const int knightMoveRankOffsets[NUM_DIRECTIONS] = { 2, 1, -1, -2, -2, -1, 1, 2 };

    std::array<Bitboard, NUM_SQUARES> moveTable{};

    for (int fromSquare = 0; fromSquare < NUM_SQUARES; ++fromSquare)
    {
        const int fromFile = fromSquare % NUM_FILES;
        const int fromRank = fromSquare / NUM_FILES;

        for (int offsetIndex = 0; offsetIndex < NUM_DIRECTIONS; ++offsetIndex)
        {
            const int attackFile = fromFile + knightMoveFileOffsets[offsetIndex];
            const int attackRank = fromRank + knightMoveRankOffsets[offsetIndex];

            if (attackFile < FILE_A || attackFile > FILE_H ||
                attackRank < RANK_1 || attackRank > RANK_8)
            {
                continue;
            }

            const Square attackSquare = FileRankToSquare(File(attackFile), Rank(attackRank));

            BB::SetSquare(moveTable[Square(fromSquare)], attackSquare);
        }
    }

    return moveTable;
}();

// Pawn attacks

constexpr std::array<std::array<Bitboard, NUM_SQUARES>, NUM_COLOURS> PAWN_ATTACK_TABLE = []()
{
    const int pawnAttackFileOffsets[2] = { 1, -1 };

    std::array<std::array<Bitboard, NUM_SQUARES>, NUM_COLOURS> moveTable{};

    for (size_t colour = 0; colour < NUM_COLOURS; ++colour)
    {
        const int pawnAttackRankOffset = Colour(colour) == WHITE ? 1 : -1;

        for (int fromSquare = 0; fromSquare < NUM_SQUARES; ++fromSquare)
        {
            const int fromFile = fromSquare % NUM_FILES;
            const int fromRank = fromSquare / NUM_FILES;

            for (size_t offsetIndex = 0; offsetIndex < 2; ++offsetIndex)
            {
                const int attackFile = fromFile + pawnAttackFileOffsets[offsetIndex];
                const int attackRank = fromRank + pawnAttackRankOffset;

                if (attackFile < FILE_A || attackFile > FILE_H ||
                    attackRank < RANK_1 || attackRank > RANK_8)
                {
                    continue;
                }

                const Square attackSquare = FileRankToSquare(File(attackFile), Rank(attackRank));

                BB::SetSquare(moveTable[colour][Square(fromSquare)], attackSquare);
            }
        }
    }

    return moveTable;
}();

constexpr std::array<Bitboard, NUM_SQUARES> KING_MOVE_TABLE = []()
{
    const int kingMoveFileOffsets[NUM_DIRECTIONS] = { 1, 1, 0, -1, -1, -1, 0, 1 };
    const int kingMoveRankOffsets[NUM_DIRECTIONS] = { 0, 1, 1, 1, 0, -1, -1, -1 };

    std::array<Bitboard, NUM_SQUARES> moveTable{};

    for (int fromSquare = 0; fromSquare < NUM_SQUARES; ++fromSquare)
    {
        const int fromFile = fromSquare % NUM_FILES;
        const int fromRank = fromSquare / NUM_FILES;

        for (int offsetIndex = 0; offsetIndex < NUM_DIRECTIONS; ++offsetIndex)
        {
            const int attackFile = fromFile + kingMoveFileOffsets[offsetIndex];
            const int attackRank = fromRank + kingMoveRankOffsets[offsetIndex];

            if (attackFile < FILE_A || attackFile > FILE_H ||
                attackRank < RANK_1 || attackRank > RANK_8)
            {
                continue;
            }

            const Square attackSquare = FileRankToSquare(File(attackFile), Rank(attackRank));

            BB::SetSquare(moveTable[Square(fromSquare)], attackSquare);
        }
    }

    return moveTable;
}();

// Slider moves

// !EXPLAIN!
constexpr size_t NUM_POSITIVE_DIRECTIONS = NUM_DIRECTIONS / 2;

constexpr size_t FIRST_DIAGONAL_DIRECTION = 1;
constexpr size_t FIRST_STRAIGHT_DIRECTION = 0;

constexpr std::array<std::array<Bitboard, NUM_SQUARES>, NUM_DIRECTIONS> RAY_MOVE_TABLE = []()
{
    const int rayMoveFileOffsets[NUM_DIRECTIONS] = { 1, 1, 0, -1, -1, -1, 0, 1 };
    const int rayMoveRankOffsets[NUM_DIRECTIONS] = { 0, 1, 1, 1, 0, -1, -1, -1 };

    std::array<std::array<Bitboard, NUM_SQUARES>, NUM_DIRECTIONS> moveTable{};

    for (int fromSquare = 0; fromSquare < NUM_SQUARES; ++fromSquare)
    {
        const int fromFile = fromSquare % NUM_FILES;
        const int fromRank = fromSquare / NUM_FILES;

        for (size_t offsetIndex = 0; offsetIndex < NUM_DIRECTIONS; ++offsetIndex)
        {
            int attackFile = fromFile + rayMoveFileOffsets[offsetIndex];
            int attackRank = fromRank + rayMoveRankOffsets[offsetIndex];

            while (attackFile >= FILE_A && attackFile <= FILE_H &&
                   attackRank >= RANK_1 && attackRank <= RANK_8)
            {
                const Square attackSquare = FileRankToSquare(File(attackFile), Rank(attackRank));

                BB::SetSquare(moveTable[offsetIndex][Square(fromSquare)], attackSquare);

                attackFile += rayMoveFileOffsets[offsetIndex];
                attackRank += rayMoveRankOffsets[offsetIndex];
            }
        }
    }

    return moveTable;
}();

// Square alignment

// !EXPLAIN!
constexpr std::array<std::array<Bitboard, NUM_SQUARES>, NUM_SQUARES> BETWEEN_TABLE = []()
{
    std::array<std::array<Bitboard, NUM_SQUARES>, NUM_SQUARES> betweenTable{};

    for (size_t fromSquare = 0; fromSquare < NUM_SQUARES; ++fromSquare)
    {
        for (size_t offsetIndex = 0; offsetIndex < NUM_DIRECTIONS; ++offsetIndex)
        {
            const size_t oppositeOffsetIndex = (offsetIndex + NUM_POSITIVE_DIRECTIONS) % NUM_DIRECTIONS;

            Bitboard rayBitboard = RAY_MOVE_TABLE[offsetIndex][fromSquare];

            while (rayBitboard)
            {
                const size_t toSquare = size_t(BB::PopLSB(rayBitboard));

                betweenTable[fromSquare][toSquare] = RAY_MOVE_TABLE[offsetIndex][fromSquare] &
                                                     RAY_MOVE_TABLE[oppositeOffsetIndex][toSquare];
            }
        }
    }

    return betweenTable;
}();

// !EXPLAIN!
constexpr std::array<std::array<Bitboard, NUM_SQUARES>, NUM_SQUARES> LINE_TABLE = []()
{
    std::array<std::array<Bitboard, NUM_SQUARES>, NUM_SQUARES> lineTable{};

    for (size_t fromSquare = 0; fromSquare < NUM_SQUARES; ++fromSquare)
    {
        for (size_t offsetIndex = 0; offsetIndex < NUM_DIRECTIONS; ++offsetIndex)
        {
            const size_t oppositeOffsetIndex = (offsetIndex + NUM_POSITIVE_DIRECTIONS) % NUM_DIRECTIONS;

            Bitboard rayBitboard = RAY_MOVE_TABLE[offsetIndex][fromSquare];

            while (rayBitboard)
            {
                const size_t toSquare = size_t(BB::PopLSB(rayBitboard));

                lineTable[fromSquare][toSquare] = RAY_MOVE_TABLE[offsetIndex][fromSquare]         |
                                                  RAY_MOVE_TABLE[oppositeOffsetIndex][fromSquare] |
                                                  SquareToBitboard(Square(fromSquare));
            }
        }
    }

    return lineTable;
}();

// Castling

constexpr Bitboard WHITE_KING_SIDE_CASTLING_PATH  = SquareToBitboard(SQUARE_F1) |
                                                    SquareToBitboard(SQUARE_G1);
constexpr Bitboard WHITE_QUEEN_SIDE_CASTLING_PATH = SquareToBitboard(SQUARE_B1) |
                                                    SquareToBitboard(SQUARE_C1) |
                                                    SquareToBitboard(SQUARE_D1);

constexpr Bitboard BLACK_KING_SIDE_CASTLING_PATH  = SquareToBitboard(SQUARE_F8) |
                                                    SquareToBitboard(SQUARE_G8);
constexpr Bitboard BLACK_QUEEN_SIDE_CASTLING_PATH = SquareToBitboard(SQUARE_B8) |
                                                    SquareToBitboard(SQUARE_C8) |
                                                    SquareToBitboard(SQUARE_D8);

// !EXPLAIN!
constexpr std::array<CastlingRight, NUM_SQUARES> CASTLING_RIGHT_MASK_TABLE = []()
{
    std::array<CastlingRight, NUM_SQUARES> maskTable{};
    maskTable.fill(ALL_CASTLING_RIGHTS);

    maskTable[SQUARE_E1] = CastlingRight(ALL_CASTLING_RIGHTS ^ (WHITE_OO | WHITE_OOO));
    maskTable[SQUARE_H1] = CastlingRight(ALL_CASTLING_RIGHTS ^ WHITE_OO);
    maskTable[SQUARE_A1] = CastlingRight(ALL_CASTLING_RIGHTS ^ WHITE_OOO);

    maskTable[SQUARE_E8] = CastlingRight(ALL_CASTLING_RIGHTS ^ (BLACK_OO | BLACK_OOO));
    maskTable[SQUARE_H8] = CastlingRight(ALL_CASTLING_RIGHTS ^ BLACK_OO);
    maskTable[SQUARE_A8] = CastlingRight(ALL_CASTLING_RIGHTS ^ BLACK_OOO);

    return maskTable;
}();

// !EXPLAIN!
constexpr Bitboard GetRayMoves(Square fromSquare, size_t offsetIndex, Bitboard occupancyBitboard)
{
    Bitboard rayBitboard = RAY_MOVE_TABLE[offsetIndex][fromSquare];
    Bitboard blockersBitboard = rayBitboard & occupancyBitboard;

    if (blockersBitboard)
    {
        Square blockerSquare = Square(offsetIndex < NUM_POSITIVE_DIRECTIONS ?
                                      BB::GetLSB(blockersBitboard) : BB::GetMSB(blockersBitboard));

        rayBitboard ^= RAY_MOVE_TABLE[offsetIndex][blockerSquare];
    }

    return rayBitboard;
}

constexpr Bitboard GetBishopMoves(Square fromSquare, Bitboard occupancyBitboard)
{
    Bitboard moveBitboard = 0ULL;

    for (size_t offsetIndex = FIRST_DIAGONAL_DIRECTION; offsetIndex < NUM_DIRECTIONS; offsetIndex += 2)
    {
        moveBitboard |= GetRayMoves(fromSquare, offsetIndex, occupancyBitboard);
    }

    return moveBitboard;
}

constexpr Bitboard GetRookMoves(Square fromSquare, Bitboard occupancyBitboard)
{
    Bitboard moveBitboard = 0ULL;

    for (size_t offsetIndex = FIRST_STRAIGHT_DIRECTION; offsetIndex < NUM_DIRECTIONS; offsetIndex += 2)
    {
        moveBitboard |= GetRayMoves(fromSquare, offsetIndex, occupancyBitboard);
    }

    return moveBitboard;
}

constexpr Bitboard GetQueenMoves(Square fromSquare, Bitboard occupancyBitboard)
{
    return GetBishopMoves(fromSquare, occupancyBitboard) | GetRookMoves(fromSquare, occupancyBitboard);
}

} // namespace Gluon::MoveTables