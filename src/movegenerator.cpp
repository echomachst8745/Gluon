#include "movegenerator.h"

#include "position.h"
#include "movelist.h"
#include "bitboard.h"
#include "movetables.h"

#include <array>

namespace Gluon {

static void GeneratePawnMoves(const Position& position, MoveList& moveList, bool capturesOnly)
{
    Colour activeColour = position.GetActiveColour();
    Bitboard friendlyPawnBitboard = position.GetPieceBitboard(activeColour == WHITE ?
                                                              WHITE_PAWN : BLACK_PAWN);

    Direction pushDirection        = activeColour == WHITE ? NORTH      : SOUTH;
    Direction eastCaptureDirection = activeColour == WHITE ? NORTH_EAST : SOUTH_EAST;
    Direction westCaptureDirection = activeColour == WHITE ? NORTH_WEST : SOUTH_WEST;

    Bitboard doublePushRankBitboard = RankToBitboard(activeColour == WHITE ? RANK_4 : RANK_5);
    Bitboard promotionRankBitboard  = RankToBitboard(activeColour == WHITE ? RANK_8 : RANK_1);

    Bitboard emptyBitboard = ~position.GetAllOccupancyBitboard();
    Bitboard enemyBitboard = position.GetOccupancyBitboard(~activeColour);

    Bitboard enPassantTargetBitboard = 0ULL;
    if (position.GetEnPassantTargetSquare() != NO_SQUARE)
    {
        BB::SetSquare(enPassantTargetBitboard, position.GetEnPassantTargetSquare());
    }

    // !EXPLAIN!
    Bitboard pushesBitboard = BB::Shift(friendlyPawnBitboard, pushDirection) & emptyBitboard;
    Bitboard doublePushesBitboard = BB::Shift(pushesBitboard, pushDirection) & emptyBitboard &
                                    doublePushRankBitboard;

    // !EXPLAIN!
    Bitboard eastAttacksBitboard = BB::Shift(friendlyPawnBitboard & (~FileToBitboard(FILE_H)),
                                             eastCaptureDirection);
    Bitboard westAttacksBitboard = BB::Shift(friendlyPawnBitboard & (~FileToBitboard(FILE_A)),
                                             westCaptureDirection);

    Bitboard eastCapturesBitboard = eastAttacksBitboard & enemyBitboard;
    Bitboard westCapturesBitboard = westAttacksBitboard & enemyBitboard;

    Bitboard pushPromotionsBitboard        = pushesBitboard        & promotionRankBitboard;
    Bitboard eastCapturePromotionsBitboard = eastCapturesBitboard  & promotionRankBitboard;
    Bitboard westCapturePromotionsBitboard = westCapturesBitboard  & promotionRankBitboard;

    Bitboard eastEnPassantCapturesBitboard = eastAttacksBitboard & enPassantTargetBitboard;
    Bitboard westEnPassantCapturesBitboard = westAttacksBitboard & enPassantTargetBitboard;

    pushesBitboard        &= ~promotionRankBitboard;
    eastCapturesBitboard  &= ~promotionRankBitboard;
    westCapturesBitboard  &= ~promotionRankBitboard;

    if (!capturesOnly)
    {
        // Pushes
        while (pushesBitboard)
        {
            Square toSquare = Square(BB::PopLSB(pushesBitboard));

            moveList.AddMove(Move(toSquare - pushDirection, toSquare, Move::QUIET_MOVE));
        }

        // Double pushes
        while (doublePushesBitboard)
        {
            Square toSquare = Square(BB::PopLSB(doublePushesBitboard));

            moveList.AddMove(Move(toSquare - pushDirection - pushDirection, toSquare, Move::DOUBLE_PAWN_PUSH));
        }
    }

    // Captures
    while (eastCapturesBitboard)
    {
        Square toSquare = Square(BB::PopLSB(eastCapturesBitboard));

        moveList.AddMove(Move(toSquare - eastCaptureDirection, toSquare, Move::CAPTURE));
    }

    while (westCapturesBitboard)
    {
        Square toSquare = Square(BB::PopLSB(westCapturesBitboard));

        moveList.AddMove(Move(toSquare - westCaptureDirection, toSquare, Move::CAPTURE));
    }

    // En passant captures
    while (eastEnPassantCapturesBitboard)
    {
        Square toSquare = Square(BB::PopLSB(eastEnPassantCapturesBitboard));

        moveList.AddMove(Move(toSquare - eastCaptureDirection, toSquare, Move::EN_PASSANT_CAPTURE));
    }

    while (westEnPassantCapturesBitboard)
    {
        Square toSquare = Square(BB::PopLSB(westEnPassantCapturesBitboard));

        moveList.AddMove(Move(toSquare - westCaptureDirection, toSquare, Move::EN_PASSANT_CAPTURE));
    }

    // Push promotions
    while (pushPromotionsBitboard)
    {
        Square toSquare = Square(BB::PopLSB(pushPromotionsBitboard));
        Square fromSquare = toSquare - pushDirection;

        moveList.AddMove(Move(fromSquare, toSquare, Move::KNIGHT_PROMOTION));
        moveList.AddMove(Move(fromSquare, toSquare, Move::BISHOP_PROMOTION));
        moveList.AddMove(Move(fromSquare, toSquare, Move::ROOK_PROMOTION));
        moveList.AddMove(Move(fromSquare, toSquare, Move::QUEEN_PROMOTION));
    }

    // Capture promotions
    while (eastCapturePromotionsBitboard)
    {
        Square toSquare = Square(BB::PopLSB(eastCapturePromotionsBitboard));
        Square fromSquare = toSquare - eastCaptureDirection;

        moveList.AddMove(Move(fromSquare, toSquare, Move::KNIGHT_PROMOTION_CAPTURE));
        moveList.AddMove(Move(fromSquare, toSquare, Move::BISHOP_PROMOTION_CAPTURE));
        moveList.AddMove(Move(fromSquare, toSquare, Move::ROOK_PROMOTION_CAPTURE));
        moveList.AddMove(Move(fromSquare, toSquare, Move::QUEEN_PROMOTION_CAPTURE));
    }

    while (westCapturePromotionsBitboard)
    {
        Square toSquare = Square(BB::PopLSB(westCapturePromotionsBitboard));
        Square fromSquare = toSquare - westCaptureDirection;

        moveList.AddMove(Move(fromSquare, toSquare, Move::KNIGHT_PROMOTION_CAPTURE));
        moveList.AddMove(Move(fromSquare, toSquare, Move::BISHOP_PROMOTION_CAPTURE));
        moveList.AddMove(Move(fromSquare, toSquare, Move::ROOK_PROMOTION_CAPTURE));
        moveList.AddMove(Move(fromSquare, toSquare, Move::QUEEN_PROMOTION_CAPTURE));
    }
}

static constexpr size_t NUM_PIECE_MOVE_PIECE_TYPES = 4;

static constexpr std::array<PieceType, NUM_PIECE_MOVE_PIECE_TYPES> PIECE_MOVE_PIECE_TYPES = {
    KNIGHT, BISHOP, ROOK, QUEEN
};

static Bitboard GetPieceAttacks(PieceType pieceType, Square square, Bitboard occupancyBitboard)
{
    switch (pieceType)
    {
        case KNIGHT: return MoveTables::KNIGHT_MOVE_TABLE[square];
        case BISHOP: return MoveTables::GetBishopMoves(square, occupancyBitboard);
        case ROOK:   return MoveTables::GetRookMoves(square, occupancyBitboard);
        case QUEEN:  return MoveTables::GetQueenMoves(square, occupancyBitboard);
        default:     return 0ULL;
    }
}

static void GeneratePieceMoves(const Position& position, MoveList& moveList, bool capturesOnly)
{
    Colour activeColour = position.GetActiveColour();

    for (PieceType pieceType : PIECE_MOVE_PIECE_TYPES)
    {
        Bitboard friendlyPieceBitboard = position.GetPieceBitboard(MakePiece(activeColour, pieceType));

        while (friendlyPieceBitboard)
        {
            Square fromSquare = Square(BB::PopLSB(friendlyPieceBitboard));
            Bitboard pieceMoveBitboard = GetPieceAttacks(pieceType, fromSquare,
                                                         position.GetAllOccupancyBitboard());

            Bitboard quietMovesBitboard = pieceMoveBitboard & (~position.GetAllOccupancyBitboard());
            Bitboard capturesBitboard = pieceMoveBitboard & (position.GetOccupancyBitboard(~activeColour));

            if (!capturesOnly)
            {
                // Quiet moves
                while (quietMovesBitboard)
                {
                    Square toSquare = Square(BB::PopLSB(quietMovesBitboard));

                    moveList.AddMove(Move(fromSquare, toSquare, Move::QUIET_MOVE));
                }
            }

            // Captures
            while (capturesBitboard)
            {
                Square toSquare = Square(BB::PopLSB(capturesBitboard));

                moveList.AddMove(Move(fromSquare, toSquare, Move::CAPTURE));
            }
        }
    }
}

static void GenerateKingMoves(const Position& position, MoveList& moveList, bool capturesOnly)
{
    Colour activeColour = position.GetActiveColour();
    Bitboard friendlyKingBitboard = position.GetPieceBitboard(activeColour == WHITE ?
                                                              WHITE_KING : BLACK_KING);

    Square fromSquare = Square(BB::PopLSB(friendlyKingBitboard));
    Bitboard kingMoveBitboard = MoveTables::KING_MOVE_TABLE[fromSquare];

    Bitboard quietMovesBitboard = kingMoveBitboard & (~position.GetAllOccupancyBitboard());
    Bitboard capturesBitboard = kingMoveBitboard & (position.GetOccupancyBitboard(~activeColour));

    if (!capturesOnly)
    {
        // Quiet moves
        while (quietMovesBitboard)
        {
            Square toSquare = Square(BB::PopLSB(quietMovesBitboard));

            moveList.AddMove(Move(fromSquare, toSquare, Move::QUIET_MOVE));
        }
    }

    // Captures
    while (capturesBitboard)
    {
        Square toSquare = Square(BB::PopLSB(capturesBitboard));

        moveList.AddMove(Move(fromSquare, toSquare, Move::CAPTURE));
    }

    if (capturesOnly)
    {
        return;
    }

    // Castling
    CastlingRight kingSideCastlingRight  = activeColour == WHITE ? WHITE_OO  : BLACK_OO;
    CastlingRight queenSideCastlingRight = activeColour == WHITE ? WHITE_OOO : BLACK_OOO;

    Bitboard kingSidePathBitboard  = activeColour == WHITE ? MoveTables::WHITE_KING_SIDE_CASTLING_PATH :
                                                             MoveTables::BLACK_KING_SIDE_CASTLING_PATH;
    Bitboard queenSidePathBitboard = activeColour == WHITE ? MoveTables::WHITE_QUEEN_SIDE_CASTLING_PATH :
                                                             MoveTables::BLACK_QUEEN_SIDE_CASTLING_PATH;

    Square kingSideToSquare  = activeColour == WHITE ? SQUARE_G1 : SQUARE_G8;
    Square queenSideToSquare = activeColour == WHITE ? SQUARE_C1 : SQUARE_C8;

    if ((position.GetCastlingRights() & kingSideCastlingRight) &&
        !(kingSidePathBitboard & position.GetAllOccupancyBitboard()))
    {
        moveList.AddMove(Move(fromSquare, kingSideToSquare, Move::KING_CASTLE));
    }

    if ((position.GetCastlingRights() & queenSideCastlingRight) &&
        !(queenSidePathBitboard & position.GetAllOccupancyBitboard()))
    {
        moveList.AddMove(Move(fromSquare, queenSideToSquare, Move::QUEEN_CASTLE));
    }
}

// !EXPLAIN!
static Bitboard GetAttackersToSquare(const Position& position, Square square, Colour attackerColour,
                                     Bitboard occupancyBitboard)
{
    Bitboard pawnBitboard   = position.GetPieceBitboard(attackerColour == WHITE ? WHITE_PAWN   : BLACK_PAWN);
    Bitboard knightBitboard = position.GetPieceBitboard(attackerColour == WHITE ? WHITE_KNIGHT : BLACK_KNIGHT);
    Bitboard bishopBitboard = position.GetPieceBitboard(attackerColour == WHITE ? WHITE_BISHOP : BLACK_BISHOP);
    Bitboard rookBitboard   = position.GetPieceBitboard(attackerColour == WHITE ? WHITE_ROOK   : BLACK_ROOK);
    Bitboard queenBitboard  = position.GetPieceBitboard(attackerColour == WHITE ? WHITE_QUEEN  : BLACK_QUEEN);
    Bitboard kingBitboard   = position.GetPieceBitboard(attackerColour == WHITE ? WHITE_KING   : BLACK_KING);

    return (MoveTables::PAWN_ATTACK_TABLE[~attackerColour][square] & pawnBitboard)                       |
           (MoveTables::KNIGHT_MOVE_TABLE[square] & knightBitboard)                                      |
           (MoveTables::KING_MOVE_TABLE[square] & kingBitboard)                                          |
           (MoveTables::GetBishopMoves(square, occupancyBitboard) & (bishopBitboard | queenBitboard))    |
           (MoveTables::GetRookMoves(square, occupancyBitboard) & (rookBitboard | queenBitboard));
}

MoveList GeneratePseudoLegalMoves(const Position& position, bool capturesOnly)
{
    MoveList moves;

    GeneratePawnMoves(position, moves, capturesOnly);
    GeneratePieceMoves(position, moves, capturesOnly);
    GenerateKingMoves(position, moves, capturesOnly);

    return moves;
}

MoveList GenerateLegalMoves(const Position& position, bool capturesOnly)
{
    MoveList pseudoLegalMoves = GeneratePseudoLegalMoves(position, capturesOnly);
    MoveList legalMoves;

    Colour activeColour = position.GetActiveColour();
    Colour enemyColour = ~activeColour;

    Bitboard allOccupancyBitboard = position.GetAllOccupancyBitboard();
    Bitboard friendlyOccupancyBitboard = position.GetOccupancyBitboard(activeColour);
    Bitboard enemyOccupancyBitboard = position.GetOccupancyBitboard(enemyColour);

    Bitboard friendlyKingBitboard = position.GetPieceBitboard(activeColour == WHITE ?
                                                              WHITE_KING : BLACK_KING);
    Square kingSquare = Square(BB::GetLSB(friendlyKingBitboard));

    Direction pushDirection = activeColour == WHITE ? NORTH : SOUTH;

    Bitboard checkersBitboard = GetAttackersToSquare(position, kingSquare, enemyColour,
                                                     allOccupancyBitboard);

    // !EXPLAIN!
    Bitboard checkMaskBitboard = ~0ULL;
    if (BB::CountBits(checkersBitboard) > 1)
    {
        checkMaskBitboard = 0ULL;
    }
    else if (checkersBitboard)
    {
        Square checkerSquare = Square(BB::GetLSB(checkersBitboard));

        checkMaskBitboard = MoveTables::BETWEEN_TABLE[kingSquare][checkerSquare] | checkersBitboard;
    }

    // !EXPLAIN!
    Bitboard enemyBishopsAndQueensBitboard =
        position.GetPieceBitboard(enemyColour == WHITE ? WHITE_BISHOP : BLACK_BISHOP) |
        position.GetPieceBitboard(enemyColour == WHITE ? WHITE_QUEEN  : BLACK_QUEEN);
    Bitboard enemyRooksAndQueensBitboard =
        position.GetPieceBitboard(enemyColour == WHITE ? WHITE_ROOK  : BLACK_ROOK) |
        position.GetPieceBitboard(enemyColour == WHITE ? WHITE_QUEEN : BLACK_QUEEN);

    Bitboard pinnersBitboard =
        (MoveTables::GetBishopMoves(kingSquare, enemyOccupancyBitboard) & enemyBishopsAndQueensBitboard) |
        (MoveTables::GetRookMoves(kingSquare, enemyOccupancyBitboard) & enemyRooksAndQueensBitboard);

    Bitboard pinnedBitboard = 0ULL;
    while (pinnersBitboard)
    {
        Square pinnerSquare = Square(BB::PopLSB(pinnersBitboard));
        Bitboard betweenBitboard = MoveTables::BETWEEN_TABLE[kingSquare][pinnerSquare] &
                                   friendlyOccupancyBitboard;

        if (BB::CountBits(betweenBitboard) == 1)
        {
            pinnedBitboard |= betweenBitboard;
        }
    }

    for (size_t moveIndex = 0; moveIndex < pseudoLegalMoves.Size(); ++moveIndex)
    {
        Move move = pseudoLegalMoves[moveIndex];

        Square fromSquare = move.GetFromSquare();
        Square toSquare = move.GetToSquare();

        // King moves
        if (fromSquare == kingSquare)
        {
            if (move.GetFlag() == Move::KING_CASTLE || move.GetFlag() == Move::QUEEN_CASTLE)
            {
                Square transitSquare = move.GetFlag() == Move::KING_CASTLE ? fromSquare + EAST
                                                                           : fromSquare + WEST;

                if (checkersBitboard ||
                    GetAttackersToSquare(position, transitSquare, enemyColour, allOccupancyBitboard) ||
                    GetAttackersToSquare(position, toSquare, enemyColour, allOccupancyBitboard))
                {
                    continue;
                }
            }
            // !EXPLAIN!
            else if (GetAttackersToSquare(position, toSquare, enemyColour,
                                          allOccupancyBitboard ^ friendlyKingBitboard))
            {
                continue;
            }
        }
        // !EXPLAIN!
        else if (move.GetFlag() == Move::EN_PASSANT_CAPTURE)
        {
            Bitboard capturedPawnBitboard = SquareToBitboard(toSquare - pushDirection);
            Bitboard occupancyBitboard = (allOccupancyBitboard ^ SquareToBitboard(fromSquare) ^
                                          capturedPawnBitboard) | SquareToBitboard(toSquare);

            if (GetAttackersToSquare(position, kingSquare, enemyColour, occupancyBitboard) &
                (~capturedPawnBitboard))
            {
                continue;
            }
        }
        // Pinned and checked pieces
        else
        {
            if (!(checkMaskBitboard & SquareToBitboard(toSquare)))
            {
                continue;
            }

            if ((pinnedBitboard & SquareToBitboard(fromSquare)) &&
                !(MoveTables::LINE_TABLE[kingSquare][fromSquare] & SquareToBitboard(toSquare)))
            {
                continue;
            }
        }

        legalMoves.AddMove(move);
    }

    return legalMoves;
}

} // namespace Gluon