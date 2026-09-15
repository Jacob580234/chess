#include <stdint.h>
#include "main.h"
#include "attackMaps.h"

#include <stdio.h>

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

    pextTableOffset[0] = 0;
    int incr[] = { -8, -1, 1, 8 };
    for (int i = 0; i < NUM_SQUARES; i++) {
        int len[] = {
            LEN_DOWN(i) - 1,
            LEN_LEFT(i) - 1,
            LEN_RIGHT(i) - 1,
            LEN_UP(i) - 1
        };
        blockerMask[i] = generateSlidingPieceMoves(0ULL, i, len, incr);
        for (int j = 0; j < 4; j++) len[j]++;
        populatePextTable(pextTable, blockerMask, pextTableOffset, len, incr, i);
    }
}


void initBishopMaps(uint64_t pextTable[], uint64_t blockerMask[], uint32_t pextTableOffset[]) {

    pextTableOffset[0] = 0;
    int incr[] = { 7, 9, -9, -7 };
    for (int i = 0; i < NUM_SQUARES; i++) {
        int len[] = {
            MIN(LEN_UP(i), LEN_LEFT(i)) - 1,
            MIN(LEN_UP(i), LEN_RIGHT(i)) - 1,
            MIN(LEN_DOWN(i), LEN_LEFT(i)) - 1,
            MIN(LEN_DOWN(i), LEN_RIGHT(i)) - 1
        };
        blockerMask[i] = generateSlidingPieceMoves(0ULL, i, len, incr);
        for (int j = 0; j < 4; j++) len[j]++;
        populatePextTable(pextTable, blockerMask, pextTableOffset, len, incr, i);
    }

}


void populatePextTable(uint64_t pextTable[], uint64_t blockerMask[], uint32_t pextTableOffset[], int len[], int incr[], int square) {

    int blockerCombinations = 1 << popcnt(blockerMask[square]); // 2^popcnt

    if (square != 63)
        pextTableOffset[square+1] = (pextTableOffset[square] + blockerCombinations);

    for (int j = 0; j < blockerCombinations; j++) {
        uint64_t blockers = pdep(blockerMask[square], j);
        pextTable[j + pextTableOffset[square]] = generateSlidingPieceMoves(blockers, square, len, incr);
    }
}


uint64_t generateSlidingPieceMoves(uint64_t blockers, int square, int len[], int incr[]) {

    uint64_t bitboard = 0;
    for (int i = 0; i < 4; i++) {
        for (int j = 0, current = square + incr[i]; j < len[i]; j++, current += incr[i]) {
            set(current, &bitboard);
            if(read(current, blockers)) break;
        }
    }

    return bitboard;
}