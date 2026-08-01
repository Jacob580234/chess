#ifndef CHESS_V2_PRINT_H
#define CHESS_V2_PRINT_H
#include "main.h"

void printPieceLookup(piece* pieceLookup);
void printBitboard(uint64_t bitboard);
void printState(gameState_s* state);
char pieceToChar(square square, gameState_s* state);
void printBoard(gameState_s* state);

#endif //CHESS_V2_PRINT_H