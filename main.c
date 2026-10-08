#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "attackMaps.h"
#include "main.h"
#include "FEN.h"
#include "move.h"
#include "print.h"


void inline set(const int index, uint64_t* bitboard) { // void?
    *bitboard |= BIT(index);
}

bool inline read(const int index, const uint64_t bitboard) {
    return (bitboard >> index) & 1ULL;
}

uint64_t inline clear (const int index, uint64_t* bitboard) {
    return *bitboard &= ~BIT(index);
}


square getIndexFromInput(char notation[]) {
    int fileIndex = (notation[1] - '0' - 1) * FILES;
    int rankIndex = notation[0] - 'a';
    square target = fileIndex + rankIndex;

    if (target > H8) return invalid; // 'target' is unsigned: no need to check '< A1'
    return target;
}

// if(kingIndex == kingStartPos) bitboard |= computeCastlingBitboard
uint64_t computeCastlingBitboard(gameState_s* state) { // state->castlingRights[white][queen] ?? hasMoved is needed

    const int sideOffset      = state->playerToMove == white ?  0 : 3;
    const square kingStartPos = state->playerToMove == white ? E1 : E8;
    const square queenSide = state->playerToMove == white ? C1 : C8;
    const square kingSide = state->playerToMove == white ? G1 : G8;

    if(!(0b000010 << sideOffset & state->castlingRights)) // king has moved --> no castling
        return 0;

    // check if rook has been captured


    if(!read(kingStartPos, state->bitboard[state->playerToMove][king]))
        state->castlingRights |= 0b000010 << sideOffset;

    return true;
}


piece validatePiece(gameState_s* state, const square start, const square end) {
    if (start == invalid || end == invalid)
        return noPiece;

    piece piece = state->pieceLookup[start];
    if (piece == noPiece || !(state->bitboard[state->playerToMove][piece] & BIT(start)))
        return noPiece;

    return piece;
}


int main(int argc, char** argv) {

    gameState_s state;

    if (argc > 2) {
        printf("\n\n%s: enclose FEN in double quotes\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char* FEN = (argc == 2 ? argv[1] : "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    parseFEN(FEN, &state);

    attackMap_s attackMap;
    initAttackMaps(&attackMap);

    char startInput[3], endInput[3];
    char afterTurnMsg[16];

    do {
        sprintf(afterTurnMsg, "%s to move:", state.playerToMove == white ? "White" : "Black");
        printBoard(&state);
        printf("\n\n%s\n", afterTurnMsg);
        scanf("%2s %2s", startInput, endInput);

        const square startSquare = getIndexFromInput(startInput);
        const square endSquare = getIndexFromInput(endInput);
        piece piece = validatePiece(&state, startSquare, endSquare);
        if(piece == noPiece) {
            strcpy(afterTurnMsg, "Invalid move.");
            continue;
        }
        uint16_t move = startSquare | endSquare << 6;

        bool (*testMove)(gameState_s*, attackMap_s*, uint16_t move);
        switch (piece) {
            case pawn:   testMove = &pawnMoves;   break;
            case knight: testMove = &knightMoves; break;
            case bishop: testMove = &bishopMoves; break;
            case rook:   testMove = &rookMoves;   break;
            case queen:  testMove = &queenMoves;  break;
            case king:   testMove = &kingMoves;   break;
            default: __builtin_unreachable();
        }
        bool isLegal = testMove(&state, &attackMap, move);

        if (isLegal) {
            state.playerToMove ^= 1;
        } else {
            //strcpy(afterTurnMsg, "Invalid move.");
        }

    } while(true);

}