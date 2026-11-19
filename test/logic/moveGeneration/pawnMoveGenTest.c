#include <stdio.h>
#include "../test.h"
#include "../../../game/main.h"
#include "../../../game/maps.h"
#include "../../../game/FEN.h"
#include "../../../engine/moveGen.h"
#include "pawnMoveGenTest.h"



bool moveExistsFunc(uint16_t move) {
    for (int i = 0; i < moveCount; i++) {
        if(moves[i] == move) return true;
    }
    return false;
}


void whitePawnPushGen_addsMovesCorrectly() {
    INIT_MOVEGEN("rnbqkbnr/pppppppp/8/8/8/3PP3/1PP1PPPP/RNBQKBNR w KQkq - 0 1");

    uint64_t pushOne = whitePawnPushOne(&state);
    whitePawnSinglePushMoveGen(pushOne, moves, &moveCount);
    whitePawnDoublePushMoveGen(pushOne, &state, moves, &moveCount);

    ASSERT(moveCount == 12);

    ASSERT(moveExists(B2, B3));
    ASSERT(moveExists(B2, B4, DOUBLE_PAWN_PUSH));
    ASSERT(moveExists(D3, D4));
    ASSERT(noSuchMove(D3, D5, DOUBLE_PAWN_PUSH));
}


void pawnPromotionGen_addsMovesCorrectly() {

    INIT_MOVEGEN("r7/PP4PP/8/2P5/7k/8/7N/RNBQKB1R w - - 0 1");

    pawnPromotionGen(whitePawnPushOne(&state), moves, &moveCount, -FILES);

    ASSERT(moveExists(B7, B8, KNIGHT_PROMO));
    ASSERT(moveExists(B7, B8, BISHOP_PROMO));
    ASSERT(moveExists(B7, B8, ROOK_PROMO));
    ASSERT(moveExists(B7, B8, QUEEN_PROMO));

    ASSERT(noSuchMove(C5, C6, QUEEN_PROMO));
    ASSERT(noSuchMove(A7, A8, QUEEN_PROMO));
}

void pawnCaptureGen_addsMovesCorrectly() {

    INIT_MOVEGEN("8/8/8/8/7k/r3bBn1/PPP1PPPP/RN1QKBNR w - - 0 1");

    pawnCaptureGen(&state, &testMap, moves, &moveCount);

    ASSERT(moveExists(B2, A3, CAPTURE));
    ASSERT(moveExists(F2, E3, CAPTURE));
    ASSERT(moveExists(F2, G3, CAPTURE));

    ASSERT(noSuchMove(E2, F3, CAPTURE)); // may not capture own piece
    ASSERT(noSuchMove(H2, A3, CAPTURE)); // may not wrap around capture
}

void pawnCaptureGen_enPassant() {
    INIT_MOVEGEN("8/8/8/8/7k/r3bBn1/PPP1PPPP/RN1QKBNR w - - 0 1");

    pawnCaptureGen(&state, &testMap, moves, &moveCount);


}

void pawnPromotionCaptureGen_addsMovesCorrectly() {

    INIT_MOVEGEN("r7/PP4PP/8/2P5/7k/8/7N/RNBQKB1R w - - 0 1");

    pawnPromotionCaptureGen(&state, &testMap, moves, &moveCount, 0x00FF000000000000);

    ASSERT(moveExists(B7, A8, QUEEN_PROMO_CAP));
    ASSERT(noSuchMove(H7, A8, QUEEN_PROMO_CAP)); // may not wrap around capture
}