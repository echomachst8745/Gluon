#include "pieceplacements.h"

namespace Gluon {

PiecePlacements::PiecePlacements()
{
    squareToBucket.fill(-1);
    squareToIndex.fill(-1);
}

void PiecePlacements::Clear()
{
    for (auto& bucket : pieceBuckets)
    {
        bucket.clear();
    }

    squareToBucket.fill(-1);
    squareToIndex.fill(-1);
}

void PiecePlacements::AddPiece(Piece::Piece piece, int square)
{
    const int bucket = BucketForPiece(piece);

    if (squareToBucket[square] != -1)
    {
        throw std::invalid_argument("Square already occupied when adding piece");
    }

    squareToBucket[square] = static_cast<std::uint8_t>(bucket);
    pieceBuckets[bucket].push_back(square);
    squareToIndex[square] = static_cast<std::uint8_t>(pieceBuckets[bucket].size() - 1);
}

void PiecePlacements::RemovePiece(int square)
{
    const int bucket = squareToBucket[square];
    if (bucket == -1)
    {
        throw std::invalid_argument("No piece at square to remove");
    }

    const int index = squareToIndex[square];
    auto& bucketVector = pieceBuckets[bucket];

    const int lastSquareInBucket = bucketVector.back();
    bucketVector[index] = lastSquareInBucket;
    bucketVector.pop_back();

    squareToIndex[lastSquareInBucket] = static_cast<std::uint8_t>(index);
    squareToBucket[square] = -1;
    squareToIndex[square] = -1;
}

void PiecePlacements::MovePiece(int fromSquare, int toSquare)
{
    const int bucket = squareToBucket[fromSquare];
    if (bucket == -1)
    {
        throw std::invalid_argument("No piece at fromSquare to move");
    }
    if (squareToBucket[toSquare] != -1)
    {
        throw std::invalid_argument("toSquare already occupied when moving piece");
    }

    const int index = squareToIndex[fromSquare];
    auto& bucketVector = pieceBuckets[bucket];

    bucketVector[index] = toSquare;

    squareToBucket[toSquare] = static_cast<std::uint8_t>(bucket);
    squareToIndex[toSquare] = static_cast<std::uint8_t>(index);

    squareToBucket[fromSquare] = -1;
    squareToIndex[fromSquare] = -1;
}

void PiecePlacements::MoveAndChangePiece(int fromSquare, int toSquare, Piece::Piece newPiece)
{
    RemovePiece(fromSquare);
    AddPiece(newPiece, toSquare);
}

int PiecePlacements::BucketForPiece(Piece::Piece piece)
{
    bool isWhite = Piece::IsWhite(piece);
    switch (Piece::GetType(piece))
    {
        case Piece::PAWN:   return isWhite ? WHITE_PAWNS   : BLACK_PAWNS;
        case Piece::KNIGHT: return isWhite ? WHITE_KNIGHTS : BLACK_KNIGHTS;
        case Piece::BISHOP: return isWhite ? WHITE_BISHOPS : BLACK_BISHOPS;
        case Piece::ROOK:   return isWhite ? WHITE_ROOKS   : BLACK_ROOKS;
        case Piece::QUEEN:  return isWhite ? WHITE_QUEENS  : BLACK_QUEENS;
        case Piece::KING:   return isWhite ? WHITE_KING    : BLACK_KING;

        default:
        {
            throw std::invalid_argument("Invalid piece for bucket");
        }
    }
}

} // namespace Gluon