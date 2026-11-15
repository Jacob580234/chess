#include "move.h"
#include <stdint.h>
#include <stdio.h>

#include "main.h"
#include "maps.h"
#include "../asm.h"


square inline getKingIndex(state_s* state) {
    return TZCNT(state->bitboard[state->playerToMove][king]);
}


// maybe do one of these per game phase since rooks probably check more in endgame etc
bool squareIsAttacked(state_s* restrict state, map_s* restrict map, const square square) {

    const side opponent = state->playerToMove^1;

    if(map->knight[square] & state->bitboard[opponent][knight]                   ||
       queenPseudoLegal(state, map, square) & state->bitboard[opponent][queen]   ||
       rookPseudoLegal(state, map, square) & state->bitboard[opponent][rook]     ||
       bishopPseudoLegal(state, map, square) & state->bitboard[opponent][bishop] ||
       map->pawn[opponent][square] & state->bitboard[opponent][pawn]             ||
       map->king[square] & state->bitboard[opponent][king]) {
        return true;
    }

    return false;
}


bool inline kingIsInCheck(state_s* restrict state, map_s* restrict map) {
    square kingIndex = getKingIndex(state);
    return squareIsAttacked(state, map, kingIndex);
}


uint64_t inline knightPseudoLegal(state_s* restrict state, map_s* restrict map, const square start) {
    return ANDN(state->piecesForSide[state->playerToMove], map->knight[start]);
}

uint64_t inline kingPseudoLegal(state_s* restrict state, map_s* restrict map, const square start) {
    return ANDN(state->piecesForSide[state->playerToMove], map->king[start]);
}

uint64_t inline rookPseudoLegal(state_s* restrict state, map_s* restrict map, const square start) {
    uint32_t index = map->rookPextTableOffset[start] + PEXT(state->allPieces, map->rookBlockerMask[start]);
    return ANDN(state->piecesForSide[state->playerToMove], map->rookPextTable[index]);
}

uint64_t inline bishopPseudoLegal(state_s* restrict state, map_s* restrict map, const square start) {
    uint32_t index = map->bishopPextTableOffset[start] + PEXT(state->allPieces, map->bishopBlockerMask[start]);
    return ANDN(state->piecesForSide[state->playerToMove], map->bishopPextTable[index]);
}

uint64_t inline queenPseudoLegal(state_s* restrict state, map_s* restrict map, const square start) {
    return bishopPseudoLegal(state, map, start) | rookPseudoLegal(state, map, start);
}

// typedef function pointers
bool moveIsLegal(state_s* restrict state, map_s* restrict map, const uint16_t move, void (*simulateMove)(state_s*, map_s*, uint16_t)) {

    state_s afterMoveState = *state;
    simulateMove(&afterMoveState, map, move);

    if (kingIsInCheck(&afterMoveState, map)) {
        return false;
    }
    *state = afterMoveState;
    return true;
}


void updatePiecePosition(state_s* state, uint16_t move) {

    const square origin = START(move);
    const square target = END(move);

    piece piece = state->pieceLookup[origin];
    state->pieceLookup[origin] = noPiece;
    state->pieceLookup[target] = piece;

    uint64_t moveBitboard = BIT(origin) | BIT(target);
    state->bitboard[state->playerToMove][piece] ^= moveBitboard;
    state->piecesForSide[state->playerToMove] ^= moveBitboard;
}


void inline handleCapture(state_s* state, const square end) {
    uint64_t endBitboard = BIT(end);
    if (endBitboard & state->piecesForSide[state->playerToMove^1]) {
        piece opponentPiece = state->pieceLookup[end];
        state->bitboard[state->playerToMove ^ 1][opponentPiece] ^= endBitboard;
        state->piecesForSide[state->playerToMove ^ 1] ^= endBitboard;
        state->fiftyMoveRule = 0;
    }
}


void inline updateCastlingRights(state_s* restrict state, map_s* restrict map, const square start, const square end) {
    if (state->castlingRights != 0b0000)
        state->castlingRights &= map->castlingMask[start] & map->castlingMask[end];
}


uint64_t castlingBitboard(state_s* restrict state, map_s* restrict map) {
    if (state->castlingRights == 0b0000) return 0;

    uint64_t bitboard = 0;
    if (state->playerToMove == white) { // if/else can removed, would require variable assignment. this should be faster and is clearer
        if(read(WHITE_OOO, state->castlingRights) && !(map->castlingPath[WHITE_OOO] & state->allPieces)) bitboard |= BIT(C1);
        if(read(WHITE_OO,  state->castlingRights) && !(map->castlingPath[WHITE_OO]  & state->allPieces)) bitboard |= BIT(G1);
    } else {
        if(read(BLACK_OOO, state->castlingRights) && !(map->castlingPath[BLACK_OOO] & state->allPieces)) bitboard |= BIT(C8);
        if(read(BLACK_OO,  state->castlingRights) && !(map->castlingPath[BLACK_OO]  & state->allPieces)) bitboard |= BIT(G8);
    }

    return bitboard;
}


// separate functions for castling and en passant moves? and perhaps promotion?
void simulateMoveGeneral(state_s* restrict state, map_s* restrict map, const uint16_t move) {

    const square start = START(move);
    const square end = END(move);

    updatePiecePosition(state, move);
    handleCapture(state, end);
}


void simulateMovePawn(state_s* restrict state, map_s* restrict map, const uint16_t move) {

    const square start = START(move);
    const square end = END(move);

    updatePiecePosition(state, move);
    handleCapture(state, end);

    if (end == state->enPassantIndex) {
        square opponentPawnIndex = state->playerToMove == white ? end + FILES : end - FILES;
        state->bitboard[state->playerToMove ^ 1][pawn] ^= BIT(opponentPawnIndex);
    }

    state->fiftyMoveRule = 0;
}


void simulateMoveKing(state_s* restrict state, map_s* restrict map, uint16_t move) {
    const square start = START(move);
    const square end = END(move);

    if(map->king[start] & end) {
        handleCapture(state, end);
    } else {
        square passThrough = start < end ? F1 : D1;
        if (state->playerToMove == white)
            passThrough += FILES * 7;

        if (squareIsAttacked(state, map, passThrough)) {
            return; // signal that it is not allowed (somehow)
        }

        // move corresponding rook
    }
    updatePiecePosition(state, move);
}


bool pawnMoves(state_s* restrict state, map_s* restrict map, const uint16_t move) {

    const square start = START(move);
    const square end = END(move);

    uint64_t attackPseudoLegal = map->pawn[state->playerToMove][start];
    uint64_t targetSquares = state->piecesForSide[state->playerToMove ^ 1] | BIT(state->enPassantIndex);
    bool isAttacking = read(end, attackPseudoLegal & targetSquares);

    uint64_t oneSquareForward = state->playerToMove == white ? BIT(start) << FILES : BIT(start) >> FILES;
    bool canMoveOne = ANDN(state->allPieces, oneSquareForward);

    if (!canMoveOne && !isAttacking) // if you cant move one, you cant move two; no matter
        return false;

    bool isMovingOne = oneSquareForward == BIT(end);
    if (isMovingOne || isAttacking)
        return moveIsLegal(state, map, move, &simulateMovePawn);


    uint64_t twoSquaresForward = state->playerToMove == white ? BIT(start) << 2*FILES : BIT(start) >> 2*FILES;
    bool isFirstPawnMove = state->playerToMove == white ? start <= H2 : start >= A7;
    bool canAndIsMovingTwo = isFirstPawnMove && (twoSquaresForward == ANDN(state->allPieces, BIT(end)));

    if (canAndIsMovingTwo) {
        state->enPassantIndex = state->playerToMove == white ? start + FILES : start - FILES; // move this to after moveIsLegal check
        return moveIsLegal(state, map, move, &simulateMovePawn);
    }

    return false;
}


bool knightMoves(state_s* restrict state, map_s* restrict map, const uint16_t move) {
    const square start = START(move);
    const square end = END(move);
    uint64_t pseudoLegal = knightPseudoLegal(state, map, start);
    if(!read(end, pseudoLegal)) return false;
    return moveIsLegal(state, map, move, &simulateMoveGeneral);
}


bool kingMoves(state_s* restrict state, map_s* restrict map, const uint16_t move) {
    const square start = START(move);
    const square end = END(move);

    uint64_t pseudoLegal = kingPseudoLegal(state, map, start);
    pseudoLegal |= castlingBitboard(state, map);
    updateCastlingRights(state, map, start, end); // TODO: shouldnt be here
    if(!read(end, pseudoLegal)) return false;

    return moveIsLegal(state, map, move, &simulateMoveKing);
}


bool rookMoves(state_s* restrict state, map_s* restrict map, const uint16_t move) {
    const square start = START(move);
    const square end = END(move);
    uint64_t pseudoLegal = rookPseudoLegal(state, map, start);
    if(!read(end, pseudoLegal)) return false;
    return moveIsLegal(state, map, move, &simulateMoveGeneral);
}


bool bishopMoves(state_s* restrict state, map_s* restrict map, const uint16_t move) {
    const square start = START(move);
    const square end = END(move);
    uint64_t pseudoLegal = bishopPseudoLegal(state, map, start);
    if(!read(end, pseudoLegal)) return false;
    return moveIsLegal(state, map, move, &simulateMoveGeneral);
}


bool queenMoves(state_s* restrict state, map_s* restrict map, const uint16_t move) {
    const square start = START(move);
    const square end = END(move);
    uint64_t pseudoLegal = queenPseudoLegal(state, map, start);
    if (!read(end, pseudoLegal)) return false;
    return moveIsLegal(state, map, move, &simulateMoveGeneral);
}