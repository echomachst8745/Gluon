#pragma once

#include "boardhelpers.h"
#include "piece.h"

#include <cstdint>
#include <array>
#include <vector>

namespace Gluon {

/*
* A structure to hold the current piece placements on the board.
*/
struct PiecePlacementMap
{
private:
	enum PieceBucket
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

		BUCKET_COUNT
	};

	std::array<std::vector<int>, PieceBucket::BUCKET_COUNT> pieceBuckets;
	std::array<PieceBucket, NUM_SQUARES> squareToBucket;
	std::array<int, NUM_SQUARES> squareToIndexInBucket;

	std::array<std::uint64_t, PieceBucket::BUCKET_COUNT> pieceBitboardBuckets;

	const PieceBucket GetBucketForPiece(Piece piece) const;

public:
	PiecePlacementMap();

	void Clear();

	void AddPiece(int square, Piece piece);

	void RemovePiece(int square);

	void MovePiece(int fromSquare, int toSquare);

	void MoveAndChangePiece(int fromSquare, int toSquare, Piece newPiece);

	inline const std::vector<int>& GetPawnPlacements(bool forWhite) const { return forWhite ? pieceBuckets[WHITE_PAWNS] : pieceBuckets[BLACK_PAWNS]; }
	inline const std::vector<int>& GetKnightPlacements(bool forWhite) const { return forWhite ? pieceBuckets[WHITE_KNIGHTS] : pieceBuckets[BLACK_KNIGHTS]; }
	inline const std::vector<int>& GetBishopPlacements(bool forWhite) const { return forWhite ? pieceBuckets[WHITE_BISHOPS] : pieceBuckets[BLACK_BISHOPS]; }
	inline const std::vector<int>& GetRookPlacements(bool forWhite) const { return forWhite ? pieceBuckets[WHITE_ROOKS] : pieceBuckets[BLACK_ROOKS]; }
	inline const std::vector<int>& GetQueenPlacements(bool forWhite) const { return forWhite ? pieceBuckets[WHITE_QUEENS] : pieceBuckets[BLACK_QUEENS]; }
	inline const std::vector<int>& GetKingPlacements(bool forWhite) const { return forWhite ? pieceBuckets[WHITE_KING] : pieceBuckets[BLACK_KING]; }

	inline std::uint64_t GetPawnBitboard(bool forWhite) const { return forWhite ? pieceBitboardBuckets[WHITE_PAWNS] : pieceBitboardBuckets[BLACK_PAWNS]; }
	inline std::uint64_t GetKnightBitboard(bool forWhite) const { return forWhite ? pieceBitboardBuckets[WHITE_KNIGHTS] : pieceBitboardBuckets[BLACK_KNIGHTS]; }
	inline std::uint64_t GetBishopBitboard(bool forWhite) const { return forWhite ? pieceBitboardBuckets[WHITE_BISHOPS] : pieceBitboardBuckets[BLACK_BISHOPS]; }
	inline std::uint64_t GetRookBitboard(bool forWhite) const { return forWhite ? pieceBitboardBuckets[WHITE_ROOKS] : pieceBitboardBuckets[BLACK_ROOKS]; }
	inline std::uint64_t GetQueenBitboard(bool forWhite) const { return forWhite ? pieceBitboardBuckets[WHITE_QUEENS] : pieceBitboardBuckets[BLACK_QUEENS]; }
	inline std::uint64_t GetKingBitboard(bool forWhite) const { return forWhite ? pieceBitboardBuckets[WHITE_KING] : pieceBitboardBuckets[BLACK_KING]; }
};

} // namespace Gluon