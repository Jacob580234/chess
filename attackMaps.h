#ifndef CHESS_V2_BITBOARDDATABASE_H
#define CHESS_V2_BITBOARDDATABASE_H
#include "main.h"

typedef struct {
    uint64_t pawn[2][NUM_SQUARES];
    uint64_t knight[NUM_SQUARES];
    uint64_t king[NUM_SQUARES];

    uint64_t rookPextTable[102400];
    uint64_t rookBlockerMask[NUM_SQUARES];
    uint32_t rookPextTableOffset[NUM_SQUARES];

    uint64_t bishopPextTable[5248];
    uint64_t bishopBlockerMask[NUM_SQUARES];
    uint32_t bishopPextTableOffset[NUM_SQUARES];
} attackMap_s;

#define RANK(src) ((src) / 8)
#define FILE(src) ((src) % 8)

void initAttackMaps(attackMap_s* attackMap);
void initKingMaps(uint64_t* bitboard);
void initKnightMaps(uint64_t* bitboard);
void initPawnMaps(uint64_t bitboard[][NUM_SQUARES]);
void initRookMaps(uint64_t pextTable[], uint64_t blockerMask[], uint32_t pextOffset[]);
void initBishopMaps(uint64_t pextTable[], uint64_t blockerMask[], uint32_t pextOffset[]);
uint64_t pext(uint64_t mask, uint64_t src);
uint64_t pdep(uint64_t mask, uint64_t src);
int popcnt(uint64_t src);
uint64_t generateSlidingPieceMoves(uint64_t blockers, int square, int len[], int incr[]);
void populatePextTable(uint64_t pextTable[], uint64_t blockerMask[], uint32_t pextTableOffset[], int len[], int incr[], int square);
// void initPawnMoveBitboards(uint64_t bitboard[][NUM_SQUARES]);
// void initPawnAttackBitboards(uint64_t bitboard[][NUM_SQUARES]);

#endif //CHESS_V2_BITBOARDDATABASE_H