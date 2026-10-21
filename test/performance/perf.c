#include <stdio.h>
#include <time.h>
#include "../../game/attackMaps.h"

#define TIME(start, end) ((double)((end) - (start)) / CLOCKS_PER_SEC)

void perf() {
    attackMap_s map;

    clock_t start = clock();
    initAttackMaps(&map);
    clock_t end = clock();
    printf("%lf", TIME(start, end));
}