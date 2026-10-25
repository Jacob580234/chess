#include <stdio.h>
#include "test.h"
#include "../../game/main.h"
#include "../../game/attackMaps.h"
#include "../../game/FEN.h"

void testTest() {
    gameState_s state;
    attackMap_s map;
    initAttackMaps(&map);
    parseFEN("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", &state, &map);

    ASSERT(1 == 2);
}


void runLogicTests() {
    testTest();


    failCount ? printf("\n%d test(s) failed\n", failCount) : printf("All tests passed\n");
}