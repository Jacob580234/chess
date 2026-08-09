#ifndef CHESS_V2_BITBOARDDATABASE_H
#define CHESS_V2_BITBOARDDATABASE_H
#include "main.h"

typedef struct {
    uint64_t pawn[2][NUM_SQUARES];
    uint64_t knight[NUM_SQUARES];
    uint64_t king[NUM_SQUARES];
} attackMap_s;

void initAttackMaps(attackMap_s* attackMap);
void initKingBitboards(uint64_t* bitboard);
void initKnightBitboards(uint64_t* bitboard);
void initPawnBitboards(uint64_t bitboard[][NUM_SQUARES]);
// void initPawnMoveBitboards(uint64_t bitboard[][NUM_SQUARES]);
// void initPawnAttackBitboards(uint64_t bitboard[][NUM_SQUARES]);

#endif //CHESS_V2_BITBOARDDATABASE_H