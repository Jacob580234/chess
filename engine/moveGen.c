#include "../game/move.h"
#include "moveGen.h"


void inline BLSR(uint64_t* src) {
    __asm__("BLSRQ %1, %0" : "+r"(*src));
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


void generateKingMoves(gameState_s* restrict state, attackMap_s* restrict map, uint16_t* restrict moves, int* restrict moveCount) {

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
    return ANDN(state->bitboard[white][pawn] << FILES, state->allPieces);
}

uint64_t inline blackPawnPushOne(gameState_s* state) {
    return ANDN(state->bitboard[black][pawn] >> FILES, state->allPieces);
}

uint64_t inline whitePawnPushTwo(gameState_s* state) {
    uint64_t pushOne = whitePawnPushOne(state);
    uint64_t fourthRankMask = 0x00000000FF000000;
    return ANDN(pushOne << FILES, state->allPieces) & fourthRankMask;
}

uint64_t inline blackPawnPushTwo(gameState_s* state) {
    uint64_t pushOne = blackPawnPushOne(state);
    uint64_t fifthRankMask = 0x000000FF00000000;
    return ANDN(pushOne >> FILES, state->allPieces) & fifthRankMask;
}


void inline addPromoMoves(uint16_t* restrict moves, int* restrict moveCount, uint16_t move) {
    uint64_t promoMask = (KNIGHT_PROMO | (uint64_t)BISHOP_PROMO << 16 | (uint64_t)ROOK_PROMO << 32 | (uint64_t)QUEEN_PROMO << 48);
    *(uint64_t*)&moves[*moveCount] = move | promoMask |
                          ((uint64_t)move << 16)      |
                          ((uint64_t)move << 32)      |
                          ((uint64_t)move << 48)      ;
    *moveCount += 4;
}


void inline addPromoCaptures(uint16_t* restrict moves, int* restrict moveCount, uint16_t move) {
    uint64_t promoMask = (KNIGHT_PROMO_CAP | (uint64_t)BISHOP_PROMO_CAP << 16 | (uint64_t)ROOK_PROMO_CAP << 32 | (uint64_t)QUEEN_PROMO_CAP << 48);
    *(uint64_t*)&moves[*moveCount] = move | promoMask |
                          ((uint64_t)move << 16)      |
                          ((uint64_t)move << 32)      |
                          ((uint64_t)move << 48)      ;
    *moveCount += 4;
}


void generatePawnMoves(gameState_s* restrict state, attackMap_s* restrict map, uint16_t* restrict moves, int* restrict moveCount) {

    if (state->playerToMove == white) {
        uint64_t pushOne = whitePawnPushOne(state);
        while(pushOne) {
            int endSquare = nextBit(&pushOne);
            uint16_t move = (endSquare - FILES) | (endSquare << 6);
            if(unlikely(endSquare >= A8))
                addPromoMoves(moves, moveCount, move);
            else moves[(*moveCount)++] = move;
        }

        uint64_t pushTwo = whitePawnPushTwo(state);
        while (pushTwo) {
            int endSquare = nextBit(&pushTwo);
            moves[(*moveCount)++] = (endSquare - 2*FILES) | (endSquare << 6) | DOUBLE_PAWN_PUSH;
        }

        uint64_t pawns = state->bitboard[white][pawn];
        while (pawns) {
            int startSquare = nextBit(&pawns);
            uint64_t attacks = map->pawn[white][startSquare] & (state->piecesForSide[black] | BIT(state->enPassantIndex));
            while(attacks) {
                int endSquare = nextBit(&attacks);
                uint16_t move = startSquare | (endSquare << 6);
                if(unlikely(endSquare == state->enPassantIndex))
                    moves[(*moveCount)++] = move | EP_CAPTURE;
                else if (unlikely(endSquare >= A8))
                    addPromoCaptures(moves, moveCount, move);
                else moves[(*moveCount)++] = move | CAPTURE;
            }
        }

    } else { // black
        uint64_t pushOne = blackPawnPushOne(state);
        while(pushOne) {
            int endSquare = nextBit(&pushOne);
            uint16_t move = (endSquare + FILES) | (endSquare << 6);
            if(unlikely(endSquare <= H1))
                addPromoMoves(moves, moveCount, move);
            else moves[(*moveCount)++] = move;
        }

        uint64_t pushTwo = blackPawnPushTwo(state);
        while (pushTwo) {
            int endSquare = nextBit(&pushTwo);
            moves[(*moveCount)++] = (endSquare + 2*FILES) | (endSquare << 6) | DOUBLE_PAWN_PUSH;
        }

        uint64_t pawns = state->bitboard[black][pawn];
        while (pawns) {
            int startSquare = nextBit(&pawns);
            uint64_t attacks = map->pawn[black][startSquare] & (state->piecesForSide[white] | BIT(state->enPassantIndex));
            while(attacks) {
                int endSquare = nextBit(&attacks);
                uint16_t move = startSquare | (endSquare << 6);
                if(unlikely(endSquare == state->enPassantIndex))
                    moves[(*moveCount)++] = move | EP_CAPTURE;
                else if (unlikely(endSquare <= H1))
                    addPromoCaptures(moves, moveCount, move);
                else moves[(*moveCount)++] = move | CAPTURE;
            }
        }
    }

}


int generateMoves(gameState_s* restrict state, attackMap_s* restrict map, uint16_t* restrict moves) {

    uint64_t cpyBoards[5] = {
        0, // buffer to make piece indexing correct. TODO: remove
        state->bitboard[state->playerToMove][knight],
        state->bitboard[state->playerToMove][bishop],
        state->bitboard[state->playerToMove][rook],
        state->bitboard[state->playerToMove][queen]
    };

    int moveCount = 0;

    generatePawnMoves(state, map, moves, &moveCount);
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
    generateKingMoves(state, map, moves, &moveCount);

    return moveCount;
}