#ifndef CHESS_V2_PERF_H
#define CHESS_V2_PERF_H

static int testCount;

#define TIME(start, end) ((double)((end) - (start)) / CLOCKS_PER_SEC)
#define REPORT_RESULT(start, end) (printf("%d-%-18s [%.3lfs]\n", ++testCount, __func__, TIME(start, end)))

void moveGeneration(char* FEN);
void runPerformanceTests();

#endif //CHESS_V2_PERF_H
