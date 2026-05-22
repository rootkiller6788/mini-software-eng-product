#ifndef BIZ_WORKFLOW_H
#define BIZ_WORKFLOW_H
#include <stdint.h>
#include <stdbool.h>

#define WF_MAX_STATES 16
#define WF_MAX_TRANSITIONS 64
#define WF_NAME_LEN 48

typedef enum { WF_PENDING, WF_IN_PROGRESS, WF_COMPLETED, WF_REJECTED, WF_CANCELLED } WfStatus;

typedef struct { int id; char name[WF_NAME_LEN]; WfStatus status; bool is_initial; bool is_final; } WfState;

typedef struct {
    int id; char name[WF_NAME_LEN]; int from_state; int to_state;
    const char *(*guard)(void *ctx);  /* returns NULL if allowed, else reason */
    void (*action)(void *ctx);        /* side effect */
} WfTransition;

typedef struct {
    WfState states[WF_MAX_STATES]; int state_count;
    WfTransition transitions[WF_MAX_TRANSITIONS]; int trans_count;
    int current_state;
    int initial_state;
} WorkflowEngine;

void wf_init(WorkflowEngine *wf, const char *name);
int  wf_add_state(WorkflowEngine *wf, const char *name, WfStatus status, bool is_initial, bool is_final);
int  wf_add_transition(WorkflowEngine *wf, const char *name, int from, int to,
    const char *(*guard)(void*), void (*action)(void*));
bool wf_can_transition(WorkflowEngine *wf, int trans_id, void *ctx);
const char *wf_execute(WorkflowEngine *wf, int trans_id, void *ctx);
int  wf_get_available_transitions(WorkflowEngine *wf, int *results, int max_results);
void wf_reset(WorkflowEngine *wf);
void wf_print(WorkflowEngine *wf);
#endif
