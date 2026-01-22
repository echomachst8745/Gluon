#include "boardhelpers.h"

namespace Gluon::BoardHelpers {

const std::string SquareToCoord(int square)
{
    if (!BoardHelpers::IsValidSquare(square))
    {
        throw std::out_of_range("Square index out of range: " + std::to_string(square));
    }

    int file = BoardHelpers::GetFileFromSquare(square);
    int rank = BoardHelpers::GetRankFromSquare(square);

    char fileChar = 'a' + file;
    char rankChar = '1' + rank;

    return std::string(1, fileChar) + std::string(1, rankChar);
}

int CoordToSquare(const std::string& coord)
{
    if (coord.length() != 2)
    {
        throw std::invalid_argument("Invalid coordinate string: " + coord);
    }

    char fileChar = coord[0];
    char rankChar = coord[1];

    if (fileChar < 'a' || fileChar > 'h' || rankChar < '1' || rankChar > '8')
    {
        throw std::invalid_argument("Invalid coordinate string: " + coord);
    }

    int file = fileChar - 'a';
    int rank = rankChar - '1';

    return BoardHelpers::FileRankToSquare(file, rank);
}

} // namespace Gluon::BoardHelpers