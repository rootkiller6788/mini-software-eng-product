#include "../include/mutation_testing.h"

mut_runner_t g_mut_runner;

void mut_init(void) {
    memset(&g_mut_runner, 0, sizeof(g_mut_runner));
    mut_config_init(&g_mut_runner.config);
}

void mut_config_init(mut_config_t *config) {
    memset(config, 0, sizeof(mut_config_t));
    mut_config_enable_operator(config, MUT_ARITHMETIC_ADD_TO_SUB);
    mut_config_enable_operator(config, MUT_ARITHMETIC_SUB_TO_ADD);
    mut_config_enable_operator(config, MUT_RELATIONAL_GT_TO_GTE);
    mut_config_enable_operator(config, MUT_RELATIONAL_GTE_TO_GT);
    mut_config_enable_operator(config, MUT_RELATIONAL_LT_TO_LTE);
    mut_config_enable_operator(config, MUT_RELATIONAL_LTE_TO_LT);
    mut_config_enable_operator(config, MUT_RELATIONAL_EQ_TO_NEQ);
    mut_config_enable_operator(config, MUT_RELATIONAL_NEQ_TO_EQ);
    mut_config_enable_operator(config, MUT_LOGICAL_AND_TO_OR);
    mut_config_enable_operator(config, MUT_LOGICAL_OR_TO_AND);
    mut_config_enable_operator(config, MUT_CONDITIONAL_TRUE);
    mut_config_enable_operator(config, MUT_CONDITIONAL_FALSE);
    mut_config_enable_operator(config, MUT_NEGATE_CONDITIONAL);
    config->detect_equivalents = true;
    config->timeout_ms = 5000;
}

void mut_config_enable_operator(mut_config_t *config, mut_operator_t op) {
    if (config->enabled_count >= MUT_MAX_OPERATORS) return;
    int i;
    for (i = 0; i < config->enabled_count; i++) {
        if (config->enabled_operators[i] == op) return;
    }
    config->enabled_operators[config->enabled_count++] = op;
}

void mut_config_add_source(mut_config_t *config, const char *filepath) {
    if (config->source_count >= 32) return;
    strncpy(config->source_files[config->source_count++], filepath, MUT_NAME_MAX - 1);
}

const char* mut_operator_name(mut_operator_t op) {
    switch (op) {
        case MUT_ARITHMETIC_ADD_TO_SUB:      return "ADD_TO_SUB";
        case MUT_ARITHMETIC_SUB_TO_ADD:      return "SUB_TO_ADD";
        case MUT_ARITHMETIC_MUL_TO_DIV:      return "MUL_TO_DIV";
        case MUT_ARITHMETIC_DIV_TO_MUL:      return "DIV_TO_MUL";
        case MUT_ARITHMETIC_MOD_TO_MUL:      return "MOD_TO_MUL";
        case MUT_RELATIONAL_GT_TO_GTE:        return "GT_TO_GTE";
        case MUT_RELATIONAL_GTE_TO_GT:        return "GTE_TO_GT";
        case MUT_RELATIONAL_LT_TO_LTE:        return "LT_TO_LTE";
        case MUT_RELATIONAL_LTE_TO_LT:        return "LTE_TO_LT";
        case MUT_RELATIONAL_EQ_TO_NEQ:        return "EQ_TO_NEQ";
        case MUT_RELATIONAL_NEQ_TO_EQ:        return "NEQ_TO_EQ";
        case MUT_LOGICAL_AND_TO_OR:          return "AND_TO_OR";
        case MUT_LOGICAL_OR_TO_AND:          return "OR_TO_AND";
        case MUT_CONDITIONAL_TRUE:           return "CONDITIONAL_TRUE";
        case MUT_CONDITIONAL_FALSE:          return "CONDITIONAL_FALSE";
        case MUT_INCREMENT_TO_DECREMENT:     return "INC_TO_DEC";
        case MUT_DECREMENT_TO_INCREMENT:     return "DEC_TO_INC";
        case MUT_ASSIGNMENT_ADD_TO_SUB:      return "ASSIGN_ADD_TO_SUB";
        case MUT_ASSIGNMENT_SUB_TO_ADD:      return "ASSIGN_SUB_TO_ADD";
        case MUT_RETURN_REMOVE:              return "RETURN_REMOVE";
        case MUT_CONSTANT_ZERO:              return "CONSTANT_ZERO";
        case MUT_CONSTANT_ONE:               return "CONSTANT_ONE";
        case MUT_CONSTANT_NEG_ONE:           return "CONSTANT_NEG_ONE";
        case MUT_NEGATE_CONDITIONAL:         return "NEGATE_COND";
        case MUT_BITWISE_AND_TO_OR:          return "BIT_AND_TO_OR";
        case MUT_BITWISE_OR_TO_AND:          return "BIT_OR_TO_AND";
        case MUT_BITWISE_XOR_TO_AND:         return "BIT_XOR_TO_AND";
        case MUT_SHIFT_LEFT_TO_RIGHT:        return "SHIFT_L_TO_R";
        case MUT_SHIFT_RIGHT_TO_LEFT:        return "SHIFT_R_TO_L";
        case MUT_REMOVE_VOID_CALL:           return "REMOVE_VOID_CALL";
        case MUT_SWAP_BRANCH:                return "SWAP_BRANCH";
        default: return "UNKNOWN";
    }
}

const char* mut_operator_description(mut_operator_t op) {
    switch (op) {
        case MUT_ARITHMETIC_ADD_TO_SUB:      return "Replace + with -";
        case MUT_ARITHMETIC_SUB_TO_ADD:      return "Replace - with +";
        case MUT_ARITHMETIC_MUL_TO_DIV:      return "Replace * with /";
        case MUT_ARITHMETIC_DIV_TO_MUL:      return "Replace / with *";
        case MUT_ARITHMETIC_MOD_TO_MUL:      return "Replace % with *";
        case MUT_RELATIONAL_GT_TO_GTE:        return "Replace > with >=";
        case MUT_RELATIONAL_GTE_TO_GT:        return "Replace >= with >";
        case MUT_RELATIONAL_LT_TO_LTE:        return "Replace < with <=";
        case MUT_RELATIONAL_LTE_TO_LT:        return "Replace <= with <";
        case MUT_RELATIONAL_EQ_TO_NEQ:        return "Replace == with !=";
        case MUT_RELATIONAL_NEQ_TO_EQ:        return "Replace != with ==";
        case MUT_LOGICAL_AND_TO_OR:          return "Replace && with ||";
        case MUT_LOGICAL_OR_TO_AND:          return "Replace || with &&";
        case MUT_CONDITIONAL_TRUE:           return "Replace condition with true";
        case MUT_CONDITIONAL_FALSE:          return "Replace condition with false";
        case MUT_INCREMENT_TO_DECREMENT:     return "Replace ++ with --";
        case MUT_DECREMENT_TO_INCREMENT:     return "Replace -- with ++";
        case MUT_ASSIGNMENT_ADD_TO_SUB:      return "Replace += with -=";
        case MUT_ASSIGNMENT_SUB_TO_ADD:      return "Replace -= with +=";
        case MUT_RETURN_REMOVE:              return "Remove return statement";
        case MUT_CONSTANT_ZERO:              return "Replace constant with 0";
        case MUT_CONSTANT_ONE:               return "Replace constant with 1";
        case MUT_CONSTANT_NEG_ONE:           return "Replace constant with -1";
        case MUT_NEGATE_CONDITIONAL:         return "Negate condition (!)";
        case MUT_BITWISE_AND_TO_OR:          return "Replace & with |";
        case MUT_BITWISE_OR_TO_AND:          return "Replace | with &";
        case MUT_BITWISE_XOR_TO_AND:         return "Replace ^ with &";
        case MUT_SHIFT_LEFT_TO_RIGHT:        return "Replace << with >>";
        case MUT_SHIFT_RIGHT_TO_LEFT:        return "Replace >> with <<";
        case MUT_REMOVE_VOID_CALL:           return "Remove void function call";
        case MUT_SWAP_BRANCH:                return "Swap if/else branches";
        default: return "Unknown mutation";
    }
}

void mut_generate_mutants(mut_runner_t *runner) {
    if (!runner) return;
    runner->mutant_count = 0;

    int fi, si;
    for (fi = 0; fi < runner->config.source_count; fi++) {
        const char *fpath = runner->config.source_files[fi];
        FILE *fp = fopen(fpath, "r");
        if (!fp) {
            fprintf(stderr, "WARNING: cannot open source file '%s'\n", fpath);
            continue;
        }

        char line[MUT_LINE_MAX];
        int line_num = 0;
        runner->source_line_count = 0;

        while (fgets(line, MUT_LINE_MAX, fp) && runner->source_line_count < MUT_FILE_MAX) {
            if ((int)strlen(line) < MUT_LINE_MAX) {
                runner->source_lines[runner->source_line_count].number = ++line_num;
                strncpy(runner->source_lines[runner->source_line_count].content, line, MUT_LINE_MAX - 1);
                runner->source_line_count++;
            }
        }
        fclose(fp);

        for (si = 0; si < runner->source_line_count; si++) {
            mut_source_line_t *sl = &runner->source_lines[si];
            int oi;
            for (oi = 0; oi < runner->config.enabled_count; oi++) {
                mut_operator_t op = runner->config.enabled_operators[oi];
                if (runner->mutant_count >= MUT_MAX_MUTANTS) break;

                const char *pattern = NULL;
                const char *replacement = NULL;

                switch (op) {
                    case MUT_ARITHMETIC_ADD_TO_SUB:  pattern = " + "; replacement = " - "; break;
                    case MUT_ARITHMETIC_SUB_TO_ADD:  pattern = " - "; replacement = " + "; break;
                    case MUT_ARITHMETIC_MUL_TO_DIV:  pattern = " * "; replacement = " / "; break;
                    case MUT_ARITHMETIC_DIV_TO_MUL:  pattern = " / "; replacement = " * "; break;
                    case MUT_RELATIONAL_GT_TO_GTE:    pattern = " > ";  replacement = " >= "; break;
                    case MUT_RELATIONAL_GTE_TO_GT:    pattern = " >= "; replacement = " > "; break;
                    case MUT_RELATIONAL_LT_TO_LTE:    pattern = " < ";  replacement = " <= "; break;
                    case MUT_RELATIONAL_LTE_TO_LT:    pattern = " <= "; replacement = " < "; break;
                    case MUT_RELATIONAL_EQ_TO_NEQ:    pattern = " == "; replacement = " != "; break;
                    case MUT_RELATIONAL_NEQ_TO_EQ:    pattern = " != "; replacement = " == "; break;
                    case MUT_LOGICAL_AND_TO_OR:      pattern = " && "; replacement = " || "; break;
                    case MUT_LOGICAL_OR_TO_AND:      pattern = " || "; replacement = " && "; break;
                    default: continue;
                }

                if (pattern) {
                    char *pos = strstr(sl->content, pattern);
                    if (pos) {
                        mut_mutant_t *m = &runner->mutants[runner->mutant_count++];
                        memset(m, 0, sizeof(mut_mutant_t));
                        m->id = runner->mutant_count;
                        m->operator_type = op;
                        m->line_number = sl->number;
                        m->col_start = (int)(pos - sl->content) + 1;
                        m->col_end = m->col_start + (int)strlen(pattern);
                        strncpy(m->original_text, pattern, 127);
                        strncpy(m->mutated_text, replacement, 127);
                        snprintf(m->description, MUT_NAME_MAX,
                                 "Mutant #%d: %s at line %d [%s -> %s]",
                                 m->id, mut_operator_name(op), sl->number,
                                 pattern, replacement);
                    }
                }
            }
        }
    }
}

int mut_run_tests_on_mutant(mut_runner_t *runner, mut_mutant_t *mutant, int (*test_func)(void)) {
    if (!runner || !mutant || !test_func) return -1;
    runner->total_tests_run++;
    int result = test_func();
    if (result != 0) {
        mut_record_killed(runner, mutant->id);
        mutant->killed = true;
        return 1;
    }
    mutant->survived = true;
    return 0;
}

void mut_record_killed(mut_runner_t *runner, int mutant_id) {
    if (!runner) return;
    mut_mutant_t *m = mut_find_mutant(runner, mutant_id);
    if (!m || m->killed) return;
    m->killed = true;
    m->survived = false;
    runner->mutants_killed++;
}

void mut_record_survived(mut_runner_t *runner, int mutant_id) {
    if (!runner) return;
    mut_mutant_t *m = mut_find_mutant(runner, mutant_id);
    if (!m) return;
    m->survived = true;
    runner->mutants_survived++;
}

void mut_record_equivalent(mut_runner_t *runner, int mutant_id) {
    if (!runner) return;
    mut_mutant_t *m = mut_find_mutant(runner, mutant_id);
    if (!m) return;
    m->equivalent = true;
    m->killed = false;
    m->survived = false;
    runner->mutants_equivalent++;
}

double mut_calculate_score(mut_runner_t *runner) {
    if (!runner || runner->mutant_count <= runner->mutants_equivalent) return 0.0;
    int effective = runner->mutant_count - runner->mutants_equivalent;
    if (effective <= 0) return 0.0;
    double score = (double)runner->mutants_killed / (double)effective * 100.0;
    runner->mutation_score = score;
    return score;
}

void mut_report(mut_runner_t *runner) {
    if (!runner) return;
    double score = mut_calculate_score(runner);
    printf("\n========================================\n");
    printf("  Mutation Testing Report\n");
    printf("========================================\n");
    printf("  Total mutants generated: %d\n", runner->mutant_count);
    printf("  Mutants killed:          %d\n", runner->mutants_killed);
    printf("  Mutants survived:        %d\n", runner->mutants_survived);
    printf("  Equivalent mutants:      %d\n", runner->mutants_equivalent);
    printf("  Tests executed:          %d\n", runner->total_tests_run);
    printf("  ----------------------------------------\n");
    printf("  Mutation score:          %.2f%%\n", score);
    if (score >= 80.0) {
        printf("  Status: GOOD (>= 80%%)\n");
    } else if (score >= 60.0) {
        printf("  Status: ADEQUATE (>= 60%%)\n");
    } else {
        printf("  Status: INSUFFICIENT (< 60%%)\n");
    }
    printf("========================================\n");
}

void mut_report_detailed(mut_runner_t *runner) {
    if (!runner) return;
    mut_report(runner);
    printf("\n  Detailed Mutant List:\n");
    printf("  ----------------------------------------\n");
    int i;
    for (i = 0; i < runner->mutant_count; i++) {
        mut_mutant_t *m = &runner->mutants[i];
        const char *status = m->equivalent ? "EQUIV" :
                             m->killed     ? "KILLED" :
                             m->survived   ? "SURVIVED" : "PENDING";
        printf("  #%d [%s] %s | line %d | %s\n",
               m->id, status, mut_operator_name(m->operator_type),
               m->line_number, m->description);
    }
    printf("========================================\n");
}

mut_mutant_t* mut_find_mutant(mut_runner_t *runner, int id) {
    if (!runner) return NULL;
    int i;
    for (i = 0; i < runner->mutant_count; i++) {
        if (runner->mutants[i].id == id) return &runner->mutants[i];
    }
    return NULL;
}

bool mut_is_equivalent(int mutant_id) {
    mut_mutant_t *m = mut_find_mutant(&g_mut_runner, mutant_id);
    return m ? m->equivalent : false;
}
