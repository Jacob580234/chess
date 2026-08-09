#ifndef CHESS_V2_MOVE_H
#define CHESS_V2_MOVE_H
#include "main.h"

bool canMoveOne(gameState_s* state, square start);
bool isMovingOne(gameState_s* state, square start, square end);
bool moveTwo(gameState_s* state, square start, square end);
bool isFirstPawnMove(gameState_s* state, square start);




#endif //CHESS_V2_MOVE_H