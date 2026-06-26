/* ============================================================
 * Example 2: Workflow Engine — BPMN Process with Gateways
 * ============================================================ */

#include "workflow_engine.h"
#include <stdio.h>
#include <string.h>

/*
 * Process: Expense Approval
 *
 *   [Start] -> [Submit Expense] -> [XOR: amount>1000?] -> (yes) [Manager Approve] -> [End]
 *                                                   \-> (no)  [Auto Approve] --------> [End]
 */

int main(void) {
    printf("=== Example 2: Workflow Engine ===\n\n");

    wf_engine engine;
    wf_engine_init(&engine);

    /* ---- Build Process Definition ---- */
    wf_process_definition expense_pd;
    wf_process_definition_init(&expense_pd, "ExpenseApproval", "Expense Approval Process");

    /* Start */
    wf_node start_node;
    wf_node_init(&start_node, 1, "Start", WF_NODE_START);
    wf_node_add_outgoing(&start_node, 2);
    wf_process_definition_add_node(&expense_pd, &start_node);

    /* Submit Expense (manual task) */
    wf_node submit;
    wf_node_init(&submit, 2, "Submit Expense", WF_NODE_MANUAL_TASK);
    strncpy(submit.task_name, "Submit Expense Form", WF_MAX_NAME - 1);
    wf_node_add_incoming(&submit, 1);
    wf_node_add_outgoing(&submit, 3);
    wf_node_add_assignee(&submit, "employee");
    wf_process_definition_add_node(&expense_pd, &submit);

    /* Exclusive Gateway */
    wf_node xgw;
    wf_node_init(&xgw, 3, "Amount Check", WF_NODE_EXCLUSIVE_GATEWAY);
    xgw.gw_direction = WF_GATEWAY_DIVERGE;
    wf_node_add_incoming(&xgw, 2);
    wf_node_add_outgoing(&xgw, 4);
    wf_node_add_outgoing(&xgw, 5);
    wf_node_set_condition(&xgw, "high_amount");
    wf_process_definition_add_node(&expense_pd, &xgw);

    /* Auto Approve */
    wf_node auto_approve;
    wf_node_init(&auto_approve, 4, "Auto Approve", WF_NODE_SERVICE_TASK);
    strncpy(auto_approve.task_name, "Auto Approve", WF_MAX_NAME - 1);
    wf_node_add_incoming(&auto_approve, 3);
    wf_node_add_outgoing(&auto_approve, 6);
    wf_process_definition_add_node(&expense_pd, &auto_approve);

    /* Manager Approve */
    wf_node manager;
    wf_node_init(&manager, 5, "Manager Approve", WF_NODE_MANUAL_TASK);
    strncpy(manager.task_name, "Manager Approval Required", WF_MAX_NAME - 1);
    wf_node_add_incoming(&manager, 3);
    wf_node_add_outgoing(&manager, 6);
    wf_node_add_assignee(&manager, "manager");
    wf_process_definition_add_node(&expense_pd, &manager);

    /* End */
    wf_node end_node;
    wf_node_init(&end_node, 6, "End", WF_NODE_END);
    wf_node_add_incoming(&end_node, 4);
    wf_node_add_incoming(&end_node, 5);
    wf_process_definition_add_node(&expense_pd, &end_node);

    /* Validate & Deploy */
    if (wf_process_definition_validate(&expense_pd)) {
        printf("Process definition '%s' is valid\n", expense_pd.name);
    } else {
        printf("Process definition INVALID!\n");
        return 1;
    }
    wf_engine_deploy(&engine, &expense_pd);

    /* ---- Run Low-Amount Instance ---- */
    printf("\n--- Instance 1: Low Amount ($500) ---\n");
    uint64_t id1 = wf_engine_start(&engine, "ExpenseApproval");
    printf("Started instance: %llu\n", (unsigned long long)id1);

    wf_process_instance *pi1 = wf_engine_get_instance(&engine, id1);
    if (pi1) {
        wf_process_instance_set_var(pi1, "high_amount", 0); /* false = low amount */
        /* step through start -> submit (waiting) */
        wf_engine_tick(&engine, id1);
        /* complete submit task */
        wf_engine_complete_task(&engine, id1, 2);
        /* should go through auto-approve -> end */
        wf_engine_tick(&engine, id1);
        printf("Instance 1 state: %s\n",
               wf_process_instance_is_complete(pi1) ? "COMPLETED" : "RUNNING");
    }

    /* ---- Run High-Amount Instance ---- */
    printf("\n--- Instance 2: High Amount ($5000) ---\n");
    uint64_t id2 = wf_engine_start(&engine, "ExpenseApproval");
    printf("Started instance: %llu\n", (unsigned long long)id2);

    wf_process_instance *pi2 = wf_engine_get_instance(&engine, id2);
    if (pi2) {
        wf_process_instance_set_var(pi2, "high_amount", 1); /* true = high amount */
        wf_engine_tick(&engine, id2);
        /* complete submit, goes to manager task */
        wf_engine_complete_task(&engine, id2, 2);
        printf("Instance 2 waiting at task node. Tasks:\n");
        for (size_t i = 0; i < pi2->task_count; i++) {
            printf("  Task %llu: '%s' assignee=%s completed=%d\n",
                   (unsigned long long)pi2->tasks[i].task_id, pi2->tasks[i].name,
                   pi2->tasks[i].assignee, pi2->tasks[i].completed);
        }
        /* complete manager task -> end */
        wf_engine_complete_task(&engine, id2, 5);
        printf("Instance 2 state: %s\n",
               wf_process_instance_is_complete(pi2) ? "COMPLETED" : "RUNNING");
    }

    printf("\n--- Engine Summary ---\n");
    printf("Definitions: %zu\n", engine.definition_count);
    printf("Instances: %zu\n", engine.instance_count);
    for (size_t i = 0; i < engine.instance_count; i++) {
        printf("  Instance %llu: %s -> state=%d\n",
               (unsigned long long)engine.instances[i].id,
               engine.instances[i].process_id,
               engine.instances[i].state);
    }

    printf("\n=== Example 2 Complete ===\n");
    return 0;
}
