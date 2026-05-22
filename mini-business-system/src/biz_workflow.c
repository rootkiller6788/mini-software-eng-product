#include "biz_workflow.h"
#include <stdio.h>
#include <string.h>

void wf_init(WorkflowEngine *wf, const char *name) { memset(wf, 0, sizeof(*wf)); wf->initial_state = -1; wf->current_state = -1; (void)name; }

int wf_add_state(WorkflowEngine *wf, const char *name, WfStatus status, bool is_initial, bool is_final) {
    if (wf->state_count >= WF_MAX_STATES) return -1;
    WfState *s = &wf->states[wf->state_count]; s->id = wf->state_count; s->status = status; s->is_initial = is_initial; s->is_final = is_final;
    strncpy(s->name, name, WF_NAME_LEN-1); s->name[WF_NAME_LEN-1]='\0';
    if (is_initial) { wf->initial_state = s->id; wf->current_state = s->id; }
    return wf->state_count++;
}

int wf_add_transition(WorkflowEngine *wf, const char *name, int from, int to, const char *(*guard)(void*), void (*action)(void*)) {
    if (wf->trans_count >= WF_MAX_TRANSITIONS) return -1;
    WfTransition *t = &wf->transitions[wf->trans_count]; t->id = wf->trans_count; t->from_state = from; t->to_state = to; t->guard = guard; t->action = action;
    strncpy(t->name, name, WF_NAME_LEN-1); t->name[WF_NAME_LEN-1]='\0';
    return wf->trans_count++;
}

bool wf_can_transition(WorkflowEngine *wf, int tid, void *ctx) { if (tid >= wf->trans_count) return false; WfTransition *t = &wf->transitions[tid]; if (t->from_state != wf->current_state) return false; return t->guard ? t->guard(ctx) == NULL : true; }

const char *wf_execute(WorkflowEngine *wf, int tid, void *ctx) {
    if (tid >= wf->trans_count) return "Invalid transition";
    WfTransition *t = &wf->transitions[tid];
    if (t->from_state != wf->current_state) return "Wrong state";
    if (t->guard) { const char *r = t->guard(ctx); if (r) return r; }
    if (t->action) t->action(ctx);
    wf->current_state = t->to_state;
    return NULL;
}

int wf_get_available_transitions(WorkflowEngine *wf, int *results, int max) { int c=0; for (int i=0; i<wf->trans_count && c<max; i++) if (wf->transitions[i].from_state==wf->current_state) results[c++]=i; return c; }

void wf_reset(WorkflowEngine *wf) { wf->current_state = wf->initial_state; }

void wf_print(WorkflowEngine *wf) {
    printf("=== Workflow ===\n  States: %d, Transitions: %d\n", wf->state_count, wf->trans_count);
    printf("  Current: %s\n", wf->current_state>=0?wf->states[wf->current_state].name:"none");
    for (int i=0; i<wf->state_count; i++) printf("    [%s]%s\n", wf->states[i].name, wf->states[i].is_initial?" (initial)":"");
}
