#pragma once

#include <stdbool.h>

void promotion();
bool kingCanMove(state_s* state, map_s* map);
bool gameIsOver(state_s* state, map_s* map);