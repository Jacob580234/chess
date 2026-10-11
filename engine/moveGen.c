#include <stdio.h>
#include <string.h>
#include "../game/move.h"
#include "../game/print.h"
#include "moveGen.h"


int inline nextBit(uint64_t* bitboard) {
    int result = TZCNT(*bitboard);
    clear(result, bitboard);
    return result;
}


uint64_t inline fetchMoves(gameState_s* restrict state, attackMap_s* restrict map, piece piece, square square) {
    switch (piece) {
        case knight: return knightPseudoLegal(state, map, square);
        case bishop: return bishopPseudoLegal(state, map, square);
        case rook:   return rookPseudoLegal(state, map, square);
        case queen:  return queenPseudoLegal(state, map, square);
        case king:   return kingPseudoLegal(state, map, square);
        default: __builtin_unreachable();
    }
}


void generateKingMoves(gameState_s* state, attackMap_s* map, uint16_t moves[], uint64_t cpyKingBoard, int *index) {

    int startSquare = getKingIndex(cpyKingBoard);
    uint64_t moveBitboard = kingPseudoLegal(state, map, startSquare);

    while(moveBitboard) {
        int endSquare = nextBit(&moveBitboard);
        moves[*index] = startSquare | endSquare << 6;
        if(state->pieceLookup[endSquare] != noPiece)
            moves[*index] |= CAPTURE;
        (*index)++;
    }

    // TODO: rewrite
    if (state->castlingRights != 0b0000) {
        uint64_t castling = castlingBitboard(state, map);

        while (castling) {
            int castlingPos = nextBit(&castling);
            square kingSide, queenSide;
            state->playerToMove == white ? (queenSide = C1, kingSide = G1) : (queenSide = C8, kingSide = G8);

            if(castlingPos == queenSide)
                moves[(*index)++] = startSquare | queenSide << 6 | QUEEN_CASTLE;
            else
                moves[(*index)++] = startSquare | kingSide << 6 | KING_CASTLE;

        }
    }

}

void generatePawnMoves(gameState_s* state, attackMap_s* map, uint16_t moves[], uint64_t cpyPawnBoard, int *index) {

}


int generateMoves(gameState_s* restrict state, attackMap_s* restrict map, uint16_t moves[]) {
    uint64_t cpyBoards[6];
    memcpy(cpyBoards, state->bitboard[state->playerToMove], sizeof(state->bitboard[state->playerToMove])); // copy bitboards as 'nextBit' alters them
    int n = 0;

    for (piece piece = knight; piece <= queen; piece++) {

        while(cpyBoards[piece]) {
            int startSquare = nextBit(&cpyBoards[piece]);
            uint64_t moveBitboard = fetchMoves(state, map, piece, startSquare);

            while(moveBitboard) {
                int endSquare = nextBit(&moveBitboard);
                moves[n] = (startSquare | endSquare << 6);
                if(state->pieceLookup[endSquare] != noPiece)
                    moves[n] |= CAPTURE;
                n++;
            }
        }
    }

    generatePawnMoves(state, map, moves, cpyBoards[pawn], &n);
    generateKingMoves(state, map, moves, cpyBoards[king], &n);

    return n;
}