#ifndef CHESS_V2_TEST_H
#define CHESS_V2_TEST_H

#include "../../game/maps.h"

extern int failCount;
extern map_s map;

#define ASSERT(exp)      \
    do {                 \
        if (!(exp)) {    \
            fprintf(stderr, "%s, %d: [%s]\n", __FILE_NAME__, __LINE__, #exp); \
            ++failCount; \
        }                \
    } while (0)

void runLogicTests();
void runPawnMoveGenTests();
#endif //CHESS_V2_TEST_H
