#include "main.h"
#include "fen.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "print.h"

piece inline charToPiece(char c) {
    switch(c) {
        case 'P': case 'p': return pawn;
        case 'N': case 'n': return knight;
        case 'B': case 'b': return bishop;
        case 'R': case 'r': return rook;
        case 'Q': case 'q': return queen;
        case 'K': case 'k': return king;
        default: THROW_EXCEPTION;
    }
}


bool isValidLetter(char c) {
    char pieceLetters[] = {
        'P', 'N', 'B', 'R', 'Q', 'K',
        'p', 'n', 'b', 'r', 'q', 'k'
    };
    for(int i = 0; i < sizeof(pieceLetters); i++) {
        if(c == pieceLetters[i])
            return true;
    }

    return false;
}

bool inline isValidDigit(char c) {
    return c >= '1' && c <= '8' ? true : false;
}

bool kingIsValid(uint64_t bitboard) {
    uint64_t result;
    __asm__("POPCNT %1, %0" : "=r"(result) : "r"(bitboard) : "cc");
    return result == 1 ? true : false;
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

    // TODO: king may not be in check on opponents turn

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

    square currentSquare;
    for (currentSquare = A8; *position; currentSquare++, position++) { // FEN goes from A8 --> H1
        char c = *position;
        if (isValidLetter(c)) {
            piece piece = charToPiece(c);
            side side = (c <= 'Z' ? white : black);
            set(currentSquare, &state->bitboard[side][piece]);
            state->pieceLookup[currentSquare] = piece;
        }
        else if (isValidDigit(c)) currentSquare += c - '0' - 1; // skip c amount of files
        else if (c == '/' && currentSquare % 8 == 0) currentSquare -= 2*FILES+1; // go to starting point of previous rank
        else return false;

    }
    if(currentSquare != H1 + 1) return false;
    return true;
}


bool parseTurn(const char turn, gameState_s* state) {
    if (turn == 'w')      state->playerToMove = white;
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
    if (parsedCount != 6) goto invalidFormatting;

    memset(state->bitboard, 0, sizeof(state->bitboard)); // clear bitboards

    if (!parsePosition(position, state))    goto invalidFormatting;
    if (!parseTurn(turn, state))            goto invalidFormatting;
    if (!parseCastling(castling, state))    goto invalidFormatting;
    if (!parseEnPassant(enPassant, state))  goto invalidFormatting;
    if (!parseHalfMoves(halfMoves, state))  goto invalidFormatting;
    if (!kingIsValid(state->bitboard[white][king]) || !kingIsValid(state->bitboard[black][king]))
        THROW_EXCEPTION;


    return;

    invalidFormatting:
        printf("Invalid FEN string\n");
        exit(EXIT_FAILURE);
}