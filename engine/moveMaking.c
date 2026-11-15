#include "moveMaking.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "../asm.h"
#include "../game/main.h"
#include "../game/maps.h"
#include "../game/move.h"

void promotion(state_s* state, piece piece, uint16_t move) {

}


void promotionCapture(state_s* state, piece piece, uint16_t move) {

}


void makeMove(state_s* state, map_s* map, uint16_t move) {

    switch (move & 0xF000) {
        case QUIET:            updatePiecePosition(state, move); break;
        case DOUBLE_PAWN_PUSH: updatePiecePosition(state, move); state->enPassantIndex = END(move) + (state->playerToMove == white ? -8 : 8); return;
        case KING_CASTLE:      updatePiecePosition(state, move); // + move rook
        case QUEEN_CASTLE:     updatePiecePosition(state, move); // + move rook
        case CAPTURE:          handleCapture(state, END(move)); break;
        case EN_PASSANT:       updatePiecePosition(state, move);
        case KNIGHT_PROMO:     promotion(state, knight, move);
        case BISHOP_PROMO:     promotion(state, bishop, move);
        case ROOK_PROMO:       promotion(state, rook, move);
        case QUEEN_PROMO:      promotion(state, queen, move);
        case KNIGHT_PROMO_CAP: promotionCapture(state, knight, move);
        case BISHOP_PROMO_CAP: promotionCapture(state, bishop, move);
        case ROOK_PROMO_CAP:   promotionCapture(state, rook, move);
        case QUEEN_PROMO_CAP:  promotionCapture(state, queen, move);
        default: UNREACHABLE();
    }
    state->enPassantIndex = invalid; // maybe make invalid index = 0 instead (or maybe dont), as it will never be that anyway, and xor is faster
}


void undoMove(state_s* state, map_s* map, uint16_t move) {
    switch (move & 0xF000) {
        case QUIET: break;
        default: break;
    }
}


void inline copyState(state_s* src, state_s* dest) {
    memcpy(dest, src, sizeof(uint64_t));
}