#include "eq_complexity.h"
#include <stdio.h>
#include <math.h>

void complexity_from_structure(int edges, int nodes, int exits, ComplexityMetrics *cm) {
    cm->cyclomatic = edges - nodes + 2 * exits;
    cm->is_maintainable = cm->cyclomatic <= 10;
    if (cm->cyclomatic <= 5) cm->risk_level = "LOW";
    else if (cm->cyclomatic <= 10) cm->risk_level = "MODERATE";
    else if (cm->cyclomatic <= 20) cm->risk_level = "HIGH";
    else cm->risk_level = "EXTREME";
    cm->essential = cm->cyclomatic - 1 > 0 ? cm->cyclomatic - 1 : 1;
}

void complexity_cognitive(int nesting_level, int operators_count, ComplexityMetrics *cm) {
    cm->cognitive = operators_count * (1 + nesting_level);
    cm->risk_level = cm->cognitive <= 15 ? "LOW" : cm->cognitive <= 30 ? "MODERATE" : cm->cognitive <= 50 ? "HIGH" : "EXTREME";
    cm->cyclomatic = cm->cognitive / 2;
}

int complexity_mccabe_threshold(ComplexityMetrics *cm) { return cm->cyclomatic <= 10 ? 0 : cm->cyclomatic <= 20 ? 1 : 2; }

void complexity_print(ComplexityMetrics *cm) {
    printf("  Cyclomatic: %d (Risk: %s)\n", cm->cyclomatic, cm->risk_level);
    printf("  Cognitive: %d, Essential: %d\n", cm->cognitive, cm->essential);
    printf("  Maintainable: %s\n", cm->is_maintainable ? "Yes" : "No");
}
