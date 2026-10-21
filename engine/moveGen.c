#include "../game/move.h"
#include "moveGen.h"


void inline BLSR(uint64_t* src) {
    __asm__("BLSRQ %1, %0" : "=r"(*src) : "r"(*src));
}


int inline nextBit(uint64_t* bitboard) {
    int result = TZCNT(*bitboard);
    BLSR(bitboard);
    return result;
}


uint64_t inline fetchMoves(gameState_s* restrict state, attackMap_s* restrict map, piece piece, square square) {
    switch (piece) {
        case knight: return knightPseudoLegal(state, map, square);
        case bishop: return bishopPseudoLegal(state, map, square);
        case rook:   return rookPseudoLegal(state, map, square);
        case queen:  return queenPseudoLegal(state, map, square);
        default: __builtin_unreachable();
    }
}


void generateKingMoves(gameState_s* state, attackMap_s* map, uint16_t moves[], int *moveCount) {

    int startSquare = getKingIndex(state);
    uint64_t moveBitboard = kingPseudoLegal(state, map, startSquare);

    while(moveBitboard) {
        int endSquare = nextBit(&moveBitboard);
        moves[*moveCount] = startSquare | endSquare << 6;
        if(state->pieceLookup[endSquare] != noPiece)
            moves[*moveCount] |= CAPTURE;
        (*moveCount)++;
    }

    // TODO: rewrite
    if (state->castlingRights != 0b0000) {
        uint64_t castling = castlingBitboard(state, map);

        while (castling) {
            int castlingPos = nextBit(&castling);
            square kingSide, queenSide;
            state->playerToMove == white ? (queenSide = C1, kingSide = G1) : (queenSide = C8, kingSide = G8);

            if(castlingPos == queenSide)
                moves[(*moveCount)++] = startSquare | queenSide << 6 | QUEEN_CASTLE;
            else
                moves[(*moveCount)++] = startSquare | kingSide << 6 | KING_CASTLE;
        }
    }

}


uint64_t inline whitePawnPushOne(gameState_s* state) {
    return ANDN(state->bitboard[white][pawn] << 8, state->allPieces);
}

uint64_t inline blackPawnPushOne(gameState_s* state) {
    return ANDN(state->bitboard[black][pawn] >> 8, state->allPieces);
}

uint64_t inline whitePawnPushTwo(gameState_s* state) {
    uint64_t pushOne = whitePawnPushOne(state);
    uint64_t fourthRankMask = 0x00000000FF000000;
    return ANDN(pushOne << 8, state->allPieces) & fourthRankMask;
}

uint64_t inline blackPawnPushTwo(gameState_s* state) {
    uint64_t pushOne = blackPawnPushOne(state);
    uint64_t fifthRankMask = 0x000000FF00000000;
    return ANDN(pushOne >> 8, state->allPieces) & fifthRankMask;
}


void generatePawnMoves(gameState_s* state, attackMap_s* map, uint16_t moves[], int *moveCount) {

    if (state->playerToMove == white) {
        uint64_t pushOne = whitePawnPushOne(state);
        while(pushOne) {
            int endSquare = nextBit(&pushOne);
            uint16_t move = (endSquare - 8) | (endSquare << 6);
            if(endSquare >= A8) {
                moves[(*moveCount)++] = move | KNIGHT_PROMO;
                moves[(*moveCount)++] = move | BISHOP_PROMO;
                moves[(*moveCount)++] = move | ROOK_PROMO;
                moves[(*moveCount)++] = move | QUEEN_PROMO;
            }
            else moves[(*moveCount)++] = move;
        }

        uint64_t pushTwo = whitePawnPushTwo(state);
        while (pushTwo) {
            int endSquare = nextBit(&pushTwo);
            moves[(*moveCount)++] = (endSquare - 16) | (endSquare << 6) | DOUBLE_PAWN_PUSH;
        }

        // attacks

    } else { // black
        uint64_t pushOne = blackPawnPushOne(state);
        while(pushOne) {
            int endSquare = nextBit(&pushOne);
            uint16_t move = (endSquare + 8) | (endSquare << 6);
            if(endSquare <= H1) {
                moves[(*moveCount)++] = move | KNIGHT_PROMO;
                moves[(*moveCount)++] = move | BISHOP_PROMO;
                moves[(*moveCount)++] = move | ROOK_PROMO;
                moves[(*moveCount)++] = move | QUEEN_PROMO;
            }
            else moves[(*moveCount)++] = move;
        }
        uint64_t pushTwo = blackPawnPushTwo(state);
        while (pushTwo) {
            int endSquare = nextBit(&pushTwo);
            moves[(*moveCount)++] = (endSquare + 16) | (endSquare << 6) | DOUBLE_PAWN_PUSH;
        }

        // attacks
    }

}


int generateMoves(gameState_s* restrict state, attackMap_s* restrict map, uint16_t moves[]) {

    uint64_t cpyBoards[5] = {
        0, // buffer to make piece indexing correct. TODO: remove
        state->bitboard[state->playerToMove][knight],
        state->bitboard[state->playerToMove][bishop],
        state->bitboard[state->playerToMove][rook],
        state->bitboard[state->playerToMove][queen]
    };

    int moveCount = 0;
    for (piece piece = knight; piece <= queen; piece++) {

        while(cpyBoards[piece]) {

            int startSquare = nextBit(&cpyBoards[piece]);
            uint64_t moveBitboard = fetchMoves(state, map, piece, startSquare);

            while(moveBitboard) {
                int endSquare = nextBit(&moveBitboard);
                uint16_t move = (startSquare | endSquare << 6);
                if(state->pieceLookup[endSquare] != noPiece)
                    moves[moveCount++] = move;
                else moves[moveCount++] = move | CAPTURE;
            }
        }
    }

    generatePawnMoves(state, map, moves, &moveCount);
    generateKingMoves(state, map, moves, &moveCount);

    return moveCount;
}