#include "approval_process.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

/* ============================================================
 * Approval Process — Multi-level approval implementation
 * ============================================================ */

/* ---------- Status ---------- */
const char *ap_status_name(ap_status status) {
    switch (status) {
    case AP_STATUS_DRAFT:      return "Draft";
    case AP_STATUS_SUBMITTED:  return "Submitted";
    case AP_STATUS_IN_REVIEW:  return "In Review";
    case AP_STATUS_APPROVED:   return "Approved";
    case AP_STATUS_REJECTED:   return "Rejected";
    case AP_STATUS_ESCALATED:  return "Escalated";
    case AP_STATUS_DELEGATED:  return "Delegated";
    case AP_STATUS_CANCELLED:  return "Cancelled";
    case AP_STATUS_TIMED_OUT:  return "Timed Out";
    default:                   return "Unknown";
    }
}

/* ---------- Condition ---------- */
void ap_condition_init(ap_condition *c, ap_condition_type type, int64_t threshold) {
    c->type = type;
    c->threshold = threshold;
    c->string_value[0] = '\0';
    c->custom_fn = NULL;
}

int ap_condition_evaluate(const ap_condition *c, int64_t amount, const char *dept) {
    switch (c->type) {
    case AP_COND_AMOUNT_GT:  return amount > c->threshold;
    case AP_COND_AMOUNT_GTE: return amount >= c->threshold;
    case AP_COND_AMOUNT_LT:  return amount < c->threshold;
    case AP_COND_TYPE_EQUALS:return strcmp(c->string_value, dept ? dept : "") == 0;
    case AP_COND_DEPT_EQUALS:return strcmp(c->string_value, dept ? dept : "") == 0;
    case AP_COND_CUSTOM:     return c->custom_fn ? c->custom_fn(NULL, amount) : 0;
    default: return 0;
    }
}

/* ---------- Approver ---------- */
void ap_approver_init(ap_approver *a, const char *name, const char *role, int64_t max_amount) {
    strncpy(a->name, name, AP_MAX_NAME - 1);
    strncpy(a->role, role, AP_MAX_NAME - 1);
    a->max_approval_amount = max_amount;
    a->can_escalate = 1;
    a->delegate_to[0] = '\0';
}

void ap_approver_set_delegate(ap_approver *a, const char *delegate_name) {
    strncpy(a->delegate_to, delegate_name, AP_MAX_NAME - 1);
}

/* ---------- Approval Level ---------- */
void ap_approval_level_init(ap_approval_level *l, int level, ap_level_type type) {
    l->level = level;
    l->type = type;
    l->approver_count = 0;
    l->condition_count = 0;
    l->required_approvals = 1;
    l->is_optional = 0;
    l->completed = 0;
    l->result = 0;
    l->timeout_ms = 0;
    l->timed_out = 0;
}

void ap_approval_level_add_approver(ap_approval_level *l, const ap_approver *a) {
    if (l->approver_count >= AP_MAX_APPROVERS) return;
    l->approvers[l->approver_count++] = *a;
}

void ap_approval_level_add_condition(ap_approval_level *l, const ap_condition *c) {
    if (l->condition_count >= AP_MAX_CONDITIONS) return;
    l->conditions[l->condition_count++] = *c;
}

void ap_approval_level_set_timeout(ap_approval_level *l, int64_t ms) {
    l->timeout_ms = ms;
}

/* ---------- Approval Instance ---------- */
void ap_approval_instance_init(ap_approval_instance *ai, uint64_t id, const char *title) {
    ai->id = id;
    strncpy(ai->title, title, AP_MAX_NAME - 1);
    ai->submitter[0] = '\0';
    ai->amount = 0;
    ai->department[0] = '\0';
    ai->status = AP_STATUS_DRAFT;
    ai->level_count = 0;
    ai->current_level = 0;
    ai->escalation_reason[0] = '\0';
    ai->submitted_at = 0;
    ai->last_action_at = 0;
    ai->completed_at = 0;
    ai->audit_count = 0;
}

void ap_approval_instance_add_level(ap_approval_instance *ai, const ap_approval_level *level) {
    if (ai->level_count >= AP_MAX_LEVELS) return;
    ai->levels[ai->level_count++] = *level;
}

int ap_approval_instance_submit(ap_approval_instance *ai, const char *submitter) {
    if (ai->status != AP_STATUS_DRAFT) return -1;
    strncpy(ai->submitter, submitter, AP_MAX_NAME - 1);
    ai->status = AP_STATUS_SUBMITTED;
    ai->submitted_at = (int64_t)time(NULL);
    ai->last_action_at = ai->submitted_at;
    ap_approval_instance_add_audit(ai, "SUBMIT", submitter, "Application submitted", AP_STATUS_SUBMITTED);
    return 0;
}

int ap_approval_instance_review(ap_approval_instance *ai) {
    if (ai->status != AP_STATUS_SUBMITTED) return -1;
    ai->status = AP_STATUS_IN_REVIEW;
    ai->current_level = 0;
    ai->last_action_at = (int64_t)time(NULL);
    ap_approval_instance_add_audit(ai, "REVIEW", "system",
                                   "Review started - Level 0", AP_STATUS_IN_REVIEW);
    return 0;
}

int ap_approval_instance_approve(ap_approval_instance *ai, const char *approver) {
    if (ai->status != AP_STATUS_IN_REVIEW) return -1;
    int cl = ai->current_level;
    if (cl < 0 || (size_t)cl >= ai->level_count) return -1;

    ai->levels[cl].completed = 1;
    ai->levels[cl].result = 'A';
    ai->last_action_at = (int64_t)time(NULL);

    char detail[128];
    snprintf(detail, sizeof(detail), "Approved by %s at level %d", approver, cl);
    ap_approval_instance_add_audit(ai, "APPROVE", approver, detail, ai->status);

    /* advance to next level */
    ai->current_level++;
    if ((size_t)ai->current_level >= ai->level_count) {
        ai->status = AP_STATUS_APPROVED;
        ai->completed_at = (int64_t)time(NULL);
        ap_approval_instance_add_audit(ai, "COMPLETE", "system",
                                       "All levels approved", AP_STATUS_APPROVED);
    }
    return 0;
}

int ap_approval_instance_reject(ap_approval_instance *ai, const char *approver, const char *reason) {
    if (ai->status != AP_STATUS_IN_REVIEW) return -1;
    ai->status = AP_STATUS_REJECTED;
    ai->completed_at = (int64_t)time(NULL);
    ai->last_action_at = ai->completed_at;
    char detail[256];
    snprintf(detail, sizeof(detail), "Rejected by %s: %s", approver,
             reason ? reason : "(no reason)");
    ap_approval_instance_add_audit(ai, "REJECT", approver, detail, AP_STATUS_REJECTED);
    return 0;
}

int ap_approval_instance_escalate(ap_approval_instance *ai, const char *actor, const char *reason) {
    if (ai->status != AP_STATUS_IN_REVIEW && ai->status != AP_STATUS_SUBMITTED) return -1;
    ai->status = AP_STATUS_ESCALATED;
    if (reason) strncpy(ai->escalation_reason, reason, AP_MAX_NAME - 1);
    ai->last_action_at = (int64_t)time(NULL);
    char detail[256];
    snprintf(detail, sizeof(detail), "Escalated by %s: %s", actor,
             reason ? reason : "(no reason)");
    ap_approval_instance_add_audit(ai, "ESCALATE", actor, detail, AP_STATUS_ESCALATED);
    return 0;
}

int ap_approval_instance_delegate(ap_approval_instance *ai, const char *from, const char *to) {
    if (ai->status != AP_STATUS_IN_REVIEW) return -1;
    char detail[256];
    snprintf(detail, sizeof(detail), "Delegated from %s to %s", from, to);
    ap_approval_instance_add_audit(ai, "DELEGATE", from, detail, AP_STATUS_DELEGATED);

    /* update current level approver */
    int cl = ai->current_level;
    if (cl >= 0 && (size_t)cl < ai->level_count) {
        for (size_t a = 0; a < ai->levels[cl].approver_count; a++) {
            if (strcmp(ai->levels[cl].approvers[a].name, from) == 0) {
                strncpy(ai->levels[cl].approvers[a].name, to, AP_MAX_NAME - 1);
                break;
            }
        }
    }
    return 0;
}

int ap_approval_instance_check_timeout(ap_approval_instance *ai, int64_t now_ms) {
    if (ai->status != AP_STATUS_IN_REVIEW) return 0;
    int cl = ai->current_level;
    if (cl < 0 || (size_t)cl >= ai->level_count) return 0;
    if (ai->levels[cl].timeout_ms <= 0) return 0;
    if ((now_ms - ai->last_action_at) >= ai->levels[cl].timeout_ms) {
        ai->levels[cl].timed_out = 1;
        ai->status = AP_STATUS_TIMED_OUT;
        ap_approval_instance_add_audit(ai, "TIMEOUT", "system",
                                       "Auto-escalated due to timeout", AP_STATUS_TIMED_OUT);
        return 1;
    }
    return 0;
}

void ap_approval_instance_add_audit(ap_approval_instance *ai, const char *action,
                                     const char *actor, const char *detail,
                                     ap_status new_status) {
    if (ai->audit_count >= AP_MAX_AUDIT) return;
    ap_audit_entry *ae = &ai->audit_log[ai->audit_count++];
    ae->timestamp = (int64_t)time(NULL);
    ae->level = ai->current_level;
    strncpy(ae->action, action, AP_MAX_NAME - 1);
    strncpy(ae->actor, actor, AP_MAX_NAME - 1);
    strncpy(ae->detail, detail, AP_MAX_NAME * 2 - 1);
    ae->new_status = new_status;
}

const ap_audit_entry *ap_approval_instance_get_audit(const ap_approval_instance *ai, size_t *count) {
    if (count) *count = ai->audit_count;
    return ai->audit_log;
}

/* ---------- Approval Engine ---------- */
void ap_engine_init(ap_engine *eng) {
    eng->instance_count = 0;
    eng->next_id = 1;
}

uint64_t ap_engine_create(ap_engine *eng, const char *title) {
    if (eng->instance_count >= AP_MAX_INSTANCES) return 0;
    uint64_t id = eng->next_id++;
    ap_approval_instance_init(&eng->instances[eng->instance_count], id, title);
    eng->instance_count++;
    return id;
}

ap_approval_instance *ap_engine_get(ap_engine *eng, uint64_t id) {
    for (size_t i = 0; i < eng->instance_count; i++) {
        if (eng->instances[i].id == id) return &eng->instances[i];
    }
    return NULL;
}

int ap_engine_submit(ap_engine *eng, uint64_t id, const char *submitter, int64_t amount, const char *dept) {
    ap_approval_instance *ai = ap_engine_get(eng, id);
    if (!ai) return -1;
    ai->amount = amount;
    if (dept) strncpy(ai->department, dept, AP_MAX_NAME - 1);
    ap_approval_instance_submit(ai, submitter);
    ap_approval_instance_review(ai);
    return 0;
}
