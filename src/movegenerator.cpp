#include "boardhelpers.h"
#include "movegenerator.h"

#include <vector>
#include <array>
#include <algorithm>
#include <cmath>

namespace Gluon::MoveGenerator {

namespace { // Anonymous namespace for helper functions

constexpr std::array<std::array<int, 8>, BoardHelpers::NUM_SQUARES> ComputeNumberOfSquaresToEdge()
{
    std::array<std::array<int, 8>, BoardHelpers::NUM_SQUARES> numSquaresToEdge{};

    for (int square = 0; square < BoardHelpers::NUM_SQUARES; ++square)
    {
        int rank = BoardHelpers::GetRankFromSquare(square);
        int file = BoardHelpers::GetFileFromSquare(square);

        int numNorth = BoardHelpers::RANK_8 - rank;
        int numSouth = rank;
        int numEast  = BoardHelpers::FILE_H - file;
        int numWest  = file;

        numSquaresToEdge[square][0] = numNorth;
        numSquaresToEdge[square][1] = numSouth;
        numSquaresToEdge[square][2] = numEast;
        numSquaresToEdge[square][3] = numWest;
        numSquaresToEdge[square][4] = std::min(numNorth, numWest); // NW
        numSquaresToEdge[square][5] = std::min(numSouth, numEast); // SE
        numSquaresToEdge[square][6] = std::min(numNorth, numEast); // NE
        numSquaresToEdge[square][7] = std::min(numSouth, numWest); // SW
    }

    return numSquaresToEdge;
}

inline constexpr auto NUMBER_OF_SQUARES_TO_EDGE = ComputeNumberOfSquaresToEdge();

constexpr std::array<std::array<int, 8>, BoardHelpers::NUM_SQUARES> ComputeKnightMoves()
{
    std::array<std::array<int, 8>, BoardHelpers::NUM_SQUARES> knightMoves{};

    constexpr int validFileOffsets[8] = { 1, 2, 2, 1, -1, -2, -2, -1 };
    constexpr int validRankOffsets[8] = { 2, 1, -1, -2, -2, -1, 1, 2 };

    for (int square = 0; square < BoardHelpers::NUM_SQUARES; ++square)
    {
        const int fromRank = BoardHelpers::GetRankFromSquare(square);
        const int fromFile = BoardHelpers::GetFileFromSquare(square);

        int moveIndex = 0;
        for (int i = 0; i < 8; ++i)
        {
            const int toFile = fromFile + validFileOffsets[i];
            const int toRank = fromRank + validRankOffsets[i];

            if (toFile < BoardHelpers::FILE_A || toFile > BoardHelpers::FILE_H || toRank < BoardHelpers::RANK_1 || toRank > BoardHelpers::RANK_8) continue;

            knightMoves[square][moveIndex++] = BoardHelpers::FileRankToSquare(toFile, toRank);
        }

        // Fill remaining moves with -1
        for (; moveIndex < 8; ++moveIndex)
        {
            knightMoves[square][moveIndex] = -1;
        }
    }

    return knightMoves;
}

inline constexpr auto KNIGHT_MOVES = ComputeKnightMoves();

constexpr std::array<std::array<int, 8>, BoardHelpers::NUM_SQUARES> ComputeKingMoves()
{
    std::array<std::array<int, 8>, BoardHelpers::NUM_SQUARES> kingMoves{};

    constexpr int validFileOffsets[8] = { -1, 0, 1, -1, 1, -1, 0, 1 };
    constexpr int validRankOffsets[8] = { 1, 1, 1, 0, 0, -1, -1, -1 };

    for (int square = 0; square < BoardHelpers::NUM_SQUARES; ++square)
    {
        const int fromRank = BoardHelpers::GetRankFromSquare(square);
        const int fromFile = BoardHelpers::GetFileFromSquare(square);

        int moveIndex = 0;
        for (int i = 0; i < 8; ++i)
        {
            const int toFile = fromFile + validFileOffsets[i];
            const int toRank = fromRank + validRankOffsets[i];

            if (toFile < BoardHelpers::FILE_A || toFile > BoardHelpers::FILE_H || toRank < BoardHelpers::RANK_1 || toRank > BoardHelpers::RANK_8) continue;

            kingMoves[square][moveIndex++] = BoardHelpers::FileRankToSquare(toFile, toRank);
        }

        // Fill remaining moves with -1
        for (; moveIndex < 8; ++moveIndex)
        {
            kingMoves[square][moveIndex] = -1;
        }
    }

    return kingMoves;
}

inline constexpr auto KING_MOVES = ComputeKingMoves();

void GeneratePawnMovesForPiece(const Board& board, Piece::Piece piece, int fromSquare, std::vector<Move>& moves)
{
    const bool pieceIsWhite = Piece::IsWhite(piece);

    const int fromFile = BoardHelpers::GetFileFromSquare(fromSquare);
    const int fromRank = BoardHelpers::GetRankFromSquare(fromSquare);

    const int direction = pieceIsWhite ? NORTH_DIRECTION : SOUTH_DIRECTION;
    
    // Single pawn push
    {
        const int toSquare = fromSquare + direction;

        if (BoardHelpers::IsValidSquare(toSquare) && board.GetPieceAtSquare(toSquare) == Piece::NONE)
        {
            // Check for promotion
            const int toRank = BoardHelpers::GetRankFromSquare(toSquare);
            if ((pieceIsWhite && toRank == BoardHelpers::RANK_8) ||
                (!pieceIsWhite && toRank == BoardHelpers::RANK_1))
            {
                moves.emplace_back(fromSquare, toSquare, MoveValues::QUEEN_PROMOTION);
                moves.emplace_back(fromSquare, toSquare, MoveValues::ROOK_PROMOTION);
                moves.emplace_back(fromSquare, toSquare, MoveValues::BISHOP_PROMOTION);
                moves.emplace_back(fromSquare, toSquare, MoveValues::KNIGHT_PROMOTION);
            }
            else
            {
                moves.emplace_back(fromSquare, toSquare, MoveValues::QUIET_MOVE);
            }
        }
    }

    // Double pawn push
    if ((pieceIsWhite && fromRank == BoardHelpers::RANK_2) ||
        (!pieceIsWhite && fromRank == BoardHelpers::RANK_7))
    {
        const int intermediateSquare = fromSquare + direction;
        const int toSquare = intermediateSquare + direction;

        if (BoardHelpers::IsValidSquare(toSquare) &&
            board.GetPieceAtSquare(intermediateSquare) == Piece::NONE &&
            board.GetPieceAtSquare(toSquare) == Piece::NONE)
        {
            moves.emplace_back(fromSquare, toSquare, MoveValues::DOUBLE_PAWN_PUSH);
        }
    }

    // Captures
    const int captureOffsets[2] = {
        direction + WEST_DIRECTION,
        direction + EAST_DIRECTION
    };

    for (int offset : captureOffsets)
    {
        const int toSquare = fromSquare + offset;

        if (!BoardHelpers::IsValidSquare(toSquare)) { continue; }

        const int toFile = BoardHelpers::GetFileFromSquare(toSquare);

        // File wrap around check
        if (std::abs(toFile - fromFile) != 1) continue;

        // En passant
        if (toSquare == board.GetEnPassantSquare())
        {
            moves.emplace_back(fromSquare, toSquare, MoveValues::EN_PASSANT);
            continue;
        }
        
        Piece::Piece targetPiece = board.GetPieceAtSquare(toSquare);
        if (targetPiece != Piece::NONE && Piece::IsWhite(targetPiece) != pieceIsWhite)
        {
            // Check for promotion
            const int toRank = BoardHelpers::GetRankFromSquare(toSquare);
            if ((pieceIsWhite && toRank == BoardHelpers::RANK_8) ||
                (!pieceIsWhite && toRank == BoardHelpers::RANK_1))
            {
                moves.emplace_back(fromSquare, toSquare, MoveValues::QUEEN_PROMOTION_CAPTURE);
                moves.emplace_back(fromSquare, toSquare, MoveValues::ROOK_PROMOTION_CAPTURE);
                moves.emplace_back(fromSquare, toSquare, MoveValues::BISHOP_PROMOTION_CAPTURE);
                moves.emplace_back(fromSquare, toSquare, MoveValues::KNIGHT_PROMOTION_CAPTURE);
            }
            else
            {
                moves.emplace_back(fromSquare, toSquare, MoveValues::CAPTURE);
            }
        }
    }
}

void GenerateKnightMovesForPiece(const Board& board, Piece::Piece piece, int fromSquare, std::vector<Move>& moves)
{
    const bool pieceIsWhite = Piece::IsWhite(piece);

    for (int toSquare : KNIGHT_MOVES[fromSquare])
    {
        if (toSquare == -1) break;

        Piece::Piece targetPiece = board.GetPieceAtSquare(toSquare);
        if (targetPiece == Piece::NONE)
        {
            moves.emplace_back(fromSquare, toSquare, MoveValues::QUIET_MOVE);
        }
        else if (Piece::IsWhite(targetPiece) != pieceIsWhite)
        {
            moves.emplace_back(fromSquare, toSquare, MoveValues::CAPTURE);
        }
    }
}

void GenerateSlidingMovesForPiece(const Board& board, Piece::Piece piece, int fromSquare, std::vector<Move>& moves)
{
    const bool pieceIsWhite = Piece::IsWhite(piece);

    const int directionsStartIndex = (Piece::GetType(piece) == Piece::BISHOP) ? 4 : 0;
    const int directionsEndIndex   = (Piece::GetType(piece) == Piece::ROOK)   ? 4 : 8;

    for (int directionIndex = directionsStartIndex; directionIndex < directionsEndIndex; ++directionIndex)
    {
        const int directionOffset = DIRECTION_OFFSETS[directionIndex];
        const int maxSteps = NUMBER_OF_SQUARES_TO_EDGE[fromSquare][directionIndex];

        for (int step = 1; step <= maxSteps; ++step)
        {
            const int toSquare = fromSquare + step * directionOffset;

            Piece::Piece targetPiece = board.GetPieceAtSquare(toSquare);
            if (targetPiece == Piece::NONE)
            {
                moves.emplace_back(fromSquare, toSquare, MoveValues::QUIET_MOVE);
            }
            else
            {
                if (Piece::IsWhite(targetPiece) != pieceIsWhite)
                {
                    moves.emplace_back(fromSquare, toSquare, MoveValues::CAPTURE);
                }
                break; // Blocked by a piece
            }
        }
    }
}

void GenerateKingMovesForPiece(const Board& board, Piece::Piece piece, int fromSquare, std::vector<Move>& moves)
{
    const bool pieceIsWhite = Piece::IsWhite(piece);

    // Quiet moves and captures
    for (int toSquare : KING_MOVES[fromSquare])
    {
        if (toSquare == -1) break;

        Piece::Piece targetPiece = board.GetPieceAtSquare(toSquare);
        if (targetPiece == Piece::NONE)
        {
            moves.emplace_back(fromSquare, toSquare, MoveValues::QUIET_MOVE);
        }
        else if (Piece::IsWhite(targetPiece) != pieceIsWhite)
        {
            moves.emplace_back(fromSquare, toSquare, MoveValues::CAPTURE);
        }
    }

    // Castling moves
    if (pieceIsWhite && board.HasCastlingRight(BoardHelpers::WHITE_KING_SIDE_CASTLE))
    {
        // White kingside castling
        if (board.GetPieceAtSquare(BoardHelpers::FileRankToSquare(BoardHelpers::FILE_F, BoardHelpers::RANK_1)) == Piece::NONE &&
            board.GetPieceAtSquare(BoardHelpers::FileRankToSquare(BoardHelpers::FILE_G, BoardHelpers::RANK_1)) == Piece::NONE)
        {
            moves.emplace_back(fromSquare, BoardHelpers::FileRankToSquare(BoardHelpers::FILE_G, BoardHelpers::RANK_1), MoveValues::KING_CASTLE);
        }
    }
    if (pieceIsWhite && board.HasCastlingRight(BoardHelpers::WHITE_QUEEN_SIDE_CASTLE))
    {
        // White queenside castling
        if (board.GetPieceAtSquare(BoardHelpers::FileRankToSquare(BoardHelpers::FILE_B, BoardHelpers::RANK_1)) == Piece::NONE &&
            board.GetPieceAtSquare(BoardHelpers::FileRankToSquare(BoardHelpers::FILE_C, BoardHelpers::RANK_1)) == Piece::NONE &&
            board.GetPieceAtSquare(BoardHelpers::FileRankToSquare(BoardHelpers::FILE_D, BoardHelpers::RANK_1)) == Piece::NONE)
        {
            moves.emplace_back(fromSquare, BoardHelpers::FileRankToSquare(BoardHelpers::FILE_C, BoardHelpers::RANK_1), MoveValues::QUEEN_CASTLE);
        }
    }
    if (!pieceIsWhite && board.HasCastlingRight(BoardHelpers::BLACK_KING_SIDE_CASTLE))
    {
        // Black kingside castling
        if (board.GetPieceAtSquare(BoardHelpers::FileRankToSquare(BoardHelpers::FILE_F, BoardHelpers::RANK_8)) == Piece::NONE &&
            board.GetPieceAtSquare(BoardHelpers::FileRankToSquare(BoardHelpers::FILE_G, BoardHelpers::RANK_8)) == Piece::NONE)
        {
            moves.emplace_back(fromSquare, BoardHelpers::FileRankToSquare(BoardHelpers::FILE_G, BoardHelpers::RANK_8), MoveValues::KING_CASTLE);
        }
    }
    if (!pieceIsWhite && board.HasCastlingRight(BoardHelpers::BLACK_QUEEN_SIDE_CASTLE))
    {
        // Black queenside castling
        if (board.GetPieceAtSquare(BoardHelpers::FileRankToSquare(BoardHelpers::FILE_B, BoardHelpers::RANK_8)) == Piece::NONE &&
            board.GetPieceAtSquare(BoardHelpers::FileRankToSquare(BoardHelpers::FILE_C, BoardHelpers::RANK_8)) == Piece::NONE &&
            board.GetPieceAtSquare(BoardHelpers::FileRankToSquare(BoardHelpers::FILE_D, BoardHelpers::RANK_8)) == Piece::NONE)
        {
            moves.emplace_back(fromSquare, BoardHelpers::FileRankToSquare(BoardHelpers::FILE_C, BoardHelpers::RANK_8), MoveValues::QUEEN_CASTLE);
        }
    }
}

} // Anonymous namespace for helper functions

std::vector<Move> GenerateMoves(const Board& board)
{
    std::vector<Move> moves;
    moves.reserve(256); // Preallocate space for moves
    moves.clear();

    const bool isWhiteToMove = board.IsWhiteToMove();

    for (int pieceSquare : board.GetPawnPlacements(isWhiteToMove))
    {
        Piece::Piece piece = board.GetPieceAtSquare(pieceSquare);
        GeneratePawnMovesForPiece(board, piece, pieceSquare, moves);
    }
    for (int pieceSquare : board.GetKnightPlacements(isWhiteToMove))
    {
        Piece::Piece piece = board.GetPieceAtSquare(pieceSquare);
        GenerateKnightMovesForPiece(board, piece, pieceSquare, moves);
    }
    for (int pieceSquare : board.GetBishopPlacements(isWhiteToMove))
    {
        Piece::Piece piece = board.GetPieceAtSquare(pieceSquare);
        GenerateSlidingMovesForPiece(board, piece, pieceSquare, moves);
    }
    for (int pieceSquare : board.GetRookPlacements(isWhiteToMove))
    {
        Piece::Piece piece = board.GetPieceAtSquare(pieceSquare);
        GenerateSlidingMovesForPiece(board, piece, pieceSquare, moves);
    }
    for (int pieceSquare : board.GetQueenPlacements(isWhiteToMove))
    {
        Piece::Piece piece = board.GetPieceAtSquare(pieceSquare);
        GenerateSlidingMovesForPiece(board, piece, pieceSquare, moves);
    }
    {
        int pieceSquare = board.GetKingSquare(isWhiteToMove);
        Piece::Piece piece = board.GetPieceAtSquare(pieceSquare);
        GenerateKingMovesForPiece(board, piece, pieceSquare, moves);
    }

    return moves;
}

} // namespace Gluon::MoveGenerator