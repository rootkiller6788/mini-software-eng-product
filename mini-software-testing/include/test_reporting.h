#ifndef TEST_REPORTING_H
#define TEST_REPORTING_H
#include <stdbool.h>
#include "test_framework.h"

#define MAX_REPORT_RUNS 64

typedef struct {
    char run_date[24]; int total; int passed; int failed; int skipped;
    int duration_ms; double pass_rate;
} TestRun;

typedef struct {
    TestRun runs[MAX_REPORT_RUNS]; int run_count;
    char project_name[64];
    /* Flaky test tracking */
    char flaky_tests[64][64]; int flaky_count;
    int flaky_threshold; /* number of alternations to be flaky */
} TestReport;

void report_init(TestReport *tr, const char *project_name, int flaky_threshold);
void report_record_run(TestReport *tr, TestRunner *runner);
double report_pass_rate_trend(TestReport *tr); /* slope of pass rate over time */
int  report_flaky_detect(TestReport *tr, TestRunner *runner); /* returns count of flaky tests */
void report_export_junit(TestReport *tr, TestRunner *runner, const char *output_path);
void report_print_history(TestReport *tr);
void report_print_trend(TestReport *tr);

/* Mutation testing helper */
typedef struct { char mutant_name[64]; bool killed; /* test caught this mutant */ int surviving; } MutationResult;
typedef struct { MutationResult mutants[64]; int mutant_count; int total_killed; } MutationReport;
void mutation_init(MutationReport *mr);
int  mutation_add_mutant(MutationReport *mr, const char *name);
void mutation_kill(MutationReport *mr, const char *name);
double mutation_score(MutationReport *mr); /* killed / total */
void mutation_print(MutationReport *mr);
#endif
