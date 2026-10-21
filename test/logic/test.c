#include <stdio.h>

int fails = 0;

#define ASSERT(exp)  \
    do {             \
        if (!exp) {  \
            fprintf(stderr, "test '%s' in %s @ line %d failed\n", #exp, __FILE_NAME__, __LINE__) \
            fails++; \
    } while (0)      \