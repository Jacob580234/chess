#include "main.h"
#include "print.h"
#include <stdio.h>
#include <stdlib.h>

void printPieceLookup(piece* pieceLookup) {
    for (int i = 8; i > 0; i--) {
        printf("\n");
        for (int j = 0; j < 8; j++) {
            printf("%d  ", pieceLookup[i * 8 - (8 - j)]);
        }
    }
}


void printBitboard(uint64_t bitboard) {
    for (int i = 8; i > 0; i--) {
        printf("\n");
        for (int j = 0; j < 8; j++) {
            printf("%llu  ", bitboard >> (i * 8 - (8 - j)) & 1);
        }
    }
}


char* pieceToStr(piece piece) {
    switch (piece) {
        case pawn:   return "pawn";
        case knight: return "knight";
        case bishop: return "bishop";
        case rook:   return "rook";
        case queen:  return "queen";
        case king:   return "king";
        default: __builtin_unreachable();
    }
}


void printState(gameState_s* state) {
    for (piece piece = pawn; piece <= king; piece++) {
        printf("\n\nWhite %s:", pieceToStr(piece));
        printBitboard(state->bitboard[white][piece]);
        printf("\n\nBlack %s:", pieceToStr(piece));
        printBitboard(state->bitboard[black][piece]);
    }
    printf("\n\nPiece Lookup:");
    printPieceLookup(state->pieceLookup);
    printf("\n\nplayerToMove: %s\ncastlingRights: 0x%x\nenPassantIndex: %d\nfiftyMoveRule: %d\n",
        state->playerToMove == white ? "white" : "black", state->castlingRights, state->enPassantIndex, state->fiftyMoveRule);
    printf("\n\nAll white pieces:\n");
    printBitboard(state->piecesForSide[white]);
    printf("\n\nAll black pieces:\n");
    printBitboard(state->piecesForSide[black]);
    printf("\n\nAll pieces:\n");
    printBitboard(state->allPieces);
}


void printBoard(gameState_s* state) { // TODO: rework this abomination

    const int nextFile = state->playerToMove      == white ? -15 : 15;
    const int fileSkip = state->playerToMove      == white ?   7 :  0;
    const int inc = state->playerToMove           == white ?   1 : -1;
    const square startIndex = state->playerToMove == white ?  A8 : H1;
    const square endIndex = state->playerToMove   == white ?  H1 : A8;
    int numbering = fileSkip + 1;

    printf("%-5d", numbering);
    for (square i = startIndex; i != endIndex; i += inc) {
        if (i % FILES == fileSkip) {
            printf("%3c \n", pieceToChar(i, state));
            printf("%-5d", numbering -= inc);
            i += nextFile;
        }
        printf("%3c ", pieceToChar(i, state));
    }
    printf("%3c\n\n    ", pieceToChar(endIndex, state));

    for (int i = 0; i < FILES; i++) {
        int lettering = state->playerToMove == white ? (i + 'A') : ('A' + 7 - i);
        printf("%4c", lettering);
    }
}


char pieceToChar(square square, gameState_s* state) { // this as well
    switch (state->pieceLookup[square]) {
        case pawn:    return BIT(square) & state->bitboard[white][pawn]   ? 'P' : 'p';
        case knight:  return BIT(square) & state->bitboard[white][knight] ? 'N' : 'n';
        case bishop:  return BIT(square) & state->bitboard[white][bishop] ? 'B' : 'b';
        case rook:    return BIT(square) & state->bitboard[white][rook]   ? 'R' : 'r';
        case queen:   return BIT(square) & state->bitboard[white][queen]  ? 'Q' : 'q';
        case king:    return BIT(square) & state->bitboard[white][king]   ? 'K' : 'k';
        case noPiece: return '-';
        default: __builtin_unreachable();
    }
}