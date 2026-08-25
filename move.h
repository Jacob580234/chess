#ifndef CHESS_V2_MOVE_H
#define CHESS_V2_MOVE_H
#include "bitboardDatabase.h"
#include "main.h"

bool kingIsAttackedByKnight(gameState_s* state, attackMap_s* map);
bool kingIsAttackedByKing(gameState_s* state, attackMap_s* map);
bool kingIsAttackedByPawn(gameState_s* state, attackMap_s* map);
bool kingIsInCheck(gameState_s* state, attackMap_s* map);
bool moveIsLegal(gameState_s* state, attackMap_s* map, square start, square end, void (*simulateMove)(gameState_s*, square, square));
void simulateMoveGeneral(gameState_s* copy, square start, square end);
bool pawnMoves(gameState_s* state, attackMap_s* map, square start, square end);
bool knightMoves(gameState_s* state, attackMap_s* map, square start, square end);
bool kingMoves(gameState_s* state, attackMap_s* map, square start, square end);
bool rookMoves(gameState_s* state, square start, square end);


#endif //CHESS_V2_MOVE_H