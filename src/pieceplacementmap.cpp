#include "pieceplacementmap.h"

#include <stdexcept>

namespace Gluon {

const PiecePlacementMap::PieceBucket PiecePlacementMap::GetBucketForPiece(Piece piece) const
{
	const bool pieceIsWhite = PieceIsWhite(piece);
	switch (GetPieceType(piece))
	{
		case PieceType::PAWN: return pieceIsWhite ? PieceBucket::WHITE_PAWNS : PieceBucket::BLACK_PAWNS;
		case PieceType::KNIGHT: return pieceIsWhite ? PieceBucket::WHITE_KNIGHTS : PieceBucket::BLACK_KNIGHTS;
		case PieceType::BISHOP: return pieceIsWhite ? PieceBucket::WHITE_BISHOPS : PieceBucket::BLACK_BISHOPS;
		case PieceType::ROOK: return pieceIsWhite ? PieceBucket::WHITE_ROOKS : PieceBucket::BLACK_ROOKS;
		case PieceType::QUEEN: return pieceIsWhite ? PieceBucket::WHITE_QUEENS : PieceBucket::BLACK_QUEENS;
		case PieceType::KING: return pieceIsWhite ? PieceBucket::WHITE_KING : PieceBucket::BLACK_KING;
		default: return PieceBucket::BUCKET_COUNT;
	}
}

PiecePlacementMap::PiecePlacementMap()
{
	Clear();
}

void PiecePlacementMap::Clear()
{
	for (auto& bucket : pieceBuckets)
	{
		bucket.clear();
	}
	squareToBucket.fill(PieceBucket::BUCKET_COUNT);
	squareToIndexInBucket.fill(-1);
	pieceBitboardBuckets.fill(0);
}

void PiecePlacementMap::AddPiece(int square, Piece piece)
{
	const PieceBucket bucket = GetBucketForPiece(piece);

	if (squareToBucket[square] != PieceBucket::BUCKET_COUNT)
	{
		throw std::runtime_error("Attempting to add a piece to a square that already has a piece.");
	}

	squareToBucket[square] = bucket;
	pieceBuckets[bucket].push_back(square);
	squareToIndexInBucket[square] = static_cast<int>(pieceBuckets[bucket].size() - 1);

	pieceBitboardBuckets[bucket] |= (1ULL << square);
}

void PiecePlacementMap::RemovePiece(int square)
{
	const PieceBucket bucket = squareToBucket[square];

	if (bucket == PieceBucket::BUCKET_COUNT)
	{
		throw std::runtime_error("Attempting to remove a piece from an empty square.");
	}

	auto& bucketVector = pieceBuckets[bucket];
	const int indexInBucket = squareToIndexInBucket[square];

	const int lastSquareInBucket = bucketVector.back();
	bucketVector[indexInBucket] = lastSquareInBucket;
	bucketVector.pop_back();

	squareToIndexInBucket[lastSquareInBucket] = indexInBucket;
	squareToBucket[square] = PieceBucket::BUCKET_COUNT;
	squareToIndexInBucket[square] = -1;

	pieceBitboardBuckets[bucket] &= ~(1ULL << square);
}

void PiecePlacementMap::MovePiece(int fromSquare, int toSquare)
{
	const PieceBucket fromBucket = squareToBucket[fromSquare];
	const PieceBucket toBucket = squareToBucket[toSquare];

	if (fromBucket == PieceBucket::BUCKET_COUNT)
	{
		throw std::runtime_error("Attempting to move a piece from an empty square.");
	}
	if (toBucket != PieceBucket::BUCKET_COUNT)
	{
		throw std::runtime_error("Attempting to move a piece to a square that already has a piece.");
	}

	auto& bucketVector = pieceBuckets[fromBucket];
	const int indexInBucket = squareToIndexInBucket[fromSquare];

	bucketVector[indexInBucket] = toSquare;

	squareToBucket[toSquare] = fromBucket;
	squareToIndexInBucket[toSquare] = indexInBucket;

	pieceBitboardBuckets[fromBucket] &= ~(1ULL << fromSquare);
	pieceBitboardBuckets[fromBucket] |= (1ULL << toSquare);

	squareToBucket[fromSquare] = PieceBucket::BUCKET_COUNT;
	squareToIndexInBucket[fromSquare] = -1;
}

void PiecePlacementMap::MoveAndChangePiece(int fromSquare, int toSquare, Piece newPiece)
{
	RemovePiece(fromSquare);
	AddPiece(toSquare, newPiece);
}

} // namespace Gluon