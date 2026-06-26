#ifndef TEST_COVERAGE_H
#define TEST_COVERAGE_H
#include <stdbool.h>

#define MAX_COV_FILES 32
#define MAX_COV_LINES_PER_FILE 256

typedef enum { CLINE_NEVER, CLINE_HIT, CLINE_PARTIAL, CLINE_BRANCH_TRUE, CLINE_BRANCH_FALSE, CLINE_BRANCH_BOTH } CovLineStatus;

typedef struct {
    int line_number; CovLineStatus status;
    int hit_count; /* number of times executed */
    bool has_branch; /* does this line contain a branch? */
    bool branch_true_hit; bool branch_false_hit;
} CoverageLine;

typedef struct {
    char file_path[128];
    CoverageLine lines[MAX_COV_LINES_PER_FILE]; int line_count;
    int total_lines; int covered_lines; int branch_points; int covered_branches;
} CoverageFile;

typedef struct {
    CoverageFile files[MAX_COV_FILES]; int file_count;
    /* MC/DC tracking */
    int mcdc_conditions; int mcdc_covered;
} CoverageData;

void coverage_init(CoverageData *cd);
int  coverage_add_file(CoverageData *cd, const char *file_path);
int  coverage_add_line(CoverageData *cd, int file_idx, int line_number, bool has_branch);
void coverage_hit(CoverageData *cd, int file_idx, int line_number);
void coverage_branch_hit(CoverageData *cd, int file_idx, int line_number, bool branch_true_hit, bool branch_false_hit);
void coverage_calculate(CoverageData *cd); /* compute overall percentages */
double coverage_line_pct(CoverageData *cd);
double coverage_branch_pct(CoverageData *cd);
double coverage_mcdc_pct(CoverageData *cd);
bool coverage_meets_threshold(CoverageData *cd, double line_pct, double branch_pct);
void coverage_print_file(CoverageData *cd, int file_idx);
void coverage_print_summary(CoverageData *cd);
#endif
