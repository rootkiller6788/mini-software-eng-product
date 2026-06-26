#ifndef RULE_ENGINE_H
#define RULE_ENGINE_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================
 * Rule Engine — Production rules, forward chaining (C99)
 * ============================================================ */

#define RE_MAX_NAME       64
#define RE_MAX_RULES      128
#define RE_MAX_CONDITIONS 16
#define RE_MAX_ACTIONS    16
#define RE_MAX_FACTS      128
#define RE_MAX_FIELD      32
#define RE_MAX_AUDIT      512
#define RE_MAX_DECISION_TABLE_ROWS 64
#define RE_MAX_DT_CONDITIONS 8

/* ---------- Fact ---------- */
typedef struct {
    uint64_t id;
    char     type[RE_MAX_NAME];
    char     fields[RE_MAX_CONDITIONS][RE_MAX_FIELD];
    int64_t  values[RE_MAX_CONDITIONS];
    size_t   field_count;
    int64_t  timestamp;
} re_fact;

void re_fact_init(re_fact *f, uint64_t id, const char *type);
int  re_fact_set_field(re_fact *f, const char *field, int64_t value);
int64_t re_fact_get_field(const re_fact *f, const char *field, int64_t default_val);

/* ---------- Condition ---------- */
typedef enum {
    RE_OP_EQ,    /* == */
    RE_OP_NEQ,   /* != */
    RE_OP_GT,    /* >  */
    RE_OP_GTE,   /* >= */
    RE_OP_LT,    /* <  */
    RE_OP_LTE,   /* <= */
    RE_OP_CONTAINS,
    RE_OP_MATCHES
} re_operator;

typedef struct {
    char        field[RE_MAX_FIELD];
    re_operator op;
    int64_t     value;
    char        string_value[RE_MAX_NAME]; /* for contains/matches */
} re_condition;

void re_condition_init(re_condition *c, const char *field, re_operator op, int64_t value);
int  re_condition_evaluate(const re_condition *cond, const re_fact *fact);

/* ---------- Action ---------- */
typedef enum {
    RE_ACTION_ASSERT,
    RE_ACTION_RETRACT,
    RE_ACTION_MODIFY,
    RE_ACTION_PRINT,
    RE_ACTION_CALLBACK
} re_action_type;

typedef struct {
    re_action_type type;
    char           target[RE_MAX_NAME];
    char           field[RE_MAX_FIELD];
    int64_t        value;
    void         (*callback)(void *ctx);
} re_action;

void re_action_init_assert(re_action *a, const char *target);
void re_action_init_modify(re_action *a, const char *field, int64_t value);
void re_action_init_print(re_action *a, const char *msg);
void re_action_init_retract(re_action *a, const char *target);

/* ---------- Rule ---------- */
typedef struct {
    char          name[RE_MAX_NAME];
    int           priority;     /* higher = fires first */
    int           salience;
    re_condition  conditions[RE_MAX_CONDITIONS];
    size_t        condition_count;
    re_action     actions[RE_MAX_ACTIONS];
    size_t        action_count;
    int           enabled;
    int           version;
    int64_t       last_fired;
} re_rule;

void re_rule_init(re_rule *r, const char *name, int priority);
void re_rule_add_condition(re_rule *r, const re_condition *cond);
void re_rule_add_action(re_rule *r, const re_action *act);
int  re_rule_evaluate(const re_rule *rule, const re_fact *facts, size_t fact_count);

/* ---------- Decision Table ---------- */
typedef struct {
    char     name[RE_MAX_NAME];
    char     conditions[RE_MAX_DT_CONDITIONS][RE_MAX_FIELD];
    size_t   condition_count;
    char     action_field[RE_MAX_FIELD];
    int64_t  conditions_values[RE_MAX_DECISION_TABLE_ROWS][RE_MAX_DT_CONDITIONS];
    int64_t  action_values[RE_MAX_DECISION_TABLE_ROWS];
    size_t   row_count;
} re_decision_table;

void re_decision_table_init(re_decision_table *dt, const char *name);
void re_decision_table_add_condition_col(re_decision_table *dt, const char *field);
int  re_decision_table_add_row(re_decision_table *dt, const int64_t *cond_values, size_t n, int64_t action);
int64_t re_decision_table_evaluate(const re_decision_table *dt, const re_fact *fact);

/* ---------- Audit Entry ---------- */
typedef struct {
    int64_t  timestamp;
    char     rule_name[RE_MAX_NAME];
    char     detail[RE_MAX_NAME * 4];
    int      fired;
} re_audit_entry;

/* ---------- Rule Engine (Rete-like) ---------- */
typedef struct {
    re_rule          rules[RE_MAX_RULES];
    size_t           rule_count;
    re_fact          facts[RE_MAX_FACTS];
    size_t           fact_count;
    re_audit_entry   audit_trail[RE_MAX_AUDIT];
    size_t           audit_count;
    uint64_t         next_fact_id;
    int64_t          tick;
} re_engine;

void re_engine_init(re_engine *eng);
int  re_engine_add_rule(re_engine *eng, const re_rule *rule);
int  re_engine_add_fact(re_engine *eng, const re_fact *fact);
int  re_engine_retract_fact(re_engine *eng, uint64_t fact_id);
int  re_engine_run(re_engine *eng);
int  re_engine_run_one(re_engine *eng);
const re_audit_entry *re_engine_get_audit(const re_engine *eng, size_t *count);
re_fact *re_engine_find_fact(re_engine *eng, uint64_t fact_id);

/* ---------- Rule Versioning ---------- */
typedef struct {
    char     rule_name[RE_MAX_NAME];
    int      versions[16];
    size_t   version_count;
    int      active_version;
} re_rule_versioning;

void re_rule_versioning_init(re_rule_versioning *rv, const char *name);
void re_rule_versioning_add_version(re_rule_versioning *rv, int version);

#ifdef __cplusplus
}
#endif

#endif /* RULE_ENGINE_H */
