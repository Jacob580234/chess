#include "move.h"
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include "main.h"
#include "bitboardDatabase.h"


bool inline canMoveOne(gameState_s* state, const square start) {
    return (state->playerToMove == white ? BIT(start) << 8 : BIT(start) >> 8) & ~state->allPieces;
}

bool inline isMovingOne(gameState_s* state, const square start, const square end) {
    return state->playerToMove == white ? BIT(start) << 8 == BIT(end) : BIT(start) >> 8 == BIT(end);
}

bool inline moveTwo(gameState_s* state, const square start, const square end) {
    return ((state->playerToMove == white ? BIT(start) << 16 : BIT(start) >> 16) == BIT(end)) & ~state->allPieces;
}

bool inline isFirstPawnMove(gameState_s* state, const square start) {
    return state->playerToMove == white ? start <= H2 : start >= A7;
}

bool pawnMoves(gameState_s* state, attackMap_s* map, const square start, const square end) {

    uint64_t attackPseudoLegal = map->pawn[state->playerToMove][start];
    if (read(end, attackPseudoLegal & state->piecesForSide[state->playerToMove ^ 1] | BIT(state->enPassantIndex))) { // combine into below normal move check? idk
        // pawn can attack and there is opponent piece there (or en passant)
        if (end == state->enPassantIndex) {
            // handle it
        }
    }


    if(!canMoveOne(state, start)) return false; // if you cant move one, you cant move two; no matter


    if (isMovingOne(state, start, end)) {
        // SIMULATE
    }
    else if (isFirstPawnMove(state, start) && moveTwo(state, start, end)) {
        state->enPassantIndex = state->playerToMove == white ? start + 8 : start - 8;
        // SIMULATE
    }
    else return false;




    // checking for promotion after move has been deemed legal or before? shouldnt matter so after is easier ?
    // perhaps something to do with stalemate?
}


bool kingIsInCheck(gameState_s* state, attackMap_s* map) {
    if(map->knight[state->kingIndex[state->playerToMove]] & state->bitboard[state->playerToMove ^ 1][knight]) return true;
    if(map->king[state->kingIndex[state->playerToMove]] & state->bitboard[state->playerToMove ^ 1][king]) return true;
    if(map->pawn[state->playerToMove ^ 1][state->kingIndex[state->playerToMove]] & state->bitboard[state->playerToMove ^ 1][pawn]) return true;

}


gameState_s* simulateMove(gameState_s* state) {

}


bool rookMoves(gameState_s* state, const square start, const square end) {
    // use PEXT with a mask of the file youre trying to move along, &'ed with ^ of blocking pieces or something. somehow check
    // or read king index or rook index or something from result

}