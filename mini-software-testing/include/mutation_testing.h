#ifndef MUTATION_TESTING_H
#define MUTATION_TESTING_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>

#define MUT_MAX_OPERATORS   32
#define MUT_MAX_MUTANTS     1024
#define MUT_FILE_MAX        10240
#define MUT_LINE_MAX        512
#define MUT_NAME_MAX        256

typedef enum {
    MUT_ARITHMETIC_ADD_TO_SUB,
    MUT_ARITHMETIC_SUB_TO_ADD,
    MUT_ARITHMETIC_MUL_TO_DIV,
    MUT_ARITHMETIC_DIV_TO_MUL,
    MUT_ARITHMETIC_MOD_TO_MUL,
    MUT_RELATIONAL_GT_TO_GTE,
    MUT_RELATIONAL_GTE_TO_GT,
    MUT_RELATIONAL_LT_TO_LTE,
    MUT_RELATIONAL_LTE_TO_LT,
    MUT_RELATIONAL_EQ_TO_NEQ,
    MUT_RELATIONAL_NEQ_TO_EQ,
    MUT_LOGICAL_AND_TO_OR,
    MUT_LOGICAL_OR_TO_AND,
    MUT_CONDITIONAL_TRUE,
    MUT_CONDITIONAL_FALSE,
    MUT_INCREMENT_TO_DECREMENT,
    MUT_DECREMENT_TO_INCREMENT,
    MUT_ASSIGNMENT_ADD_TO_SUB,
    MUT_ASSIGNMENT_SUB_TO_ADD,
    MUT_RETURN_REMOVE,
    MUT_CONSTANT_ZERO,
    MUT_CONSTANT_ONE,
    MUT_CONSTANT_NEG_ONE,
    MUT_NEGATE_CONDITIONAL,
    MUT_BITWISE_AND_TO_OR,
    MUT_BITWISE_OR_TO_AND,
    MUT_BITWISE_XOR_TO_AND,
    MUT_SHIFT_LEFT_TO_RIGHT,
    MUT_SHIFT_RIGHT_TO_LEFT,
    MUT_REMOVE_VOID_CALL,
    MUT_SWAP_BRANCH,
    MUT_COUNT
} mut_operator_t;

typedef struct mut_source_line {
    int  number;
    char content[MUT_LINE_MAX];
} mut_source_line_t;

typedef struct mut_mutant {
    int             id;
    mut_operator_t  operator_type;
    int             line_number;
    int             col_start;
    int             col_end;
    char            original_text[128];
    char            mutated_text[128];
    bool            killed;
    bool            survived;
    bool            equivalent;
    bool            compiled;
    char            description[MUT_NAME_MAX];
} mut_mutant_t;

typedef struct mut_config {
    mut_operator_t enabled_operators[MUT_MAX_OPERATORS];
    int            enabled_count;
    char           source_files[32][MUT_NAME_MAX];
    int            source_count;
    bool           detect_equivalents;
    int            timeout_ms;
} mut_config_t;

typedef struct mut_runner {
    mut_config_t    config;
    mut_mutant_t    mutants[MUT_MAX_MUTANTS];
    int             mutant_count;
    int             mutants_killed;
    int             mutants_survived;
    int             mutants_equivalent;
    int             total_tests_run;
    double          mutation_score;
    mut_source_line_t source_lines[MUT_FILE_MAX];
    int             source_line_count;
} mut_runner_t;

extern mut_runner_t g_mut_runner;

void        mut_init(void);
void        mut_config_init(mut_config_t *config);
void        mut_config_enable_operator(mut_config_t *config, mut_operator_t op);
void        mut_config_add_source(mut_config_t *config, const char *filepath);

void        mut_generate_mutants(mut_runner_t *runner);
int         mut_run_tests_on_mutant(mut_runner_t *runner, mut_mutant_t *mutant, int (*test_func)(void));

void        mut_record_killed(mut_runner_t *runner, int mutant_id);
void        mut_record_survived(mut_runner_t *runner, int mutant_id);
void        mut_record_equivalent(mut_runner_t *runner, int mutant_id);

double      mut_calculate_score(mut_runner_t *runner);
void        mut_report(mut_runner_t *runner);
void        mut_report_detailed(mut_runner_t *runner);

const char* mut_operator_name(mut_operator_t op);
const char* mut_operator_description(mut_operator_t op);

bool        mut_is_equivalent(int mutant_id);
mut_mutant_t* mut_find_mutant(mut_runner_t *runner, int id);

#endif
