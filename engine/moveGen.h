#pragma once

#include "../game/main.h"
#include "../game/maps.h"
#include <stdint.h>
#include <intrin.h>
#include "../asm.h"

#define PROMO_MASK (uint64_t)(KNIGHT_PROMO | ((uint64_t)BISHOP_PROMO << 16) | ((uint64_t)ROOK_PROMO << 32) | ((uint64_t)QUEEN_PROMO << 48))
#define PROMO_CAP_MASK (uint64_t)(KNIGHT_PROMO_CAP | ((uint64_t)BISHOP_PROMO_CAP << 16) | ((uint64_t)ROOK_PROMO_CAP << 32) | ((uint64_t)QUEEN_PROMO_CAP << 48))

#ifdef __AVX512VBMI2__
    ALWAYS_INLINE __m512i indexVector512() {
        return _mm512_set_epi8(
                    63, 62, 61, 60, 59, 58, 57, 56, 55, 54, 53, 52, 51, 50, 49, 48,
                    47, 46, 45, 44, 43, 42, 41, 40, 39, 38, 37, 36, 35, 34, 33, 32,
                    31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16,
                    15, 14, 13, 12, 11, 10,  9,  8,  7,  6,  5,  4,  3,  2,  1,  0
                );
    }
#endif

uint64_t fetchMoves(state_s* restrict state, map_s* restrict map, piece piece, square square);
int generateMoves(state_s* restrict state, map_s* restrict map, uint16_t moves[]);
void generateKingMoves(state_s* state, map_s* map, uint16_t moves[], int *moveCount);
void generatePawnMoves(state_s* state, map_s* map, uint16_t moves[], int *moveCount);
void addPromoMoves(uint16_t moves[], int* moveCount, uint16_t move, uint64_t promoMask);
int nextBit(uint64_t* bitboard);

uint64_t whitePawnPushOne(state_s* state);
uint64_t blackPawnPushOne(state_s* state);
uint64_t whitePawnPushTwo(uint64_t pushOne, state_s* state);
uint64_t blackPawnPushTwo(uint64_t pushOne, state_s* state);

void pawnCaptureGen(state_s* state, map_s* map, uint16_t moves[restrict static 218], int* restrict moveCount);
void pawnPromotionGen(uint64_t pushOne, uint16_t moves[restrict static 218], int* restrict moveCount, int previousRank);
void pawnPromotionCaptureGen(state_s* state, map_s* map, uint16_t moves[restrict static 218], int* restrict moveCount, uint64_t lastRankMask);

void blackPawnSinglePushMoveGen(uint64_t pushOne, uint16_t moves[static restrict 128], int* restrict moveCount);
void blackPawnDoublePushMoveGen(uint64_t pushOne, state_s* restrict state, uint16_t moves[static restrict 218], int* restrict moveCount);

void whitePawnSinglePushMoveGen(uint64_t pushOne, uint16_t moves[static restrict 128], int* restrict moveCount);
void whitePawnDoublePushMoveGen(uint64_t pushOne, state_s* restrict state, uint16_t moves[static restrict 218], int* restrict moveCount);