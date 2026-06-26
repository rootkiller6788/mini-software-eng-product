#include "eq_complexity.h"
#include "eq_halstead.h"
#include "eq_maintainability.h"
#include "eq_quality_gate.h"
#include "eq_static_analysis.h"
#include <stdio.h>

int main(void) {
    printf("=== Engineering Quality Demo ===\n\n");
    /* Complexity */ ComplexityMetrics cm;
    complexity_from_structure(25, 20, 1, &cm);
    complexity_cognitive(3, 12, &cm);
    complexity_print(&cm);
    /* Halstead */ HalsteadMetrics hm;
    halstead_compute(15, 25, 120, 200, &hm);
    printf("\n"); halstead_print(&hm);
    /* Maintainability */ MaintainabilityReport mr;
    maint_compute(hm.volume, cm.cyclomatic, 500, 80, 50.0, 3, &mr);
    printf("\n"); maint_print(&mr);
    /* Quality Gate */ GateReport gr;
    gate_evaluate(&gr, &cm, &mr);
    printf("\n"); gate_print(&gr);
    /* Static Analysis */ StaticAnalysisResult sar;
    const char *code = "int main() {\n  if (x) {\n    while (y) {\n      if (z) {\n        return 1;\n      }\n    }\n  }\n  return 0;\n}";
    static_analyze(code, &sar);
    printf("\n"); static_print(&sar);
    return 0;
}
