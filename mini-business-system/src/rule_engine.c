#include "rule_engine.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

/* ============================================================
 * Rule Engine — Production rules, forward chaining
 * ============================================================ */

/* ---------- Fact ---------- */
void re_fact_init(re_fact *f, uint64_t id, const char *type) {
    f->id = id;
    strncpy(f->type, type, RE_MAX_NAME - 1);
    f->field_count = 0;
    f->timestamp = (int64_t)time(NULL);
}

int re_fact_set_field(re_fact *f, const char *field, int64_t value) {
    for (size_t i = 0; i < f->field_count; i++) {
        if (strcmp(f->fields[i], field) == 0) { f->values[i] = value; return 0; }
    }
    if (f->field_count >= RE_MAX_CONDITIONS) return -1;
    strncpy(f->fields[f->field_count], field, RE_MAX_FIELD - 1);
    f->values[f->field_count] = value;
    f->field_count++;
    return 0;
}

int64_t re_fact_get_field(const re_fact *f, const char *field, int64_t default_val) {
    for (size_t i = 0; i < f->field_count; i++) {
        if (strcmp(f->fields[i], field) == 0) return f->values[i];
    }
    return default_val;
}

/* ---------- Condition ---------- */
void re_condition_init(re_condition *c, const char *field, re_operator op, int64_t value) {
    strncpy(c->field, field, RE_MAX_FIELD - 1);
    c->op = op;
    c->value = value;
    memset(c->string_value, 0, RE_MAX_NAME);
}

int re_condition_evaluate(const re_condition *cond, const re_fact *fact) {
    int64_t fv = re_fact_get_field(fact, cond->field, INT64_MIN);
    if (fv == INT64_MIN) return 0;
    switch (cond->op) {
    case RE_OP_EQ:  return fv == cond->value;
    case RE_OP_NEQ: return fv != cond->value;
    case RE_OP_GT:  return fv >  cond->value;
    case RE_OP_GTE: return fv >= cond->value;
    case RE_OP_LT:  return fv <  cond->value;
    case RE_OP_LTE: return fv <= cond->value;
    case RE_OP_CONTAINS: return strstr(cond->string_value, cond->string_value) != NULL;
    default: return 0;
    }
}

/* ---------- Action ---------- */
void re_action_init_assert(re_action *a, const char *target) {
    a->type = RE_ACTION_ASSERT;
    strncpy(a->target, target, RE_MAX_NAME - 1);
}

void re_action_init_modify(re_action *a, const char *field, int64_t value) {
    a->type = RE_ACTION_MODIFY;
    strncpy(a->field, field, RE_MAX_FIELD - 1);
    a->value = value;
}

void re_action_init_print(re_action *a, const char *msg) {
    a->type = RE_ACTION_PRINT;
    strncpy(a->target, msg, RE_MAX_NAME - 1);
}

void re_action_init_retract(re_action *a, const char *target) {
    a->type = RE_ACTION_RETRACT;
    strncpy(a->target, target, RE_MAX_NAME - 1);
}

/* ---------- Rule ---------- */
void re_rule_init(re_rule *r, const char *name, int priority) {
    strncpy(r->name, name, RE_MAX_NAME - 1);
    r->priority = priority;
    r->salience = priority;
    r->condition_count = 0;
    r->action_count = 0;
    r->enabled = 1;
    r->version = 1;
    r->last_fired = 0;
}

void re_rule_add_condition(re_rule *r, const re_condition *cond) {
    if (r->condition_count >= RE_MAX_CONDITIONS) return;
    r->conditions[r->condition_count++] = *cond;
}

void re_rule_add_action(re_rule *r, const re_action *act) {
    if (r->action_count >= RE_MAX_ACTIONS) return;
    r->actions[r->action_count++] = *act;
}

int re_rule_evaluate(const re_rule *rule, const re_fact *facts, size_t fact_count) {
    if (!rule->enabled) return 0;
    if (rule->condition_count == 0) return 1;
    for (size_t c = 0; c < rule->condition_count; c++) {
        int matched = 0;
        for (size_t f = 0; f < fact_count; f++) {
            if (strcmp(facts[f].type, rule->conditions[c].field) == 0 ||
                strchr(rule->conditions[c].field, '.') != NULL) {
                if (re_condition_evaluate(&rule->conditions[c], &facts[f])) {
                    matched = 1; break;
                }
            }
        }
        /* also check if condition field matches any fact field directly */
        if (!matched) {
            for (size_t f = 0; f < fact_count; f++) {
                if (re_condition_evaluate(&rule->conditions[c], &facts[f])) {
                    matched = 1; break;
                }
            }
        }
        if (!matched) return 0;
    }
    return 1;
}

/* ---------- Decision Table ---------- */
void re_decision_table_init(re_decision_table *dt, const char *name) {
    strncpy(dt->name, name, RE_MAX_NAME - 1);
    dt->condition_count = 0;
    dt->row_count = 0;
    memset(dt->action_field, 0, RE_MAX_FIELD);
}

void re_decision_table_add_condition_col(re_decision_table *dt, const char *field) {
    if (dt->condition_count >= RE_MAX_DT_CONDITIONS) return;
    strncpy(dt->conditions[dt->condition_count++], field, RE_MAX_FIELD - 1);
}

int re_decision_table_add_row(re_decision_table *dt, const int64_t *cond_values, size_t n, int64_t action) {
    if (dt->row_count >= RE_MAX_DECISION_TABLE_ROWS || n > dt->condition_count) return -1;
    for (size_t i = 0; i < n; i++) dt->conditions_values[dt->row_count][i] = cond_values[i];
    dt->action_values[dt->row_count] = action;
    dt->row_count++;
    return 0;
}

int64_t re_decision_table_evaluate(const re_decision_table *dt, const re_fact *fact) {
    for (size_t r = 0; r < dt->row_count; r++) {
        int match = 1;
        for (size_t c = 0; c < dt->condition_count; c++) {
            int64_t fv = re_fact_get_field(fact, dt->conditions[c], INT64_MIN);
            if (fv != dt->conditions_values[r][c]) { match = 0; break; }
        }
        if (match) return dt->action_values[r];
    }
    return 0;
}

/* ---------- Rule Engine ---------- */
void re_engine_init(re_engine *eng) {
    eng->rule_count = 0;
    eng->fact_count = 0;
    eng->audit_count = 0;
    eng->next_fact_id = 1;
    eng->tick = 0;
}

int re_engine_add_rule(re_engine *eng, const re_rule *rule) {
    if (eng->rule_count >= RE_MAX_RULES) return -1;
    eng->rules[eng->rule_count++] = *rule;
    return 0;
}

int re_engine_add_fact(re_engine *eng, const re_fact *fact) {
    if (eng->fact_count >= RE_MAX_FACTS) return -1;
    eng->facts[eng->fact_count++] = *fact;
    return 0;
}

int re_engine_retract_fact(re_engine *eng, uint64_t fact_id) {
    for (size_t i = 0; i < eng->fact_count; i++) {
        if (eng->facts[i].id == fact_id) {
            memmove(&eng->facts[i], &eng->facts[i + 1],
                    (eng->fact_count - i - 1) * sizeof(re_fact));
            eng->fact_count--;
            return 0;
        }
    }
    return -1;
}

static int _rule_cmp(const void *a, const void *b) {
    const re_rule *ra = (const re_rule *)a;
    const re_rule *rb = (const re_rule *)b;
    return rb->salience - ra->salience;
}

int re_engine_run(re_engine *eng) {
    int fired;
    do { fired = re_engine_run_one(eng); } while (fired);
    return 0;
}

int re_engine_run_one(re_engine *eng) {
    /* sort by salience */
    qsort(eng->rules, eng->rule_count, sizeof(re_rule), _rule_cmp);
    for (size_t r = 0; r < eng->rule_count; r++) {
        if (!eng->rules[r].enabled) continue;
        if (re_rule_evaluate(&eng->rules[r], eng->facts, eng->fact_count)) {
            /* audit */
            if (eng->audit_count < RE_MAX_AUDIT) {
                re_audit_entry *ae = &eng->audit_trail[eng->audit_count++];
                ae->timestamp = (int64_t)time(NULL);
                strncpy(ae->rule_name, eng->rules[r].name, RE_MAX_NAME - 1);
                snprintf(ae->detail, sizeof(ae->detail), "Rule '%s' fired (salience=%d)",
                     eng->rules[r].name, eng->rules[r].salience);
                ae->fired = 1;
            }
            eng->rules[r].last_fired = ++eng->tick;
            eng->rules[r].enabled = 0; /* prevent re-fire in same cycle */
            /* execute actions */
            for (size_t a = 0; a < eng->rules[r].action_count; a++) {
                re_action *act = &eng->rules[r].actions[a];
                switch (act->type) {
                case RE_ACTION_PRINT:
                    printf("[RULE] %s: %s\n", eng->rules[r].name, act->target);
                    break;
                case RE_ACTION_MODIFY:
                    for (size_t f = 0; f < eng->fact_count; f++)
                        re_fact_set_field(&eng->facts[f], act->field, act->value);
                    break;
                case RE_ACTION_ASSERT:
                    {
                        re_fact nf;
                        re_fact_init(&nf, eng->next_fact_id++, act->target);
                        re_engine_add_fact(eng, &nf);
                    }
                    break;
                case RE_ACTION_RETRACT:
                    for (size_t f = 0; f < eng->fact_count; f++) {
                        if (strcmp(eng->facts[f].type, act->target) == 0) {
                            re_engine_retract_fact(eng, eng->facts[f].id);
                            break;
                        }
                    }
                    break;
                default: break;
                }
            }
            return 1;
        }
    }
    return 0;
}

const re_audit_entry *re_engine_get_audit(const re_engine *eng, size_t *count) {
    if (count) *count = eng->audit_count;
    return eng->audit_trail;
}

re_fact *re_engine_find_fact(re_engine *eng, uint64_t fact_id) {
    for (size_t i = 0; i < eng->fact_count; i++) {
        if (eng->facts[i].id == fact_id) return &eng->facts[i];
    }
    return NULL;
}

/* ---------- Rule Versioning ---------- */
void re_rule_versioning_init(re_rule_versioning *rv, const char *name) {
    strncpy(rv->rule_name, name, RE_MAX_NAME - 1);
    rv->version_count = 0;
    rv->active_version = 1;
}

void re_rule_versioning_add_version(re_rule_versioning *rv, int version) {
    if (rv->version_count >= 16) return;
    rv->versions[rv->version_count++] = version;
}
