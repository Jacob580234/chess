#include "main.h"
#include "fen.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "move.h"

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
    return POPCNT(bitboard) == 1;
}

bool stateIsValid(gameState_s* state, attackMap_s* map) {

    if (!kingIsValid(state->bitboard[white][king])) return false;
    if (!kingIsValid(state->bitboard[black][king])) return false;

    for (square square = A1; square <= H1; square++) { // no pawns on the 1st rank
        if (read(square, state->bitboard[white][pawn])) return false;
        if (read(square, state->bitboard[black][pawn])) return false;
    }
    for (square square = A8; square <= H8; square++) { // no pawns on the 8th rank
        if (read(square, state->bitboard[white][pawn])) return false;
        if (read(square, state->bitboard[black][pawn])) return false;
    }

    if (state->fiftyMoveRule > 100) return false;

    const square enemyKingIndex = getKingIndex(state->bitboard[state->playerToMove^1][king]);
    return squareIsAttacked(state, map, enemyKingIndex); // only checks opponents pieces, so return inverse to get whether opponent king is in check
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
    if(currentSquare != H1) return false; // TODO: is broken

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
    state->castlingRights = 0b0000;
    if (*castling == '-') return true;
    for(;*castling; castling++) {
        switch (*castling) {
            case 'Q': state->castlingRights |= BIT(WHITE_OOO); break;
            case 'K': state->castlingRights |= BIT(WHITE_OO); break;
            case 'q': state->castlingRights |= BIT(BLACK_OOO); break;
            case 'k': state->castlingRights |= BIT(BLACK_OO); break;
            default: return false;
        }
    }
    return true;
}


bool parseEnPassant(char* enPassant, gameState_s* state) {
    if (*enPassant == '-') state->enPassantIndex = invalid;
    else if ((state->enPassantIndex = getIndexFromInput(enPassant)) == invalid) return false;
    return true;
}


bool parseHalfMoves(char* halfMoves, gameState_s* state) {
    if ((int)(state->fiftyMoveRule = strToInt(halfMoves)) == -1) return false;
    return true;
}

void parseFEN(char* FEN, gameState_s* state, attackMap_s* map) {

    char position[72], turn, castling[5], enPassant[3], halfMoves[4], fullMoves[5]; // fullMoves unused atp

    int parsedCount = sscanf(FEN, "%s %c %s %s %s %s", position, &turn, castling, enPassant, halfMoves, fullMoves);
    if (parsedCount != 6) goto invalid;

    memset(state->bitboard, 0, sizeof(state->bitboard) + sizeof(state->piecesForSide) + sizeof(state->allPieces)); // clear bitboards

    if(parsePosition(position, state) && parseTurn(turn, state) && parseCastling(castling, state) &&
    parseEnPassant(enPassant, state) && parseHalfMoves(halfMoves, state) && stateIsValid(state, map)) {
        return;
    }

    invalid:
        printf("Invalid FEN string\n");
        exit(EXIT_FAILURE);
}
