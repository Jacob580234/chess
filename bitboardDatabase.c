#include <stdint.h>
#include "main.h"
#include "bitboardDatabase.h"
#include "print.h"

void initAttackMaps(attackMap_s* attackMap) {
    initPawnBitboards(attackMap->pawn);
    initPawnBitboards(attackMap->pawn);
    initKnightBitboards(attackMap->knight);
    initKingBitboards(attackMap->king);
}


void initKingBitboards(uint64_t bitboard[]) {

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


void initKnightBitboards(uint64_t bitboard[]) {
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


void initPawnBitboards(uint64_t bitboard[][NUM_SQUARES]) {
    const uint64_t AFileMask = 0xFEFEFEFEFEFEFEFE;
    const uint64_t HFileMask = 0x7F7F7F7F7F7F7F7F;
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