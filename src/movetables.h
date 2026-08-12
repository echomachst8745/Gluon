#pragma once

#include "types.h"
#include "bitboard.h"

#include <array>
#include <cassert>

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

constexpr Bitboard GetSliderMoves(Square fromSquare, size_t firstOffsetIndex,
                                  Bitboard occupancyBitboard)
{
    Bitboard moveBitboard = 0ULL;

    for (size_t offsetIndex = firstOffsetIndex; offsetIndex < NUM_DIRECTIONS; offsetIndex += 2)
    {
        moveBitboard |= GetRayMoves(fromSquare, offsetIndex, occupancyBitboard);
    }

    return moveBitboard;
}

// Magic bitboards

// !EXPLAIN!
constexpr Bitboard GetSliderMask(Square fromSquare, size_t firstOffsetIndex)
{
    const Bitboard edgeFileBitboard = (FileToBitboard(FILE_A) | FileToBitboard(FILE_H)) &
                                      (~FileToBitboard(SquareToFile(fromSquare)));
    const Bitboard edgeRankBitboard = (RankToBitboard(RANK_1) | RankToBitboard(RANK_8)) &
                                      (~RankToBitboard(SquareToRank(fromSquare)));

    return GetSliderMoves(fromSquare, firstOffsetIndex, 0ULL) &
           (~(edgeFileBitboard | edgeRankBitboard));
}

constexpr size_t GetSliderAttackTableSize(size_t firstOffsetIndex)
{
    size_t attackTableSize = 0;

    for (size_t fromSquare = 0; fromSquare < NUM_SQUARES; ++fromSquare)
    {
        const Bitboard maskBitboard = GetSliderMask(Square(fromSquare), firstOffsetIndex);

        attackTableSize += size_t(1) << BB::CountBits(maskBitboard);
    }

    return attackTableSize;
}

constexpr size_t BISHOP_ATTACK_TABLE_SIZE = GetSliderAttackTableSize(FIRST_DIAGONAL_DIRECTION);
constexpr size_t ROOK_ATTACK_TABLE_SIZE   = GetSliderAttackTableSize(FIRST_STRAIGHT_DIRECTION);
constexpr size_t SLIDER_ATTACK_TABLE_SIZE = BISHOP_ATTACK_TABLE_SIZE + ROOK_ATTACK_TABLE_SIZE;

// !EXPLAIN!
struct MagicEntry
{
    Bitboard maskBitboard;

    Bitboard magicNumber;

    size_t attackTableOffset;

    int indexShift;
};

// !EXPLAIN!
constexpr size_t GetMagicIndex(const MagicEntry& magicEntry, Bitboard occupancyBitboard)
{
    const Bitboard maskedOccupancyBitboard = occupancyBitboard & magicEntry.maskBitboard;

    return magicEntry.attackTableOffset +
           ((maskedOccupancyBitboard * magicEntry.magicNumber) >> magicEntry.indexShift);
}

// !EXPLAIN!
constexpr std::array<Bitboard, NUM_SQUARES> BISHOP_MAGIC_NUMBERS = {
    0xC222021C08020042ULL, 0x10021004210040C8ULL, 0x80F0810841008400ULL, 0x88A804A500444000ULL,
    0x00AC2C2000040231ULL, 0x4022084208000014ULL, 0x0202008A21500100ULL, 0x2006820082200200ULL,
    0x0560241010020690ULL, 0x4060210808828480ULL, 0x0100101100450100ULL, 0x0181209093000004ULL,
    0x2110040420000000ULL, 0x0042008824408490ULL, 0x00400C0104022060ULL, 0x0010702218240400ULL,
    0x1408810420040410ULL, 0x0820880484040046ULL, 0x0802001000220020ULL, 0x8000810802004018ULL,
    0x00040016020A3800ULL, 0x4332000108120200ULL, 0x0800800408882804ULL, 0x0004410114480C04ULL,
    0x548208101010100CULL, 0x0214200042420452ULL, 0x0080221014080200ULL, 0x0210040098440008ULL,
    0x0004082004002000ULL, 0x0084820084880405ULL, 0x8009050008480800ULL, 0x08004A0003820140ULL,
    0x0001100884102020ULL, 0x0000880808041000ULL, 0x0004240110700900ULL, 0x3104A20180080080ULL,
    0x8008120400481100ULL, 0x4200980200084100ULL, 0x1010022080006C00ULL, 0x1200810100044400ULL,
    0x0002011040240821ULL, 0x8000809010002888ULL, 0x0201920110080100ULL, 0x0302502128002400ULL,
    0x03026000A4010080ULL, 0x0210200080250100ULL, 0x6804040084080200ULL, 0x0024040880288E00ULL,
    0x0400820110405201ULL, 0x0080828808034004ULL, 0x8800008400880000ULL, 0x0006000020882000ULL,
    0x40401020A4A41000ULL, 0x2B140408100C4100ULL, 0x5009110408004028ULL, 0x4011441114082000ULL,
    0x2001040202010400ULL, 0x8008002114100420ULL, 0x1014081840441000ULL, 0x0000042004208808ULL,
    0x0020020040028221ULL, 0x808804C0908C0120ULL, 0x0400204801012400ULL, 0x0108A04082044104ULL
};

constexpr std::array<Bitboard, NUM_SQUARES> ROOK_MAGIC_NUMBERS = {
    0x6080002010400088ULL, 0x0440200010004000ULL, 0x2100090020004010ULL, 0x0100090110000520ULL,
    0x0200020010082004ULL, 0x0200080402001001ULL, 0x0400080102009004ULL, 0x02000A0080410824ULL,
    0x0001002080004108ULL, 0x2100400020100044ULL, 0x6002001200804020ULL, 0x0009001000210208ULL,
    0x2002000408211200ULL, 0x4022000200100804ULL, 0x4114001052280401ULL, 0x4001000040822100ULL,
    0x0000208000400088ULL, 0xA8018A8020014000ULL, 0x5222020010408020ULL, 0x0000818008005000ULL,
    0x0041030008001004ULL, 0x8500808004000200ULL, 0x0300440010414208ULL, 0x0202C20001105084ULL,
    0x2080004240042002ULL, 0x1220004040100028ULL, 0x0000200880100080ULL, 0x0180120200200840ULL,
    0x1004080080800400ULL, 0x0088020080800400ULL, 0x000A000200040801ULL, 0x2100828200290154ULL,
    0x0080804008800020ULL, 0x1520400080802000ULL, 0x0000200101001041ULL, 0x2001001001000825ULL,
    0x0940080082800400ULL, 0x0900100408014020ULL, 0x000A121144001008ULL, 0x0210801040802100ULL,
    0x0040814000218000ULL, 0x428020015000C000ULL, 0x0022420014820023ULL, 0x1030000800108080ULL,
    0x8020080011010004ULL, 0x0011000400090022ULL, 0x0001020001008080ULL, 0x0140040040820001ULL,
    0x7840004021800180ULL, 0x1418802200410200ULL, 0x14E7200048130300ULL, 0x8000821800300280ULL,
    0x0080110088000500ULL, 0x0081000400080300ULL, 0x0810800100020080ULL, 0x4840010040AC0200ULL,
    0x0001028001102541ULL, 0x01A20C2100108042ULL, 0x1010401500200009ULL, 0x0B10000810200501ULL,
    0x0151004800021005ULL, 0x0212004421083002ULL, 0x000A080081621004ULL, 0x0001504110802402ULL
};

constexpr std::array<MagicEntry, NUM_SQUARES> MakeMagicTable(size_t firstOffsetIndex,
                                                             size_t firstAttackTableOffset)
{
    const std::array<Bitboard, NUM_SQUARES>& magicNumbers =
        firstOffsetIndex == FIRST_DIAGONAL_DIRECTION ? BISHOP_MAGIC_NUMBERS : ROOK_MAGIC_NUMBERS;

    std::array<MagicEntry, NUM_SQUARES> magicTable{};

    size_t attackTableOffset = firstAttackTableOffset;

    for (size_t fromSquare = 0; fromSquare < NUM_SQUARES; ++fromSquare)
    {
        const Bitboard maskBitboard = GetSliderMask(Square(fromSquare), firstOffsetIndex);
        const int maskBitCount = BB::CountBits(maskBitboard);

        magicTable[fromSquare] = { maskBitboard, magicNumbers[fromSquare], attackTableOffset,
                                   NUM_SQUARES - maskBitCount };

        attackTableOffset += size_t(1) << maskBitCount;
    }

    return magicTable;
}

constexpr std::array<MagicEntry, NUM_SQUARES> BISHOP_MAGIC_TABLE =
    MakeMagicTable(FIRST_DIAGONAL_DIRECTION, 0);
constexpr std::array<MagicEntry, NUM_SQUARES> ROOK_MAGIC_TABLE =
    MakeMagicTable(FIRST_STRAIGHT_DIRECTION, BISHOP_ATTACK_TABLE_SIZE);

// !EXPLAIN!
constexpr void FillSliderAttackTable(std::array<Bitboard, SLIDER_ATTACK_TABLE_SIZE>& attackTable,
                                     const std::array<MagicEntry, NUM_SQUARES>& magicTable,
                                     size_t firstOffsetIndex)
{
    for (size_t fromSquare = 0; fromSquare < NUM_SQUARES; ++fromSquare)
    {
        const MagicEntry& magicEntry = magicTable[fromSquare];

        Bitboard occupancyBitboard = 0ULL;

        do
        {
            const Bitboard moveBitboard = GetSliderMoves(Square(fromSquare), firstOffsetIndex,
                                                         occupancyBitboard);
            const size_t attackTableIndex = GetMagicIndex(magicEntry, occupancyBitboard);

            assert((attackTable[attackTableIndex] == 0ULL ||
                    attackTable[attackTableIndex] == moveBitboard) && "Magic number collision");

            attackTable[attackTableIndex] = moveBitboard;

            occupancyBitboard = (occupancyBitboard - magicEntry.maskBitboard) & magicEntry.maskBitboard;
        } while (occupancyBitboard);
    }
}

// !EXPLAIN!
inline constexpr std::array<Bitboard, SLIDER_ATTACK_TABLE_SIZE> SLIDER_ATTACK_TABLE = []()
{
    std::array<Bitboard, SLIDER_ATTACK_TABLE_SIZE> attackTable{};

    FillSliderAttackTable(attackTable, BISHOP_MAGIC_TABLE, FIRST_DIAGONAL_DIRECTION);
    FillSliderAttackTable(attackTable, ROOK_MAGIC_TABLE, FIRST_STRAIGHT_DIRECTION);

    return attackTable;
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

constexpr Bitboard GetBishopMoves(Square fromSquare, Bitboard occupancyBitboard)
{
    return SLIDER_ATTACK_TABLE[GetMagicIndex(BISHOP_MAGIC_TABLE[fromSquare], occupancyBitboard)];
}

constexpr Bitboard GetRookMoves(Square fromSquare, Bitboard occupancyBitboard)
{
    return SLIDER_ATTACK_TABLE[GetMagicIndex(ROOK_MAGIC_TABLE[fromSquare], occupancyBitboard)];
}

constexpr Bitboard GetQueenMoves(Square fromSquare, Bitboard occupancyBitboard)
{
    return GetBishopMoves(fromSquare, occupancyBitboard) | GetRookMoves(fromSquare, occupancyBitboard);
}

} // namespace Gluon::MoveTables