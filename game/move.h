#ifndef CHESS_V2_MOVE_H
#define CHESS_V2_MOVE_H
#include "maps.h"
#include "main.h"

#define LEN_DOWN(src)  ((src) / 8)
#define LEN_LEFT(src)  ((src) % 8)
#define LEN_RIGHT(src) (7 - (src) % 8)
#define LEN_UP(src)    (7 - (src) / 8)
#define MIN(a,b) ((a) < (b) ? (a) : (b))

bool kingIsInCheck(state_s* state, map_s* map);
bool squareIsAttacked(state_s* state, map_s* map, square square);

bool moveIsLegal(state_s* state, map_s* map, uint16_t move, void (*simulateMove)(state_s*, map_s*, uint16_t));
void simulateMoveGeneral(state_s* copy, map_s* map, uint16_t move);

void updateCastlingRights(state_s* state, map_s* map, square start, square end);
uint64_t castlingBitboard(state_s* state, map_s* map);

bool pawnMoves(state_s* state, map_s* map, uint16_t move);
bool knightMoves(state_s* state, map_s* map, uint16_t move);
bool kingMoves(state_s* state, map_s* map, uint16_t move);
bool rookMoves(state_s* state, map_s* map, uint16_t move);
bool bishopMoves(state_s* state, map_s* map, uint16_t move);
bool queenMoves(state_s* state, map_s* map, uint16_t move);
square getKingIndex(state_s* state);
uint64_t rookPseudoLegal(state_s* state, map_s* map, square start);
uint64_t bishopPseudoLegal(state_s* state, map_s* map, square start);
uint64_t queenPseudoLegal(state_s* state, map_s* map, square start);
uint64_t knightPseudoLegal(state_s* state, map_s* map, square start);
uint64_t kingPseudoLegal(state_s* state, map_s* map, square start);

void handleCapture(state_s* state, square end);
void updatePiecePosition(state_s* state, uint16_t move);

#endif //CHESS_V2_MOVE_H