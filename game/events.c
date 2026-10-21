#include <stdbool.h>
#include "attackMaps.h"
#include "move.h"
#include "events.h"
#include "../engine/moveGen.h" // move nextBit into something else


bool gameIsOver(gameState_s* state, attackMap_s* map) {
    if(!kingCanMove(state, map)) {
        if (kingIsInCheck(state, map))
            return true;
        // check for stalemate
        // if(!otherPiecesCanMove()) return true;

    }
    return false;
}


void promotion() {}

bool kingCanMove(gameState_s* state, attackMap_s* map) {
    uint64_t kingSquares = kingPseudoLegal(state, map, getKingIndex(state));
    while(kingSquares) {
        const square move = nextBit(&kingSquares);
        if(!squareIsAttacked(state, map, move)) return true;
    }
    return false;
}