#ifndef CHESS_V2_PERF_H
#define CHESS_V2_PERF_H

static int testCount;

#define TIME(start, end) ((double)((end) - (start)) / CLOCKS_PER_SEC)

#define REPORT_RESULT(start, end) (printf("%d-%-18s [%.3lfs]\n", ++testCount, __func__, TIME(start, end)))

#define BENCH(func, iterations)    \
    do {                           \
        clock_t start = clock();   \
        for (int i = 0; i < iterations; i++) { \
            func;                  \
        }                          \
        clock_t end = clock();     \
        REPORT_RESULT(start, end); \
    } while (0)

void moveGeneration(char* FEN);
void runPerformanceTests();

#endif //CHESS_V2_PERF_H
