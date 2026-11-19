#include "../game/move.h"
#include "moveGen.h"
#include "../asm.h"
#include "../game/print.h"

int inline nextBit(uint64_t* bitboard) {
    int result = TZCNT(*bitboard);
    *bitboard = BLSR(*bitboard);
    return result;
}


uint64_t inline fetchMoves(state_s* restrict state, map_s* restrict map, piece piece, square square) {
    switch (piece) {
        case knight: return knightPseudoLegal(state, map, square);
        case bishop: return bishopPseudoLegal(state, map, square);
        case rook:   return rookPseudoLegal(state, map, square);
        case queen:  return queenPseudoLegal(state, map, square);
        default: UNREACHABLE();
    }
}


void generateKingMoves(state_s* restrict state, map_s* restrict map, uint16_t* restrict moves, int* restrict moveCount) {

    int startSquare = getKingIndex(state);
    uint64_t moveBitboard = kingPseudoLegal(state, map, startSquare);

    while(moveBitboard) {
        int endSquare = nextBit(&moveBitboard);
        uint16_t move = startSquare | (endSquare << 6);
        if(state->pieceLookup[endSquare] != noPiece)
            moves[*moveCount] = move | CAPTURE;
        else moves[*moveCount] = move;
        (*moveCount)++;
    }

    if (state->castlingRights != 0b0000) {

        if (state->playerToMove == white) {
            if(WHITE_OOO & state->castlingRights && !(map->castlingPath[WHITE_OOO] & state->allPieces))
                moves[(*moveCount)++] = startSquare | (C1 << 6) | QUEEN_CASTLE;
            if(WHITE_OO & state->castlingRights && !(map->castlingPath[WHITE_OO]  & state->allPieces))
                moves[(*moveCount)++] = startSquare | (G1 << 6) | KING_CASTLE;

        } else {
            if(BLACK_OOO & state->castlingRights && !(map->castlingPath[BLACK_OOO] & state->allPieces))
                moves[(*moveCount)++] = startSquare | (C8 << 6) | QUEEN_CASTLE;
            if(BLACK_OO & state->castlingRights && !(map->castlingPath[BLACK_OO]  & state->allPieces))
                moves[(*moveCount)++] = startSquare | (G8 << 6) | KING_CASTLE;
        }
    }
}


uint64_t inline whitePawnPushOne(state_s* state) {
    return ANDN(state->allPieces, state->bitboard[white][pawn] << FILES);
}

uint64_t inline blackPawnPushOne(state_s* state) {
    return ANDN(state->allPieces, state->bitboard[black][pawn] >> FILES);
}

uint64_t inline whitePawnPushTwo(uint64_t pushOne, state_s* state) {
    uint64_t fourthRankMask = 0x00000000FF000000;
    return ANDN(state->allPieces, pushOne << FILES) & fourthRankMask;
}

uint64_t inline blackPawnPushTwo(uint64_t pushOne, state_s* state) {
    uint64_t fifthRankMask = 0x000000FF00000000;
    return ANDN(state->allPieces, pushOne >> FILES) & fifthRankMask;
}


void inline addPromoMoves(uint16_t* restrict moves, int* restrict moveCount, uint16_t move, uint64_t promoMask) {
    *(uint64_t*)&moves[*moveCount] = (move         |
                           ((uint64_t)move << 16)  |
                           ((uint64_t)move << 32)  |
                           ((uint64_t)move << 48)) | promoMask;
    *moveCount += 4;
}


void inline whitePawnSinglePushMoveGen(uint64_t pushOne, uint16_t moves[static restrict 218], int* restrict moveCount) {
#ifdef __AVX512VBMI2__
    uint64_t onePushNoPromo = pushOne  & 0x00FFFFFFFFFFFF00;
    __m512i endSquareVector512 = _mm512_maskz_compress_epi8(onePushNoPromo, indexVector512()); // simulated vector TZCNT on each bit
    __m128i endSquareVector128 = _mm_cvtepu8_epi16(_mm512_castsi512_si128(endSquareVector512));
    __m128i startSquareVector128 = _mm_sub_epi16(endSquareVector128, _mm_set1_epi16(8));
    __m128i moveVector128 = _mm_or_si128(startSquareVector128, _mm_slli_epi16(endSquareVector128, 6));
    _mm_storeu_epi16(&moves[*moveCount], moveVector128); // storing extra 0-moves here doesn't matter as moveCount only gets incremented by the actual amount of moves
    *moveCount += POPCNT(onePushNoPromo);
#else
    while(pushOne) {
        int endSquare = nextBit(&pushOne);
        moves[(*moveCount)++] = (endSquare - FILES) | (endSquare << 6);
    }
#endif
}

void inline whitePawnDoublePushMoveGen(uint64_t pushOne, state_s* restrict state, uint16_t moves[static restrict 218], int* restrict moveCount) {
#ifdef __AVX512VBMI2__
    uint64_t pushTwo = whitePawnPushTwo(pushOne, state);
    int doublePushes = POPCNT(pushTwo);
    if(doublePushes != 0) {
        __m128i endSquareVector128 = _mm_cvtepu8_epi16(_mm512_castsi512_si128(_mm512_maskz_compress_epi8(pushTwo, indexVector512())));
        __m128i startSquareVector128 = _mm_sub_epi16(endSquareVector128, _mm_set1_epi16(16));
        __m128i moveVector128 = _mm_or_si128(startSquareVector128, _mm_slli_epi16(endSquareVector128, 6));
        _mm_storeu_epi16(&moves[*moveCount], _mm_or_si128(moveVector128, _mm_set1_epi16(DOUBLE_PAWN_PUSH)));
        *moveCount += POPCNT(pushTwo);
    }
#else
    uint64_t pushTwo = whitePawnPushTwo(pushOne, state);
    while (pushTwo) {
        int endSquare = nextBit(&pushTwo);
        moves[(*moveCount)++] = (endSquare - 2*FILES) | (endSquare << 6) | DOUBLE_PAWN_PUSH;
    }
#endif
}


void inline blackPawnSinglePushMoveGen(uint64_t pushOne, uint16_t moves[static restrict 128], int* restrict moveCount) {
#ifdef __AVX512VBMI2__
    uint64_t onePushNoPromo = pushOne & 0x00FFFFFFFFFFFF00;
    __m128i endSquareVector128 = _mm_cvtepu8_epi16(_mm512_castsi512_si128(_mm512_maskz_compress_epi8(onePushNoPromo, indexVector512())));
    __m128i startSquareVector128 = _mm_add_epi16(endSquareVector128, _mm_set1_epi16(8));
    __m128i moveVector128 = _mm_or_si128(startSquareVector128, _mm_slli_epi16(endSquareVector128, 6));
    _mm_storeu_epi16(&moves[*moveCount], moveVector128);
    *moveCount += POPCNT(onePushNoPromo);
#else
    while(pushOne) {
        int endSquare = nextBit(&pushOne);
        moves[(*moveCount)++] = (endSquare + FILES) | (endSquare << 6);
    }
#endif
}

void inline blackPawnDoublePushMoveGen(uint64_t pushOne, state_s* restrict state, uint16_t moves[static restrict 218], int* restrict moveCount) {
#ifdef __AVX512VBMI2__
    uint64_t pushTwo = blackPawnPushTwo(pushOne, state);
    int doublePushes = POPCNT(pushTwo);
    if(doublePushes != 0) {
        __m128i endSquareVector128 = _mm_cvtepu8_epi16(_mm512_castsi512_si128(_mm512_maskz_compress_epi8(pushTwo, indexVector512())));
        __m128i startSquareVector128 = _mm_add_epi16(endSquareVector128, _mm_set1_epi16(16));
        __m128i moveVector128 = _mm_or_si128(startSquareVector128, _mm_slli_epi16(endSquareVector128, 6));
        _mm_storeu_epi16(&moves[*moveCount], _mm_or_si128(moveVector128, _mm_set1_epi16(DOUBLE_PAWN_PUSH)));
        *moveCount += doublePushes;
    }
#else
    uint64_t pushTwo = blackPawnPushTwo(pushOne, state);
    while (pushTwo) {
        int endSquare = nextBit(&pushTwo);
        moves[(*moveCount)++] = (endSquare + 2*FILES) | (endSquare << 6) | DOUBLE_PAWN_PUSH;
    }
#endif
}


void inline pawnCaptureGen(state_s* restrict state, map_s* restrict map, uint16_t moves[restrict static 218], int* restrict moveCount) {
    uint64_t nonPromoPawns = state->bitboard[state->playerToMove][pawn] & 0x00FFFFFFFFFFFF00;

    while (nonPromoPawns) {
        int startSquare = nextBit(&nonPromoPawns);
        uint64_t attacks = map->pawn[state->playerToMove][startSquare] & (state->piecesForSide[state->playerToMove^1] | BIT(state->enPassantIndex)); // UB if EPI invalid

        while(attacks) {
            int endSquare = nextBit(&attacks);
            uint16_t move = startSquare | (endSquare << 6);
            if(UNLIKELY(endSquare == state->enPassantIndex))
                moves[(*moveCount)++] = move | EN_PASSANT;
            else moves[(*moveCount)++] = move | CAPTURE;
        }
    }
}


void inline pawnPromotionGen(uint64_t pushOne, uint16_t moves[restrict static 218], int* restrict moveCount, int previousRank) {
    uint64_t promoPawns = pushOne & 0xFF000000000000FF;
    while (UNLIKELY(promoPawns)) { // unlikely?
        int endSquare = nextBit(&promoPawns);
        uint16_t move = (endSquare + previousRank) | (endSquare << 6);
        addPromoMoves(moves, moveCount, move, PROMO_MASK);
    }
}


void inline pawnPromotionCaptureGen(state_s* state, map_s* map, uint16_t moves[restrict static 218], int* restrict moveCount, uint64_t lastRankMask) {
    uint64_t promoCapturePawns = state->bitboard[state->playerToMove][pawn] & lastRankMask;

    while(promoCapturePawns) {
        int startSquare = nextBit(&promoCapturePawns);
        uint64_t attacks = map->pawn[state->playerToMove][startSquare] & state->piecesForSide[state->playerToMove^1];
        while(UNLIKELY(attacks)) { // probably better way than while loop since max is 2 iterations: 0 99% of the time
            int endSquare = nextBit(&attacks);
            uint16_t move = startSquare | (endSquare << 6);
            addPromoMoves(moves, moveCount, move, PROMO_CAP_MASK);
        }
    }
}


void inline generatePawnMoves(state_s* restrict state, map_s* restrict map, uint16_t moves[restrict static 218], int* restrict moveCount) {

    if (state->playerToMove == white) {
        uint64_t pushOne = whitePawnPushOne(state);
        whitePawnSinglePushMoveGen(pushOne, moves, moveCount);
        whitePawnDoublePushMoveGen(pushOne, state, moves, moveCount);
        pawnPromotionGen(pushOne, moves, moveCount, -FILES);
        pawnPromotionCaptureGen(state, map, moves, moveCount, 0x00FF000000000000);
    } else { // black
        uint64_t pushOne = blackPawnPushOne(state);
        blackPawnSinglePushMoveGen(pushOne, moves, moveCount);
        blackPawnDoublePushMoveGen(pushOne, state, moves, moveCount);
        pawnPromotionGen(pushOne, moves, moveCount, FILES);
        pawnPromotionCaptureGen(state, map, moves, moveCount, 0x000000000000FF00);
    }

    pawnCaptureGen(state, map, moves, moveCount);
}



void inline generateMovesGeneralSIMD(state_s* state, map_s* map, __m512i indexVector512) { // give piece as input, should be possible
    if(POPCNT(state->bitboard[state->playerToMove][queen]) != 1) return; // handle later
    int startSquare = TZCNT(state->bitboard[state->playerToMove][queen]);
    uint64_t attacks = queenPseudoLegal(state, map, startSquare);
    __m512i endSquareVector512 = _mm512_maskz_compress_epi8(attacks, indexVector512);
}


int generateMoves(state_s* restrict state, map_s* restrict map, uint16_t moves[restrict static 218]) {

    uint64_t cpyBoards[5] = {
        0, // buffer to make piece indexing correct. (seems like its optimized away)
        state->bitboard[state->playerToMove][knight],
        state->bitboard[state->playerToMove][bishop],
        state->bitboard[state->playerToMove][rook],
        state->bitboard[state->playerToMove][queen]
    };

    int moveCount = 0;

    generatePawnMoves(state, map, moves, &moveCount);

    for (piece piece = knight; piece <= queen; piece++) {

        while(cpyBoards[piece]) {
            int startSquare = nextBit(&cpyBoards[piece]);
            uint64_t moveBitboard = fetchMoves(state, map, piece, startSquare);

            while(moveBitboard) {
                int endSquare = nextBit(&moveBitboard);
                uint16_t move = startSquare | (endSquare << 6);
                if(state->pieceLookup[endSquare] == noPiece)
                    moves[moveCount++] = move;
                else moves[moveCount++] = move | CAPTURE;
            }
        }
    }

    generateKingMoves(state, map, moves, &moveCount);

    return moveCount;
}