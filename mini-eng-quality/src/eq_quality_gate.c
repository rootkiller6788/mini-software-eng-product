#include "eq_quality_gate.h"
#include <stdio.h>
#include <string.h>

void gate_init_default_thresholds(QualityThresholds *qt) {
    qt->max_cyclomatic = 15.0;
    qt->min_maintainability = 50.0;
    qt->max_technical_debt = 0.5;
    qt->max_nesting_depth = 4;
    qt->max_file_loc = 1000;
}

GateResult gate_check_cyclomatic(int cc, double threshold) { return (double)cc <= threshold ? GATE_PASS : (double)cc <= threshold * 1.5 ? GATE_WARN : GATE_FAIL; }

GateResult gate_check_maintainability(double mi, double threshold) { return mi >= threshold ? GATE_PASS : mi >= threshold * 0.7 ? GATE_WARN : GATE_FAIL; }

GateResult gate_check_technical_debt(double tdr, double threshold) { return tdr <= threshold ? GATE_PASS : tdr <= threshold * 1.5 ? GATE_WARN : GATE_FAIL; }

void gate_evaluate(GateReport *gr, ComplexityMetrics *cm, MaintainabilityReport *mr) {
    gate_init_default_thresholds(&gr->thresholds);
    gr->complexity_result = gate_check_cyclomatic(cm->cyclomatic, gr->thresholds.max_cyclomatic);
    gr->maintainability_result = gate_check_maintainability(mr->maintainability_index, gr->thresholds.min_maintainability);
    gr->debt_result = gate_check_technical_debt(mr->technical_debt_ratio, gr->thresholds.max_technical_debt);
    int score = (int)gr->complexity_result + (int)gr->maintainability_result + (int)gr->debt_result;
    gr->overall = score == 0 ? GATE_PASS : score <= 2 ? GATE_WARN : GATE_FAIL;
    gr->passed_all = gr->overall == GATE_PASS;
}

const char* gate_result_name(GateResult r) { switch(r) { case GATE_PASS: return "PASS"; case GATE_WARN: return "WARN"; case GATE_FAIL: return "FAIL"; default: return "?"; } }

void gate_print(GateReport *gr) {
    printf("=== Quality Gate ===\n");
    printf("  Complexity: %s  Maintainability: %s  Debt: %s\n", gate_result_name(gr->complexity_result), gate_result_name(gr->maintainability_result), gate_result_name(gr->debt_result));
    printf("  OVERALL: %s\n", gr->passed_all ? "PASSED" : "FAILED");
}
