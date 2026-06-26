#include "test_reporting.h"
#include <stdio.h>
#include <string.h>

void report_init(TestReport *tr, const char *project_name, int flaky_threshold) {
    memset(tr, 0, sizeof(*tr));
    strncpy(tr->project_name, project_name, 63); tr->project_name[63] = '\0';
    tr->flaky_threshold = flaky_threshold;
}

void report_record_run(TestReport *tr, TestRunner *runner) {
    if (tr->run_count >= MAX_REPORT_RUNS) return;
    TestRun *run = &tr->runs[tr->run_count];
    int total = runner->total_pass + runner->total_fail + runner->total_error + runner->total_skip;
    snprintf(run->run_date, sizeof(run->run_date), "run-%d", tr->run_count + 1);
    run->total = total; run->passed = runner->total_pass; run->failed = runner->total_fail;
    run->skipped = runner->total_skip; run->duration_ms = runner->total_duration_ms;
    run->pass_rate = total > 0 ? 100.0 * runner->total_pass / total : 0;
    tr->run_count++;
}

double report_pass_rate_trend(TestReport *tr) {
    if (tr->run_count < 2) return 0;
    int n = tr->run_count;
    double sum_x = 0, sum_y = 0, sum_xy = 0, sum_x2 = 0;
    for (int i = 0; i < n; i++) {
        sum_x += i; sum_y += tr->runs[i].pass_rate;
        sum_xy += i * tr->runs[i].pass_rate; sum_x2 += i * i;
    }
    double denom = n * sum_x2 - sum_x * sum_x;
    if (denom < 1e-9) return 0;
    return (n * sum_xy - sum_x * sum_y) / denom;
}

int report_flaky_detect(TestReport *tr, TestRunner *runner) {
    (void)runner; /* In real impl, would compare successive run results */
    return tr->flaky_count;
}

void report_export_junit(TestReport *tr, TestRunner *runner, const char *output_path) {
    FILE *f = fopen(output_path, "w");
    if (!f) return;
    fprintf(f, "<?xml version=\"1.0\"?>\n<testsuites name=\"%s\" tests=\"%d\">\n",
            tr->project_name, runner->total_pass + runner->total_fail + runner->total_error);
    for (int i = 0; i < runner->suite_count; i++) {
        TestSuite *ts = &runner->suites[i];
        fprintf(f, "  <testsuite name=\"%s\" tests=\"%d\">\n", ts->name, ts->test_count);
        for (int j = 0; j < ts->test_count; j++) {
            TestCase *tc = &ts->tests[j];
            const char *ss[] = {"passed","failed","error","skipped"};
            fprintf(f, "    <testcase name=\"%s\" status=\"%s\" time=\"%d\"/>\n",
                    tc->name, ss[tc->result], tc->duration_ms);
        }
        fprintf(f, "  </testsuite>\n");
    }
    fprintf(f, "</testsuites>\n");
    fclose(f);
}

void report_print_history(TestReport *tr) {
    printf("=== Test History: %s ===\n", tr->project_name);
    for (int i = 0; i < tr->run_count; i++)
        printf("  %s: %d/%d passed (%.1f%%) %dms\n",
               tr->runs[i].run_date, tr->runs[i].passed, tr->runs[i].total,
               tr->runs[i].pass_rate, tr->runs[i].duration_ms);
}

void report_print_trend(TestReport *tr) {
    double trend = report_pass_rate_trend(tr);
    printf("  Pass rate trend: %+.1f%%/run (%s)\n", trend,
           trend > 0 ? "IMPROVING" : trend < 0 ? "DECLINING" : "STABLE");
}

/* Mutation testing */
void mutation_init(MutationReport *mr) { memset(mr, 0, sizeof(*mr)); }

int mutation_add_mutant(MutationReport *mr, const char *name) {
    if (mr->mutant_count >= 64) return -1;
    MutationResult *m = &mr->mutants[mr->mutant_count];
    strncpy(m->mutant_name, name, 63); m->mutant_name[63] = '\0';
    m->killed = false; m->surviving = 0;
    return mr->mutant_count++;
}

void mutation_kill(MutationReport *mr, const char *name) {
    for (int i = 0; i < mr->mutant_count; i++)
        if (strcmp(mr->mutants[i].mutant_name, name) == 0) {
            if (!mr->mutants[i].killed) { mr->mutants[i].killed = true; mr->total_killed++; }
            return;
        }
}

double mutation_score(MutationReport *mr) {
    return mr->mutant_count > 0 ? 100.0 * mr->total_killed / mr->mutant_count : 0;
}

void mutation_print(MutationReport *mr) {
    printf("=== Mutation Testing ===\n");
    printf("  Killed: %d/%d (%.1f%% mutation score)\n", mr->total_killed, mr->mutant_count, mutation_score(mr));
    for (int i = 0; i < mr->mutant_count; i++)
        printf("  %s: %s\n", mr->mutants[i].mutant_name, mr->mutants[i].killed ? "KILLED" : "SURVIVED");
}
