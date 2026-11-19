#include "main.h"
#include "fen.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "move.h"
#include "../asm.h"
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


bool inline isValidLetter(char c) {
    return charToPiece(c) != noPiece;
}

bool inline isValidDigit(char c) {
    return c >= '1' && c <= '8';
}


bool inline otherKingIsInCheck(state_s* restrict state, map_s* restrict map) {
    state_s opponentState = *state;
    opponentState.playerToMove ^= 1;
    return kingIsInCheck(&opponentState, map);
}


char* stateIsValid(state_s* restrict state, map_s* restrict map) {

    if (POPCNT(state->bitboard[white][king]) != 1 || POPCNT(state->bitboard[black][king]) != 1)
        return "Illegal amount of kings";

    if((state->bitboard[black][pawn] | state->bitboard[white][pawn]) & 0xFF000000000000FF)
        return "Pawn on illegal index";

    if(state->enPassantIndex != invalid) {
        if((state->playerToMove == black && (state->enPassantIndex < A3 || state->enPassantIndex > H3)) ||
           (state->playerToMove == white && (state->enPassantIndex < A6 || state->enPassantIndex > H6)))
            return "Invalid en passant square";

        if((state->pieceLookup[state->enPassantIndex] != noPiece)                                             ||
           (state->playerToMove == black && !(state->bitboard[white][pawn] & BIT(state->enPassantIndex + 8))) ||
           (state->playerToMove == white && !(state->bitboard[black][pawn] & BIT(state->enPassantIndex - 8))))
            return "En passant square and position do not concur";
    }

    bool whiteKingMoved = TZCNT(state->bitboard[white][king]) != E1;
    bool blackKingMoved = TZCNT(state->bitboard[black][king]) != E8;
    if((state->castlingRights & BIT(WHITE_OOO) && (!(state->bitboard[white][rook] & BIT(A1)) || whiteKingMoved)) ||
       (state->castlingRights & BIT(WHITE_OO)  && (!(state->bitboard[white][rook] & BIT(H1)) || whiteKingMoved)) ||
       (state->castlingRights & BIT(BLACK_OOO) && (!(state->bitboard[black][rook] & BIT(A8)) || blackKingMoved)) ||
       (state->castlingRights & BIT(BLACK_OO)  && (!(state->bitboard[black][rook] & BIT(H8)) || blackKingMoved))) // horrible parenthesization
        return "Castling rights and position do not concur";

    if(otherKingIsInCheck(state, map))
        return "Player to move may not have other king in check";

    return NULL;
}


int inline strToInt(char* str) { // atoi not adequate: must return error value on invalid input
    int result = 0;
    for (; *str; str++) {
        if (*str < '0' || *str > '9') return -1;
        result = result * 10 + (*str - '0');
    }
    return result;
}


bool parsePosition(char* restrict position, state_s* restrict state) {

    memset(state->bitboard, 0, sizeof(state->bitboard) + sizeof(state->piecesForSide) + sizeof(state->allPieces)); // clear bitboards
    for (square square = A1; square <= H8; square++)
        state->pieceLookup[square] = noPiece;

    int rank, file;
    for (rank = 7, file = 0; *position; file++, position++) { // FEN goes from A8 --> H1
        square currentSquare = rank * 8 + file;
        char c = *position;

        if (c == '/' && file == 8) { rank--, file = -1;   } // go to starting point of previous rank
        else if (isValidDigit(c))  { file += c - '0' - 1; } // skip c amount of files
        else if (isValidLetter(c)) {
            piece piece = charToPiece(c);
            side side = (c <= 'Z' ? white : black);
            set(currentSquare, &state->bitboard[side][piece]);
            set(currentSquare, &state->piecesForSide[side]);
            state->pieceLookup[currentSquare] = piece;
        }
        else return false;
    }
    if((rank * 8 + file - 1) != H1) return false;

    state->allPieces = state->piecesForSide[white] | state->piecesForSide[black];
    return true;
}


bool inline parseTurn(const char turn, state_s* state) {
    if      (turn == 'w') state->playerToMove = white;
    else if (turn == 'b') state->playerToMove = black;
    else return false;
    return true;
}


bool inline parseCastling(char* restrict castling, state_s* restrict state) {
    state->castlingRights = 0b0000;
    if (*castling == '-') return true;
    for(;*castling; castling++) {
        switch (*castling) {
            case 'Q': state->castlingRights |= BIT(WHITE_OOO); break;
            case 'K': state->castlingRights |= BIT(WHITE_OO);  break;
            case 'q': state->castlingRights |= BIT(BLACK_OOO); break;
            case 'k': state->castlingRights |= BIT(BLACK_OO);  break;
            default: return false;
        }
    }
    return true;
}


bool inline parseEnPassant(char* restrict enPassant, state_s* restrict state) {
    if (*enPassant == '-') state->enPassantIndex = invalid;
    else if ((state->enPassantIndex = getIndexFromInput(enPassant)) == invalid) return false;
    return true;
}


bool inline parseHalfMoves(char* restrict halfMoves, state_s* restrict state) {
    if ((int)(state->fiftyMoveRule = strToInt(halfMoves)) == -1) return false;
    return true;
}


void parseFEN(char* restrict FEN, state_s* restrict state, map_s* restrict map) {

    char position[72], turn, castling[5], enPassant[3], halfMoves[4], fullMoves[5]; // fullMoves unused atp
    int parsedCount = sscanf(FEN, "%s %c %s %s %s %s", position, &turn, castling, enPassant, halfMoves, fullMoves);

    if(parsedCount != 6                 ||
      !parsePosition(position, state)   ||
      !parseTurn(turn, state)           ||
      !parseCastling(castling, state)   ||
      !parseEnPassant(enPassant, state) ||
      !parseHalfMoves(halfMoves, state)) {
        printf("Invalid FEN string\n");
        exit(EXIT_FAILURE);
    }

    char* validity = stateIsValid(state, map);
    if (validity != NULL) {
        printf("%s\n", validity);
        exit(EXIT_FAILURE);
    }
}