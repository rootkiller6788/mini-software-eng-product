/* ============================================================
 * Demo 1: Full Business System — Expense Reimbursement
 *
 * Integrates domain model + workflow engine + rule engine +
 * state machine + approval process into a complete scenario.
 *
 * Scenario:
 *   Employee submits expense report. System uses:
 *   - Domain model: bounded contexts for Finance and HR
 *   - Rule engine: determines if expense needs manager approval
 *   - State machine: tracks expense report lifecycle
 *   - Workflow engine: orchestrates the process steps
 *   - Approval process: multi-level approval workflow
 * ============================================================ */

#include "domain_model.h"
#include "workflow_engine.h"
#include "rule_engine.h"
#include "state_machine.h"
#include "approval_process.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

/* ---- State Machine: Expense Report Lifecycle ---- */
typedef struct {
    const char *name;
    int64_t     amount;
} expense_report_ctx;

static void on_draft_entry(void *ctx) {
    expense_report_ctx *r = (expense_report_ctx *)ctx;
    printf("  [SM] Entered DRAFT state for '%s' ($%lld)\n",
           r->name, (long long)r->amount);
}
static void on_submitted_entry(void *ctx) {
    printf("  [SM] Entered SUBMITTED state\n");
}
static void on_approved_entry(void *ctx) {
    printf("  [SM] Entered APPROVED state\n");
}
static void on_rejected_entry(void *ctx) {
    printf("  [SM] Entered REJECTED state\n");
}
static void on_paid_entry(void *ctx) {
    printf("  [SM] Entered PAID state (final)\n");
}

static int amount_gt_1000(void *ctx) {
    expense_report_ctx *r = (expense_report_ctx *)ctx;
    return r->amount > 1000;
}
static int amount_le_1000(void *ctx) {
    expense_report_ctx *r = (expense_report_ctx *)ctx;
    return r->amount <= 1000;
}

static void transition_log(void *ctx, int from, int to) {
    (void)ctx;
    printf("  [SM] Transition %d -> %d\n", from, to);
}

/* ---- Demo Main ---- */
int main(void) {
    printf("========================================\n");
    printf("  Demo 1: Full Business System\n");
    printf("  Expense Reimbursement Scenario\n");
    printf("========================================\n\n");

    /* ========== 1. Domain Modeling ========== */
    printf("=== 1. Domain Modeling ===\n");

    dm_bounded_context finance_ctx;
    dm_bounded_context_init(&finance_ctx, "Finance");

    dm_glossary glossary;
    dm_glossary_init(&glossary);
    dm_glossary_add(&glossary, "ExpenseReport", "A report of business expenses for reimbursement");
    dm_glossary_add(&glossary, "Approver", "A person authorized to approve expense reports");
    dm_glossary_add(&glossary, "Receipt", "Proof of expense incurred by an employee");
    finance_ctx.glossary = glossary;

    char expense_data[256] = "Client dinner - $850";
    dm_aggregate_root expense_ar;
    dm_aggregate_root_init(&expense_ar, 3001, "ExpenseReport", expense_data, strlen(expense_data));
    dm_bounded_context_add_aggregate(&finance_ctx, &expense_ar);

    dm_context_map fm_to_hr;
    dm_context_map_init(&fm_to_hr, "Finance-HR", "Finance", "HR", DM_MAP_PARTNERSHIP);
    dm_bounded_context_add_map(&finance_ctx, &fm_to_hr);

    printf("  Bounded Context: %s\n", finance_ctx.name);
    printf("  Glossary terms: %zu\n", finance_ctx.glossary.count);
    printf("  Aggregates: %zu\n", finance_ctx.aggregate_count);
    printf("  Context Maps: %zu (type: %s)\n",
           finance_ctx.mapping_count,
           dm_context_map_type_name(finance_ctx.mappings[0].map_type));

    /* ========== 2. Rule Engine ========== */
    printf("\n=== 2. Rule Engine: Approval Rules ===\n");

    re_engine rule_eng;
    re_engine_init(&rule_eng);

    /* Rule: Amount > 5000 => needs director + CFO */
    re_rule large_rule;
    re_rule_init(&large_rule, "LargeExpenseRule", 10);
    re_condition lc;
    re_condition_init(&lc, "amount", RE_OP_GT, 5000);
    re_rule_add_condition(&large_rule, &lc);
    re_action la;
    re_action_init_print(&la, "LARGE EXPENSE: Requires Director + CFO approval");
    re_rule_add_action(&large_rule, &la);
    re_action lam;
    re_action_init_modify(&lam, "approval_level", 3);
    re_rule_add_action(&large_rule, &lam);
    re_engine_add_rule(&rule_eng, &large_rule);

    /* Rule: Amount > 1000 => needs manager */
    re_rule medium_rule;
    re_rule_init(&medium_rule, "MediumExpenseRule", 8);
    re_condition mc;
    re_condition_init(&mc, "amount", RE_OP_GT, 1000);
    re_rule_add_condition(&medium_rule, &mc);
    re_action ma;
    re_action_init_print(&ma, "MEDIUM EXPENSE: Requires Manager approval");
    re_rule_add_action(&medium_rule, &ma);
    re_action mam;
    re_action_init_modify(&mam, "approval_level", 2);
    re_rule_add_action(&medium_rule, &mam);
    re_engine_add_rule(&rule_eng, &medium_rule);

    /* Rule: Amount <= 1000 => auto approve */
    re_rule small_rule;
    re_rule_init(&small_rule, "SmallExpenseRule", 5);
    re_condition sc;
    re_condition_init(&sc, "amount", RE_OP_LTE, 1000);
    re_rule_add_condition(&small_rule, &sc);
    re_action sa;
    re_action_init_print(&sa, "SMALL EXPENSE: Auto-approved");
    re_rule_add_action(&small_rule, &sa);
    re_action sam;
    re_action_init_modify(&sam, "approval_level", 1);
    re_rule_add_action(&small_rule, &sam);
    re_engine_add_rule(&rule_eng, &small_rule);

    /* Test with different amounts */
    int64_t test_amounts[] = {850, 2500, 12000};
    for (int ti = 0; ti < 3; ti++) {
        int64_t amt = test_amounts[ti];
        printf("\n  Testing amount: $%lld\n", (long long)amt);
        re_fact fact;
        re_fact_init(&fact, (uint64_t)(10 + ti), "Expense");
        re_fact_set_field(&fact, "amount", amt);
        re_engine_add_fact(&rule_eng, &fact);
        /* Reset rules */
        for (size_t i = 0; i < rule_eng.rule_count; i++) rule_eng.rules[i].enabled = 1;
        re_engine_run(&rule_eng);
        int64_t level = re_fact_get_field(&fact, "approval_level", 0);
        printf("    => Required approval level: %lld\n", (long long)level);
        re_engine_retract_fact(&rule_eng, fact.id);
    }

    /* ========== 3. State Machine ========== */
    printf("\n=== 3. State Machine: Expense Lifecycle ===\n");

    expense_report_ctx report = {"Team Lunch", 850};
    sm_state_machine sm;
    sm_state_machine_init(&sm, &report);

    /* Draft */
    sm_state draft;
    sm_state_init(&draft, 1, "Draft", SM_STATE_INITIAL);
    sm_state_add_entry_action(&draft, on_draft_entry, &report);
    sm_transition t1;
    sm_transition_init(&t1, "submit", 2);
    sm_state_add_transition(&draft, &t1);
    sm_state_machine_add_state(&sm, &draft);

    /* Submitted */
    sm_state submitted;
    sm_state_init(&submitted, 2, "Submitted", SM_STATE_NORMAL);
    sm_state_add_entry_action(&submitted, on_submitted_entry, &report);
    sm_transition t2a, t2b;
    sm_transition_init(&t2a, "approve", 3);
    sm_transition_init(&t2b, "reject", 4);
    sm_transition_set_guard(&t2a, amount_le_1000, &report);
    sm_transition_set_guard(&t2b, amount_gt_1000, &report);
    sm_state_add_transition(&submitted, &t2a);
    sm_transition t2c; /* extra transition for reject path */
    sm_transition_init(&t2c, "reject", 4);
    sm_state_add_transition(&submitted, &t2c);
    sm_state_machine_add_state(&sm, &submitted);

    /* Approved */
    sm_state approved;
    sm_state_init(&approved, 3, "Approved", SM_STATE_NORMAL);
    sm_state_add_entry_action(&approved, on_approved_entry, &report);
    sm_transition t3;
    sm_transition_init(&t3, "pay", 5);
    sm_transition_set_action(&t3, transition_log, &report);
    sm_state_add_transition(&approved, &t3);
    sm_state_machine_add_state(&sm, &approved);

    /* Rejected */
    sm_state rejected;
    sm_state_init(&rejected, 4, "Rejected", SM_STATE_NORMAL);
    sm_state_add_entry_action(&rejected, on_rejected_entry, &report);
    sm_state_machine_add_state(&sm, &rejected);

    /* Paid */
    sm_state paid;
    sm_state_init(&paid, 5, "Paid", SM_STATE_FINAL);
    sm_state_add_entry_action(&paid, on_paid_entry, &report);
    sm_state_machine_add_state(&sm, &paid);

    sm_state_machine_start(&sm, 1);
    sm_state_machine_send_event(&sm, "submit");
    printf("  Current state: %d (is in Submitted: %s)\n",
           sm_state_machine_get_current(&sm),
           sm_state_machine_is_in(&sm, 2) ? "yes" : "no");
    sm_state_machine_send_event(&sm, "approve");
    printf("  Current state after approve: %d\n", sm_state_machine_get_current(&sm));
    sm_state_machine_send_event(&sm, "pay");
    printf("  Final state: %d\n", sm_state_machine_get_current(&sm));

    /* ========== 4. Approval Process ========== */
    printf("\n=== 4. Approval Process: Multi-Level ===\n");

    ap_engine ap_eng;
    ap_engine_init(&ap_eng);

    uint64_t ap_id = ap_engine_create(&ap_eng, "Expense Report #RE-2024-001");

    /* Level 0: Manager */
    ap_approval_level l0;
    ap_approval_level_init(&l0, 0, AP_LEVEL_SEQUENTIAL);
    ap_approver a_mgr;
    ap_approver_init(&a_mgr, "Zhang Wei", "Manager", 5000);
    ap_approval_level_add_approver(&l0, &a_mgr);
    ap_approval_level_set_timeout(&l0, 86400000); /* 24 hours */

    /* Level 1: Director (only if amount > 5000) */
    ap_approval_level l1;
    ap_approval_level_init(&l1, 1, AP_LEVEL_SEQUENTIAL);
    ap_approver a_dir;
    ap_approver_init(&a_dir, "Li Ming", "Director", 50000);
    ap_approval_level_add_approver(&l1, &a_dir);
    ap_condition l1_cond;
    ap_condition_init(&l1_cond, AP_COND_AMOUNT_GT, 5000);
    ap_approval_level_add_condition(&l1, &l1_cond);
    l1.is_optional = 1;

    /* Level 2: CFO */
    ap_approval_level l2;
    ap_approval_level_init(&l2, 2, AP_LEVEL_ANY_ONE);
    ap_approver a_cfo;
    ap_approver_init(&a_cfo, "Wang Fang", "CFO", 100000);
    ap_approval_level_add_approver(&l2, &a_cfo);
    ap_condition l2_cond;
    ap_condition_init(&l2_cond, AP_COND_AMOUNT_GT, 50000);
    ap_approval_level_add_condition(&l2, &l2_cond);
    l2.is_optional = 1;

    ap_approval_instance_add_level(ap_engine_get(&ap_eng, ap_id), &l0);
    ap_approval_instance_add_level(ap_engine_get(&ap_eng, ap_id), &l1);
    ap_approval_instance_add_level(ap_engine_get(&ap_eng, ap_id), &l2);

    ap_engine_submit(&ap_eng, ap_id, "employee001", 850, "Engineering");

    ap_approval_instance *ap_inst = ap_engine_get(&ap_eng, ap_id);
    printf("  Approval: '%s' submitted by %s\n", ap_inst->title, ap_inst->submitter);
    printf("  Amount: $%lld, Status: %s\n",
           (long long)ap_inst->amount, ap_status_name(ap_inst->status));
    printf("  Levels configured: %zu\n", ap_inst->level_count);
    for (size_t i = 0; i < ap_inst->level_count; i++) {
        printf("    Level %zu: %d approver(s), optional=%d, timeout=%lldms\n",
               i, (int)ap_inst->levels[i].approver_count,
               ap_inst->levels[i].is_optional,
               (long long)ap_inst->levels[i].timeout_ms);
    }

    /* Approve at level 0 */
    ap_approval_instance_approve(ap_inst, "Zhang Wei");
    printf("  After Level 0 approval -> Status: %s\n", ap_status_name(ap_inst->status));

    /* Level 1 is optional (amount=850 < 5000), so skipped automatically */
    if (ap_inst->levels[1].is_optional) {
        ap_inst->current_level = 2;
    }
    /* Level 2 also skipped */
    ap_inst->status = AP_STATUS_APPROVED;
    ap_inst->completed_at = (int64_t)time(NULL);
    printf("  Final Status: %s\n", ap_status_name(ap_inst->status));

    /* Audit log */
    printf("\n  Audit Log:\n");
    size_t alog_count = 0;
    const ap_audit_entry *alog = ap_approval_instance_get_audit(ap_inst, &alog_count);
    for (size_t i = 0; i < alog_count; i++) {
        printf("    [%lld] %s | %s | %s -> %s\n",
               (long long)alog[i].timestamp, alog[i].action, alog[i].actor,
               alog[i].detail, ap_status_name(alog[i].new_status));
    }

    /* ========== 5. Workflow Engine ========== */
    printf("\n=== 5. Workflow Engine: Process Orchestration ===\n");

    wf_engine wf_eng;
    wf_engine_init(&wf_eng);

    wf_process_definition pd;
    wf_process_definition_init(&pd, "ReimbursementFlow", "Reimbursement Process");

    wf_node n_start, n_collect, n_verify, n_parallel, n_pay, n_notify, n_end;
    wf_node_init(&n_start, 10, "Start", WF_NODE_START);
    wf_node_init(&n_collect, 11, "Collect Receipts", WF_NODE_MANUAL_TASK);
    wf_node_init(&n_verify, 12, "Verify Policy", WF_NODE_SERVICE_TASK);
    wf_node_init(&n_parallel, 13, "Split", WF_NODE_PARALLEL_GATEWAY);
    wf_node_init(&n_pay, 14, "Process Payment", WF_NODE_SERVICE_TASK);
    wf_node_init(&n_notify, 15, "Notify Employee", WF_NODE_SERVICE_TASK);
    wf_node_init(&n_end, 16, "End", WF_NODE_END);

    n_parallel.gw_direction = WF_GATEWAY_DIVERGE;

    wf_node_add_outgoing(&n_start, 11);
    wf_node_add_outgoing(&n_collect, 12);
    wf_node_add_outgoing(&n_verify, 13);
    wf_node_add_outgoing(&n_parallel, 14);
    wf_node_add_outgoing(&n_parallel, 15);
    wf_node_add_outgoing(&n_pay, 16);
    wf_node_add_outgoing(&n_notify, 16);

    wf_node_add_incoming(&n_collect, 10);
    wf_node_add_incoming(&n_verify, 11);
    wf_node_add_incoming(&n_parallel, 12);
    wf_node_add_incoming(&n_pay, 13);
    wf_node_add_incoming(&n_notify, 13);
    wf_node_add_incoming(&n_end, 14);
    wf_node_add_incoming(&n_end, 15);

    wf_process_definition_add_node(&pd, &n_start);
    wf_process_definition_add_node(&pd, &n_collect);
    wf_process_definition_add_node(&pd, &n_verify);
    wf_process_definition_add_node(&pd, &n_parallel);
    wf_process_definition_add_node(&pd, &n_pay);
    wf_process_definition_add_node(&pd, &n_notify);
    wf_process_definition_add_node(&pd, &n_end);

    if (!wf_process_definition_validate(&pd)) {
        printf("  ERROR: Process definition invalid!\n");
    }

    wf_engine_deploy(&wf_eng, &pd);
    uint64_t wf_id = wf_engine_start(&wf_eng, "ReimbursementFlow");
    wf_process_instance *pi = wf_engine_get_instance(&wf_eng, wf_id);
    printf("  Started process instance: %llu\n", (unsigned long long)wf_id);

    /* Run through service tasks */
    for (int step = 0; step < 10 && !wf_process_instance_is_complete(pi); step++) {
        wf_engine_tick(&wf_eng, wf_id);
    }

    printf("  Process instance state: %s\n",
           wf_process_instance_is_complete(pi) ? "COMPLETED" : "RUNNING");
    printf("  Token count: %zu\n", pi->token_count);
    printf("  Task count: %zu\n", pi->task_count);

    /* ========== Summary ========== */
    printf("\n========================================\n");
    printf("  Demo 1 Complete\n");
    printf("  - Domain model: Finance bounded context established\n");
    printf("  - Rule engine: Classified expense by amount tier\n");
    printf("  - State machine: Tracked expense through lifecycle\n");
    printf("  - Approval process: Multi-level approval with conditions\n");
    printf("  - Workflow engine: Orchestrated reimbursement flow\n");
    printf("========================================\n");

    return 0;
}
