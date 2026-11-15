#include "../game/move.h"
#include "moveGen.h"
#include "../asm.h"
#include "../game/print.h"

int inline nextBit(uint64_t* bitboard) {
    int result = TZCNT(*bitboard);
    *bitboard = BLSR(*bitboard);
    return result;
}


uint64_t inline fetchMoves(state_s* restrict state, map_s* restrict map, piece piece, square square) {
    switch (piece) {
        case knight: return knightPseudoLegal(state, map, square);
        case bishop: return bishopPseudoLegal(state, map, square);
        case rook:   return rookPseudoLegal(state, map, square);
        case queen:  return queenPseudoLegal(state, map, square);
        default: UNREACHABLE();
    }
}


void generateKingMoves(state_s* restrict state, map_s* restrict map, uint16_t* restrict moves, int* restrict moveCount) {

    int startSquare = getKingIndex(state);
    uint64_t moveBitboard = kingPseudoLegal(state, map, startSquare);

    while(moveBitboard) {
        int endSquare = nextBit(&moveBitboard);
        uint16_t move = startSquare | (endSquare << 6);
        if(state->pieceLookup[endSquare] != noPiece)
            moves[*moveCount] = move | CAPTURE;
        else moves[*moveCount] = move;
        (*moveCount)++;
    }

    if (state->castlingRights != 0b0000) {

        if (state->playerToMove == white) {
            if(WHITE_OOO & state->castlingRights && !(map->castlingPath[WHITE_OOO] & state->allPieces))
                moves[(*moveCount)++] = startSquare | (C1 << 6) | QUEEN_CASTLE;
            if(WHITE_OO & state->castlingRights && !(map->castlingPath[WHITE_OO]  & state->allPieces))
                moves[(*moveCount)++] = startSquare | (G1 << 6) | KING_CASTLE;

        } else {
            if(BLACK_OOO & state->castlingRights && !(map->castlingPath[BLACK_OOO] & state->allPieces))
                moves[(*moveCount)++] = startSquare | (C8 << 6) | QUEEN_CASTLE;
            if(BLACK_OO & state->castlingRights && !(map->castlingPath[BLACK_OO]  & state->allPieces))
                moves[(*moveCount)++] = startSquare | (G8 << 6) | KING_CASTLE;
        }

    }

}


uint64_t inline whitePawnPushOne(state_s* state) {
    return ANDN(state->allPieces, state->bitboard[white][pawn] << FILES);
}

uint64_t inline blackPawnPushOne(state_s* state) {
    return ANDN(state->allPieces, state->bitboard[black][pawn] >> FILES);
}

uint64_t inline whitePawnPushTwo(uint64_t pushOne, state_s* state) {
    uint64_t fourthRankMask = 0x00000000FF000000;
    return ANDN(state->allPieces, pushOne << FILES) & fourthRankMask;
}

uint64_t inline blackPawnPushTwo(uint64_t pushOne, state_s* state) {
    uint64_t fifthRankMask = 0x000000FF00000000;
    return ANDN(state->allPieces, pushOne >> FILES) & fifthRankMask;
}


void inline addPromoMoves(uint16_t* restrict moves, int* restrict moveCount, uint16_t move, uint64_t promoMask) {
    *(uint64_t*)&moves[*moveCount] = (move         |
                           ((uint64_t)move << 16)  |
                           ((uint64_t)move << 32)  |
                           ((uint64_t)move << 48)) | promoMask;
    *moveCount += 4;
}


void inline whitePawnPushMoveGenSIMD(__m512i indexVector512, state_s* restrict state, uint16_t moves[static restrict 218], int* restrict moveCount) {
    uint64_t pushOne = whitePawnPushOne(state);
    uint64_t onePushNoPromo = pushOne  & 0x00FFFFFFFFFFFF00;

    __m512i endSquareVector512 = _mm512_maskz_compress_epi8(onePushNoPromo, indexVector512); // simulated vector TZCNT on each bit
    __m128i endSquareVector128 = _mm_cvtepu8_epi16(_mm512_castsi512_si128(endSquareVector512));
    __m128i startSquareVector128 = _mm_sub_epi16(endSquareVector128, _mm_set1_epi16(8));
    __m128i moveVector128 = _mm_or_si128(startSquareVector128, _mm_slli_epi16(endSquareVector128, 6));
    _mm_storeu_epi16(&moves[*moveCount], moveVector128); // storing extra 0-moves here doesn't matter as moveCount only gets incremented by the actual amount of moves
    *moveCount += POPCNT(onePushNoPromo);

    // double moves
    uint64_t pushTwo = whitePawnPushTwo(pushOne, state);
    int doublePushes = POPCNT(pushTwo);
    if(doublePushes != 0) {
        __m512i endSquareVector512D = _mm512_maskz_compress_epi8(pushTwo, indexVector512);
        __m128i endSquareVector128D = _mm_cvtepu8_epi16(_mm512_castsi512_si128(endSquareVector512D));
        __m128i startSquareVector128D = _mm_sub_epi16(endSquareVector128D, _mm_set1_epi16(16));
        __m128i moveVector128D = _mm_or_si128(startSquareVector128D, _mm_slli_epi16(endSquareVector128D, 6));
        __m128i finalMoveVector128D = _mm_or_si128(moveVector128D, _mm_set1_epi16(DOUBLE_PAWN_PUSH));
        _mm_storeu_epi16(&moves[*moveCount], finalMoveVector128D);
        *moveCount += POPCNT(pushTwo);
    }

    // promotions
    pawnPromotionGen(pushOne, moves, moveCount, -FILES);
}


void inline blackPawnPushMoveGenSIMD(__m512i indexVector512, state_s* restrict state, uint16_t moves[static restrict 218], int* restrict moveCount) {

    // single moves
    uint64_t pushOne = blackPawnPushOne(state); // blackPawnPushOne
    uint64_t onePushNoPromo = pushOne & 0x00FFFFFFFFFFFF00;
    __m512i endSquareVector512 = _mm512_maskz_compress_epi8(onePushNoPromo, indexVector512);
    __m128i endSquareVector128 = _mm_cvtepu8_epi16(_mm512_castsi512_si128(endSquareVector512));
    __m128i startSquareVector128 = _mm_add_epi16(endSquareVector128, _mm_set1_epi16(8)); // add/sub
    __m128i moveVector128 = _mm_or_si128(startSquareVector128, _mm_slli_epi16(endSquareVector128, 6));
    _mm_storeu_epi16(&moves[*moveCount], moveVector128);
    *moveCount += POPCNT(onePushNoPromo);

    // double moves (probably isnt even worth it to vectorize these)
    uint64_t pushTwo = blackPawnPushTwo(pushOne, state);
    int doublePushes = POPCNT(pushTwo);
    if(doublePushes != 0) {
        __m512i endSquareVector512D = _mm512_maskz_compress_epi8(pushTwo, indexVector512);
        __m128i endSquareVector128D = _mm_cvtepu8_epi16(_mm512_castsi512_si128(endSquareVector512D));
        __m128i startSquareVector128D = _mm_add_epi16(endSquareVector128D, _mm_set1_epi16(16)); // add/sub
        __m128i moveVector128D = _mm_or_si128(startSquareVector128D, _mm_slli_epi16(endSquareVector128D, 6));
        __m128i finalMoveVector128D = _mm_or_si128(moveVector128D, _mm_set1_epi16(DOUBLE_PAWN_PUSH));
        _mm_storeu_epi16(&moves[*moveCount], finalMoveVector128D);
        *moveCount += doublePushes;
    }

    // promotions
    pawnPromotionGen(pushOne, moves, moveCount, FILES);
}


void inline pawnCaptureGen(state_s* restrict state, map_s* restrict map, uint16_t moves[restrict static 218], int* restrict moveCount) {
    uint64_t nonPromoPawns = state->bitboard[state->playerToMove][pawn] & 0x00FFFFFFFFFFFF00;

    while (nonPromoPawns) {
        int startSquare = nextBit(&nonPromoPawns);
        uint64_t attacks = map->pawn[state->playerToMove
            ][startSquare] & (state->piecesForSide[state->playerToMove^1] | BIT(state->enPassantIndex));

        while(attacks) {
            int endSquare = nextBit(&attacks);
            uint16_t move = startSquare | (endSquare << 6);
            if(UNLIKELY(endSquare == state->enPassantIndex))
                moves[(*moveCount)++] = move | EN_PASSANT;
            else moves[(*moveCount)++] = move | CAPTURE;
        }
    }
}

void inline pawnPromotionGen(uint64_t pushOne, uint16_t moves[restrict static 218], int* restrict moveCount, int previousRank) {
    uint64_t promoPawns = pushOne & 0xFF000000000000FF;
    while (promoPawns) {
        int endSquare = nextBit(&promoPawns);
        uint16_t move = (endSquare + previousRank) | (endSquare << 6);
        addPromoMoves(moves, moveCount, move, PROMO_MASK);
    }
}

void inline pawnPromotionCaptureGen(state_s* state, map_s* map, uint16_t moves[restrict static 218], int* restrict moveCount, uint64_t lastRankMask) {
    uint64_t promoCapturePawns = state->bitboard[state->playerToMove][pawn] & lastRankMask;

    while(promoCapturePawns) {
        int startSquare = nextBit(&promoCapturePawns);
        uint64_t attacks = map->pawn[state->playerToMove][startSquare] & state->piecesForSide[state->playerToMove^1];
        while(UNLIKELY(attacks)) { // probably better way than while loop since max is 2 iterations: 0 99% of the time
            int endSquare = nextBit(&attacks);
            uint16_t move = startSquare | (endSquare << 6);
            addPromoMoves(moves, moveCount, move, PROMO_CAP_MASK);
        }
    }
}

void inline whitePawnPushMoveGen(state_s* restrict state, uint16_t moves[static restrict 218], int* restrict moveCount) {
    uint64_t pushOne = whitePawnPushOne(state);
    while(pushOne) {
        int endSquare = nextBit(&pushOne);
        moves[(*moveCount)++] = (endSquare - FILES) | (endSquare << 6);
    }

    pawnPromotionGen(pushOne, moves, moveCount, -FILES);

    uint64_t pushTwo = whitePawnPushTwo(pushOne, state);
    while (pushTwo) {
        int endSquare = nextBit(&pushTwo);
        moves[(*moveCount)++] = (endSquare - 2*FILES) | (endSquare << 6) | DOUBLE_PAWN_PUSH;
    }
}



void inline blackPawnPushMoveGen(state_s* restrict state, uint16_t moves[static restrict 218], int* restrict moveCount) {
    uint64_t pushOne = blackPawnPushOne(state);
    while(pushOne) {
        int endSquare = nextBit(&pushOne);
        moves[(*moveCount)++] = (endSquare + FILES) | (endSquare << 6);
    }

    pawnPromotionGen(pushOne, moves, moveCount, FILES);

    uint64_t pushTwo = blackPawnPushTwo(pushOne, state);
    while (pushTwo) {
        int endSquare = nextBit(&pushTwo);
        moves[(*moveCount)++] = (endSquare + 2*FILES) | (endSquare << 6) | DOUBLE_PAWN_PUSH;
    }
}


// TODO: refactor this
void inline generatePawnMoves(state_s* restrict state, map_s* restrict map, uint16_t moves[restrict static 218], int* restrict moveCount) {

    #ifdef __AVX512VBMI2__
        __m512i indexVector512 = _mm512_set_epi8(
            63, 62, 61, 60, 59, 58, 57, 56, 55, 54, 53, 52, 51, 50, 49, 48,
            47, 46, 45, 44, 43, 42, 41, 40, 39, 38, 37, 36, 35, 34, 33, 32,
            31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16,
            15, 14, 13, 12, 11, 10,  9,  8,  7,  6,  5,  4,  3,  2,  1,  0
        );

        if(state->playerToMove == white) {
            whitePawnPushMoveGenSIMD(indexVector512, state, moves, moveCount);
            pawnPromotionCaptureGen(state, map, moves, moveCount, 0x00FF000000000000);
        } else { // black
            blackPawnPushMoveGenSIMD(indexVector512, state, moves, moveCount);
            pawnPromotionCaptureGen(state, map, moves, moveCount, 0x000000000000FF00);
        }
    #else
        if (state->playerToMove == white) {
            whitePawnPushMoveGen(state, moves, moveCount);
            pawnPromotionCaptureGen(state, map, moves, moveCount, 0x00FF000000000000);
        } else { // black
            blackPawnPushMoveGen(state, moves, moveCount);
            pawnPromotionCaptureGen(state, map, moves, moveCount, 0x000000000000FF00);
        }
    #endif

    pawnCaptureGen(state, map, moves, moveCount);
}


int generateMoves(state_s* restrict state, map_s* restrict map, uint16_t* restrict moves) {

    uint64_t cpyBoards[5] = {
        0, // buffer to make piece indexing correct. TODO: remove
        state->bitboard[state->playerToMove][knight],
        state->bitboard[state->playerToMove][bishop],
        state->bitboard[state->playerToMove][rook],
        state->bitboard[state->playerToMove][queen]
    };

    int moveCount = 0;

    generatePawnMoves(state, map, moves, &moveCount);

    // TODO: also make vectorized functions for these (especially queen)
    for (piece piece = knight; piece <= queen; piece++) {

        while(cpyBoards[piece]) {
            int startSquare = nextBit(&cpyBoards[piece]);
            uint64_t moveBitboard = fetchMoves(state, map, piece, startSquare);

            while(moveBitboard) {
                int endSquare = nextBit(&moveBitboard);
                uint16_t move = startSquare | (endSquare << 6);
                if(state->pieceLookup[endSquare] == noPiece)
                    moves[moveCount++] = move;
                else moves[moveCount++] = move | CAPTURE;
            }
        }
    }

    generateKingMoves(state, map, moves, &moveCount);

    return moveCount;
}