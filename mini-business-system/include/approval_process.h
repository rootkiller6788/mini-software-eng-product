#ifndef APPROVAL_PROCESS_H
#define APPROVAL_PROCESS_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================
 * Approval Process — Multi-level approval workflow (C99)
 * ============================================================ */

#define AP_MAX_NAME      64
#define AP_MAX_LEVELS    16
#define AP_MAX_INSTANCES 128
#define AP_MAX_AUDIT     512
#define AP_MAX_APPROVERS 16
#define AP_MAX_CONDITIONS 4

/* ---------- Approval Status ---------- */
typedef enum {
    AP_STATUS_DRAFT,
    AP_STATUS_SUBMITTED,
    AP_STATUS_IN_REVIEW,
    AP_STATUS_APPROVED,
    AP_STATUS_REJECTED,
    AP_STATUS_ESCALATED,
    AP_STATUS_DELEGATED,
    AP_STATUS_CANCELLED,
    AP_STATUS_TIMED_OUT
} ap_status;

const char *ap_status_name(ap_status status);

/* ---------- Approval Condition ---------- */
typedef enum {
    AP_COND_AMOUNT_GT,
    AP_COND_AMOUNT_GTE,
    AP_COND_AMOUNT_LT,
    AP_COND_TYPE_EQUALS,
    AP_COND_DEPT_EQUALS,
    AP_COND_CUSTOM
} ap_condition_type;

typedef struct {
    ap_condition_type type;
    int64_t           threshold;
    char              string_value[AP_MAX_NAME];
    int               (*custom_fn)(void *ctx, int64_t amount);
} ap_condition;

void ap_condition_init(ap_condition *c, ap_condition_type type, int64_t threshold);
int  ap_condition_evaluate(const ap_condition *c, int64_t amount, const char *dept);

/* ---------- Approver ---------- */
typedef struct {
    char     name[AP_MAX_NAME];
    char     role[AP_MAX_NAME];
    int64_t  max_approval_amount;
    int      can_escalate;
    char     delegate_to[AP_MAX_NAME]; /* empty = no delegate */
} ap_approver;

void ap_approver_init(ap_approver *a, const char *name, const char *role, int64_t max_amount);
void ap_approver_set_delegate(ap_approver *a, const char *delegate_name);

/* ---------- Approval Level ---------- */
typedef enum {
    AP_LEVEL_SEQUENTIAL,  /* one after another */
    AP_LEVEL_PARALLEL,    /* all must approve */
    AP_LEVEL_DYNAMIC,     /* based on conditions */
    AP_LEVEL_ANY_ONE      /* any one approver suffices */
} ap_level_type;

typedef struct {
    int           level;
    ap_level_type type;
    ap_approver   approvers[AP_MAX_APPROVERS];
    size_t        approver_count;
    ap_condition  conditions[AP_MAX_CONDITIONS];
    size_t        condition_count;
    int           required_approvals; /* for parallel: how many needed */
    int           is_optional;        /* dynamic level may skip */
    int           completed;
    char          result;             /* 'A'=approved, 'R'=rejected */
    int64_t       timeout_ms;
    int           timed_out;
} ap_approval_level;

void ap_approval_level_init(ap_approval_level *l, int level, ap_level_type type);
void ap_approval_level_add_approver(ap_approval_level *l, const ap_approver *a);
void ap_approval_level_add_condition(ap_approval_level *l, const ap_condition *c);
void ap_approval_level_set_timeout(ap_approval_level *l, int64_t ms);

/* ---------- Audit Log Entry ---------- */
typedef struct {
    int64_t  timestamp;
    int      level;
    char     action[AP_MAX_NAME];
    char     actor[AP_MAX_NAME];
    char     detail[AP_MAX_NAME * 2];
    ap_status new_status;
} ap_audit_entry;

/* ---------- Approval Instance ---------- */
typedef struct {
    uint64_t           id;
    char               title[AP_MAX_NAME];
    char               submitter[AP_MAX_NAME];
    int64_t            amount;
    char               department[AP_MAX_NAME];
    ap_status          status;
    ap_approval_level  levels[AP_MAX_LEVELS];
    size_t             level_count;
    int                current_level;
    char               escalation_reason[AP_MAX_NAME];
    int64_t            submitted_at;
    int64_t            last_action_at;
    int64_t            completed_at;
    ap_audit_entry     audit_log[AP_MAX_AUDIT];
    size_t             audit_count;
} ap_approval_instance;

void ap_approval_instance_init(ap_approval_instance *ai, uint64_t id, const char *title);
void ap_approval_instance_add_level(ap_approval_instance *ai, const ap_approval_level *level);
int  ap_approval_instance_submit(ap_approval_instance *ai, const char *submitter);
int  ap_approval_instance_review(ap_approval_instance *ai);
int  ap_approval_instance_approve(ap_approval_instance *ai, const char *approver);
int  ap_approval_instance_reject(ap_approval_instance *ai, const char *approver, const char *reason);
int  ap_approval_instance_escalate(ap_approval_instance *ai, const char *actor, const char *reason);
int  ap_approval_instance_delegate(ap_approval_instance *ai, const char *from, const char *to);
int  ap_approval_instance_check_timeout(ap_approval_instance *ai, int64_t now_ms);
void ap_approval_instance_add_audit(ap_approval_instance *ai, const char *action, const char *actor, const char *detail, ap_status new_status);
const ap_audit_entry *ap_approval_instance_get_audit(const ap_approval_instance *ai, size_t *count);

/* ---------- Approval Engine ---------- */
typedef struct {
    ap_approval_instance instances[AP_MAX_INSTANCES];
    size_t               instance_count;
    uint64_t             next_id;
} ap_engine;

void ap_engine_init(ap_engine *eng);
uint64_t ap_engine_create(ap_engine *eng, const char *title);
ap_approval_instance *ap_engine_get(ap_engine *eng, uint64_t id);
int  ap_engine_submit(ap_engine *eng, uint64_t id, const char *submitter, int64_t amount, const char *dept);

#ifdef __cplusplus
}
#endif

#endif /* APPROVAL_PROCESS_H */
