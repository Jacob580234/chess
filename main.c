#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "bitboardDatabase.h"
#include "main.h"
#include "FEN.h"
#include "print.h"

/*
 *  checking for kingCheck: can tailor and optimize check for which bitboard youre checking. i.e. probably
 *  some constant time check to see if opposite color bishop is on same diagonal as king; if not, no need to check further
 *
 *  when attempting to make a move, first check if its on the same file / rank (for rooks), diagonal (for bishops etc...)
 *  if not, simply exit the computation. then check if there are any pieces in the way, and lastly, if you would be in check
 *  for knight, perhaps you can do something like, if desired index is above knight position, no need to check the 4
 *  indices that would be above the knight (10, 11, 1, 2), and vice versa with below (4, 5, 7, 8)
 *
 *  make early returns for hasMoved check --> if left rook has moved, no need to check all the stuff for that etc...
 *
 *  for general "if king would be in check" checks, use same formula to see if there are any bishops on its diagonal,
 *  rooks on its file/rank, knights on the knight clock squares etc... "pieces attacking this square". if the | of all
 *  pieces attacking the king == 0, king is not in check. reverse engineer attack maps?
 *
 *  for checking pawn moves: move_bitboard & own_pieces & enemy_pieces == 0
 */


uint64_t inline set(const int index, uint64_t* bitboard) { // void?
    return *bitboard |= BIT(index);
}

bool inline read(const int index, const uint64_t bitboard) {
    return (bitboard >> index) & 1ULL;
}

uint64_t inline clear (const int index, uint64_t* bitboard) {
    return *bitboard &= ~BIT(index);
}

uint64_t inline getAllPiecesForSide(side side, gameState_s* state) { // perhaps have a single function updating bitboard and piece array
    return (
        state->bitboard[side][pawn]   |
        state->bitboard[side][knight] |
        state->bitboard[side][bishop] |
        state->bitboard[side][rook]   |
        state->bitboard[side][queen]  |
        state->bitboard[side][king]
    );
}

square getIndexFromInput(char notation[]) {
    int fileIndex = (notation[1] - '0' - 1) * FILES;
    int rankIndex = notation[0] - 'a';
    square target = fileIndex + rankIndex;

    if (target < A1 || target > H8) return invalid;
    return target;
}


uint64_t computeCastlingBitboard(gameState_s* state) { // state->castlingRights[white][queen] ?? hasMoved is needed

    const int sideOffset      = state->playerToMove == white ?  0 : 3;
    const square kingStartPos = state->playerToMove == white ? E1 : E8;

    if(read(0b000010 << sideOffset, state->castlingRights)) // king has moved --> no castling
        return 0;

    if(!read(kingStartPos, state->bitboard[state->playerToMove][king]))
        state->castlingRights |= 0b000010 << sideOffset;

    return true;
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
    sprintf(afterTurnMsg, "%s to move:", state.playerToMove == white ? "White" : "Black");

    do {
        printBoard(&state);
        printf("\n\n%s\n", afterTurnMsg);
        scanf("%2s %2s", startInput, endInput);

        const square startSquare = getIndexFromInput(startInput);
        const square endSquare = getIndexFromInput(endInput);

        piece piece;
        if (startSquare == invalid || endSquare == invalid || !(state.bitboard[state.playerToMove][piece = state.pieceLookup[startSquare]] & BIT(startSquare))) {
            strcpy(afterTurnMsg, "Invalid move.");
            continue;
        }

        // streamline above and below
        uint64_t map;
        switch (piece) {
            case pawn: map = attackMap.pawn[state.playerToMove][startSquare]; break;
            case knight: map = attackMap.knight[startSquare]; break;
            case king: map = attackMap.king[startSquare]; break;
            default: map = 0; break;
        }



        if (1/* MOVE IS LEGAL */) {
            state.playerToMove ^= 1; // change turn
            sprintf(afterTurnMsg, "%s to move:", state.playerToMove == white ? "White" : "Black");
        } else {
            strcpy(afterTurnMsg, "Invalid move.");
        }

    } while(true);

}