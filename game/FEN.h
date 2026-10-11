#ifndef CHESS_V2_FEN_H
#define CHESS_V2_FEN_H
#include "attackMaps.h"

piece charToPiece(char c);
bool isValidLetter(char c);
bool isValidDigit(char c);
bool kingIsValid(uint64_t bitboard);
bool stateIsValid(gameState_s* state, attackMap_s* map);
int strToInt(char* str);
bool parsePosition(char* position, gameState_s* state);
bool parseTurn(char turn, gameState_s* state);
bool parseCastling(char* castling, gameState_s* state);
bool parseEnPassant(char* enPassant, gameState_s* state);
bool parseHalfMoves(char* halfMoves, gameState_s* state);
void parseFEN(char* FEN, gameState_s* state, attackMap_s* map);


#endif //CHESS_V2_FEN_H