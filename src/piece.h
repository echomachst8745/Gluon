#pragma once

#include <cstdint>
#include <cctype>
#include <stdexcept>

namespace Gluon::Piece {

typedef std::uint8_t Piece;

inline constexpr std::uint8_t NONE   = 0;
inline constexpr std::uint8_t PAWN   = 1 << 0;
inline constexpr std::uint8_t KNIGHT = 1 << 1;
inline constexpr std::uint8_t BISHOP = 1 << 2;
inline constexpr std::uint8_t ROOK   = 1 << 3;
inline constexpr std::uint8_t QUEEN  = 1 << 4;
inline constexpr std::uint8_t KING   = 1 << 5;

inline constexpr std::uint8_t WHITE  = 1 << 6;
inline constexpr std::uint8_t BLACK  = 1 << 7;

inline constexpr std::uint8_t TYPE_MASK  = PAWN | KNIGHT | BISHOP | ROOK | QUEEN | KING;
inline constexpr std::uint8_t COLOUR_MASK = WHITE | BLACK;

inline constexpr std::uint8_t GetType(Piece piece) { return piece & TYPE_MASK; }
inline constexpr bool IsWhite(Piece piece) { return (piece & COLOUR_MASK) == WHITE; }

inline constexpr Piece MakePiece(std::uint8_t type, bool isWhite) { return static_cast<Piece>(type | (isWhite ? WHITE : BLACK)); }

inline constexpr Piece CharToPiece(char c)
{
    bool isWhite = std::isupper(c);
    switch (std::tolower(c))
    {
        case 'p': return MakePiece(PAWN, isWhite);
        case 'n': return MakePiece(KNIGHT, isWhite);
        case 'b': return MakePiece(BISHOP, isWhite);
        case 'r': return MakePiece(ROOK, isWhite);
        case 'q': return MakePiece(QUEEN, isWhite);
        case 'k': return MakePiece(KING, isWhite);

        default:
        {
            throw std::invalid_argument("Invalid piece character");
        }
    }
}

inline constexpr char PieceToChar(Piece piece)
{
    char c = ' ';
    switch (GetType(piece))
    {
        case NONE: break;

        case PAWN:   c = 'p'; break;
        case KNIGHT: c = 'n'; break;
        case BISHOP: c = 'b'; break;
        case ROOK:   c = 'r'; break;
        case QUEEN:  c = 'q'; break;
        case KING:   c = 'k'; break;

        default:
        {
            throw std::invalid_argument("Invalid piece type");
        }
    }

    if (IsWhite(piece))
    {
        c = std::toupper(c);
    }

    return c;
}

} // namespace Gluon::Piece