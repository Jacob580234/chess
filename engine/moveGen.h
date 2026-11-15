#ifndef CHESS_V2_MOVEGEN_H
#define CHESS_V2_MOVEGEN_H

#include "../game/main.h"
#include "../game/maps.h"
#include <stdint.h>
#include <intrin.h>

#define PROMO_MASK (uint64_t)(KNIGHT_PROMO | ((uint64_t)BISHOP_PROMO << 16) | ((uint64_t)ROOK_PROMO << 32) | ((uint64_t)QUEEN_PROMO << 48))
#define PROMO_CAP_MASK (uint64_t)(KNIGHT_PROMO_CAP | ((uint64_t)BISHOP_PROMO_CAP << 16) | ((uint64_t)ROOK_PROMO_CAP << 32) | ((uint64_t)QUEEN_PROMO_CAP << 48))


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

#ifdef __AVX512VBMI2__
    void whitePawnPushMoveGenSIMD(__m512i indexVector512, state_s* restrict state, uint16_t moves[static restrict 218], int* restrict moveCount);
    void blackPawnPushMoveGenSIMD(__m512i indexVector512, state_s* restrict state, uint16_t moves[static restrict 218], int* restrict moveCount);
#endif

void pawnCaptureGen(state_s* state, map_s* map, uint16_t moves[restrict static 218], int* restrict moveCount);
void pawnPromotionGen(uint64_t pushOne, uint16_t moves[restrict static 218], int* restrict moveCount, int previousRank);
void pawnPromotionCaptureGen(state_s* state, map_s* map, uint16_t moves[restrict static 218], int* restrict moveCount, uint64_t lastRankMask);

void blackPawnPushMoveGen(state_s* restrict state, uint16_t moves[static restrict 218], int* restrict moveCount);
void whitePawnPushMoveGen(state_s* restrict state, uint16_t moves[static restrict 218], int* restrict moveCount);
#endif //CHESS_V2_MOVEGEN_H