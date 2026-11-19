#pragma once

#include "../../game/maps.h"

extern int failCount;
extern map_s testMap;

#define ASSERT(exp)      \
    do {                 \
        if (!(exp)) {    \
            fprintf(stderr, "%s, %d: [%s]\n", __FILE_NAME__, __LINE__, #exp); \
            ++failCount; \
        }                \
    } while (0)

void runLogicTests();
void runPawnMoveGenTests();
