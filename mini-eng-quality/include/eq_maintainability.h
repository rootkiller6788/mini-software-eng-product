#ifndef EQ_MAINTAINABILITY_H
#define EQ_MAINTAINABILITY_H
#include <stdbool.h>

typedef struct {
    double maintainability_index; /* MI = 171 - 5.2*ln(V) - 0.23*CC - 16.2*ln(LOC) */
    double technical_debt_ratio;  /* TDR = cost_to_fix / cost_to_develop */
    double code_churn;            /* lines changed per week */
    double defect_density;        /* bugs per KLOC */
    int    remediation_cost_hours;
    char   grade;                 /* A-F */
} MaintainabilityReport;

void maint_compute(double halstead_volume, int cyclomatic, int loc, int comments, double churn, int bugs, MaintainabilityReport *mr);
void maint_assess_grade(MaintainabilityReport *mr);
void maint_print(MaintainabilityReport *mr);
const char *maint_grade_description(char grade);
#endif
