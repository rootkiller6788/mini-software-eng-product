#ifndef TDD_BDD_H
#define TDD_BDD_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define TDD_FEATURE_MAX   32
#define TDD_SCENARIO_MAX  64
#define TDD_STEP_MAX      128
#define TDD_NAME_MAX      256

typedef enum {
    TDD_RED,
    TDD_GREEN,
    TDD_REFACTOR
} tdd_phase_t;

typedef enum {
    TDD_STEP_GIVEN,
    TDD_STEP_WHEN,
    TDD_STEP_THEN,
    TDD_STEP_AND,
    TDD_STEP_BUT
} tdd_step_type_t;

typedef struct tdd_step {
    tdd_step_type_t type;
    char            description[TDD_NAME_MAX];
    void          (*action)(void);
} tdd_step_t;

typedef struct tdd_scenario {
    char       name[TDD_NAME_MAX];
    tdd_step_t steps[TDD_STEP_MAX];
    int        step_count;
    bool       passed;
    char       failure_msg[TDD_NAME_MAX];
} tdd_scenario_t;

typedef struct tdd_feature {
    char           name[TDD_NAME_MAX];
    char           description[TDD_NAME_MAX];
    tdd_scenario_t scenarios[TDD_SCENARIO_MAX];
    int            scenario_count;
    int            passed;
    int            failed;
} tdd_feature_t;

typedef struct tdd_runner {
    tdd_feature_t features[TDD_FEATURE_MAX];
    int           feature_count;
    tdd_phase_t   current_phase;
    tdd_feature_t *current_feature;
    tdd_scenario_t *current_scenario;
    int           total_scenarios;
    int           total_passed;
    int           total_failed;
} tdd_runner_t;

extern tdd_runner_t g_tdd_runner;

void        tdd_init(void);

tdd_feature_t* tdd_feature(const char *name, const char *description);
tdd_scenario_t* tdd_scenario(const char *name);

void        tdd_given(const char *description);
void        tdd_when(const char *description);
void        tdd_then(const char *description);
void        tdd_and(const char *description);
void        tdd_but(const char *description);

void        tdd_step_given(const char *description, void (*action)(void));
void        tdd_step_when(const char *description, void (*action)(void));
void        tdd_step_then(const char *description, void (*action)(void));
void        tdd_step_and(const char *description, void (*action)(void));
void        tdd_step_but(const char *description, void (*action)(void));

void        tdd_assert_true(bool condition, const char *msg);
void        tdd_assert_eq_int(int expected, int actual, const char *msg);
void        tdd_assert_eq_str(const char *expected, const char *actual, const char *msg);

void        tdd_set_phase(tdd_phase_t phase);
tdd_phase_t tdd_get_phase(void);

void        tdd_run_all(void);
void        tdd_report(void);
int         tdd_exit_code(void);

void        tdd_generate_living_doc(const char *filepath);

#define FEATURE(name, desc) \
    tdd_feature_t *_tdd_cur = tdd_feature((name), (desc))

#define SCENARIO(name) \
    tdd_scenario_t *_tdd_scn = tdd_scenario((name))

#define GIVEN(desc)   tdd_given(desc)
#define WHEN(desc)    tdd_when(desc)
#define THEN(desc)    tdd_then(desc)
#define AND(desc)     tdd_and(desc)
#define BUT(desc)     tdd_but(desc)

#define GIVEN_STEP(desc, act) tdd_step_given(desc, act)
#define WHEN_STEP(desc, act)  tdd_step_when(desc, act)
#define THEN_STEP(desc, act)  tdd_step_then(desc, act)
#define AND_STEP(desc, act)   tdd_step_and(desc, act)
#define BUT_STEP(desc, act)   tdd_step_but(desc, act)

#define SHOW(expr)   printf("  %s = %d\n", #expr, (int)(expr))
#define PENDING(msg) printf("  [PENDING] %s\n", (msg))

#endif
