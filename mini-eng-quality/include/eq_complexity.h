#ifndef EQ_COMPLEXITY_H
#define EQ_COMPLEXITY_H
#include <stdbool.h>

typedef struct {
    int cyclomatic;      /* M = E - N + 2P */
    int cognitive;       /* nesting-aware complexity */
    int essential;       /* structured-programming reduction */
    bool is_maintainable; /* M <= 10 */
    const char *risk_level; /* LOW/MODERATE/HIGH/EXTREME */
} ComplexityMetrics;

void complexity_from_structure(int edges, int nodes, int exits, ComplexityMetrics *cm);
void complexity_cognitive(int nesting_level, int operators_count, ComplexityMetrics *cm);
int  complexity_mccabe_threshold(ComplexityMetrics *cm); /* 0=OK, 1=warn, 2=critical */
void complexity_print(ComplexityMetrics *cm);
#endif
