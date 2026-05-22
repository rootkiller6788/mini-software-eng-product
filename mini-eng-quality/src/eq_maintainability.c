#include "eq_maintainability.h"
#include <stdio.h>
#include <math.h>

void maint_compute(double hv, int cc, int loc, int comments, double churn, int bugs, MaintainabilityReport *mr) {
    double safe_loc = loc > 0 ? (double)loc : 1.0;
    double safe_hv = hv > 0 ? hv : 1.0;
    mr->maintainability_index = 171.0 - 5.2 * log(safe_hv) - 0.23 * cc - 16.2 * log(safe_loc);
    if (mr->maintainability_index < 0) mr->maintainability_index = 0;
    if (mr->maintainability_index > 171) mr->maintainability_index = 171;
    double cost_fix = hv * 0.1 + cc * 2.0;
    double cost_dev = (double)loc * 0.05;
    mr->technical_debt_ratio = cost_dev > 0 ? cost_fix / cost_dev : 0;
    mr->code_churn = churn;
    mr->defect_density = loc > 0 ? (double)bugs * 1000.0 / loc : 0;
    mr->remediation_cost_hours = (int)(cost_fix / 10.0);
    maint_assess_grade(mr);
}

void maint_assess_grade(MaintainabilityReport *mr) {
    double mi = mr->maintainability_index;
    if (mi > 85) mr->grade = 'A';
    else if (mi > 65) mr->grade = 'B';
    else if (mi > 45) mr->grade = 'C';
    else if (mi > 25) mr->grade = 'D';
    else mr->grade = 'F';
}

const char *maint_grade_description(char grade) {
    switch (grade) { case 'A': return "Highly maintainable"; case 'B': return "Good"; case 'C': return "Adequate"; case 'D': return "At risk"; case 'F': return "Critical"; default: return "?"; }
}

void maint_print(MaintainabilityReport *mr) {
    printf("  MI: %.1f (Grade %c: %s)\n", mr->maintainability_index, mr->grade, maint_grade_description(mr->grade));
    printf("  TDR: %.3f, Defect Density: %.2f/KLOC\n", mr->technical_debt_ratio, mr->defect_density);
    printf("  Remediation: %d hours\n", mr->remediation_cost_hours);
}
