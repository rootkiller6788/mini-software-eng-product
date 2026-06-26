#include "test_framework.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void runner_init(TestRunner *tr) { memset(tr, 0, sizeof(*tr)); }

int suite_add(TestRunner *tr, const char *name, void(*setup)(void), void(*teardown)(void)) {
    if (tr->suite_count >= MAX_SUITES) return -1;
    TestSuite *ts = &tr->suites[tr->suite_count];
    strncpy(ts->name, name, 63); ts->name[63] = '\0';
    ts->test_count = 0; ts->setup = setup; ts->teardown = teardown;
    return tr->suite_count++;
}

int test_add(TestRunner *tr, int suite_idx, const char *name) {
    if (suite_idx < 0 || suite_idx >= tr->suite_count) return -1;
    TestSuite *ts = &tr->suites[suite_idx];
    if (ts->test_count >= MAX_TESTS_PER_SUITE) return -1;
    TestCase *tc = &ts->tests[ts->test_count];
    strncpy(tc->name, name, 63); tc->name[63] = '\0';
    tc->result = TRES_PASS; tc->assert_count = 0; tc->duration_ms = 0;
    return ts->test_count++;
}

void test_record_assert(TestRunner *tr, int suite_idx, int test_idx, int line, const char *file, const char *msg, bool passed) {
    if (suite_idx < 0 || suite_idx >= tr->suite_count) return;
    TestSuite *ts = &tr->suites[suite_idx];
    if (test_idx < 0 || test_idx >= ts->test_count) return;
    TestCase *tc = &ts->tests[test_idx];
    if (tc->assert_count >= MAX_ASSERTS_PER_TEST) return;
    Assertion *a = &tc->assertions[tc->assert_count];
    a->line = line; a->passed = passed;
    strncpy(a->file, file, 63); a->file[63] = '\0';
    strncpy(a->message, msg, 255); a->message[255] = '\0';
    tc->assert_count++;
}

void test_finish(TestRunner *tr, int suite_idx, int test_idx, TestResult result, int duration_ms) {
    if (suite_idx < 0 || suite_idx >= tr->suite_count) return;
    TestSuite *ts = &tr->suites[suite_idx];
    if (test_idx < 0 || test_idx >= ts->test_count) return;
    ts->tests[test_idx].result = result;
    ts->tests[test_idx].duration_ms = duration_ms;
    switch (result) {
        case TRES_PASS: tr->total_pass++; break;
        case TRES_FAIL: tr->total_fail++; break;
        case TRES_ERROR: tr->total_error++; break;
        case TRES_SKIP: tr->total_skip++; break;
    }
    tr->total_duration_ms += duration_ms;
}

int runner_run_all(TestRunner *tr) {
    tr->total_pass = 0; tr->total_fail = 0; tr->total_error = 0; tr->total_skip = 0; tr->total_duration_ms = 0;
    for (int i = 0; i < tr->suite_count; i++) {
        TestSuite *ts = &tr->suites[i];
        printf("\n=== Suite: %s ===\n", ts->name);
        for (int j = 0; j < ts->test_count; j++) {
            TestCase *tc = &ts->tests[j];
            if (ts->setup) ts->setup();
            bool all_pass = true;
            for (int k = 0; k < tc->assert_count; k++)
                if (!tc->assertions[k].passed) all_pass = false;
            TestResult res = all_pass ? TRES_PASS : TRES_FAIL;
            int dur = rand() % 100; /* simulated duration */
            test_finish(tr, i, j, res, dur);
            const char *ss[] = {"PASS","FAIL","ERROR","SKIP"};
            printf("  %s %s (%dms)\n", ss[res], tc->name, dur);
            if (!all_pass)
                for (int k = 0; k < tc->assert_count; k++)
                    if (!tc->assertions[k].passed)
                        printf("    FAIL: %s:%d - %s\n", tc->assertions[k].file, tc->assertions[k].line, tc->assertions[k].message);
            if (ts->teardown) ts->teardown();
        }
    }
    return tr->total_fail + tr->total_error;
}

void runner_print_summary(TestRunner *tr) {
    int total = tr->total_pass + tr->total_fail + tr->total_error + tr->total_skip;
    printf("\n=== Test Summary ===\n");
    printf("  Total: %d | Pass: %d | Fail: %d | Error: %d | Skip: %d\n",
           total, tr->total_pass, tr->total_fail, tr->total_error, tr->total_skip);
    printf("  Duration: %dms\n", tr->total_duration_ms);
    printf("  Result: %s\n", (tr->total_fail + tr->total_error) == 0 ? "ALL PASSED" : "FAILURES");
}
