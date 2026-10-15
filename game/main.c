#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "attackMaps.h"
#include "main.h"
#include "FEN.h"
#include "move.h"
#include "print.h"
#include "events.h"


void inline set(const int index, uint64_t* bitboard) {
    *bitboard |= BIT(index);
}

bool inline read(const int index, const uint64_t bitboard) {
    return (bitboard >> index) & 1ULL;
}

void inline clear (const int index, uint64_t* bitboard) {
    *bitboard &= ~BIT(index);
}


square getIndexFromInput(char notation[]) {
    int fileIndex = (notation[1] - '0' - 1) * FILES;
    int rankIndex = notation[0] - 'a';
    square target = fileIndex + rankIndex;

    if (target > H8) return invalid; // 'target' is unsigned: no need to check '< A1'
    return target;
}


piece getPieceOnSquare(gameState_s* state, const square start, const square end) {
    if (start == invalid || end == invalid)
        return noPiece;

    piece piece = state->pieceLookup[start];
    if (piece == noPiece || !(state->bitboard[state->playerToMove][piece] & BIT(start)))
        return noPiece;

    return piece;
}


int main(int argc, char** argv) {

    if (argc > 2) {
        printf("\n\n%s: enclose FEN in double quotes\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    gameState_s state;
    attackMap_s map;
    initAttackMaps(&map);

    char* FEN = (argc == 2 ? argv[1] : "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    parseFEN(FEN, &state, &map);

    char startInput[3], endInput[3];
    char afterTurnMsg[16];

    while(!gameIsOver(&state, &map)) {

        sprintf(afterTurnMsg, "%s to move:", state.playerToMove == white ? "White" : "Black");
        printBoard(&state);
        printf("\n\n%s\n", afterTurnMsg);
        scanf("%2s %2s", startInput, endInput);

        const square startSquare = getIndexFromInput(startInput);
        const square endSquare = getIndexFromInput(endInput);

        piece piece = getPieceOnSquare(&state, startSquare, endSquare);
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
        bool isLegal = testMove(&state, &map, move);

        if (isLegal) {
            state.playerToMove ^= 1;
        } else {
            //strcpy(afterTurnMsg, "Invalid move.");
        }

    }

}