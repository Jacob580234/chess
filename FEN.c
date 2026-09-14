#include "main.h"
#include "fen.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "move.h"
#include "print.h"

piece inline charToPiece(char c) {
    switch(c) {
        case 'P': case 'p': return pawn;
        case 'N': case 'n': return knight;
        case 'B': case 'b': return bishop;
        case 'R': case 'r': return rook;
        case 'Q': case 'q': return queen;
        case 'K': case 'k': return king;
        default: return noPiece;
    }
}


bool isValidLetter(char c) {
    return charToPiece(c) != noPiece;
}

bool inline isValidDigit(char c) {
    return c >= '1' && c <= '8';
}

bool kingIsValid(uint64_t bitboard) {
    uint64_t result;
    __asm__("POPCNT %1, %0" : "=r"(result) : "r"(bitboard) : "cc");
    return result == 1;
}

bool stateIsValid(gameState_s* state) {

    if (!kingIsValid(state->bitboard[white][king])) return false;
    if (!kingIsValid(state->bitboard[black][king])) return false;

    for (square square = A1; square < H1; square++) { // no pawns on the 1st rank
        if (read(square, state->bitboard[white][pawn])) return false;
        if (read(square, state->bitboard[black][pawn])) return false;
    }
    for (square square = A8; square < H8; square++) { // no pawns on the 8th rank
        if (read(square, state->bitboard[white][pawn])) return false;
        if (read(square, state->bitboard[black][pawn])) return false;
    }

    if (state->fiftyMoveRule > 100) return false;

    /*
    attackMap_s* map;
    state->playerToMove ^= 1;
    return !kingIsInCheck(state, map);
    state->playerToMove ^= 1;
    */
}


int strToInt(char* str) { // atoi not adequate: must return error value on invalid input
    int result = 0;
    for (; *str; str++) {
        if (*str < '0' || *str > '9') return -1;
        result = result * 10 + (*str - '0');
    }
    return result;
}

bool parsePosition(char* position, gameState_s* state) {

    for (square square = A1; square <= H8; square++)
        state->pieceLookup[square] = noPiece;

    square currentSquare = invalid;
    for (int rank = 7, file = 0; *position; file++, position++) { // FEN goes from A8 --> H1

        currentSquare = rank * 8 + file;
        char c = *position;

        if (c == '/' && file == 8) { rank--, file = -1;  } // go to starting point of previous rank
        else if (isValidDigit(c))  { file += c - '0' - 1;} // skip c amount of files
        else if (isValidLetter(c)) {
            piece piece = charToPiece(c);
            side side = (c <= 'Z' ? white : black);
            set(currentSquare, &state->bitboard[side][piece]);
            state->piecesForSide[side] |= BIT(currentSquare);
            state->pieceLookup[currentSquare] = piece;
        }
        else return false;
    }
    if(currentSquare != H1) return false; // TODO: doesnt work if first rank is empty

    state->allPieces = state->piecesForSide[white] | state->piecesForSide[black];

    return true;
}


bool parseTurn(const char turn, gameState_s* state) {
    if      (turn == 'w') state->playerToMove = white;
    else if (turn == 'b') state->playerToMove = black;
    else return false;

    return true;
}


bool parseCastling(char* castling, gameState_s* state) {
    state->castlingRights = 0b111111; // assume no castling; clear accordingly
    if (*castling == '-') return true;
    for(;*castling; castling++) {
        switch (*castling) {
            case 'K': state->castlingRights &= 0b111100; break;
            case 'Q': state->castlingRights &= 0b111001; break;
            case 'k': state->castlingRights &= 0b100111; break;
            case 'q': state->castlingRights &= 0b001111; break;
            default: return false;
        }
    }
    return true;
}

bool parseEnPassant(char* enPassant, gameState_s* state) {
    if (*enPassant == '-') state->enPassantIndex = invalid;
    else if (state->enPassantIndex = getIndexFromInput(enPassant) == invalid) return false;
    return true;
}

bool parseHalfMoves(char* halfMoves, gameState_s* state) {
    if (state->fiftyMoveRule = strToInt(halfMoves) == -1) return false;
    return true;
}


void parseFEN(char* FEN, gameState_s* state) {

    char position[72], turn, castling[5], enPassant[3], halfMoves[4], fullMoves[5]; // fullMoves unused atp

    int parsedCount = sscanf(FEN, "%s %c %s %s %s %s", position, &turn, castling, enPassant, halfMoves, fullMoves);
    if (parsedCount != 6) goto invalid;

    memset(state->bitboard, 0, sizeof(state->bitboard) + sizeof(state->piecesForSide) + sizeof(state->allPieces)); // clear bitboards

    if (!parsePosition(position, state))    goto invalid;
    if (!parseTurn(turn, state))            goto invalid;
    if (!parseCastling(castling, state))    goto invalid;
    if (!parseEnPassant(enPassant, state))  goto invalid;
    if (!parseHalfMoves(halfMoves, state))  goto invalid;

    //if(!stateIsValid(state))                goto invalid;

    return;

    invalid:
        printf("Invalid FEN string\n");
        exit(EXIT_FAILURE);
}