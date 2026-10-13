#include "move.h"
#include <stdint.h>
#include "main.h"
#include "attackMaps.h"


int inline TZCNT(uint64_t src) {
    uint64_t result;
    __asm__("TZCNTQ %1, %0" : "=r"(result) : "r"(src));
    return (int)result;
}


uint64_t inline ANDN(uint64_t op1, uint64_t op2Neg) {
    uint64_t result;
    __asm__("ANDNQ %1, %2, %0" : "=r"(result) : "r"(op1), "r"(op2Neg));
    return result;
}

// perhaps make input be state and extract king bitboard in function to prevent misuse
square inline getKingIndex(uint64_t bitboard) {
    return TZCNT(bitboard);
}

// which pieces usually check the king? put them first for optimal short-circuiting
// maybe do one of these per game phase since rooks probably check more in endgame etc
bool squareIsAttacked(gameState_s* restrict state, attackMap_s* restrict map, const square square) {

    const side opponent = state->playerToMove^1;

    uint64_t opponentPawns = state->bitboard[opponent][pawn];
    if(map->pawn[opponent][square] & opponentPawns)
        return true;

    uint64_t opponentKnights = state->bitboard[opponent][knight];
    if(map->knight[square] & opponentKnights)
        return true;

    uint64_t opponentBishops = state->bitboard[opponent][bishop];
    if(bishopPseudoLegal(state, map, square) & opponentBishops)
        return true;

    uint64_t opponentRooks = state->bitboard[opponent][rook];
    if (rookPseudoLegal(state, map, square) & opponentRooks)
        return true;

    uint64_t opponentQueens = state->bitboard[opponent][queen];
    if (queenPseudoLegal(state, map, square) & opponentQueens)
        return true;

    uint64_t opponentKing = state->bitboard[opponent][king];
    if (map->king[square] & opponentKing)
        return true;

    return false;
}


bool inline kingIsInCheck(gameState_s* restrict state, attackMap_s* restrict map) {
    square kingIndex = getKingIndex(state->bitboard[state->playerToMove][king]);
    return squareIsAttacked(state, map, kingIndex);
}


uint64_t inline knightPseudoLegal(gameState_s* restrict state, attackMap_s* restrict map, const square start) {
    return ANDN(map->knight[start], state->piecesForSide[state->playerToMove]);
}

uint64_t inline kingPseudoLegal(gameState_s* restrict state, attackMap_s* restrict map, const square start) {
    return ANDN(map->king[start],state->piecesForSide[state->playerToMove]);
}

uint64_t inline rookPseudoLegal(gameState_s* restrict state, attackMap_s* restrict map, const square start) {
    uint32_t index = map->rookPextTableOffset[start] + PEXT(map->rookBlockerMask[start], state->allPieces);
    return ANDN(map->rookPextTable[index], state->piecesForSide[state->playerToMove]);
}

uint64_t inline bishopPseudoLegal(gameState_s* restrict state, attackMap_s* restrict map, const square start) {
    uint32_t index = map->bishopPextTableOffset[start] + PEXT(map->bishopBlockerMask[start], state->allPieces);
    return ANDN(map->bishopPextTable[index], state->piecesForSide[state->playerToMove]);
}

uint64_t inline queenPseudoLegal(gameState_s* restrict state, attackMap_s* restrict map, const square start) {
    return bishopPseudoLegal(state, map, start) | rookPseudoLegal(state, map, start);
}


bool moveIsLegal(gameState_s* restrict state, attackMap_s* restrict map, const uint16_t move, void (*simulateMove)(gameState_s*, attackMap_s*, uint16_t)) {

    gameState_s afterMoveState = *state;
    simulateMove(&afterMoveState, map, move);

    if (kingIsInCheck(&afterMoveState, map)) {
        return false;
    }
    *state = afterMoveState;
    return true;
}


void updatePiecePosition(gameState_s* state, uint16_t move) {

    const square origin = START(move);
    const square target = END(move);

    piece piece = state->pieceLookup[origin];
    state->pieceLookup[origin] = noPiece;
    state->pieceLookup[target] = piece;

    uint64_t moveBitboard = BIT(origin) | BIT(target);
    state->bitboard[state->playerToMove][piece] ^= moveBitboard;
    state->piecesForSide[state->playerToMove] ^= moveBitboard;
}


void inline handleCapture(gameState_s* state, const square end) {
    uint64_t endBitboard = BIT(end);
    if (endBitboard & state->piecesForSide[state->playerToMove^1]) {
        piece opponentPiece = state->pieceLookup[end];
        state->bitboard[state->playerToMove ^ 1][opponentPiece] ^= endBitboard;
        state->piecesForSide[state->playerToMove ^ 1] ^= endBitboard;
        state->fiftyMoveRule = 0;
    }
}


void inline updateCastlingRights(gameState_s* restrict state, attackMap_s* restrict map, const square start, const square end) {
    if (state->castlingRights != 0b0000)
        state->castlingRights &= map->castlingMask[start] | map->castlingMask[end];
}


uint64_t castlingBitboard(gameState_s* restrict state, attackMap_s* restrict map) {
    if (state->castlingRights == 0b0000) return 0;

    /*
    int ownRights;
    if (state->playerToMove == white ? (ownRights = state->castlingRights & 0b0011) : (ownRights = state->castlingRights >> 2)) {
        if(state->playerToMove == white ?
    }
    */

    // maybe check if player to move has any rights, as to not check all this stuff when you cant castle anyway
    uint64_t bitboard = 0;
    if (state->playerToMove == white) { // if/else can removed, would require variable assignment. this should be faster and is clearer
        if(read(WHITE_OOO, state->castlingRights) && !(map->castlingPath[WHITE_OOO] & state->allPieces)) bitboard |= BIT(C1);
        if(read(WHITE_OO,  state->castlingRights) && !(map->castlingPath[WHITE_OO]  & state->allPieces)) bitboard |= BIT(G1);
    } else {
        if(read(BLACK_OOO, state->castlingRights) && !(map->castlingPath[BLACK_OOO] & state->allPieces)) bitboard |= BIT(C8);
        if(read(BLACK_OO,  state->castlingRights) && !(map->castlingPath[BLACK_OO]  & state->allPieces)) bitboard |= BIT(G8);
    }
    return bitboard;
}


// separate functions for castling and en passant moves? and perhaps promotion?
void simulateMoveGeneral(gameState_s* restrict state, attackMap_s* restrict map, const uint16_t move) {

    const square start = START(move);
    const square end = END(move);

    updatePiecePosition(state, move);
    handleCapture(state, end);
}


void simulateMovePawn(gameState_s* restrict state, attackMap_s* restrict map, const uint16_t move) {

    const square start = START(move);
    const square end = END(move);

    updatePiecePosition(state, move);
    handleCapture(state, end);

    if (end == state->enPassantIndex) {
        square opponentPawnIndex = state->playerToMove == white ? end + 8 : end - 8;
        state->bitboard[state->playerToMove ^ 1][pawn] ^= BIT(opponentPawnIndex);
    }

    state->fiftyMoveRule = 0;
}


void makeMove(gameState_s* state, uint16_t move) {
    const square end = END(move);
    updatePiecePosition(state, move);
    handleCapture(state, end);
    state->allPieces = state->piecesForSide[white] | state->piecesForSide[black];
}


void simulateMoveKing(gameState_s* restrict state, attackMap_s* restrict map, uint16_t move) {
    const square start = START(move);
    const square end = END(move);

    if(map->king[start] & end) {
        handleCapture(state, end);
    } else {
        square passThrough = start < end ? F1 : D1;
        if (state->playerToMove == white)
            passThrough += FILES * 7;

        if (squareIsAttacked(state, map, passThrough)) {
            return; // signal that it is not allowed (somehow)
        }

        // move corresponding rook
    }
    updatePiecePosition(state, move);
}


bool pawnMoves(gameState_s* restrict state, attackMap_s* restrict map, const uint16_t move) {

    const square start = START(move);
    const square end = END(move);

    uint64_t attackPseudoLegal = map->pawn[state->playerToMove][start];
    uint64_t targetSquares = state->piecesForSide[state->playerToMove ^ 1] | BIT(state->enPassantIndex);
    bool isAttacking = read(end, attackPseudoLegal & targetSquares);

    uint64_t oneSquareForward = state->playerToMove == white ? BIT(start) << 8 : BIT(start) >> 8;
    bool canMoveOne = ANDN(oneSquareForward, state->allPieces);

    if (!canMoveOne && !isAttacking) // if you cant move one, you cant move two; no matter
        return false;

    bool isMovingOne = oneSquareForward == BIT(end);
    if (isMovingOne || isAttacking)
        return moveIsLegal(state, map, move, &simulateMovePawn);


    uint64_t twoSquaresForward = state->playerToMove == white ? BIT(start) << 16 : BIT(start) >> 16;
    bool isFirstPawnMove = state->playerToMove == white ? start <= H2 : start >= A7;
    bool canAndIsMovingTwo = isFirstPawnMove && (twoSquaresForward == ANDN(BIT(end), state->allPieces));

    if (canAndIsMovingTwo) {
        state->enPassantIndex = state->playerToMove == white ? start + 8 : start - 8; // move this to after moveIsLegal check
        return moveIsLegal(state, map, move, &simulateMovePawn);
    }

    return false;
}


bool knightMoves(gameState_s* restrict state, attackMap_s* restrict map, const uint16_t move) {
    const square start = START(move);
    const square end = END(move);
    uint64_t pseudoLegal = ANDN(map->knight[start], state->piecesForSide[state->playerToMove]);
    if(!read(end, pseudoLegal)) return false;
    return moveIsLegal(state, map, move, &simulateMoveGeneral);
}


bool kingMoves(gameState_s* restrict state, attackMap_s* restrict map, const uint16_t move) {
    const square start = START(move);
    const square end = END(move);

    uint64_t pseudoLegal = ANDN(map->king[start], state->piecesForSide[state->playerToMove]);
    pseudoLegal |= castlingBitboard(state, map);
    updateCastlingRights(state, map, start, end);
    if(!read(end, pseudoLegal)) return false;

    return moveIsLegal(state, map, move, &simulateMoveKing);
}


bool rookMoves(gameState_s* restrict state, attackMap_s* restrict map, const uint16_t move) {
    const square start = START(move);
    const square end = END(move);
    uint64_t pseudoLegal = rookPseudoLegal(state, map, start);
    if(!read(end, pseudoLegal)) return false;
    return moveIsLegal(state, map, move, &simulateMoveGeneral);
}


bool bishopMoves(gameState_s* restrict state, attackMap_s* restrict map, const uint16_t move) {
    const square start = START(move);
    const square end = END(move);
    uint64_t pseudoLegal = bishopPseudoLegal(state, map, start);
    if(!read(end, pseudoLegal)) return false;
    return moveIsLegal(state, map, move, &simulateMoveGeneral);
}


bool queenMoves(gameState_s* restrict state, attackMap_s* restrict map, const uint16_t move) {
    const square start = START(move);
    const square end = END(move);
    uint64_t pseudoLegal = queenPseudoLegal(state, map, start);
    if (!read(end, pseudoLegal)) return false;
    return moveIsLegal(state, map, move, &simulateMoveGeneral);
}