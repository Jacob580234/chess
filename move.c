#include "move.h"
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include "main.h"
#include "attackMaps.h"
#include "print.h"


square inline getKingIndex(uint64_t bitboard) {
    uint64_t result;
    __asm__("BSFQ %1, %0" : "=r"(result) : "r"(bitboard));
    return (square)result;
}


bool inline kingIsAttackedByKnight(gameState_s* restrict state, attackMap_s* restrict map) {
    uint64_t opponentKnights = state->bitboard[state->playerToMove ^ 1][knight];
    square kingIndex = getKingIndex(state->bitboard[state->playerToMove][king]);
    return map->knight[kingIndex] & opponentKnights;
}


bool inline kingIsAttackedByKing(gameState_s* restrict state, attackMap_s* restrict map) {
    uint64_t opponentKing = state->bitboard[state->playerToMove ^ 1][king];
    square kingIndex = getKingIndex(state->bitboard[state->playerToMove][king]);
    return map->king[kingIndex] & opponentKing;
}


bool inline kingIsAttackedByPawn(gameState_s* restrict state, attackMap_s* restrict map) {
    uint64_t opponentPawns = state->bitboard[state->playerToMove ^ 1][pawn];
    square kingIndex = getKingIndex(state->bitboard[state->playerToMove][king]);
    return map->pawn[state->playerToMove ^ 1][kingIndex] & opponentPawns;
}

bool inline kingIsAttackedByRook(gameState_s* restrict state, attackMap_s* restrict map) {
    uint64_t opponentRooks = state->bitboard[state->playerToMove ^ 1][rook];
    square kingIndex = getKingIndex(state->bitboard[state->playerToMove][king]);
    return rookMoves(state, map, kingIndex, -9999999999999999) & opponentRooks;
}


bool kingIsInCheck(gameState_s* restrict state, attackMap_s* restrict map) {

    return (
        kingIsAttackedByKnight(state, map) ||
        kingIsAttackedByKing(state, map)   ||
        kingIsAttackedByPawn(state, map)   ||
        kingIsAttackedByRook(state, map)
    );
}


bool moveIsLegal(gameState_s* restrict state, attackMap_s* restrict map, const square start, const square end, void (*simulateMove)(gameState_s*, square, square)) {

    gameState_s afterMoveState = *state;
    simulateMove(&afterMoveState, start, end);

    if (kingIsInCheck(&afterMoveState, map))
        return false;

    *state = afterMoveState;
    return true;
}


void removeSelectedPiece(gameState_s* state, const square start, piece piece) {
    state->pieceLookup[start] = noPiece;
    clear(start, &state->bitboard[state->playerToMove][piece]);
}


void handleCapture(gameState_s* state, const square end) {
    piece opponentPiece = state->pieceLookup[end];
    if (opponentPiece != noPiece) {
        clear(end, &state->bitboard[state->playerToMove ^ 1][opponentPiece]);
        clear(end, &state->piecesForSide[state->playerToMove ^ 1]);
        state->fiftyMoveRule = 0; // reset 50-move rule on capture
    }
}


void moveSelectedPiece(gameState_s* state, const square end, piece piece) {
    state->pieceLookup[end] = piece;
    set(end, &state->bitboard[state->playerToMove][piece]);
}


void updateGeneralBitboards(gameState_s* state, const square start, const square end) {
    clear(start, &state->piecesForSide[state->playerToMove]);
    set(end, &state->piecesForSide[state->playerToMove]);
    state->allPieces = state->piecesForSide[white] | state->piecesForSide[black];
}

// separate functions for castling and en passant moves? and perhaps promotion?
void simulateMoveGeneral(gameState_s* state, const square start, const square end) {
    
    piece ownPiece = state->pieceLookup[start];

    removeSelectedPiece(state, start, ownPiece);
    handleCapture(state, end);
    moveSelectedPiece(state, end, ownPiece);
    updateGeneralBitboards(state, start, end);

}


void simulateMovePawn(gameState_s* state, const square start, const square end) {

    piece pawn = state->pieceLookup[start];

    removeSelectedPiece(state, start, pawn);
    handleCapture(state, end);
    moveSelectedPiece(state, end, pawn);

    if (end == state->enPassantIndex) {
        square opponentPawnIndex = state->playerToMove == white ? end + 8 : end - 8;
        clear(opponentPawnIndex, &state->bitboard[state->playerToMove ^ 1][pawn]);
    }

    updateGeneralBitboards(state, start, end);
    state->fiftyMoveRule = 0;
}


void simulateMoveKing(gameState_s* state, const square start, const square end) {
    // needs to handle moving rook. also check for if in check for castling? perhaps in "kingMoves" function, not this one
}


bool pawnMoves(gameState_s* restrict state, attackMap_s* restrict map, const square start, const square end) {

    uint64_t attackPseudoLegal = map->pawn[state->playerToMove][start];

    bool isAttacking = read(end, attackPseudoLegal & (state->piecesForSide[state->playerToMove ^ 1] | BIT(state->enPassantIndex)));
    bool canMoveOne, isMovingOne, moveTwo, isFirstPawnMove;

    state->playerToMove == white ? ( // dont assign all at once, do if by if(?)
        canMoveOne      = BIT(start) << 8 & ~state->allPieces,
        isMovingOne     = BIT(start) << 8 == BIT(end),
        moveTwo         = BIT(start) << 16 & ~state->allPieces,
        isFirstPawnMove = start <= H2
    ) : (
        canMoveOne      = BIT(start) >> 8 & ~state->allPieces,
        isMovingOne     = BIT(start) >> 8 == BIT(end),
        moveTwo         = BIT(start) >> 16 == (BIT(end) & ~state->allPieces),
        isFirstPawnMove = start >= A7
    );

    if(!canMoveOne && !isAttacking) return false; // if you cant move one, you cant move two; no matter

    if (isMovingOne || isAttacking) {
        if (isFirstPawnMove && moveTwo)
            state->enPassantIndex = state->playerToMove == white ? start + 8 : start - 8;

        return moveIsLegal(state, map, start, end, &simulateMovePawn); // needs to be simulateMovePawn
    }

    return false;
}


bool knightMoves(gameState_s* restrict state, attackMap_s* restrict map, const square start, const square end) {
    // add that you cant land on an own piece
    uint64_t pseudoLegal = map->knight[start] & ~state->piecesForSide[state->playerToMove];
    if(!read(end, pseudoLegal)) return false;
    return moveIsLegal(state, map, start, end, &simulateMoveGeneral);
}


bool kingMoves(gameState_s* restrict state, attackMap_s* restrict map, const square start, const square end) {
    // castling
    // pseudoLegal |= castling
    // if kingSquare != original: return 0;
    // else check if (something)
    uint64_t pseudoLegal = map->king[start] & ~state->piecesForSide[state->playerToMove];
    if(!read(end, pseudoLegal & ~state->piecesForSide[state->playerToMove])) return false;
}


bool rookMoves(gameState_s* restrict state, attackMap_s* restrict map, const square start, const square end) {
    uint32_t index = map->rookPextTableOffset[start] + pext(map->rookBlockerMask[start], state->allPieces);
    return map->rookPextTable[index] & ~state->piecesForSide[state->playerToMove];
}