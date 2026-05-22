#ifndef WORKFLOW_ENGINE_H
#define WORKFLOW_ENGINE_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================
 * Workflow Engine — BPMN-like process engine (C99)
 * ============================================================ */

#define WF_MAX_NAME       64
#define WF_MAX_NODES      64
#define WF_MAX_INSTANCES  128
#define WF_MAX_TASKS      64
#define WF_MAX_ASSIGNEES  8
#define WF_MAX_TOKENS     32
#define WF_MAX_INCOMING   8
#define WF_MAX_OUTGOING   8

/* ---------- Node Types ---------- */
typedef enum {
    WF_NODE_START,
    WF_NODE_TASK,
    WF_NODE_EXCLUSIVE_GATEWAY,
    WF_NODE_PARALLEL_GATEWAY,
    WF_NODE_END,
    WF_NODE_TIMER_EVENT,
    WF_NODE_MANUAL_TASK,
    WF_NODE_SERVICE_TASK,
    WF_NODE_CALL_ACTIVITY
} wf_node_type;

/* ---------- Gateway Types ---------- */
typedef enum {
    WF_GATEWAY_DIVERGE,
    WF_GATEWAY_CONVERGE
} wf_gateway_direction;

/* ---------- Process Node ---------- */
typedef struct wf_node {
    uint64_t id;
    char     name[WF_MAX_NAME];
    wf_node_type type;
    /* gateway specific */
    wf_gateway_direction gw_direction;
    char     condition[WF_MAX_NAME]; /* for exclusive gateway outgoing */
    /* timer */
    int64_t  timer_duration_ms;
    /* edges */
    uint64_t incoming[WF_MAX_INCOMING];
    size_t   incoming_count;
    uint64_t outgoing[WF_MAX_OUTGOING];
    size_t   outgoing_count;
    /* task */
    char     task_name[WF_MAX_NAME];
    char     assignees[WF_MAX_ASSIGNEES][WF_MAX_NAME];
    size_t   assignee_count;
} wf_node;

void wf_node_init(wf_node *n, uint64_t id, const char *name, wf_node_type type);
void wf_node_add_outgoing(wf_node *n, uint64_t target_id);
void wf_node_add_incoming(wf_node *n, uint64_t source_id);
void wf_node_add_assignee(wf_node *n, const char *assignee);
void wf_node_set_condition(wf_node *n, const char *condition);
void wf_node_set_timer(wf_node *n, int64_t duration_ms);

/* ---------- Process Definition ---------- */
typedef struct {
    char     id[WF_MAX_NAME];
    char     name[WF_MAX_NAME];
    wf_node  nodes[WF_MAX_NODES];
    size_t   node_count;
    int      valid;
} wf_process_definition;

void wf_process_definition_init(wf_process_definition *pd, const char *id, const char *name);
int  wf_process_definition_add_node(wf_process_definition *pd, const wf_node *node);
wf_node *wf_process_definition_find_node(wf_process_definition *pd, uint64_t node_id);
int  wf_process_definition_validate(const wf_process_definition *pd);
wf_node *wf_process_definition_get_start(wf_process_definition *pd);

/* ---------- Token (for parallel execution) ---------- */
typedef struct {
    uint64_t node_id;
    int      active;
    int64_t  arrived_at;
} wf_token;

/* ---------- Task Instance ---------- */
typedef struct {
    uint64_t task_id;
    char     name[WF_MAX_NAME];
    char     assignee[WF_MAX_NAME];
    int      completed;
    int64_t  created_at;
    int64_t  completed_at;
} wf_task_instance;

/* ---------- Process Instance ---------- */
typedef enum {
    WF_INSTANCE_RUNNING,
    WF_INSTANCE_COMPLETED,
    WF_INSTANCE_TERMINATED,
    WF_INSTANCE_WAITING
} wf_instance_state;

typedef struct {
    uint64_t              id;
    char                  process_id[WF_MAX_NAME];
    wf_instance_state     state;
    wf_token              tokens[WF_MAX_TOKENS];
    size_t                token_count;
    wf_task_instance      tasks[WF_MAX_TASKS];
    size_t                task_count;
    int64_t               started_at;
    int64_t               ended_at;
    /* pointer to definition */
    wf_process_definition *definition;
    /* variables (key=value for expressions) */
    char                  var_keys[32][WF_MAX_NAME];
    int64_t               var_values[32];
    size_t                var_count;
} wf_process_instance;

void wf_process_instance_init(wf_process_instance *pi, uint64_t id,
                              wf_process_definition *def);
int  wf_process_instance_start(wf_process_instance *pi);
int  wf_process_instance_step(wf_process_instance *pi);
int  wf_process_instance_complete_task(wf_process_instance *pi, uint64_t task_node_id);
int  wf_process_instance_is_complete(const wf_process_instance *pi);
void wf_process_instance_set_var(wf_process_instance *pi, const char *key, int64_t value);
int64_t wf_process_instance_get_var(const wf_process_instance *pi, const char *key, int64_t default_val);
int  wf_evaluate_condition(const wf_node *node, const wf_process_instance *pi);

/* ---------- Workflow Engine ---------- */
typedef struct {
    wf_process_definition  definitions[WF_MAX_INSTANCES];
    size_t                 definition_count;
    wf_process_instance    instances[WF_MAX_INSTANCES];
    size_t                 instance_count;
    uint64_t               next_instance_id;
} wf_engine;

void wf_engine_init(wf_engine *eng);
int  wf_engine_deploy(wf_engine *eng, const wf_process_definition *pd);
uint64_t wf_engine_start(wf_engine *eng, const char *process_id);
int  wf_engine_tick(wf_engine *eng, uint64_t instance_id);
int  wf_engine_complete_task(wf_engine *eng, uint64_t instance_id, uint64_t task_node_id);
wf_process_instance *wf_engine_get_instance(wf_engine *eng, uint64_t instance_id);

#ifdef __cplusplus
}
#endif

#endif /* WORKFLOW_ENGINE_H */
