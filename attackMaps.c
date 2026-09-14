#include <stdint.h>
#include "main.h"
#include "attackMaps.h"
#include "move.h"
#include "print.h"


uint64_t inline pext(uint64_t mask, uint64_t src) {
    uint64_t result;
    __asm__("PEXTQ %1, %2, %0" : "=r"(result) : "r"(mask), "r"(src) : "cc");
    return result;
}


uint64_t inline pdep(uint64_t mask, uint64_t src) {
    uint64_t result;
    __asm__("PDEPQ %1, %2, %0" : "=r"(result) : "r"(mask), "r"(src) : "cc");
    return result;
}

int inline popcnt(uint64_t src) {
    uint64_t result;
    __asm__("POPCNTQ %1, %0" : "=r"(result) : "r"(src) : "cc");
    return (int)result;
}


void initAttackMaps(attackMap_s* attackMap) {
    initPawnMaps(attackMap->pawn);
    initKnightMaps(attackMap->knight);
    initKingMaps(attackMap->king);
    initRookMaps(attackMap->rookPextTable, attackMap->rookBlockerMask, attackMap->rookPextTableOffset);
    initBishopMaps(attackMap->bishopPextTable, attackMap->bishopBlockerMask, attackMap->bishopPextTableOffset);
}


void initKingMaps(uint64_t bitboard[]) {

    const uint64_t AFileMask = 0xFEFEFEFEFEFEFEFE; // bitboard layout means this filters out the left (A) file
    const uint64_t HFileMask = 0x7F7F7F7F7F7F7F7F; // bitboard layout means this filters out the right (H) file
    const uint64_t source    = 0x000001C141C00000; // attack map for king @ index 31 (with file spill)

    for (int i = 0; i < NUM_SQUARES/2; i++) {
        int leftIndex  = 32+i;  // index 32 --> 33 --> 34 ...
        int rightIndex = 31-i; // index 31 --> 30 --> 29 ...

        bitboard[leftIndex]  = source << (i+1);
        bitboard[rightIndex] = source >> i;

        if (i % 8 == 7) { // jumps when rightIndex is on the A file, spilling on the right, vice versa for right
            bitboard[leftIndex]  &= AFileMask;  // remove left spill
            bitboard[rightIndex] &= HFileMask; // remove right spill
        }
        else if (i % 8 == 0) { // jumps when rightIndex is on the H file, spilling on the left, vice versa for left
            bitboard[leftIndex]  &= HFileMask;  // remove right spill
            bitboard[rightIndex] &= AFileMask; // remove left spill
        }
    }
}


void initKnightMaps(uint64_t bitboard[]) {
    // beware of potential issues with shifting these (HFileMask >> x)
    const uint64_t AFileMask = 0xFEFEFEFEFEFEFEFE;
    const uint64_t HFileMask = 0x7F7F7F7F7F7F7F7F;
    const uint64_t source    = 0x0001422002214000; // attack map for knight @ index 31 (with file spill)

    for (int i = 0; i < NUM_SQUARES/2; i++) {
        int leftIndex  = 32+i;  // index 32 --> 33 --> 34 ...
        int rightIndex = 31-i; // index 31 --> 30 --> 29 ...

        bitboard[leftIndex]  = source << (i+1);
        bitboard[rightIndex] = source >> i;

        switch (i % 8) {
            case 0: // rightIndex H file, leftIndex A file
                bitboard[leftIndex]  &= HFileMask >> 1; // remove further right spill
                bitboard[rightIndex] &= AFileMask << 1; // remove further left spill
            case 1: // rightIndex G file, leftIndex B file
                bitboard[leftIndex]  &= HFileMask; // remove right spill
                bitboard[rightIndex] &= AFileMask; // remove left spill
                break;

            case 7: // rightIndex A file, leftIndex H file
                bitboard[leftIndex]  &= AFileMask << 1; // remove further left spill
                bitboard[rightIndex] &= HFileMask >> 1; // remove further right spill
            case 6: // rightIndex B file, leftIndex G file
                bitboard[leftIndex]  &= AFileMask; // remove left spill
                bitboard[rightIndex] &= HFileMask; // remove right spill

            default: break;
        }
    }
}

/*
void initPawnBitboards(uint64_t bitboard[][2][NUM_SQUARES]) {
    initPawnAttackBitboards(bitboard[0]);
    initPawnMoveBitboards(bitboard[1]);
}

void initPawnMoveBitboards(uint64_t bitboard[][NUM_SQUARES]) {

    for (square square = A2; square <= H2; square++) bitboard[white][square] = (BIT(square) << 8) | (BIT(square) << 16);
    for (square square = A7; square <= H7; square++) bitboard[black][square] = (BIT(square) >> 8) | (BIT(square) >> 16);

    for (square square = A3; square <= H7; square++) bitboard[white][square] = BIT(square) << 8;
    for (square square = H6; square >= A2; square--) bitboard[black][square] = BIT(square) >> 8;
}
*/


void initPawnMaps(uint64_t bitboard[][NUM_SQUARES]) {
    const uint64_t AFileMask   = 0xFEFEFEFEFEFEFEFE;
    const uint64_t HFileMask   = 0x7F7F7F7F7F7F7F7F;
    const uint64_t whiteSource = 0x0000000000028000; // attack map for white pawn @ index 8 (with file spill)
    const uint64_t blackSource = 0x0001400000000000; // attack map for black pawn @ index 55 (with file spill)

    for (int i = 0; i < NUM_SQUARES - (2*FILES); i++) {
        int whiteIndex = 8+i;
        int blackIndex = 55-i;

        bitboard[white][whiteIndex] = whiteSource << i;
        bitboard[black][blackIndex] = blackSource >> i;

        if (i % 8 == 7) {
            bitboard[white][whiteIndex] &= AFileMask;
            bitboard[black][blackIndex] &= HFileMask;
        }
        else if (i % 8 == 0) {
            bitboard[white][whiteIndex] &= HFileMask;
            bitboard[black][blackIndex] &= AFileMask;
        }
    }
}

void initRookMaps(uint64_t pextTable[], uint64_t blockerMask[], uint32_t pextTableOffset[]) {

    uint64_t AFileMask     = 0x0101010101010101;
    uint64_t FirstRankMask = 0x00000000000000FF;

    for (int rank = 0; rank < 8; rank++) {
        for (int file = 0; file < 8; file++) {
            square src = rank * 8 + file;
            uint64_t bitboard = (AFileMask << file) | (FirstRankMask << rank * 8);
            clear(src, &bitboard);

            if (RANK(src) != 0) bitboard &= ~FirstRankMask;
            if (RANK(src) != 7) bitboard &= ~(FirstRankMask << 7 * 8);
            if (FILE(src) != 0) bitboard &= ~AFileMask;
            if (FILE(src) != 7) bitboard &= ~(AFileMask << 7);

            blockerMask[src] = bitboard;
        }
    }

    pextTableOffset[0] = 0;
    for (int i = 0; i < 64; i++) {
        int combinations = 1 << popcnt(blockerMask[i]);
        if (i != 63) pextTableOffset[i+1] = (pextTableOffset[i] + combinations);
        for (int j = 0; j < combinations; j++) {
            uint64_t blockers = pdep(blockerMask[i], j);
            pextTable[j + pextTableOffset[i]] = generateRookMoves(blockers, i);
        }
    }
}

void initBishopMaps(uint64_t pextTable[], uint64_t blockerMask[], uint32_t pextTableOffset[]) {

    uint64_t diagonal = 0x8040201008040201;
    uint64_t antiDiagonal = 0x0102040810204080;
    printBitboard(diagonal | antiDiagonal);
}


uint64_t generateRookMoves(uint64_t blockers, int square) {

    uint64_t bitboard = 0;

    for (int i = 0, current = square - 8; i < LEN_DOWN(square); i++, current -= 8) {
        set(current, &bitboard);
        if(read(current, blockers)) break;
    }
    for (int i = 0, current = square - 1; i < LEN_LEFT(square); i++, current--) {
        set(current, &bitboard);
        if(read(current, blockers)) break;
    }
    for (int i = 0, current = square + 1; i < LEN_RIGHT(square); i++, current++) {
        set(current, &bitboard);
        if(read(current, blockers)) break;
    }
    for (int i = 0, current = square + 8; i < LEN_UP(square); i++, current += 8) {
        set(current, &bitboard);
        if(read(current, blockers)) break;
    }

    return bitboard;
}