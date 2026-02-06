#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <array>

namespace Gluon {

inline static const std::string STARTING_POSITION_FEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

constexpr int RANK_FILE_SIZE = 8;
constexpr int NUM_SQUARES = RANK_FILE_SIZE * RANK_FILE_SIZE;

constexpr int RANK_1 = 0;
constexpr int RANK_2 = 1;
constexpr int RANK_3 = 2;
constexpr int RANK_4 = 3;
constexpr int RANK_5 = 4;
constexpr int RANK_6 = 5;
constexpr int RANK_7 = 6;
constexpr int RANK_8 = 7;

constexpr int FILE_A = 0;
constexpr int FILE_B = 1;
constexpr int FILE_C = 2;
constexpr int FILE_D = 3;
constexpr int FILE_E = 4;
constexpr int FILE_F = 5;
constexpr int FILE_G = 6;
constexpr int FILE_H = 7;

// Direction offsets for a single step in each of the 8 possible directions (N, E, S, W, NE, NW, SE, SW)
constexpr std::array<int, 8> DIRECTION_OFFSETS = { 8, 1, -8, -1, 9, 7, -7, -9 };
constexpr int NORTH_DIRECTION = DIRECTION_OFFSETS[0];
constexpr int EAST_DIRECTION = DIRECTION_OFFSETS[1];
constexpr int SOUTH_DIRECTION = DIRECTION_OFFSETS[2];
constexpr int WEST_DIRECTION = DIRECTION_OFFSETS[3];
constexpr int NORTH_EAST_DIRECTION = DIRECTION_OFFSETS[4];
constexpr int NORTH_WEST_DIRECTION = DIRECTION_OFFSETS[5];
constexpr int SOUTH_EAST_DIRECTION = DIRECTION_OFFSETS[6];
constexpr int SOUTH_WEST_DIRECTION = DIRECTION_OFFSETS[7];
constexpr int NUM_DIRECTIONS = static_cast<int>(DIRECTION_OFFSETS.size());

constexpr std::uint8_t NO_CASTLING_RIGHTS      = 0;
constexpr std::uint8_t WHITE_KING_SIDE_CASTLE  = 1 << 0;
constexpr std::uint8_t WHITE_QUEEN_SIDE_CASTLE = 1 << 1;
constexpr std::uint8_t BLACK_KING_SIDE_CASTLE  = 1 << 2;
constexpr std::uint8_t BLACK_QUEEN_SIDE_CASTLE = 1 << 3;
constexpr std::uint8_t CASTLING_RIGHTS_MASK    = WHITE_KING_SIDE_CASTLE | WHITE_QUEEN_SIDE_CASTLE |
											     BLACK_KING_SIDE_CASTLE | BLACK_QUEEN_SIDE_CASTLE;

constexpr int NO_EN_PASSANT = -1;

constexpr bool IsValidSquare(int square) noexcept
{
    return square >= 0 && square < NUM_SQUARES;
}

constexpr bool IsValidFileRank(int file, int rank) noexcept
{
    return file >= FILE_A && file <= FILE_H && rank >= RANK_1 && rank <= RANK_8;
}

constexpr int FileRankToSquare(int file, int rank)
{
    return rank * RANK_FILE_SIZE + file;
}

constexpr int GetFileFromSquare(int square)
{
    return square % RANK_FILE_SIZE;
}

constexpr int GetRankFromSquare(int square)
{
    return square / RANK_FILE_SIZE;
}

inline const std::string SquareToCoord(int square)
{
    int file = GetFileFromSquare(square);
    int rank = GetRankFromSquare(square);

    char fileChar = 'a' + (char)file;
    char rankChar = '1' + (char)rank;

    return std::string(1, fileChar) + std::string(1, rankChar);
}

inline int CoordToSquare(const std::string& coord)
{
    char fileChar = coord[0];
    char rankChar = coord[1];

    int file = fileChar - 'a';
    int rank = rankChar - '1';

    return FileRankToSquare(file, rank);
}

} // namespace Gluon