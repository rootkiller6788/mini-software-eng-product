/* ============================================================
 * Demo 2: Approval Flow — Purchase Order Scenario
 *
 * Full approval workflow with:
 *  - Conditional routing (amount-based)
 *  - Delegate approval
 *  - Timeout → Auto-escalate
 *  - Decision table for vendor rating
 *  - State machine tracking PO states
 *  - Audit logging
 * ============================================================ */

#include "approval_process.h"
#include "rule_engine.h"
#include "state_machine.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

/* ---- PO State Machine ---- */
typedef struct {
    const char *po_id;
    int64_t     amount;
} po_context;

static void po_state_entry(void *ctx, const char *state) {
    po_context *po = (po_context *)ctx;
    printf("  [SM] PO %s => %s ($%lld)\n", po->po_id, state, (long long)po->amount);
}

static void on_draft(void *ctx)      { po_state_entry(ctx, "DRAFT"); }
static void on_pending(void *ctx)    { po_state_entry(ctx, "PENDING_APPROVAL"); }
static void on_approved(void *ctx)   { po_state_entry(ctx, "APPROVED"); }
static void on_rejected(void *ctx)   { po_state_entry(ctx, "REJECTED"); }
static void on_escalated(void *ctx)  { po_state_entry(ctx, "ESCALATED"); }
static void on_fulfilled(void *ctx)  { po_state_entry(ctx, "FULFILLED"); }

int main(void) {
    printf("========================================\n");
    printf("  Demo 2: Purchase Order Approval Flow\n");
    printf("========================================\n\n");

    /* ---- 1. Vendor Decision Table ---- */
    printf("=== 1. Vendor Decision Table ===\n");

    re_decision_table vendor_dt;
    re_decision_table_init(&vendor_dt, "VendorSelection");
    re_decision_table_add_condition_col(&vendor_dt, "category");
    re_decision_table_add_condition_col(&vendor_dt, "amount");

    /* Category: 1=IT, 2=Office, 3=Services
     * Action: vendor_id to use */
    int64_t r1[] = {1, 0};    re_decision_table_add_row(&vendor_dt, r1, 2, 101); /* IT small -> Vendor A */
    int64_t r2[] = {1, 5000}; re_decision_table_add_row(&vendor_dt, r2, 2, 102); /* IT large -> Vendor B */
    int64_t r3[] = {2, 0};    re_decision_table_add_row(&vendor_dt, r3, 2, 201); /* Office -> Vendor C */
    int64_t r4[] = {3, 0};    re_decision_table_add_row(&vendor_dt, r4, 2, 301); /* Services -> Vendor D */

    const char *vendor_names[] = {"", "IT-Vendor-A (Prime)", "IT-Vendor-B (Enterprise)",
                                   "", "Office-Vendor-C (Supplies)", "", "Services-Vendor-D (Consulting)"};

    re_fact po_fact;
    re_fact_init(&po_fact, 1, "PurchaseOrder");
    re_fact_set_field(&po_fact, "category", 1);
    re_fact_set_field(&po_fact, "amount", 12000);
    int64_t selected = re_decision_table_evaluate(&vendor_dt, &po_fact);
    printf("  PO Category=IT, Amount=$12000 -> Vendor ID=%lld (%s)\n",
           (long long)selected, vendor_names[selected > 200 ? 0 : selected > 100 ? (selected - 99) : selected]);

    /* ---- 2. Approval Engine Setup ---- */
    printf("\n=== 2. Approval Engine: PO-2024-00501 ===\n");

    ap_engine eng;
    ap_engine_init(&eng);

    /* ---- Build multi-level approval with conditions ---- */
    /* PO #1: IT equipment, $12000 - needs multiple levels */
    uint64_t po1_id = ap_engine_create(&eng, "PO-2024-00501: IT Servers");
    ap_approval_instance *po1 = ap_engine_get(&eng, po1_id);

    /* Level 0: Team Lead (always) */
    ap_approval_level l0;
    ap_approval_level_init(&l0, 0, AP_LEVEL_SEQUENTIAL);
    ap_approver a_tl;
    ap_approver_init(&a_tl, "Chen Wei", "Team Lead", 3000);
    a_tl.can_escalate = 1;
    ap_approval_level_add_approver(&l0, &a_tl);
    ap_approval_level_set_timeout(&l0, 5000); /* 5 sec for demo */

    /* Level 1: Department Manager */
    ap_approval_level l1;
    ap_approval_level_init(&l1, 1, AP_LEVEL_SEQUENTIAL);
    ap_approver a_mgr;
    ap_approver_init(&a_mgr, "Liu Yang", "Manager", 10000);
    ap_approval_level_add_approver(&l1, &a_mgr);
    ap_condition l1_cond;
    ap_condition_init(&l1_cond, AP_COND_AMOUNT_GT, 3000);
    ap_approval_level_add_condition(&l1, &l1_cond);

    /* Level 2: Director */
    ap_approval_level l2;
    ap_approval_level_init(&l2, 2, AP_LEVEL_SEQUENTIAL);
    ap_approver a_dir;
    ap_approver_init(&a_dir, "Zhao Ming", "Director", 50000);
    ap_approval_level_add_approver(&l2, &a_dir);
    ap_condition l2_cond;
    ap_condition_init(&l2_cond, AP_COND_AMOUNT_GT, 10000);
    ap_approval_level_add_condition(&l2, &l2_cond);

    /* Level 3: VP (parallel with CFO) */
    ap_approval_level l3;
    ap_approval_level_init(&l3, 3, AP_LEVEL_PARALLEL);
    l3.required_approvals = 2;
    ap_approver a_vp, a_cfo;
    ap_approver_init(&a_vp, "Sun Hong", "VP", 100000);
    ap_approver_init(&a_cfo, "Huang Li", "CFO", 200000);
    ap_approval_level_add_approver(&l3, &a_vp);
    ap_approval_level_add_approver(&l3, &a_cfo);
    ap_condition l3_cond;
    ap_condition_init(&l3_cond, AP_COND_AMOUNT_GT, 50000);
    ap_approval_level_add_condition(&l3, &l3_cond);
    l3.is_optional = 1;

    ap_approval_instance_add_level(po1, &l0);
    ap_approval_instance_add_level(po1, &l1);
    ap_approval_instance_add_level(po1, &l2);
    ap_approval_instance_add_level(po1, &l3);

    /* Submit */
    ap_engine_submit(&eng, po1_id, "employee_zhang", 12000, "IT");

    printf("  PO: %s\n", po1->title);
    printf("  Submitter: %s | Amount: $%lld | Dept: %s\n",
           po1->submitter, (long long)po1->amount, po1->department);
    printf("  Status: %s\n", ap_status_name(po1->status));
    printf("  Levels: %zu\n", po1->level_count);

    /* ---- 3. Approval Flow Simulation ---- */
    printf("\n=== 3. Approval Flow Simulation ===\n");

    /* Level 0: Team Lead approves */
    printf("--- Level 0: Team Lead ---\n");
    ap_approval_instance_approve(po1, "Chen Wei");
    printf("  After L0 approval -> Status: %s, Next Level: %d\n",
           ap_status_name(po1->status), po1->current_level);

    /* Level 1: Manager approves */
    printf("--- Level 1: Manager ---\n");
    ap_approval_instance_approve(po1, "Liu Yang");
    printf("  After L1 approval -> Status: %s, Next Level: %d\n",
           ap_status_name(po1->status), po1->current_level);

    /* Level 2: Director approves */
    printf("--- Level 2: Director ---\n");
    ap_approval_instance_approve(po1, "Zhao Ming");
    printf("  After L2 approval -> Status: %s, Next Level: %d\n",
           ap_status_name(po1->status), po1->current_level);

    /* Level 3 is optional (amount=12000 < 50000, skipped) */
    if (po1->levels[3].is_optional) {
        printf("--- Level 3 (VP+CFO): SKIPPED (amount < 50000) ---\n");
        po1->status = AP_STATUS_APPROVED;
        po1->completed_at = (int64_t)time(NULL);
    }

    printf("\n  Final Status: %s\n", ap_status_name(po1->status));

    /* ---- 4. Second PO: With Delegation ---- */
    printf("\n=== 4. Delegation Scenario ===\n");

    uint64_t po2_id = ap_engine_create(&eng, "PO-2024-00502: Office Supplies");
    ap_approval_instance *po2 = ap_engine_get(&eng, po2_id);

    ap_approval_level l0b;
    ap_approval_level_init(&l0b, 0, AP_LEVEL_SEQUENTIAL);
    ap_approver a_tl2, a_delegate;
    ap_approver_init(&a_tl2, "Wang Lei", "Team Lead", 5000);
    ap_approver_set_delegate(&a_tl2, "Zhang Min");
    ap_approval_level_add_approver(&l0b, &a_tl2);
    ap_approval_level_set_timeout(&l0b, 86400000);
    ap_approval_instance_add_level(po2, &l0b);

    ap_approval_level l1b;
    ap_approval_level_init(&l1b, 1, AP_LEVEL_ANY_ONE);
    ap_approver a_mgr2;
    ap_approver_init(&a_mgr2, "Xu Jing", "Manager", 20000);
    ap_approval_level_add_approver(&l1b, &a_mgr2);
    ap_approval_instance_add_level(po2, &l1b);

    ap_engine_submit(&eng, po2_id, "employee_li", 2500, "Office");

    printf("  PO: %s ($%lld)\n", po2->title, (long long)po2->amount);
    printf("  Delegation: %s -> %s\n", a_tl2.name, a_tl2.delegate_to);
    ap_approval_instance_delegate(po2, "Wang Lei", "Zhang Min");
    printf("  Status after delegation: %s\n", ap_status_name(po2->status));
    po2->status = AP_STATUS_IN_REVIEW; /* Put back in review for new delegate */
    ap_approval_instance_approve(po2, "Zhang Min");
    ap_approval_instance_approve(po2, "Xu Jing");
    po2->status = AP_STATUS_APPROVED;
    printf("  Final Status: %s\n", ap_status_name(po2->status));

    /* ---- 5. Timeout → Auto-Escalate ---- */
    printf("\n=== 5. Timeout & Auto-Escalate ===\n");

    uint64_t po3_id = ap_engine_create(&eng, "PO-2024-00503: Urgent Parts");
    ap_approval_instance *po3 = ap_engine_get(&eng, po3_id);

    ap_approval_level l0c;
    ap_approval_level_init(&l0c, 0, AP_LEVEL_SEQUENTIAL);
    ap_approver a_urgent;
    ap_approver_init(&a_urgent, "Slow Approver", "Manager", 10000);
    ap_approval_level_add_approver(&l0c, &a_urgent);
    ap_approval_level_set_timeout(&l0c, 1); /* 1ms - immediate timeout for demo */
    ap_approval_instance_add_level(po3, &l0c);

    ap_engine_submit(&eng, po3_id, "urgent_employee", 8000, "Manufacturing");
    printf("  PO: %s ($%lld), Timeout set: %lldms\n",
           po3->title, (long long)po3->amount, (long long)po3->levels[0].timeout_ms);

    printf("  Before timeout check -> Status: %s\n", ap_status_name(po3->status));
    int64_t future_time = po3->last_action_at + 100; /* 100ms later */
    ap_approval_instance_check_timeout(po3, future_time);
    printf("  After timeout check -> Status: %s (timed_out=%d)\n",
           ap_status_name(po3->status), po3->levels[0].timed_out);

    /* ---- 6. State Machine: PO Lifecycle ---- */
    printf("\n=== 6. State Machine: PO Lifecycle ===\n");

    po_context po_ctx = {"PO-2024-00777", 15000};
    sm_state_machine posm;
    sm_state_machine_init(&posm, &po_ctx);

    sm_state s_draft, s_pending, s_approved, s_rejected, s_escalated, s_fulfilled;
    sm_state_init(&s_draft,      10, "Draft",            SM_STATE_INITIAL);
    sm_state_init(&s_pending,    20, "PendingApproval",  SM_STATE_NORMAL);
    sm_state_init(&s_approved,   30, "Approved",         SM_STATE_NORMAL);
    sm_state_init(&s_rejected,   40, "Rejected",         SM_STATE_NORMAL);
    sm_state_init(&s_escalated,  50, "Escalated",        SM_STATE_NORMAL);
    sm_state_init(&s_fulfilled,  60, "Fulfilled",        SM_STATE_FINAL);

    sm_state_add_entry_action(&s_draft, on_draft, &po_ctx);
    sm_state_add_entry_action(&s_pending, on_pending, &po_ctx);
    sm_state_add_entry_action(&s_approved, on_approved, &po_ctx);
    sm_state_add_entry_action(&s_rejected, on_rejected, &po_ctx);
    sm_state_add_entry_action(&s_escalated, on_escalated, &po_ctx);
    sm_state_add_entry_action(&s_fulfilled, on_fulfilled, &po_ctx);

    sm_transition t_submit, t_approve, t_reject, t_escalate, t_fulfill, t_resubmit;

    sm_transition_init(&t_submit, "submit", 20);
    sm_transition_init(&t_approve, "approve", 30);
    sm_transition_init(&t_reject, "reject", 40);
    sm_transition_init(&t_escalate, "escalate", 50);
    sm_transition_init(&t_fulfill, "fulfill", 60);
    sm_transition_init(&t_resubmit, "resubmit", 20);

    sm_state_add_transition(&s_draft, &t_submit);
    sm_state_add_transition(&s_pending, &t_approve);
    sm_state_add_transition(&s_pending, &t_reject);
    sm_state_add_transition(&s_pending, &t_escalate);
    sm_state_add_transition(&s_approved, &t_fulfill);
    sm_state_add_transition(&s_rejected, &t_resubmit);
    sm_state_add_transition(&s_escalated, &t_approve);
    sm_state_add_transition(&s_escalated, &t_reject);

    sm_state_machine_add_state(&posm, &s_draft);
    sm_state_machine_add_state(&posm, &s_pending);
    sm_state_machine_add_state(&posm, &s_approved);
    sm_state_machine_add_state(&posm, &s_rejected);
    sm_state_machine_add_state(&posm, &s_escalated);
    sm_state_machine_add_state(&posm, &s_fulfilled);

    sm_state_machine_start(&posm, 10);

    const char *events[] = {"submit", "escalate", "approve", "fulfill"};
    for (int e = 0; e < 4; e++) {
        int result = sm_state_machine_send_event(&posm, events[e]);
        printf("  Event '%s' -> result=%d, state=%d (%s)\n",
               events[e], result,
               sm_state_machine_get_current(&posm),
               posm.active_states[0] ? posm.active_states[0]->name : "?");
    }

    /* ---- 7. Audit Summary ---- */
    printf("\n=== 7. Audit Summary ===\n");

    for (int p = 0; p < 3; p++) {
        uint64_t ids[] = {po1_id, po2_id, po3_id};
        ap_approval_instance *ai = ap_engine_get(&eng, ids[p]);
        if (!ai) continue;
        printf("  --- %s ---\n", ai->title);
        printf("  Status: %s | Amount: $%lld | Submitter: %s\n",
               ap_status_name(ai->status), (long long)ai->amount, ai->submitter);

        size_t ac = 0;
        const ap_audit_entry *ae = ap_approval_instance_get_audit(ai, &ac);
        for (size_t i = 0; i < ac; i++) {
            printf("    %zu. [%lld] %-10s by %-12s -> %s | %s\n",
                   i + 1, (long long)ae[i].timestamp,
                   ae[i].action, ae[i].actor,
                   ap_status_name(ae[i].new_status), ae[i].detail);
        }
    }

    /* ---- 8. Rule Engine: Post-Approve Processing ---- */
    printf("\n=== 8. Rule Engine: Post-Approve Processing ===\n");

    re_engine re;
    re_engine_init(&re);

    re_rule notify_rule;
    re_rule_init(&notify_rule, "NotifyVendor", 5);
    re_condition nc;
    re_condition_init(&nc, "status", RE_OP_EQ, 3); /* APPROVED */
    re_rule_add_condition(&notify_rule, &nc);
    re_action na;
    re_action_init_print(&na, "Dispatching order to vendor...");
    re_rule_add_action(&notify_rule, &na);
    re_engine_add_rule(&re, &notify_rule);

    re_rule budget_rule;
    re_rule_init(&budget_rule, "UpdateBudget", 3);
    re_condition bc;
    re_condition_init(&bc, "status", RE_OP_EQ, 3);
    re_rule_add_condition(&budget_rule, &bc);
    re_action ba;
    re_action_init_print(&ba, "Updating department budget...");
    re_rule_add_action(&budget_rule, &ba);
    re_engine_add_rule(&re, &budget_rule);

    re_fact nf;
    re_fact_init(&nf, 1, "Notification");
    re_fact_set_field(&nf, "status", 3); /* APPROVED */
    re_fact_set_field(&nf, "amount", 12000);
    re_engine_add_fact(&re, &nf);

    printf("  Running post-approval rules...\n");
    re_engine_run(&re);

    size_t ad_cnt = 0;
    const re_audit_entry *ad = re_engine_get_audit(&re, &ad_cnt);
    printf("  Rules fired: %zu\n", ad_cnt);

    printf("\n========================================\n");
    printf("  Demo 2 Complete\n");
    printf("  Summary:\n");
    printf("  - Vendor selection via decision table\n");
    printf("  - 3 approval instances processed\n");
    printf("  - Delegation demonstrated\n");
    printf("  - Timeout auto-escalation tested\n");
    printf("  - State machine lifecycle tracking\n");
    printf("  - Complete audit trail for all POs\n");
    printf("  - Post-approval rule processing\n");
    printf("========================================\n");

    return 0;
}
