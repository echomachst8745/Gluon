#pragma once

#include <cstdint>
#include <cctype>

namespace Gluon {

// A piece is represented as a single byte, where the lower 6 bits represent the type of piece and the upper 2 bits represent the colour.
typedef std::uint8_t Piece;

constexpr Piece NONE_PIECE = 0;

// The piece types are represented as bit flags.
enum PieceType : std::uint8_t
{
	NONE   = 0,
	PAWN   = 1 << 0,
	KNIGHT = 1 << 1,
	BISHOP = 1 << 2,
	ROOK   = 1 << 3,
	QUEEN  = 1 << 4,
	KING   = 1 << 5
};

// The piece colours are represented as bit flags.
enum PieceColour : std::uint8_t
{
	WHITE = 1 << 6,
	BLACK = 1 << 7
};

// The piece type mask is used to extract the piece type from a piece.
constexpr std::uint8_t PIECE_TYPE_MASK = PieceType::PAWN | PieceType::KNIGHT |
										 PieceType::BISHOP | PieceType::ROOK |
										 PieceType::QUEEN | PieceType::KING;

// The piece colour mask is used to extract the piece colour from a piece.
constexpr std::uint8_t PIECE_COLOUR_MASK = PieceColour::WHITE | PieceColour::BLACK;

constexpr Piece MakePiece(PieceType type, PieceColour colour)
{
	return static_cast<Piece>(type) | static_cast<Piece>(colour);
}

constexpr PieceType GetPieceType(Piece piece)
{
	return static_cast<PieceType>(piece & PIECE_TYPE_MASK);
}

constexpr bool PieceIsWhite(Piece piece)
{
	return (piece & PIECE_COLOUR_MASK) == PieceColour::WHITE;
}

inline Piece CharToPiece(char c)
{
    PieceColour colour = std::isupper(c) ? PieceColour::WHITE : PieceColour::BLACK;
    switch (std::tolower(c))
    {
        case 'p': return MakePiece(PieceType::PAWN, colour);
        case 'n': return MakePiece(PieceType::KNIGHT, colour);
        case 'b': return MakePiece(PieceType::BISHOP, colour);
        case 'r': return MakePiece(PieceType::ROOK, colour);
        case 'q': return MakePiece(PieceType::QUEEN, colour);
        case 'k': return MakePiece(PieceType::KING, colour);
		default: return MakePiece(PieceType::NONE, colour);
    }
}

constexpr char PieceToChar(Piece piece)
{
    char c = ' ';
    switch (GetPieceType(piece))
    {
        case NONE: break;
        case PAWN:   c = 'p'; break;
        case KNIGHT: c = 'n'; break;
        case BISHOP: c = 'b'; break;
        case ROOK:   c = 'r'; break;
        case QUEEN:  c = 'q'; break;
        case KING:   c = 'k'; break;
		default: break;
    }

    if (PieceIsWhite(piece))
    {
        c = (char)std::toupper((int)c);
    }

    return c;
}

} // namespace Gluon