#ifndef CHESS_V2_TEST_H
#define CHESS_V2_TEST_H

static int failCount = 0;

#define ASSERT(exp)      \
    do {                 \
        if (!(exp)) {    \
            fprintf(stderr, "%s, %d: [%s]\n", __FILE_NAME__, __LINE__, #exp); \
            failCount++; \
        }                \
    } while (0)

void runLogicTests();

#endif //CHESS_V2_TEST_H
