#ifndef CHESS_V2_MOVE_H
#define CHESS_V2_MOVE_H
#include "attackMaps.h"
#include "main.h"

#define LEN_DOWN(src)  ((src) / 8)
#define LEN_LEFT(src)  ((src) % 8)
#define LEN_RIGHT(src) (7 - (src) % 8)
#define LEN_UP(src)    (7 - (src) / 8)
#define MIN(a,b) ((a) < (b) ? (a) : (b))

bool kingIsInCheck(gameState_s* state, attackMap_s* map);

bool moveIsLegal(gameState_s* state, attackMap_s* map, uint16_t move, void (*simulateMove)(gameState_s*, attackMap_s*, uint16_t));
void simulateMoveGeneral(gameState_s* copy, attackMap_s* map, uint16_t move);

bool pawnMoves(gameState_s* state, attackMap_s* map, uint16_t move);
bool knightMoves(gameState_s* state, attackMap_s* map, uint16_t move);
bool kingMoves(gameState_s* state, attackMap_s* map, uint16_t move);
bool rookMoves(gameState_s* state, attackMap_s* map, uint16_t move);
bool bishopMoves(gameState_s* state, attackMap_s* map, uint16_t move);
bool queenMoves(gameState_s* state, attackMap_s* map, uint16_t move);
square getKingIndex(uint64_t bitboard);
uint64_t rookPseudoLegal(gameState_s* state, attackMap_s* map, square start);
uint64_t bishopPseudoLegal(gameState_s* state, attackMap_s* map, square start);
uint64_t queenPseudoLegal(gameState_s* state, attackMap_s* map, square start);

uint64_t ANDN(uint64_t negOp1, uint64_t op2);
int TZCNT(uint64_t src);

#endif //CHESS_V2_MOVE_H