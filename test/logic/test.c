#include <stdio.h>
#include "test.h"
#include "moveGeneration/pawnMoveGenTest.h"
#include "../../game/maps.h"

int failCount = 0;
map_s map;

void runLogicTests() {
    initMaps(&map);
    runPawnMoveGenTests();

    failCount ? printf("\n%d test(s) failed\n", failCount) : printf("All tests passed\n");
}


void runPawnMoveGenTests() {
    whitePawnPushGenSIMD_addsMovesCorrectly(); // add ifdef's to make this compile without avx support
    pawnPromotionGen_addsMovesCorrectly();
    pawnCaptureGen_addsMovesCorrectly();
    pawnPromotionCaptureGen_addsMovesCorrectly();
}
