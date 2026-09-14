#ifndef CHESS_V2_MOVE_H
#define CHESS_V2_MOVE_H
#include "attackMaps.h"
#include "main.h"

#define ANDN(src1Neg, src2, res) __asm__("ANDN %1, %2, %0" : "=r"(res) : "r"(src1Neg), "r"(src2))
#define LEN_DOWN(src)  ((src) / 8)
#define LEN_LEFT(src)  ((src) % 8)
#define LEN_RIGHT(src) (7 - (src) % 8)
#define LEN_UP(src)    (7 - (src) / 8)
#define MIN(a,b) ((a) < (b) ? (a) : (b))

bool kingIsAttackedByKnight(gameState_s* state, attackMap_s* map);
bool kingIsAttackedByKing(gameState_s* state, attackMap_s* map);
bool kingIsAttackedByPawn(gameState_s* state, attackMap_s* map);
bool kingIsInCheck(gameState_s* state, attackMap_s* map);
bool moveIsLegal(gameState_s* state, attackMap_s* map, square start, square end, void (*simulateMove)(gameState_s*, square, square));
void simulateMoveGeneral(gameState_s* copy, square start, square end);
bool pawnMoves(gameState_s* state, attackMap_s* map, square start, square end);
bool knightMoves(gameState_s* state, attackMap_s* map, square start, square end);
bool kingMoves(gameState_s* state, attackMap_s* map, square start, square end);
bool rookMoves(gameState_s* state, attackMap_s* map, square start, square end);
square getKingIndex(uint64_t bitboard);


#endif //CHESS_V2_MOVE_H