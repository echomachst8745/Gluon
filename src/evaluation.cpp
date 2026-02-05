#include "evaluation.h"

#include "bitboard.h"
#include "movetables.h"

#include <algorithm>
#include <array>

namespace Gluon {

// [ Table indexing ]
static constexpr size_t MIRRORED_SQUARE_MASK = 56;

static constexpr std::array<PieceType, NUM_PIECE_TYPES> EVALUATED_PIECE_TYPES = {
    PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING
};

static constexpr std::array<Colour, NUM_COLOURS> EVALUATED_COLOURS = { WHITE, BLACK };

// [ Game phase ]
static constexpr std::array<int, NUM_PIECE_TYPES> PHASE_WEIGHT_TABLE = { 0, 1, 1, 2, 4, 0 };

// [ Pawn masks ]
static constexpr std::array<Bitboard, NUM_FILES> ADJACENT_FILE_TABLE = []()
{
    std::array<Bitboard, NUM_FILES> maskTable{};

    for (size_t file = 0; file < NUM_FILES; ++file)
    {
        if (file > FILE_A)
        {
            maskTable[file] |= FileToBitboard(File(file - 1));
        }

        if (file < FILE_H)
        {
            maskTable[file] |= FileToBitboard(File(file + 1));
        }
    }

    return maskTable;
}();

static constexpr std::array<std::array<Bitboard, NUM_SQUARES>, NUM_COLOURS> FORWARD_FILE_TABLE = []()
{
    std::array<std::array<Bitboard, NUM_SQUARES>, NUM_COLOURS> maskTable{};

    for (size_t colour = 0; colour < NUM_COLOURS; ++colour)
    {
        const int rankStep = Colour(colour) == WHITE ? 1 : -1;

        for (size_t square = 0; square < NUM_SQUARES; ++square)
        {
            const Bitboard fileBitboard = FileToBitboard(SquareToFile(Square(square)));

            for (int rank = int(SquareToRank(Square(square))) + rankStep;
                 rank >= RANK_1 && rank <= RANK_8; rank += rankStep)
            {
                maskTable[colour][square] |= RankToBitboard(Rank(rank)) & fileBitboard;
            }
        }
    }

    return maskTable;
}();

// !EXPLAIN!
static constexpr std::array<std::array<Bitboard, NUM_SQUARES>, NUM_COLOURS> PASSED_PAWN_MASK_TABLE = []()
{
    std::array<std::array<Bitboard, NUM_SQUARES>, NUM_COLOURS> maskTable{};

    for (size_t colour = 0; colour < NUM_COLOURS; ++colour)
    {
        for (size_t square = 0; square < NUM_SQUARES; ++square)
        {
            const File file = SquareToFile(Square(square));

            maskTable[colour][square] = FORWARD_FILE_TABLE[colour][square];

            if (file > FILE_A)
            {
                maskTable[colour][square] |= FORWARD_FILE_TABLE[colour][square - 1];
            }

            if (file < FILE_H)
            {
                maskTable[colour][square] |= FORWARD_FILE_TABLE[colour][square + 1];
            }
        }
    }

    return maskTable;
}();

// [ Positional bonuses ]
static constexpr std::array<int, NUM_PHASES> BISHOP_PAIR_BONUS = { 25, 50 };

static constexpr std::array<int, NUM_PHASES> DOUBLED_PAWN_PENALTY = { -10, -25 };

static constexpr std::array<int, NUM_PHASES> ISOLATED_PAWN_PENALTY = { -15, -12 };

static constexpr std::array<std::array<int, NUM_RANKS>, NUM_PHASES> PASSED_PAWN_BONUS_TABLE = { {
    { 0,  2,  5, 12, 28,  55,  85, 0 },
    { 0, 10, 18, 35, 65, 110, 165, 0 }
} };

static constexpr std::array<int, NUM_PHASES> ROOK_OPEN_FILE_BONUS = { 30, 12 };

static constexpr std::array<int, NUM_PHASES> ROOK_SEMI_OPEN_FILE_BONUS = { 14, 6 };

static constexpr std::array<int, NUM_PHASES> ROOK_ON_SEVENTH_BONUS = { 22, 32 };

// [ Mobility ]
static constexpr size_t NUM_MOBILITY_PIECE_TYPES = 3;

static constexpr std::array<PieceType, NUM_MOBILITY_PIECE_TYPES> MOBILITY_PIECE_TYPES = {
    KNIGHT, BISHOP, ROOK
};

// !EXPLAIN!
static constexpr std::array<int, NUM_PIECE_TYPES> MOBILITY_OFFSET_TABLE = { 0, 4, 6, 7, 14, 0 };

static constexpr std::array<std::array<int, NUM_PIECE_TYPES>, NUM_PHASES> MOBILITY_WEIGHT_TABLE = { {
    { 0, 8,  8, 4, 0, 0 },
    { 0, 8, 10, 8, 0, 0 }
} };

// [ Piece values ]
static constexpr std::array<std::array<int, NUM_PIECE_TYPES>, NUM_PHASES> PIECE_VALUE_TABLE = { {
    { 100, 411, 445, 582, 1250, 0 },
    { 115, 343, 362, 624, 1141, 0 }
} };

// [ Piece square tables ]
static constexpr std::array<std::array<std::array<int, NUM_SQUARES>, NUM_PIECE_TYPES>,
                            NUM_PHASES> PIECE_SQUARE_TABLE = { {
    { {
        {
               0,    0,    0,    0,    0,    0,    0,    0,
             -43,   -1,  -24,  -28,  -18,   29,   46,  -27,
             -32,   -5,   -5,  -12,    4,    4,   40,  -15,
             -33,   -2,   -6,   15,   21,    7,   12,  -30,
             -17,   16,    7,   26,   28,   15,   21,  -28,
              -7,    9,   32,   38,   79,   68,   30,  -24,
             120,  163,   74,  116,   83,  154,   41,  -13,
               0,    0,    0,    0,    0,    0,    0,    0,
        },
        {
            -128,  -26,  -71,  -40,  -21,  -34,  -23,  -28,
             -35,  -65,  -15,   -4,   -1,   22,  -17,  -23,
             -28,  -11,   15,   12,   23,   21,   30,  -20,
             -16,    5,   20,   16,   34,   23,   26,  -10,
             -11,   21,   23,   65,   45,   84,   22,   27,
             -57,   73,   45,   79,  102,  157,   89,   54,
             -89,  -50,   88,   44,   28,   76,    9,  -21,
            -204, -109,  -41,  -60,   74, -118,  -18, -130,
        },
        {
             -40,   -4,  -17,  -26,  -16,  -15,  -48,  -26,
               5,   18,   20,    0,    9,   26,   40,    1,
               0,   18,   18,   18,   17,   33,   22,   12,
              -7,   16,   16,   32,   41,   15,   12,    5,
              -5,    6,   23,   61,   45,   45,    9,   -2,
             -20,   45,   52,   49,   43,   61,   45,   -2,
             -32,   20,  -22,  -16,   37,   72,   22,  -57,
             -35,    5, -100,  -45,  -30,  -51,    9,  -10,
        },
        {
             -23,  -16,    1,   21,   20,    9,  -45,  -32,
             -54,  -20,  -24,  -11,   -1,   13,   -7,  -87,
             -55,  -30,  -20,  -21,    4,    0,   -6,  -40,
             -44,  -32,  -15,   -1,   11,   -9,    7,  -28,
             -29,  -13,    9,   32,   29,   43,  -10,  -24,
              -6,   23,   32,   44,   21,   55,   74,   20,
              33,   39,   71,   76,   98,   82,   32,   54,
              39,   51,   39,   62,   77,   11,   38,   52,
        },
        {
              -1,  -22,  -11,   12,  -18,  -30,  -38,  -61,
             -43,  -10,   13,    2,   10,   18,   -4,    1,
             -17,    2,  -13,   -2,   -6,    2,   17,    6,
             -11,  -32,  -11,  -12,   -2,   -5,    4,   -4,
             -33,  -33,  -20,  -20,   -1,   21,   -2,    1,
             -16,  -21,    9,   10,   35,   68,   57,   70,
             -29,  -48,   -6,    1,  -20,   70,   34,   66,
             -34,    0,   35,   15,   72,   54,   52,   55,
        },
        {
             -18,   44,   15,  -66,   10,  -34,   29,   17,
               1,    9,  -10,  -78,  -52,  -20,   11,   10,
             -17,  -17,  -27,  -56,  -54,  -37,  -18,  -33,
             -60,   -1,  -33,  -48,  -56,  -54,  -40,  -62,
             -21,  -24,  -15,  -33,  -37,  -30,  -17,  -44,
             -11,   29,    2,  -20,  -24,    7,   27,  -27,
              35,   -1,  -24,   -9,  -10,   -5,  -46,  -35,
             -79,   28,   20,  -18,  -68,  -41,    2,   16,
        },
    } },
    { {
        {
               0,    0,    0,    0,    0,    0,    0,    0,
              16,   10,   10,   12,   16,    0,    2,   -9,
               5,    9,   -7,    1,    0,   -6,   -1,  -10,
              16,   11,   -4,   -9,   -9,  -10,    4,   -1,
              39,   29,   16,    6,   -2,    5,   21,   21,
             115,  122,  104,   82,   68,   65,  100,  102,
             217,  211,  193,  163,  179,  161,  201,  228,
               0,    0,    0,    0,    0,    0,    0,    0,
        },
        {
             -35,  -62,  -28,  -18,  -27,  -22,  -61,  -78,
             -51,  -24,  -12,   -6,   -2,  -24,  -28,  -54,
             -28,   -4,   -1,   18,   12,   -4,  -24,  -27,
             -22,   -7,   20,   30,   20,   21,    5,  -22,
             -21,    4,   27,   27,   27,   13,   10,  -22,
             -29,  -24,   12,   11,   -1,  -11,  -23,  -50,
             -30,  -10,  -30,   -2,  -11,  -30,  -29,  -63,
             -71,  -46,  -16,  -34,  -38,  -33,  -77, -121,
        },
        {
             -28,  -11,  -28,   -6,  -11,  -20,   -6,  -21,
             -17,  -22,   -9,   -1,    5,  -11,  -18,  -33,
             -15,   -4,   10,   12,   16,    4,   -9,  -18,
              -7,    4,   16,   23,    9,   12,   -4,  -11,
              -4,   11,   15,   11,   17,   12,    4,    2,
               2,  -10,    0,   -1,   -2,    7,    0,    5,
             -10,   -5,    9,  -15,   -4,  -16,   -5,  -17,
             -17,  -26,  -13,  -10,   -9,  -11,  -21,  -29,
        },
        {
             -11,    2,    4,   -1,   -6,  -16,    5,  -24,
              -7,   -7,    0,    2,  -11,  -11,  -13,   -4,
              -5,    0,   -6,   -1,   -9,  -15,  -10,  -20,
               4,    6,   10,    5,   -6,   -7,  -10,  -13,
               5,    4,   16,    1,    2,    1,   -1,    2,
               9,    9,    9,    6,    5,   -4,   -6,   -4,
              13,   16,   16,   13,   -4,    4,   10,    4,
              16,   12,   22,   18,   15,   15,   10,    6,
        },
        {
             -40,  -34,  -27,  -52,   -6,  -39,  -24,  -50,
             -27,  -28,  -37,  -20,  -20,  -28,  -44,  -39,
             -20,  -33,   18,    7,   11,   21,   12,    6,
             -22,   34,   23,   57,   38,   41,   48,   28,
               4,   27,   29,   55,   70,   49,   70,   44,
             -24,    7,   11,   60,   57,   43,   23,   11,
             -21,   24,   39,   50,   71,   30,   37,    0,
             -11,   27,   27,   33,   33,   23,   12,   24,
        },
        {
             -65,  -41,  -26,  -13,  -34,  -17,  -29,  -52,
             -33,  -13,    5,   16,   17,    5,   -6,  -21,
             -23,   -4,   13,   26,   28,   20,    9,  -11,
             -22,   -5,   26,   29,   33,   28,   11,  -13,
             -10,   27,   29,   33,   32,   40,   32,    4,
              12,   21,   28,   18,   24,   55,   54,   16,
             -15,   21,   17,   21,   21,   46,   28,   13,
             -90,  -43,  -22,  -22,  -13,   18,    5,  -21,
        },
    } },
} };

static void AddScore(std::array<int, NUM_PHASES>& phaseScores,
                     const std::array<int, NUM_PHASES>& bonus, int colourSign)
{
    for (size_t phaseIndex = 0; phaseIndex < NUM_PHASES; ++phaseIndex)
    {
        phaseScores[phaseIndex] += colourSign * bonus[phaseIndex];
    }
}

static void EvaluatePawnStructure(const Position& position, Colour colour, int colourSign,
                                  std::array<int, NUM_PHASES>& phaseScores)
{
    const Bitboard friendlyPawnsBitboard = position.GetPieceBitboard(MakePiece(colour, PAWN));
    const Bitboard enemyPawnsBitboard = position.GetPieceBitboard(MakePiece(~colour, PAWN));

    Bitboard pawnsBitboard = friendlyPawnsBitboard;

    while (pawnsBitboard)
    {
        const size_t square = size_t(BB::PopLSB(pawnsBitboard));

        if (friendlyPawnsBitboard & FORWARD_FILE_TABLE[colour][square])
        {
            AddScore(phaseScores, DOUBLED_PAWN_PENALTY, colourSign);
        }

        if (!(friendlyPawnsBitboard & ADJACENT_FILE_TABLE[SquareToFile(Square(square))]))
        {
            AddScore(phaseScores, ISOLATED_PAWN_PENALTY, colourSign);
        }

        if (!(enemyPawnsBitboard & PASSED_PAWN_MASK_TABLE[colour][square]))
        {
            // !EXPLAIN!
            const Rank relativeRank = colour == WHITE ? SquareToRank(Square(square))
                                                      : Rank(RANK_8 - SquareToRank(Square(square)));

            for (size_t phaseIndex = 0; phaseIndex < NUM_PHASES; ++phaseIndex)
            {
                phaseScores[phaseIndex] += colourSign *
                                           PASSED_PAWN_BONUS_TABLE[phaseIndex][relativeRank];
            }
        }
    }
}

static void EvaluateRooks(const Position& position, Colour colour, int colourSign,
                          std::array<int, NUM_PHASES>& phaseScores)
{
    const Bitboard friendlyPawnsBitboard = position.GetPieceBitboard(MakePiece(colour, PAWN));
    const Bitboard enemyPawnsBitboard = position.GetPieceBitboard(MakePiece(~colour, PAWN));

    const Rank seventhRank = colour == WHITE ? RANK_7 : RANK_2;

    Bitboard rooksBitboard = position.GetPieceBitboard(MakePiece(colour, ROOK));

    while (rooksBitboard)
    {
        const Square square = Square(BB::PopLSB(rooksBitboard));
        const Bitboard fileBitboard = FileToBitboard(SquareToFile(square));

        if (!(friendlyPawnsBitboard & fileBitboard))
        {
            AddScore(phaseScores, (enemyPawnsBitboard & fileBitboard) ? ROOK_SEMI_OPEN_FILE_BONUS
                                                                      : ROOK_OPEN_FILE_BONUS,
                     colourSign);
        }

        if (SquareToRank(square) == seventhRank)
        {
            AddScore(phaseScores, ROOK_ON_SEVENTH_BONUS, colourSign);
        }
    }
}

static Bitboard GetPieceAttacks(PieceType pieceType, Square square, Bitboard occupancyBitboard)
{
    switch (pieceType)
    {
        case KNIGHT: return MoveTables::KNIGHT_MOVE_TABLE[square];
        case BISHOP: return MoveTables::GetBishopMoves(square, occupancyBitboard);
        case ROOK:   return MoveTables::GetRookMoves(square, occupancyBitboard);
        case QUEEN:  return MoveTables::GetQueenMoves(square, occupancyBitboard);
        default:     return 0ULL;
    }
}

static void EvaluateMobility(const Position& position, Colour colour, int colourSign,
                             std::array<int, NUM_PHASES>& phaseScores)
{
    const Bitboard occupancyBitboard = position.GetAllOccupancyBitboard();
    const Bitboard friendlyOccupancyBitboard = position.GetOccupancyBitboard(colour);

    for (PieceType pieceType : MOBILITY_PIECE_TYPES)
    {
        const size_t pieceIndex = PieceTypeToIndex(pieceType);

        Bitboard pieceBitboard = position.GetPieceBitboard(MakePiece(colour, pieceType));

        while (pieceBitboard)
        {
            const Square square = Square(BB::PopLSB(pieceBitboard));

            const Bitboard attackBitboard = GetPieceAttacks(pieceType, square, occupancyBitboard) &
                                            ~friendlyOccupancyBitboard;

            const int mobility = BB::CountBits(attackBitboard) - MOBILITY_OFFSET_TABLE[pieceIndex];

            for (size_t phaseIndex = 0; phaseIndex < NUM_PHASES; ++phaseIndex)
            {
                phaseScores[phaseIndex] += colourSign *
                                           MOBILITY_WEIGHT_TABLE[phaseIndex][pieceIndex] * mobility;
            }
        }
    }
}

int Evaluate(const Position& position)
{
    std::array<int, NUM_PHASES> phaseScores = { 0, 0 };

    int phase = 0;

    for (Colour colour : EVALUATED_COLOURS)
    {
        const int colourSign = colour == WHITE ? 1 : -1;

        if (BB::CountBits(position.GetPieceBitboard(MakePiece(colour, BISHOP))) >= 2)
        {
            AddScore(phaseScores, BISHOP_PAIR_BONUS, colourSign);
        }

        EvaluatePawnStructure(position, colour, colourSign, phaseScores);

        EvaluateRooks(position, colour, colourSign, phaseScores);

        EvaluateMobility(position, colour, colourSign, phaseScores);

        for (PieceType pieceType : EVALUATED_PIECE_TYPES)
        {
            const size_t pieceIndex = PieceTypeToIndex(pieceType);

            Bitboard pieceBitboard = position.GetPieceBitboard(MakePiece(colour, pieceType));

            while (pieceBitboard)
            {
                const size_t square = size_t(BB::PopLSB(pieceBitboard));

                // !EXPLAIN!
                const size_t tableSquare = colour == WHITE ? square : square ^ MIRRORED_SQUARE_MASK;

                phase += PHASE_WEIGHT_TABLE[pieceIndex];

                for (size_t phaseIndex = 0; phaseIndex < NUM_PHASES; ++phaseIndex)
                {
                    phaseScores[phaseIndex] += colourSign *
                                               (PIECE_VALUE_TABLE[phaseIndex][pieceIndex] +
                                                PIECE_SQUARE_TABLE[phaseIndex][pieceIndex][tableSquare]);
                }
            }
        }
    }

    // !EXPLAIN!
    const int clampedPhase = std::min(phase, MAX_PHASE);

    const int score = (phaseScores[MIDGAME] * clampedPhase +
                       phaseScores[ENDGAME] * (MAX_PHASE - clampedPhase)) / MAX_PHASE;

    return position.GetActiveColour() == WHITE ? score : -score;
}

} // namespace Gluon
