#pragma once

#include "boardhelpers.h"
#include "piece.h"

#include <vector>
#include <array>
#include <cstdint>

namespace Gluon {

class PiecePlacements
{
public:
    enum PieceBucket : std::uint8_t
    {
        WHITE_PAWNS,
        WHITE_KNIGHTS,
        WHITE_BISHOPS,
        WHITE_ROOKS,
        WHITE_QUEENS,
        WHITE_KING,
        BLACK_PAWNS,
        BLACK_KNIGHTS,
        BLACK_BISHOPS,
        BLACK_ROOKS,
        BLACK_QUEENS,
        BLACK_KING,
        NUM_PIECE_BUCKETS
    };

    PiecePlacements();

    void Clear();

    void AddPiece(Piece::Piece piece, int square);

    void RemovePiece(int square);

    void MovePiece(int fromSquare, int toSquare);

    void MoveAndChangePiece(int fromSquare, int toSquare, Piece::Piece newPiece);

    const std::vector<int>& GetPiecesInBucket(PieceBucket bucket) const { return pieceBuckets[bucket]; };

    const std::vector<int>& GetPawnBucket(bool isWhite)   const { return isWhite ? pieceBuckets[WHITE_PAWNS] : pieceBuckets[BLACK_PAWNS]; }
    const std::vector<int>& GetKnightBucket(bool isWhite) const { return isWhite ? pieceBuckets[WHITE_KNIGHTS] : pieceBuckets[BLACK_KNIGHTS]; }
    const std::vector<int>& GetBishopBucket(bool isWhite) const { return isWhite ? pieceBuckets[WHITE_BISHOPS] : pieceBuckets[BLACK_BISHOPS]; }
    const std::vector<int>& GetRookBucket(bool isWhite)   const { return isWhite ? pieceBuckets[WHITE_ROOKS] : pieceBuckets[BLACK_ROOKS]; }
    const std::vector<int>& GetQueenBucket(bool isWhite)  const { return isWhite ? pieceBuckets[WHITE_QUEENS] : pieceBuckets[BLACK_QUEENS]; }
    const std::vector<int>& GetKingBucket(bool isWhite)   const { return isWhite ? pieceBuckets[WHITE_KING] : pieceBuckets[BLACK_KING]; }

private:
    std::array<std::vector<int>, NUM_PIECE_BUCKETS> pieceBuckets;
    std::array<std::int8_t, BoardHelpers::NUM_SQUARES> squareToBucket;
    std::array<std::int8_t, BoardHelpers::NUM_SQUARES> squareToIndex;

    static int BucketForPiece(Piece::Piece piece);
};

} // namespace Gluon