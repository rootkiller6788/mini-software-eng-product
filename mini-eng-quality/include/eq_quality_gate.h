#ifndef EQ_QUALITY_GATE_H
#define EQ_QUALITY_GATE_H
#include "eq_complexity.h"
#include "eq_maintainability.h"
#include <stdbool.h>

typedef enum { GATE_PASS, GATE_WARN, GATE_FAIL } GateResult;

typedef struct { double max_cyclomatic; double min_maintainability; double max_technical_debt; int max_nesting_depth; int max_file_loc; } QualityThresholds;

typedef struct {
    QualityThresholds thresholds;
    GateResult overall;
    GateResult complexity_result;
    GateResult maintainability_result;
    GateResult debt_result;
    bool passed_all;
} GateReport;

void gate_init_default_thresholds(QualityThresholds *qt);
GateResult gate_check_cyclomatic(int cc, double threshold);
GateResult gate_check_maintainability(double mi, double threshold);
GateResult gate_check_technical_debt(double tdr, double threshold);
void gate_evaluate(GateReport *gr, ComplexityMetrics *cm, MaintainabilityReport *mr);
void gate_print(GateReport *gr);
#endif
