#include "boardhelpers.h"
#include "movegenerator.h"
#include "legalmoveinfo.h"

#include <vector>
#include <array>
#include <algorithm>
#include <cmath>
#include <iostream>

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

void GeneratePawnMovesForPiece(const Board& board, Piece::Piece piece, int fromSquare, MoveList& moves)
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
                moves.AddMove(Move(fromSquare, toSquare, MoveValues::QUEEN_PROMOTION));
                moves.AddMove(Move(fromSquare, toSquare, MoveValues::ROOK_PROMOTION));
                moves.AddMove(Move(fromSquare, toSquare, MoveValues::BISHOP_PROMOTION));
                moves.AddMove(Move(fromSquare, toSquare, MoveValues::KNIGHT_PROMOTION));
            }
            else
            {
                moves.AddMove(Move(fromSquare, toSquare, MoveValues::QUIET_MOVE));
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
            moves.AddMove(Move(fromSquare, toSquare, MoveValues::DOUBLE_PAWN_PUSH));
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
            moves.AddMove(Move(fromSquare, toSquare, MoveValues::EN_PASSANT));
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
                moves.AddMove(Move(fromSquare, toSquare, MoveValues::QUEEN_PROMOTION_CAPTURE));
                moves.AddMove(Move(fromSquare, toSquare, MoveValues::ROOK_PROMOTION_CAPTURE));
                moves.AddMove(Move(fromSquare, toSquare, MoveValues::BISHOP_PROMOTION_CAPTURE));
                moves.AddMove(Move(fromSquare, toSquare, MoveValues::KNIGHT_PROMOTION_CAPTURE));
            }
            else
            {
                moves.AddMove(Move(fromSquare, toSquare, MoveValues::CAPTURE));
            }
        }
    }
}

void GenerateKnightMovesForPiece(const Board& board, Piece::Piece piece, int fromSquare, MoveList& moves)
{
    const bool pieceIsWhite = Piece::IsWhite(piece);

    for (int toSquare : KNIGHT_MOVES[fromSquare])
    {
        if (toSquare == -1) break;

        Piece::Piece targetPiece = board.GetPieceAtSquare(toSquare);
        if (targetPiece == Piece::NONE)
        {
            moves.AddMove(Move(fromSquare, toSquare, MoveValues::QUIET_MOVE));
        }
        else if (Piece::IsWhite(targetPiece) != pieceIsWhite)
        {
            moves.AddMove(Move(fromSquare, toSquare, MoveValues::CAPTURE));
        }
    }
}

void GenerateSlidingMovesForPiece(const Board& board, Piece::Piece piece, int fromSquare, MoveList& moves)
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
                moves.AddMove(Move(fromSquare, toSquare, MoveValues::QUIET_MOVE));
            }
            else
            {
                if (Piece::IsWhite(targetPiece) != pieceIsWhite)
                {
                    moves.AddMove(Move(fromSquare, toSquare, MoveValues::CAPTURE));
                }
                break; // Blocked by a piece
            }
        }
    }
}

void GenerateKingMovesForPiece(const Board& board, Piece::Piece piece, int fromSquare, MoveList& moves)
{
    const bool pieceIsWhite = Piece::IsWhite(piece);

    // Quiet moves and captures
    for (int toSquare : KING_MOVES[fromSquare])
    {
        if (toSquare == -1) break;

        Piece::Piece targetPiece = board.GetPieceAtSquare(toSquare);
        if (targetPiece == Piece::NONE)
        {
            moves.AddMove(Move(fromSquare, toSquare, MoveValues::QUIET_MOVE));
        }
        else if (Piece::IsWhite(targetPiece) != pieceIsWhite)
        {
            moves.AddMove(Move(fromSquare, toSquare, MoveValues::CAPTURE));
        }
    }

    // Castling moves
    if (pieceIsWhite && board.HasCastlingRight(BoardHelpers::WHITE_KING_SIDE_CASTLE))
    {
        // White kingside castling
        if (board.GetPieceAtSquare(BoardHelpers::FileRankToSquare(BoardHelpers::FILE_F, BoardHelpers::RANK_1)) == Piece::NONE &&
            board.GetPieceAtSquare(BoardHelpers::FileRankToSquare(BoardHelpers::FILE_G, BoardHelpers::RANK_1)) == Piece::NONE)
        {
            moves.AddMove(Move(fromSquare, BoardHelpers::FileRankToSquare(BoardHelpers::FILE_G, BoardHelpers::RANK_1), MoveValues::KING_CASTLE));
        }
    }
    if (pieceIsWhite && board.HasCastlingRight(BoardHelpers::WHITE_QUEEN_SIDE_CASTLE))
    {
        // White queenside castling
        if (board.GetPieceAtSquare(BoardHelpers::FileRankToSquare(BoardHelpers::FILE_B, BoardHelpers::RANK_1)) == Piece::NONE &&
            board.GetPieceAtSquare(BoardHelpers::FileRankToSquare(BoardHelpers::FILE_C, BoardHelpers::RANK_1)) == Piece::NONE &&
            board.GetPieceAtSquare(BoardHelpers::FileRankToSquare(BoardHelpers::FILE_D, BoardHelpers::RANK_1)) == Piece::NONE)
        {
            moves.AddMove(Move(fromSquare, BoardHelpers::FileRankToSquare(BoardHelpers::FILE_C, BoardHelpers::RANK_1), MoveValues::QUEEN_CASTLE));
        }
    }
    if (!pieceIsWhite && board.HasCastlingRight(BoardHelpers::BLACK_KING_SIDE_CASTLE))
    {
        // Black kingside castling
        if (board.GetPieceAtSquare(BoardHelpers::FileRankToSquare(BoardHelpers::FILE_F, BoardHelpers::RANK_8)) == Piece::NONE &&
            board.GetPieceAtSquare(BoardHelpers::FileRankToSquare(BoardHelpers::FILE_G, BoardHelpers::RANK_8)) == Piece::NONE)
        {
            moves.AddMove(Move(fromSquare, BoardHelpers::FileRankToSquare(BoardHelpers::FILE_G, BoardHelpers::RANK_8), MoveValues::KING_CASTLE));
        }
    }
    if (!pieceIsWhite && board.HasCastlingRight(BoardHelpers::BLACK_QUEEN_SIDE_CASTLE))
    {
        // Black queenside castling
        if (board.GetPieceAtSquare(BoardHelpers::FileRankToSquare(BoardHelpers::FILE_B, BoardHelpers::RANK_8)) == Piece::NONE &&
            board.GetPieceAtSquare(BoardHelpers::FileRankToSquare(BoardHelpers::FILE_C, BoardHelpers::RANK_8)) == Piece::NONE &&
            board.GetPieceAtSquare(BoardHelpers::FileRankToSquare(BoardHelpers::FILE_D, BoardHelpers::RANK_8)) == Piece::NONE)
        {
            moves.AddMove(Move(fromSquare, BoardHelpers::FileRankToSquare(BoardHelpers::FILE_C, BoardHelpers::RANK_8), MoveValues::QUEEN_CASTLE));
        }
    }
}

LegalMoveInfo GenerateLegalMoveInfo(const Board& board)
{
    LegalMoveInfo legalMoveInfo;

    const bool isWhiteToMove = board.IsWhiteToMove();
    const int friendlyKingSquare = board.GetKingSquare(isWhiteToMove);

    // Calculate pawn attacks
    const int pawnAttackOffsets[2] = {
        isWhiteToMove ? SOUTH_WEST_DIRECTION : NORTH_WEST_DIRECTION,
        isWhiteToMove ? SOUTH_EAST_DIRECTION : NORTH_EAST_DIRECTION
    };
    for (int pieceSquare : board.GetPawnPlacements(!isWhiteToMove))
    {
        for (int offset : pawnAttackOffsets)
        {
            const int attackSquare = pieceSquare + offset;

            if (!BoardHelpers::IsValidSquare(attackSquare)) { continue; }

            const int fromFile = BoardHelpers::GetFileFromSquare(pieceSquare);
            const int toFile   = BoardHelpers::GetFileFromSquare(attackSquare);

            // File wrap around check
            if (std::abs(toFile - fromFile) != 1) continue;

            legalMoveInfo.SetSquareAsAttacked(attackSquare);

            if (attackSquare == friendlyKingSquare)
            {
                std::uint64_t removeBlockCheckBitboard = (1ULL << pieceSquare);

                int possibleEnPassantSquare = pieceSquare + (isWhiteToMove ? NORTH_DIRECTION : SOUTH_DIRECTION);
                if (possibleEnPassantSquare == board.GetEnPassantSquare())
                {
                    // En passant capture also attacks the king
                    removeBlockCheckBitboard |= (1ULL << possibleEnPassantSquare);
                }

                legalMoveInfo.AddChecker(pieceSquare, removeBlockCheckBitboard);
            }
        }
    }

    // Calculate knight attacks
    for (int pieceSquare : board.GetKnightPlacements(!isWhiteToMove))
    {
        for (int attackSquare : KNIGHT_MOVES[pieceSquare])
        {
            if (attackSquare == -1) break;

            legalMoveInfo.SetSquareAsAttacked(attackSquare);

            if (attackSquare == friendlyKingSquare)
            {
                legalMoveInfo.AddChecker(pieceSquare, (1ULL << pieceSquare));
            }
        }
    }

    // Calculate bishop attacks
    for (int pieceSquare : board.GetBishopPlacements(!isWhiteToMove))
    {
        const int directionsStartIndex = 4;
        const int directionsEndIndex   = 8;

        for (int directionIndex = directionsStartIndex; directionIndex < directionsEndIndex; ++directionIndex)
        {
            const int directionOffset = DIRECTION_OFFSETS[directionIndex];
            const int maxSteps = NUMBER_OF_SQUARES_TO_EDGE[pieceSquare][directionIndex];

            std::uint64_t removeBlockCheckBitboard = (1ULL << pieceSquare);
            bool kingCheckFound = false;

            for (int step = 1; step <= maxSteps; ++step)
            {
                const int attackSquare = pieceSquare + step * directionOffset;

                removeBlockCheckBitboard |= (1ULL << attackSquare);

                legalMoveInfo.SetSquareAsAttacked(attackSquare);

                if (attackSquare == friendlyKingSquare && !kingCheckFound)
                {
                    legalMoveInfo.AddChecker(pieceSquare, removeBlockCheckBitboard);
                    kingCheckFound = true;

                    continue;
                }

                Piece::Piece targetPiece = board.GetPieceAtSquare(attackSquare);
                if (targetPiece != Piece::NONE)
                {
                    break; // Blocked by a piece
                }
            }
        }
    }

    // Calculate rook attacks
    for (int pieceSquare : board.GetRookPlacements(!isWhiteToMove))
    {
        const int directionsStartIndex = 0;
        const int directionsEndIndex   = 4;

        for (int directionIndex = directionsStartIndex; directionIndex < directionsEndIndex; ++directionIndex)
        {
            const int directionOffset = DIRECTION_OFFSETS[directionIndex];
            const int maxSteps = NUMBER_OF_SQUARES_TO_EDGE[pieceSquare][directionIndex];

            std::uint64_t removeBlockCheckBitboard = (1ULL << pieceSquare);
            bool kingCheckFound = false;

            for (int step = 1; step <= maxSteps; ++step)
            {
                const int attackSquare = pieceSquare + step * directionOffset;

                removeBlockCheckBitboard |= (1ULL << attackSquare);

                legalMoveInfo.SetSquareAsAttacked(attackSquare);

                if (attackSquare == friendlyKingSquare && !kingCheckFound)
                {
                    legalMoveInfo.AddChecker(pieceSquare, removeBlockCheckBitboard);
                    kingCheckFound = true;
                    continue;
                }

                Piece::Piece targetPiece = board.GetPieceAtSquare(attackSquare);
                if (targetPiece != Piece::NONE)
                {
                    break; // Blocked by a piece
                }
            }
        }
    }

    // Calculate queen attacks
    for (int pieceSquare : board.GetQueenPlacements(!isWhiteToMove))
    {
        const int directionsStartIndex = 0;
        const int directionsEndIndex   = 8;

        for (int directionIndex = directionsStartIndex; directionIndex < directionsEndIndex; ++directionIndex)
        {
            const int directionOffset = DIRECTION_OFFSETS[directionIndex];
            const int maxSteps = NUMBER_OF_SQUARES_TO_EDGE[pieceSquare][directionIndex];

            std::uint64_t removeBlockCheckBitboard = (1ULL << pieceSquare);
            bool kingCheckFound = false;

            for (int step = 1; step <= maxSteps; ++step)
            {
                const int attackSquare = pieceSquare + step * directionOffset;

                removeBlockCheckBitboard |= (1ULL << attackSquare);

                legalMoveInfo.SetSquareAsAttacked(attackSquare);

                if (attackSquare == friendlyKingSquare && !kingCheckFound)
                {
                    legalMoveInfo.AddChecker(pieceSquare, removeBlockCheckBitboard);
                    kingCheckFound = true;
                    continue;
                }

                Piece::Piece targetPiece = board.GetPieceAtSquare(attackSquare);
                if (targetPiece != Piece::NONE)
                {
                    break; // Blocked by a piece
                }
            }
        }
    }

    // Calculate king attacks
    {
        int pieceSquare = board.GetKingSquare(!isWhiteToMove);
        for (int attackSquare : KING_MOVES[pieceSquare])
        {
            if (attackSquare == -1) break;

            legalMoveInfo.SetSquareAsAttacked(attackSquare);
        }
    }

    // Calculate pinned pieces
    for (int directionIndex = 0; directionIndex < 8; ++directionIndex)
    {
        const int directionOffset = DIRECTION_OFFSETS[directionIndex];
        const int maxSteps = NUMBER_OF_SQUARES_TO_EDGE[friendlyKingSquare][directionIndex];

        int potentialPinnedPieceSquare = -1;
        std::uint64_t pinnedPieceLegalMovesBitboard = 0ULL;

        for (int step = 1; step <= maxSteps; ++step)
        {
            const int scanSquare = friendlyKingSquare + step * directionOffset;

            pinnedPieceLegalMovesBitboard |= (1ULL << scanSquare);

            Piece::Piece targetPiece = board.GetPieceAtSquare(scanSquare);
            if (targetPiece == Piece::NONE)
            {
                continue;
            }

            if (Piece::IsWhite(targetPiece) == isWhiteToMove)
            {
                // Friendly piece
                if (potentialPinnedPieceSquare == -1)
                {
                    potentialPinnedPieceSquare = scanSquare;
                }
                else
                {
                    break; // Second friendly piece blocks the line
                }
            }
            else
            {
                // Opponent piece
                auto targetType = Piece::GetType(targetPiece);
                bool isSlidingPiece = (targetType == Piece::QUEEN) ||
                                      (targetType == Piece::ROOK && directionIndex < 4) ||
                                      (targetType == Piece::BISHOP && directionIndex >= 4);

                if (isSlidingPiece && potentialPinnedPieceSquare != -1)
                {
                    // Found a pinned piece
                    legalMoveInfo.SetPiecePinned(potentialPinnedPieceSquare, pinnedPieceLegalMovesBitboard);
                }
                break; // Encountered opponent piece, stop scanning
            }
        }
    }

    return legalMoveInfo;
}

bool MoveIsIllegal(const Board& board, const LegalMoveInfo& legalMoveInfo, const MoveGenerator::Move& move) noexcept
{
    const bool isWhiteToMove = board.IsWhiteToMove();
    const int fromSquare = move.GetFromSquare();
    const int toSquare = move.GetToSquare();
    const Piece::Piece movingPiece = board.GetPieceAtSquare(fromSquare);
    const int friendlyKingSquare = board.GetKingSquare(isWhiteToMove);

    // If in double check, only king moves are legal
    if (legalMoveInfo.KingInDoubleCheck())
    {
        if (Piece::GetType(movingPiece) != Piece::KING)
        {
            return true; // Illegal move
        }
    }

    // King cannot castle out of, through, or into check
    if (move.HasFlag(MoveValues::KING_CASTLE))
    {
        if (legalMoveInfo.KingInCheck())
        {
            return true; // Illegal move
        }

        if (isWhiteToMove)
        {
            // White kingside castling
            if (legalMoveInfo.attackedSquaresBitboard & ((1ULL << BoardHelpers::FileRankToSquare(BoardHelpers::FILE_F, BoardHelpers::RANK_1)) |
                                                         (1ULL << BoardHelpers::FileRankToSquare(BoardHelpers::FILE_G, BoardHelpers::RANK_1))))
            {
                return true; // Illegal move
            }
        }
        else
        {
            // Black kingside castling
            if (legalMoveInfo.attackedSquaresBitboard & ((1ULL << BoardHelpers::FileRankToSquare(BoardHelpers::FILE_F, BoardHelpers::RANK_8)) |
                                                         (1ULL << BoardHelpers::FileRankToSquare(BoardHelpers::FILE_G, BoardHelpers::RANK_8))))
            {
                return true; // Illegal move
            }
        }
    }
    else if (move.HasFlag(MoveValues::QUEEN_CASTLE))
    {
        if (legalMoveInfo.KingInCheck())
        {
            return true; // Illegal move
        }

        if (isWhiteToMove)
        {
            // White queenside castling
            if (legalMoveInfo.attackedSquaresBitboard & ((1ULL << BoardHelpers::FileRankToSquare(BoardHelpers::FILE_D, BoardHelpers::RANK_1)) |
                                                         (1ULL << BoardHelpers::FileRankToSquare(BoardHelpers::FILE_C, BoardHelpers::RANK_1))))
            {
                return true; // Illegal move
            }
        }
        else
        {
            // Black queenside castling
            if (legalMoveInfo.attackedSquaresBitboard & ((1ULL << BoardHelpers::FileRankToSquare(BoardHelpers::FILE_D, BoardHelpers::RANK_8)) |
                                                         (1ULL << BoardHelpers::FileRankToSquare(BoardHelpers::FILE_C, BoardHelpers::RANK_8))))
            {
                return true; // Illegal move
            }
        }
    }

    // If the moving piece is pinned, check if the move is along the pin line
    if (legalMoveInfo.pinnedPiecesBitboard & (1ULL << fromSquare))
    {
        std::uint64_t legalMovesBitboard = legalMoveInfo.pinnedPieceLegalMovesBitboards[fromSquare];
        if (!(legalMovesBitboard & (1ULL << toSquare)))
        {
            return true; // Illegal move
        }
    }

    // If in single check, the move must block or capture the checker
    if (legalMoveInfo.KingInCheck() && Piece::GetType(movingPiece) != Piece::KING)
    {
        if (Piece::GetType(movingPiece) != Piece::PAWN)
        {
            if (toSquare == board.GetEnPassantSquare())
            {
                return true; // Illegal move
            }
        }

        if (!(legalMoveInfo.removeBlockCheckBitboard & (1ULL << toSquare)))
        {
            return true; // Illegal move
        }
    }

    if (Piece::GetType(movingPiece) == Piece::KING)
    {
        // King cannot move to an attacked square
        if (legalMoveInfo.attackedSquaresBitboard & (1ULL << toSquare))
        {
            return true; // Illegal move
        }
    }

    if (move.HasFlag(MoveValues::EN_PASSANT))
    {
        auto boardSquares = board.GetSquares();

        boardSquares[move.GetFromSquare()] = Piece::NONE;
        boardSquares[move.GetToSquare()] = movingPiece;
        const int capturedPawnSquare = isWhiteToMove ? (move.GetToSquare() + SOUTH_DIRECTION) : (move.GetToSquare() + NORTH_DIRECTION);
        boardSquares[capturedPawnSquare] = Piece::NONE;

        for (int directionIndex = 0; directionIndex < 8; ++directionIndex)
        {
            const int directionOffset = DIRECTION_OFFSETS[directionIndex];
            const int maxSteps = NUMBER_OF_SQUARES_TO_EDGE[friendlyKingSquare][directionIndex];

            for (int step = 1; step <= maxSteps; ++step)
            {
                const int scanSquare = friendlyKingSquare + step * directionOffset;

                Piece::Piece targetPiece = boardSquares[scanSquare];
                if (targetPiece == Piece::NONE)
                {
                    continue;
                }

                if (Piece::IsWhite(targetPiece) == isWhiteToMove)
                {
                    // Friendly piece
                    break; // Blocked by friendly piece
                }
                else
                {
                    // Opponent piece
                    auto targetType = Piece::GetType(targetPiece);
                    bool isSlidingPiece = (targetType == Piece::QUEEN) ||
                                          (targetType == Piece::ROOK && directionIndex < 4) ||
                                          (targetType == Piece::BISHOP && directionIndex >= 4);

                    if (isSlidingPiece)
                    {
                        return true; // Illegal move
                    }
                    break; // Encountered opponent piece, stop scanning
                }
            }
        }
    }

    return false; // Move is legal
}

} // Anonymous namespace for helper functions

MoveList GeneratePseudoLegalMoves(const Board& board)
{
    MoveList moves;

    const bool isWhiteToMove = board.IsWhiteToMove();

    {
        int pieceSquare = board.GetKingSquare(isWhiteToMove);
        Piece::Piece piece = board.GetPieceAtSquare(pieceSquare);
        GenerateKingMovesForPiece(board, piece, pieceSquare, moves);
    }
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

    return moves;
}

MoveList GenerateMoves(Board& board, bool onlyCaptures)
{
    MoveList moves;

    const bool isWhiteToMove = board.IsWhiteToMove();

    const LegalMoveInfo friendlyLegalMoveInfo = GenerateLegalMoveInfo(board);

    board.SetCurrentPlayerInCheck(friendlyLegalMoveInfo.KingInCheck());

    {
        int pieceSquare = board.GetKingSquare(isWhiteToMove);
        Piece::Piece piece = board.GetPieceAtSquare(pieceSquare);
        GenerateKingMovesForPiece(board, piece, pieceSquare, moves);
    }
    if (!friendlyLegalMoveInfo.KingInDoubleCheck())
    {
        // Generate moves for other pieces
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
    }

    MoveList legalMoves;

    for (int i = 0; i < moves.moveCount; ++i)
    {
        const auto& move = moves.moves[i];

        if (onlyCaptures)
        {
            if (move.IsCapture() && !MoveIsIllegal(board, friendlyLegalMoveInfo, move))
            {
                legalMoves.AddMove(move);
            }
        }
        else if (!MoveIsIllegal(board, friendlyLegalMoveInfo, move))
        {
            legalMoves.AddMove(move);
        }
    }

    Board tempBoard = board; // Create a temporary copy of the board to make and undo moves
    for (int i = 0; i < legalMoves.moveCount; ++i)
    {
        auto& move = legalMoves.moves[i];
        tempBoard.MakeMove(move);
        if (tempBoard.IsCurrentPlayerInCheck())
        {
            move.isCheckingMove = true;
        }
        tempBoard.UnmakeLastMove();
    }

    return legalMoves;
}

} // namespace Gluon::MoveGenerator