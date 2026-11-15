#ifndef CHESS_V2_PAWNMOVEGENERATION_H
#define CHESS_V2_PAWNMOVEGENERATION_H

#include <stdint.h>
#include <stdbool.h>

#define INIT_MOVEGEN(FEN) \
    state_s state;      \
    parseFEN(FEN, &state, &map); \
    moveCount = 0; \
    memset(moves, 0, 218);

#define DISPATCH(_1, _2, _3, x, ...) x

#define EXISTS_WITH_FLAGS(start, end, flags) moveExistsFunc(start | (end << 6) | flags)
#define EXISTS_WITHOUT_FLAGS(start, end) moveExistsFunc(start | (end << 6))
#define moveExists(...) DISPATCH(__VA_ARGS__, EXISTS_WITH_FLAGS, EXISTS_WITHOUT_FLAGS)(__VA_ARGS__)

#define DOESNT_EXIST_WITH_FLAGS(start, end, flags) noSuchMoveFunc(start | (end << 6) | flags)
#define DOESNT_EXIST_WITHOUT_FLAGS(start, end) noSuchMoveFunc(start | (end << 6))
#define noSuchMove(...) DISPATCH(__VA_ARGS__, DOESNT_EXIST_WITH_FLAGS, DOESNT_EXIST_WITHOUT_FLAGS)(__VA_ARGS__)

static int moveCount;
static uint16_t moves[218] = {0};

void whitePawnPushGenSIMD_addsMovesCorrectly();
void pawnPromotionGen_addsMovesCorrectly();
void pawnCaptureGen_addsMovesCorrectly();
void pawnPromotionCaptureGen_addsMovesCorrectly();

bool moveExistsFunc(uint16_t move);
bool noSuchMoveFunc(uint16_t move);


#endif //CHESS_V2_PAWNMOVEGENERATION_H
