#pragma once

#include <cstdint>
#include <string>

namespace Gluon {

// Bitboard type

using Bitboard = uint64_t;

// HashKey type

using HashKey = uint64_t;

// Colour type

enum Colour : uint8_t
{
    WHITE,
    BLACK,

    NUM_COLOURS
};

constexpr Colour operator~(Colour colour)
{
    return Colour(colour ^ BLACK);
}

// PieceType type

constexpr uint8_t PIECE_TYPE_MASK = 0x3FU;

// !EXPLAIN!
enum PieceType : uint8_t
{
    NO_PIECE_TYPE = 0U,

    PAWN   = (1U << 0),
    KNIGHT = (1U << 1),
    BISHOP = (1U << 2),
    ROOK   = (1U << 3),
    QUEEN  = (1U << 4),
    KING   = (1U << 5),

    NUM_PIECE_TYPES = 6U
};

// Piece type

constexpr uint8_t WHITE_PIECE_COLOUR_MASK = 0x80U;
constexpr uint8_t BLACK_PIECE_COLOUR_MASK = 0x40U;

enum Piece : uint8_t
{
    NO_PIECE = 0U,

    WHITE_PAWN   = PAWN   + WHITE_PIECE_COLOUR_MASK,
    WHITE_KNIGHT = KNIGHT + WHITE_PIECE_COLOUR_MASK,
    WHITE_BISHOP = BISHOP + WHITE_PIECE_COLOUR_MASK,
    WHITE_ROOK   = ROOK   + WHITE_PIECE_COLOUR_MASK,
    WHITE_QUEEN  = QUEEN  + WHITE_PIECE_COLOUR_MASK,
    WHITE_KING   = KING   + WHITE_PIECE_COLOUR_MASK,

    BLACK_PAWN   = PAWN   + BLACK_PIECE_COLOUR_MASK,
    BLACK_KNIGHT = KNIGHT + BLACK_PIECE_COLOUR_MASK,
    BLACK_BISHOP = BISHOP + BLACK_PIECE_COLOUR_MASK,
    BLACK_ROOK   = ROOK   + BLACK_PIECE_COLOUR_MASK,
    BLACK_QUEEN  = QUEEN  + BLACK_PIECE_COLOUR_MASK,
    BLACK_KING   = KING   + BLACK_PIECE_COLOUR_MASK,

    NUM_PIECES = 12U
};

// !EXPLAIN!
constexpr Colour GetColour(Piece piece)
{
    return (piece & WHITE_PIECE_COLOUR_MASK) ? Colour::WHITE : Colour::BLACK;
}

constexpr PieceType GetType(Piece piece)
{
    return PieceType(piece & PIECE_TYPE_MASK);
}

constexpr Piece MakePiece(Colour colour, PieceType pieceType)
{
    return Piece(pieceType + (colour == WHITE ? WHITE_PIECE_COLOUR_MASK : BLACK_PIECE_COLOUR_MASK));
}

constexpr char PieceToChar(Piece piece)
{
    switch (piece)
    {
        case WHITE_PAWN:   return 'P';
        case WHITE_KNIGHT: return 'N';
        case WHITE_BISHOP: return 'B';
        case WHITE_ROOK:   return 'R';
        case WHITE_QUEEN:  return 'Q';
        case WHITE_KING:   return 'K';
        case BLACK_PAWN:   return 'p';
        case BLACK_KNIGHT: return 'n';
        case BLACK_BISHOP: return 'b';
        case BLACK_ROOK:   return 'r';
        case BLACK_QUEEN:  return 'q';
        case BLACK_KING:   return 'k';
        default:           return ' ';
    }
}

constexpr Piece CharToPiece(char pieceChar)
{
    switch (pieceChar)
    {
        case 'P': return WHITE_PAWN;   case 'p': return BLACK_PAWN;
        case 'N': return WHITE_KNIGHT; case 'n': return BLACK_KNIGHT;
        case 'B': return WHITE_BISHOP; case 'b': return BLACK_BISHOP;
        case 'R': return WHITE_ROOK;   case 'r': return BLACK_ROOK;
        case 'Q': return WHITE_QUEEN;  case 'q': return BLACK_QUEEN;
        case 'K': return WHITE_KING;   case 'k': return BLACK_KING;
        default:  return NO_PIECE;
    }
}

// Direction type

enum Direction : int8_t
{
    NORTH = 8,
    EAST  = 1,
    SOUTH = -NORTH,
    WEST  = -EAST,

    NORTH_EAST = NORTH + EAST,
    SOUTH_EAST = SOUTH + EAST,
    SOUTH_WEST = SOUTH + WEST,
    NORTH_WEST = NORTH + WEST,

    NUM_DIRECTIONS = 8
};

// File type

enum File : uint8_t
{
    FILE_A,
    FILE_B,
    FILE_C,
    FILE_D,
    FILE_E,
    FILE_F,
    FILE_G,
    FILE_H,

    NUM_FILES = 8U
};

constexpr Bitboard FileToBitboard(File file)
{
    return Bitboard(0x0101010101010101ULL << file);
}

// Rank type

enum Rank : uint8_t
{
    RANK_1,
    RANK_2,
    RANK_3,
    RANK_4,
    RANK_5,
    RANK_6,
    RANK_7,
    RANK_8,

    NUM_RANKS = 8U
};

constexpr Bitboard RankToBitboard(Rank rank)
{
    return Bitboard(0xFFULL << (int(rank) * NUM_FILES));
}

// Square type

enum Square : uint8_t
{
    SQUARE_A1, SQUARE_B1, SQUARE_C1, SQUARE_D1, SQUARE_E1, SQUARE_F1, SQUARE_G1, SQUARE_H1,
    SQUARE_A2, SQUARE_B2, SQUARE_C2, SQUARE_D2, SQUARE_E2, SQUARE_F2, SQUARE_G2, SQUARE_H2,
    SQUARE_A3, SQUARE_B3, SQUARE_C3, SQUARE_D3, SQUARE_E3, SQUARE_F3, SQUARE_G3, SQUARE_H3,
    SQUARE_A4, SQUARE_B4, SQUARE_C4, SQUARE_D4, SQUARE_E4, SQUARE_F4, SQUARE_G4, SQUARE_H4,
    SQUARE_A5, SQUARE_B5, SQUARE_C5, SQUARE_D5, SQUARE_E5, SQUARE_F5, SQUARE_G5, SQUARE_H5,
    SQUARE_A6, SQUARE_B6, SQUARE_C6, SQUARE_D6, SQUARE_E6, SQUARE_F6, SQUARE_G6, SQUARE_H6,
    SQUARE_A7, SQUARE_B7, SQUARE_C7, SQUARE_D7, SQUARE_E7, SQUARE_F7, SQUARE_G7, SQUARE_H7,
    SQUARE_A8, SQUARE_B8, SQUARE_C8, SQUARE_D8, SQUARE_E8, SQUARE_F8, SQUARE_G8, SQUARE_H8,

    NUM_SQUARES,

    NO_SQUARE = NUM_SQUARES
};

constexpr Square operator+(Square square, Direction direction)
{
    return Square(int(square) + int(direction));
}

constexpr Square operator-(Square square, Direction direction)
{
    return Square(int(square) - int(direction));
}

constexpr Square FileRankToSquare(File file, Rank rank)
{
    return Square((rank << 3) | file);
}

constexpr Bitboard SquareToBitboard(Square square)
{
    return Bitboard(1ULL << square);
}

constexpr File SquareToFile(Square square)
{
    return File(square & 7);
}

constexpr Rank SquareToRank(Square square)
{
    return Rank(square >> 3);
}

inline Square CoordStringToSquare(const std::string& coordString)
{
    return FileRankToSquare(File(coordString[0] - 'a'), Rank(coordString[1] - '1'));
}

inline std::string SquareToCoordString(Square square)
{
    return std::string{ char('a' + SquareToFile(square)), char('1' + SquareToRank(square)) };
}

// CastlingRight type

enum CastlingRight : uint8_t
{
    NO_CASTLING_RIGHTS = 0U,

    WHITE_OO  = (1U << 0),
    WHITE_OOO = (1U << 1),
    BLACK_OO  = (1U << 2),
    BLACK_OOO = (1U << 3),

    ALL_CASTLING_RIGHTS = 0xFU
};

constexpr CastlingRight operator|(CastlingRight a, CastlingRight b)
{
    return CastlingRight(uint8_t(a) | uint8_t(b));
}

constexpr CastlingRight& operator|=(CastlingRight& a, CastlingRight b)
{
    return a = a | b;
}

} // namespace Gluon