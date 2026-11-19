#include <stdio.h>
#include "test.h"
#include "moveGeneration/pawnMoveGenTest.h"
#include "../../game/maps.h"

int failCount = 0;
map_s testMap;

void runLogicTests() {
    initMaps(&testMap);
    runPawnMoveGenTests();

    failCount ? printf("\n%d test(s) failed\n", failCount) : printf("All tests passed\n");
}


void runPawnMoveGenTests() {
    whitePawnPushGen_addsMovesCorrectly(); // must disable march=native flag to test sequential implementation; not ideal
    pawnPromotionGen_addsMovesCorrectly();
    pawnCaptureGen_addsMovesCorrectly();
    pawnPromotionCaptureGen_addsMovesCorrectly();
}
