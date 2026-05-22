#include "workflow_engine.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

/* ============================================================
 * Workflow Engine — BPMN-like process implementation
 * ============================================================ */

/* ---------- Node ---------- */
void wf_node_init(wf_node *n, uint64_t id, const char *name, wf_node_type type) {
    n->id = id;
    strncpy(n->name, name, WF_MAX_NAME - 1);
    n->type = type;
    n->gw_direction = WF_GATEWAY_DIVERGE;
    n->timer_duration_ms = 0;
    n->incoming_count = 0;
    n->outgoing_count = 0;
    n->assignee_count = 0;
    memset(n->condition, 0, WF_MAX_NAME);
}

void wf_node_add_outgoing(wf_node *n, uint64_t target_id) {
    if (n->outgoing_count >= WF_MAX_OUTGOING) return;
    n->outgoing[n->outgoing_count++] = target_id;
}

void wf_node_add_incoming(wf_node *n, uint64_t source_id) {
    if (n->incoming_count >= WF_MAX_INCOMING) return;
    n->incoming[n->incoming_count++] = source_id;
}

void wf_node_add_assignee(wf_node *n, const char *assignee) {
    if (n->assignee_count >= WF_MAX_ASSIGNEES) return;
    strncpy(n->assignees[n->assignee_count++], assignee, WF_MAX_NAME - 1);
}

void wf_node_set_condition(wf_node *n, const char *condition) {
    strncpy(n->condition, condition, WF_MAX_NAME - 1);
}

void wf_node_set_timer(wf_node *n, int64_t duration_ms) {
    n->timer_duration_ms = duration_ms;
}

/* ---------- Process Definition ---------- */
void wf_process_definition_init(wf_process_definition *pd, const char *id, const char *name) {
    strncpy(pd->id, id, WF_MAX_NAME - 1);
    strncpy(pd->name, name, WF_MAX_NAME - 1);
    pd->node_count = 0;
    pd->valid = 0;
}

int wf_process_definition_add_node(wf_process_definition *pd, const wf_node *node) {
    if (pd->node_count >= WF_MAX_NODES) return -1;
    pd->nodes[pd->node_count++] = *node;
    return 0;
}

wf_node *wf_process_definition_find_node(wf_process_definition *pd, uint64_t node_id) {
    for (size_t i = 0; i < pd->node_count; i++) {
        if (pd->nodes[i].id == node_id) return &pd->nodes[i];
    }
    return NULL;
}

int wf_process_definition_validate(const wf_process_definition *pd) {
    int has_start = 0, has_end = 0;
    for (size_t i = 0; i < pd->node_count; i++) {
        if (pd->nodes[i].type == WF_NODE_START) has_start = 1;
        if (pd->nodes[i].type == WF_NODE_END) has_end = 1;
    }
    return has_start && has_end;
}

wf_node *wf_process_definition_get_start(wf_process_definition *pd) {
    for (size_t i = 0; i < pd->node_count; i++) {
        if (pd->nodes[i].type == WF_NODE_START) return &pd->nodes[i];
    }
    return NULL;
}

/* ---------- Process Instance ---------- */
void wf_process_instance_init(wf_process_instance *pi, uint64_t id,
                              wf_process_definition *def) {
    pi->id = id;
    strncpy(pi->process_id, def->id, WF_MAX_NAME - 1);
    pi->state = WF_INSTANCE_RUNNING;
    pi->token_count = 0;
    pi->task_count = 0;
    pi->started_at = 0;
    pi->ended_at = 0;
    pi->definition = def;
    pi->var_count = 0;
}

int wf_process_instance_start(wf_process_instance *pi) {
    wf_node *start_node = wf_process_definition_get_start(pi->definition);
    if (!start_node) return -1;
    /* place token at start */
    pi->tokens[0].node_id = start_node->id;
    pi->tokens[0].active = 1;
    pi->tokens[0].arrived_at = (int64_t)time(NULL);
    pi->token_count = 1;
    pi->started_at = (int64_t)time(NULL);
    pi->state = WF_INSTANCE_RUNNING;
    return 0;
}

static void _move_token(wf_process_instance *pi, size_t tidx, uint64_t next_id) {
    wf_node *next = wf_process_definition_find_node(pi->definition, next_id);
    if (!next) { pi->tokens[tidx].active = 0; return; }
    pi->tokens[tidx].node_id = next_id;
    pi->tokens[tidx].arrived_at = (int64_t)time(NULL);
    /* create task if it's a task node */
    if (next->type == WF_NODE_TASK || next->type == WF_NODE_MANUAL_TASK ||
        next->type == WF_NODE_SERVICE_TASK) {
        wf_task_instance *ti = &pi->tasks[pi->task_count++];
        ti->task_id = next->id;
        strncpy(ti->name, next->task_name[0] ? next->task_name : next->name, WF_MAX_NAME - 1);
        strncpy(ti->assignee, (next->assignee_count > 0) ? next->assignees[0] : "", WF_MAX_NAME - 1);
        ti->completed = 0;
        ti->created_at = (int64_t)time(NULL);
        ti->completed_at = 0;
    }
}

int wf_process_instance_step(wf_process_instance *pi) {
    if (pi->state != WF_INSTANCE_RUNNING) return -1;
    size_t cnt = pi->token_count;
    for (size_t t = 0; t < cnt; t++) {
        if (!pi->tokens[t].active) continue;
        wf_node *cur = wf_process_definition_find_node(pi->definition, pi->tokens[t].node_id);
        if (!cur) { pi->tokens[t].active = 0; continue; }

        switch (cur->type) {
        case WF_NODE_START:
        case WF_NODE_SERVICE_TASK:
            /* auto-step */
            if (cur->outgoing_count == 1) {
                _move_token(pi, t, cur->outgoing[0]);
            }
            break;
        case WF_NODE_END:
            pi->tokens[t].active = 0;
            break;
        case WF_NODE_EXCLUSIVE_GATEWAY:
            if (cur->gw_direction == WF_GATEWAY_DIVERGE) {
                /* evaluate conditions on outgoing edges */
                int chosen = 0;
                for (size_t o = 0; o < cur->outgoing_count; o++) {
                    wf_node *edge = wf_process_definition_find_node(pi->definition, cur->outgoing[o]);
                    if (edge && edge->gw_direction == WF_GATEWAY_CONVERGE) {
                        /* find the source edge from our node to this */
                        wf_node *target = wf_process_definition_find_node(pi->definition, cur->outgoing[o]);
                        if (!target) continue;
                        /* find incoming edge that belongs to target and check condition */
                        for (size_t ti = 0; ti < target->incoming_count; ti++) {
                            wf_node *src = wf_process_definition_find_node(pi->definition, target->incoming[ti]);
                            if (src && src->id == cur->id && src->condition[0]) {
                                int64_t val = wf_process_instance_get_var(pi, src->condition, 0);
                                if (val) { _move_token(pi, t, target->id); chosen = 1; break; }
                            }
                        }
                    }
                }
                /* default path: first outgoing */
                if (!chosen && cur->outgoing_count > 0) {
                    _move_token(pi, t, cur->outgoing[0]);
                }
            } else {
                /* converge: just pass through */
                if (cur->outgoing_count == 1) {
                    _move_token(pi, t, cur->outgoing[0]);
                }
                pi->tokens[t].active = 0;
            }
            break;
        case WF_NODE_PARALLEL_GATEWAY:
            if (cur->gw_direction == WF_GATEWAY_DIVERGE) {
                /* fork: create new tokens */
                for (size_t o = 0; o < cur->outgoing_count; o++) {
                    if (pi->token_count >= WF_MAX_TOKENS) continue;
                    pi->tokens[pi->token_count].node_id = cur->outgoing[o];
                    pi->tokens[pi->token_count].active = 1;
                    pi->tokens[pi->token_count].arrived_at = (int64_t)time(NULL);
                    pi->token_count++;
                }
                pi->tokens[t].active = 0;
            } else {
                /* join: wait for all incoming tokens */
                int all_arrived = 1;
                /* simplified: just pass through if outgoing exists */
                if (cur->outgoing_count == 1) {
                    _move_token(pi, t, cur->outgoing[0]);
                }
                pi->tokens[t].active = 0;
            }
            break;
        case WF_NODE_TASK:
        case WF_NODE_MANUAL_TASK:
            /* waiting for manual completion */
            pi->state = WF_INSTANCE_WAITING;
            break;
        default:
            break;
        }
    }

    /* check if all tokens inactive -> completed */
    int all_done = 1;
    for (size_t t = 0; t < pi->token_count; t++) {
        if (pi->tokens[t].active) { all_done = 0; break; }
    }
    if (all_done && pi->token_count > 0) {
        pi->state = WF_INSTANCE_COMPLETED;
        pi->ended_at = (int64_t)time(NULL);
    }
    return 0;
}

int wf_process_instance_complete_task(wf_process_instance *pi, uint64_t task_node_id) {
    for (size_t i = 0; i < pi->task_count; i++) {
        if (pi->tasks[i].task_id == task_node_id && !pi->tasks[i].completed) {
            pi->tasks[i].completed = 1;
            pi->tasks[i].completed_at = (int64_t)time(NULL);
            pi->state = WF_INSTANCE_RUNNING;
            break;
        }
    }
    /* move token past the task node */
    for (size_t t = 0; t < pi->token_count; t++) {
        if (pi->tokens[t].active && pi->tokens[t].node_id == task_node_id) {
            wf_node *task = wf_process_definition_find_node(pi->definition, task_node_id);
            if (task && task->outgoing_count == 1) {
                _move_token(pi, t, task->outgoing[0]);
            }
            break;
        }
    }
    return wf_process_instance_step(pi);
}

int wf_process_instance_is_complete(const wf_process_instance *pi) {
    return pi->state == WF_INSTANCE_COMPLETED || pi->state == WF_INSTANCE_TERMINATED;
}

void wf_process_instance_set_var(wf_process_instance *pi, const char *key, int64_t value) {
    for (size_t i = 0; i < pi->var_count; i++) {
        if (strcmp(pi->var_keys[i], key) == 0) { pi->var_values[i] = value; return; }
    }
    if (pi->var_count >= 32) return;
    strncpy(pi->var_keys[pi->var_count], key, WF_MAX_NAME - 1);
    pi->var_values[pi->var_count] = value;
    pi->var_count++;
}

int64_t wf_process_instance_get_var(const wf_process_instance *pi, const char *key, int64_t default_val) {
    for (size_t i = 0; i < pi->var_count; i++) {
        if (strcmp(pi->var_keys[i], key) == 0) return pi->var_values[i];
    }
    return default_val;
}

int wf_evaluate_condition(const wf_node *node, const wf_process_instance *pi) {
    if (!node->condition[0]) return 1;
    return (wf_process_instance_get_var(pi, node->condition, 0) != 0);
}

/* ---------- Workflow Engine ---------- */
void wf_engine_init(wf_engine *eng) {
    eng->definition_count = 0;
    eng->instance_count = 0;
    eng->next_instance_id = 1;
}

int wf_engine_deploy(wf_engine *eng, const wf_process_definition *pd) {
    if (eng->definition_count >= WF_MAX_INSTANCES) return -1;
    eng->definitions[eng->definition_count++] = *pd;
    return 0;
}

uint64_t wf_engine_start(wf_engine *eng, const char *process_id) {
    wf_process_definition *def = NULL;
    for (size_t i = 0; i < eng->definition_count; i++) {
        if (strcmp(eng->definitions[i].id, process_id) == 0) { def = &eng->definitions[i]; break; }
    }
    if (!def) return 0;
    if (eng->instance_count >= WF_MAX_INSTANCES) return 0;
    uint64_t id = eng->next_instance_id++;
    wf_process_instance_init(&eng->instances[eng->instance_count], id, def);
    wf_process_instance_start(&eng->instances[eng->instance_count]);
    eng->instance_count++;
    return id;
}

int wf_engine_tick(wf_engine *eng, uint64_t instance_id) {
    wf_process_instance *pi = wf_engine_get_instance(eng, instance_id);
    if (!pi) return -1;
    return wf_process_instance_step(pi);
}

int wf_engine_complete_task(wf_engine *eng, uint64_t instance_id, uint64_t task_node_id) {
    wf_process_instance *pi = wf_engine_get_instance(eng, instance_id);
    if (!pi) return -1;
    return wf_process_instance_complete_task(pi, task_node_id);
}

wf_process_instance *wf_engine_get_instance(wf_engine *eng, uint64_t instance_id) {
    for (size_t i = 0; i < eng->instance_count; i++) {
        if (eng->instances[i].id == instance_id) return &eng->instances[i];
    }
    return NULL;
}
