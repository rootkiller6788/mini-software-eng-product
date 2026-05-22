#include "../include/tdd_bdd.h"

tdd_runner_t g_tdd_runner;

void tdd_init(void) {
    memset(&g_tdd_runner, 0, sizeof(g_tdd_runner));
    g_tdd_runner.current_phase = TDD_RED;
}

tdd_feature_t* tdd_feature(const char *name, const char *description) {
    if (g_tdd_runner.feature_count >= TDD_FEATURE_MAX) {
        fprintf(stderr, "ERROR: too many features (max %d)\n", TDD_FEATURE_MAX);
        return NULL;
    }
    tdd_feature_t *f = &g_tdd_runner.features[g_tdd_runner.feature_count++];
    memset(f, 0, sizeof(tdd_feature_t));
    strncpy(f->name, name, TDD_NAME_MAX - 1);
    if (description) strncpy(f->description, description, TDD_NAME_MAX - 1);
    g_tdd_runner.current_feature = f;
    return f;
}

tdd_scenario_t* tdd_scenario(const char *name) {
    if (!g_tdd_runner.current_feature) {
        fprintf(stderr, "ERROR: no active feature for scenario '%s'\n", name);
        return NULL;
    }
    tdd_feature_t *f = g_tdd_runner.current_feature;
    if (f->scenario_count >= TDD_SCENARIO_MAX) {
        fprintf(stderr, "ERROR: too many scenarios in feature '%s' (max %d)\n", f->name, TDD_SCENARIO_MAX);
        return NULL;
    }
    tdd_scenario_t *s = &f->scenarios[f->scenario_count++];
    memset(s, 0, sizeof(tdd_scenario_t));
    strncpy(s->name, name, TDD_NAME_MAX - 1);
    s->passed = true;
    g_tdd_runner.current_scenario = s;
    g_tdd_runner.total_scenarios++;
    return s;
}

static void tdd_add_step(tdd_step_type_t type, const char *description, void (*action)(void)) {
    if (!g_tdd_runner.current_scenario) {
        fprintf(stderr, "ERROR: no active scenario for step '%s'\n", description);
        return;
    }
    tdd_scenario_t *s = g_tdd_runner.current_scenario;
    if (s->step_count >= TDD_STEP_MAX) {
        fprintf(stderr, "ERROR: too many steps in scenario '%s'\n", s->name);
        return;
    }
    tdd_step_t *step = &s->steps[s->step_count++];
    memset(step, 0, sizeof(tdd_step_t));
    step->type = type;
    strncpy(step->description, description, TDD_NAME_MAX - 1);
    step->action = action;
}

void tdd_given(const char *description) { tdd_add_step(TDD_STEP_GIVEN, description, NULL); }
void tdd_when(const char *description)  { tdd_add_step(TDD_STEP_WHEN, description, NULL); }
void tdd_then(const char *description)  { tdd_add_step(TDD_STEP_THEN, description, NULL); }
void tdd_and(const char *description)   { tdd_add_step(TDD_STEP_AND, description, NULL); }
void tdd_but(const char *description)   { tdd_add_step(TDD_STEP_BUT, description, NULL); }

void tdd_step_given(const char *description, void (*action)(void)) { tdd_add_step(TDD_STEP_GIVEN, description, action); }
void tdd_step_when(const char *description, void (*action)(void))  { tdd_add_step(TDD_STEP_WHEN, description, action); }
void tdd_step_then(const char *description, void (*action)(void))  { tdd_add_step(TDD_STEP_THEN, description, action); }
void tdd_step_and(const char *description, void (*action)(void))   { tdd_add_step(TDD_STEP_AND, description, action); }
void tdd_step_but(const char *description, void (*action)(void))   { tdd_add_step(TDD_STEP_BUT, description, action); }

void tdd_assert_true(bool condition, const char *msg) {
    if (!g_tdd_runner.current_scenario) return;
    if (!condition) {
        g_tdd_runner.current_scenario->passed = false;
        if (msg) strncpy(g_tdd_runner.current_scenario->failure_msg, msg, TDD_NAME_MAX - 1);
    }
}

void tdd_assert_eq_int(int expected, int actual, const char *msg) {
    if (!g_tdd_runner.current_scenario) return;
    if (expected != actual) {
        g_tdd_runner.current_scenario->passed = false;
        if (msg) {
            strncpy(g_tdd_runner.current_scenario->failure_msg, msg, TDD_NAME_MAX - 1);
        } else {
            snprintf(g_tdd_runner.current_scenario->failure_msg, TDD_NAME_MAX,
                     "expected %d, got %d", expected, actual);
        }
    }
}

void tdd_assert_eq_str(const char *expected, const char *actual, const char *msg) {
    if (!g_tdd_runner.current_scenario) return;
    if (strcmp(expected, actual) != 0) {
        g_tdd_runner.current_scenario->passed = false;
        if (msg) {
            strncpy(g_tdd_runner.current_scenario->failure_msg, msg, TDD_NAME_MAX - 1);
        } else {
            snprintf(g_tdd_runner.current_scenario->failure_msg, TDD_NAME_MAX,
                     "expected \"%s\", got \"%s\"", expected, actual);
        }
    }
}

void tdd_set_phase(tdd_phase_t phase) {
    g_tdd_runner.current_phase = phase;
    const char *name = phase == TDD_RED ? "RED" : phase == TDD_GREEN ? "GREEN" : "REFACTOR";
    printf("  >>> TDD Phase: %s <<<\n", name);
}

tdd_phase_t tdd_get_phase(void) {
    return g_tdd_runner.current_phase;
}

static const char* tdd_step_type_name(tdd_step_type_t type) {
    switch (type) {
        case TDD_STEP_GIVEN: return "GIVEN";
        case TDD_STEP_WHEN:  return "WHEN";
        case TDD_STEP_THEN:  return "THEN";
        case TDD_STEP_AND:   return "AND";
        case TDD_STEP_BUT:   return "BUT";
        default: return "UNKNOWN";
    }
}

void tdd_run_all(void) {
    printf("\n========================================\n");
    printf("  BDD Test Runner\n");
    printf("========================================\n");

    int fi, si, sti;
    for (fi = 0; fi < g_tdd_runner.feature_count; fi++) {
        tdd_feature_t *f = &g_tdd_runner.features[fi];
        printf("\n  Feature: %s\n", f->name);
        printf("    %s\n", f->description);
        printf("  ----------------------------------------\n");

        f->passed = 0;
        f->failed = 0;

        for (si = 0; si < f->scenario_count; si++) {
            tdd_scenario_t *s = &f->scenarios[si];
            g_tdd_runner.current_scenario = s;
            s->passed = true;

            printf("  Scenario: %s\n", s->name);

            for (sti = 0; sti < s->step_count; sti++) {
                tdd_step_t *step = &s->steps[sti];
                printf("    %s %s", tdd_step_type_name(step->type), step->description);
                if (step->action) {
                    step->action();
                }
                printf("\n");
            }

            if (s->passed) {
                f->passed++;
                g_tdd_runner.total_passed++;
                printf("    => PASSED\n");
            } else {
                f->failed++;
                g_tdd_runner.total_failed++;
                printf("    => FAILED: %s\n", s->failure_msg);
            }
        }
        printf("  ----------------------------------------\n");
        printf("  Feature result: %d passed, %d failed\n", f->passed, f->failed);
    }
}

void tdd_report(void) {
    printf("\n========================================\n");
    printf("  BDD Report Summary\n");
    printf("========================================\n");
    printf("  Total features:  %d\n", g_tdd_runner.feature_count);
    printf("  Total scenarios: %d\n", g_tdd_runner.total_scenarios);
    printf("  Passed:          %d\n", g_tdd_runner.total_passed);
    printf("  Failed:          %d\n", g_tdd_runner.total_failed);
    printf("========================================\n");
}

int tdd_exit_code(void) {
    return (g_tdd_runner.total_failed > 0) ? 1 : 0;
}

void tdd_generate_living_doc(const char *filepath) {
    FILE *fp = fopen(filepath, "w");
    if (!fp) {
        fprintf(stderr, "ERROR: cannot write living doc to '%s'\n", filepath);
        return;
    }

    fprintf(fp, "# Living Documentation\n\n");
    fprintf(fp, "Generated: %s\n\n", __DATE__);

    int fi, si, sti;
    for (fi = 0; fi < g_tdd_runner.feature_count; fi++) {
        tdd_feature_t *f = &g_tdd_runner.features[fi];
        fprintf(fp, "## Feature: %s\n\n", f->name);
        fprintf(fp, "%s\n\n", f->description);

        for (si = 0; si < f->scenario_count; si++) {
            tdd_scenario_t *s = &f->scenarios[si];
            fprintf(fp, "### Scenario: %s\n\n", s->name);

            char status = s->passed ? 'Y' : 'N';
            fprintf(fp, "| Status | Step | Description |\n");
            fprintf(fp, "|--------|------|-------------|\n");

            for (sti = 0; sti < s->step_count; sti++) {
                tdd_step_t *step = &s->steps[sti];
                fprintf(fp, "| %c | %s | %s |\n",
                        status, tdd_step_type_name(step->type), step->description);
            }
            fprintf(fp, "\n");
        }
    }

    fclose(fp);
    printf("  Living documentation written to: %s\n", filepath);
}
