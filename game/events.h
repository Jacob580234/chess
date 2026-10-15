#ifndef CHESS_V2_EVENTS_H
#define CHESS_V2_EVENTS_H
#include <stdbool.h>

void promotion();
bool kingCanMove(gameState_s* state, attackMap_s* map);
bool gameIsOver(gameState_s* state, attackMap_s* map);

#endif //CHESS_V2_EVENTS_H
