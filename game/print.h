#pragma once

#include "main.h"
#include <stdio.h>

void printBinary(uint64_t num);
void printPieceLookup(piece* pieceLookup);
void printBitboard(uint64_t bitboard);
void printState(state_s* state);
char pieceToChar(square square, state_s* state);
void printBoard(state_s* state);
char* pieceToStr(piece piece);