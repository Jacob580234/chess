#ifndef CHESS_V2_FEN_H
#define CHESS_V2_FEN_H
#include "maps.h"

piece charToPiece(char c);
bool isValidLetter(char c);
bool isValidDigit(char c);
bool enemyKingIsSafe(state_s* state, map_s* map);
bool stateIsValid(state_s* state, map_s* map);
int strToInt(char* str);
bool parsePosition(char* position, state_s* state);
bool parseTurn(char turn, state_s* state);
bool parseCastling(char* castling, state_s* state);
bool parseEnPassant(char* enPassant, state_s* state);
bool parseHalfMoves(char* halfMoves, state_s* state);
void parseFEN(char* FEN, state_s* state, map_s* map);


#endif //CHESS_V2_FEN_H