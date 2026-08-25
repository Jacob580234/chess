#include "move.h"
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include "main.h"
#include "bitboardDatabase.h"

bool inline kingIsAttackedByKnight(gameState_s* state, attackMap_s* map) {
    uint64_t opponentKnights = state->bitboard[state->playerToMove ^ 1][knight];
    uint8_t kingIndex = state->kingIndex[state->playerToMove];
    return map->knight[kingIndex] & opponentKnights;
}

bool inline kingIsAttackedByKing(gameState_s* state, attackMap_s* map) {
    uint64_t opponentKing = state->bitboard[state->playerToMove ^ 1][king];
    uint8_t kingIndex = state->kingIndex[state->playerToMove];
    return map->king[kingIndex] & opponentKing;
}

bool inline kingIsAttackedByPawn(gameState_s* state, attackMap_s* map) {
    uint64_t opponentPawns = state->bitboard[state->playerToMove ^ 1][pawn];
    uint8_t kingIndex = state->kingIndex[state->playerToMove];
    return map->pawn[state->playerToMove ^ 1][kingIndex] & opponentPawns;
}

bool kingIsInCheck(gameState_s* state, attackMap_s* map) {
    if(kingIsAttackedByKnight(state, map)) return true;
    if(kingIsAttackedByKing(state, map))   return true;
    if(kingIsAttackedByPawn(state, map))   return true;

    return false;
}

bool moveIsLegal(gameState_s* state, attackMap_s* map, const square start, const square end, void (*simulateMove)(gameState_s*, square, square)) {

    gameState_s afterMoveState = *state;
    simulateMove(&afterMoveState, start, end);

    if (!kingIsInCheck(&afterMoveState, map))
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


bool pawnMoves(gameState_s* state, attackMap_s* map, const square start, const square end) {

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



bool knightMoves(gameState_s* state, attackMap_s* map, const square start, const square end) {
    uint64_t pseudoLegal = map->knight[start];
    if(!read(end, pseudoLegal)) return false;
    return moveIsLegal(state, map, start, end, &simulateMoveGeneral);
}

bool kingMoves(gameState_s* state, attackMap_s* map, const square start, const square end) {
    // castling
    uint64_t pseudoLegal = map->king[start];
    if(!read(end, pseudoLegal & ~state->piecesForSide[state->playerToMove])) return false;
}


bool rookMoves(gameState_s* state, const square start, const square end) {


}