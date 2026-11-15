#ifndef CHESS_V2_EVENTS_H
#define CHESS_V2_EVENTS_H
#include <stdbool.h>

void promotion();
bool kingCanMove(state_s* state, map_s* map);
bool gameIsOver(state_s* state, map_s* map);

#endif //CHESS_V2_EVENTS_H
