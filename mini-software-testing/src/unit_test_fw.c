#include "../include/unit_test_fw.h"

utfw_runner_t g_ut_runner;

void utfw_init(void) {
    memset(&g_ut_runner, 0, sizeof(g_ut_runner));
    g_ut_runner.current_suite = NULL;
    g_ut_runner.current_case = NULL;
    g_ut_runner.current_failed = false;
    g_ut_runner.current_skip = false;
}

utfw_suite_t* utfw_register_suite(const char *name, void (*setup)(void), void (*teardown)(void)) {
    if (g_ut_runner.suite_count >= UTFW_SUITES_MAX) {
        fprintf(stderr, "ERROR: too many test suites (max %d)\n", UTFW_SUITES_MAX);
        return NULL;
    }
    int i;
    for (i = 0; i < g_ut_runner.suite_count; i++) {
        if (strcmp(g_ut_runner.suites[i].name, name) == 0) {
            if (setup) g_ut_runner.suites[i].setup = setup;
            if (teardown) g_ut_runner.suites[i].teardown = teardown;
            return &g_ut_runner.suites[i];
        }
    }
    utfw_suite_t *s = &g_ut_runner.suites[g_ut_runner.suite_count++];
    memset(s, 0, sizeof(utfw_suite_t));
    strncpy(s->name, name, UTFW_NAME_MAX - 1);
    s->setup = setup;
    s->teardown = teardown;
    return s;
}

utfw_case_t* utfw_register_case(utfw_suite_t *suite, const char *name, const char *file, int line, void (*func)(void)) {
    if (!suite) return NULL;
    if (suite->case_count >= UTFW_CASES_MAX) {
        fprintf(stderr, "ERROR: too many test cases in suite '%s' (max %d)\n", suite->name, UTFW_CASES_MAX);
        return NULL;
    }
    utfw_case_t *c = &suite->cases[suite->case_count++];
    memset(c, 0, sizeof(utfw_case_t));
    strncpy(c->name, name, UTFW_NAME_MAX - 1);
    if (file) strncpy(c->file, file, UTFW_NAME_MAX - 1);
    c->line = line;
    c->func = func;
    c->result = UTFW_PASS;
    return c;
}

static void utfw_run_suite(utfw_suite_t *suite) {
    g_ut_runner.current_suite = suite;
    printf("\n  Suite: %s\n", suite->name);
    printf("  ----------------------------------------\n");

    clock_t suite_start = clock();
    int i;
    for (i = 0; i < suite->case_count; i++) {
        utfw_case_t *c = &suite->cases[i];
        g_ut_runner.current_case = c;
        g_ut_runner.current_failed = false;
        g_ut_runner.current_skip = false;
        c->result = UTFW_PASS;

        if (suite->setup) suite->setup();

        clock_t case_start = clock();
        if (c->func) {
            c->func();
        }
        clock_t case_end = clock();
        if (suite->teardown) suite->teardown();

        if (g_ut_runner.current_skip) {
            c->result = UTFW_SKIP;
            suite->skipped++;
            g_ut_runner.total_skipped++;
            printf("    [SKIP] %s - %s\n", c->name, c->message);
        } else if (g_ut_runner.current_failed) {
            if (c->result == UTFW_PASS) c->result = UTFW_FAIL;
            suite->failed++;
            g_ut_runner.total_failed++;
            printf("    [FAIL] %s (%s:%d) - %s\n", c->name, c->file, c->line, c->message);
        } else {
            suite->passed++;
            g_ut_runner.total_passed++;
            printf("    [PASS] %s\n", c->name);
        }

        c->elapsed_ms = (double)(case_end - case_start) * 1000.0 / CLOCKS_PER_SEC;
    }

    clock_t suite_end = clock();
    suite->elapsed_ms = (double)(suite_end - suite_start) * 1000.0 / CLOCKS_PER_SEC;
    printf("  ----------------------------------------\n");
    printf("  Suite result: %d passed, %d failed, %d skipped  [%.2f ms]\n",
           suite->passed, suite->failed, suite->skipped, suite->elapsed_ms);
}

void utfw_run_all(void) {
    printf("\n========================================\n");
    printf("  Unit Test Runner\n");
    printf("========================================\n");

    clock_t total_start = clock();
    int i;
    for (i = 0; i < g_ut_runner.suite_count; i++) {
        utfw_run_suite(&g_ut_runner.suites[i]);
    }
    clock_t total_end = clock();

    g_ut_runner.total_elapsed_ms = (double)(total_end - total_start) * 1000.0 / CLOCKS_PER_SEC;
}

void utfw_report(void) {
    printf("\n========================================\n");
    printf("  Test Report Summary\n");
    printf("========================================\n");
    printf("  Total suites:     %d\n", g_ut_runner.suite_count);
    printf("  Total tests:      %d\n", (int)(g_ut_runner.total_passed + g_ut_runner.total_failed + g_ut_runner.total_skipped));
    printf("  Total assertions: %d\n", g_ut_runner.total_assertions);
    printf("  ----------------------------------------\n");
    printf("  Passed:           %d\n", g_ut_runner.total_passed);
    printf("  Failed:           %d\n", g_ut_runner.total_failed);
    printf("  Skipped:          %d\n", g_ut_runner.total_skipped);
    printf("  Total time:       %.2f ms\n", g_ut_runner.total_elapsed_ms);
    printf("========================================\n");

    if (g_ut_runner.total_failed > 0) {
        printf("\n  FAILURES:\n");
        printf("  ----------------------------------------\n");
        int si, ci;
        for (si = 0; si < g_ut_runner.suite_count; si++) {
            utfw_suite_t *s = &g_ut_runner.suites[si];
            for (ci = 0; ci < s->case_count; ci++) {
                utfw_case_t *c = &s->cases[ci];
                if (c->result == UTFW_FAIL) {
                    printf("  [%s] %s (%s:%d)\n     %s\n",
                           s->name, c->name, c->file, c->line, c->message);
                }
            }
        }
    }
}

int utfw_exit_code(void) {
    return (g_ut_runner.total_failed > 0) ? 1 : 0;
}

void utfw_record_pass(void) {
    g_ut_runner.current_failed = false;
}

void utfw_record_fail(const char *file, int line, const char *msg) {
    g_ut_runner.current_failed = true;
    if (g_ut_runner.current_case) {
        g_ut_runner.current_case->result = UTFW_FAIL;
        if (file) strncpy(g_ut_runner.current_case->file, file, UTFW_NAME_MAX - 1);
        g_ut_runner.current_case->line = line;
        if (msg) strncpy(g_ut_runner.current_case->message, msg, UTFW_MSG_MAX - 1);
    }
}

void utfw_record_skip(const char *msg) {
    g_ut_runner.current_skip = true;
    if (g_ut_runner.current_case) {
        if (msg) strncpy(g_ut_runner.current_case->message, msg, UTFW_MSG_MAX - 1);
    }
}

void utfw_record_error(const char *file, int line, const char *msg) {
    g_ut_runner.current_failed = true;
    if (g_ut_runner.current_case) {
        g_ut_runner.current_case->result = UTFW_ERROR;
        if (file) strncpy(g_ut_runner.current_case->file, file, UTFW_NAME_MAX - 1);
        g_ut_runner.current_case->line = line;
        if (msg) strncpy(g_ut_runner.current_case->message, msg, UTFW_MSG_MAX - 1);
    }
}
