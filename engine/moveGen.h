#ifndef CHESS_V2_MOVEGEN_H
#define CHESS_V2_MOVEGEN_H

#include "../game/main.h"
#include "../game/attackMaps.h"
#include <stdint.h>

uint64_t fetchMoves(gameState_s* restrict state, attackMap_s* restrict map, piece piece, square square);
int generateMoves(gameState_s* restrict state, attackMap_s* restrict map, uint16_t moves[]);
void generateKingMoves(gameState_s* state, attackMap_s* map, uint16_t moves[], uint64_t cpyKingBoard, int *index);
void generatePawnMoves(gameState_s* state, attackMap_s* map, uint16_t moves[], uint64_t cpyPawnBoard, int *index);
int nextBit(uint64_t* bitboard);

#endif //CHESS_V2_MOVEGEN_H