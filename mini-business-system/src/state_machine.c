#include "state_machine.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

/* ============================================================
 * State Machine — Harel statecharts implementation
 * ============================================================ */

/* ---------- Action ---------- */
void sm_action_init(sm_action *a, const char *name, sm_action_fn fn, void *ctx) {
    strncpy(a->name, name, SM_MAX_NAME - 1);
    a->fn = fn;
    a->ctx = ctx;
}

void sm_action_execute(const sm_action *a) {
    if (a && a->fn) a->fn(a->ctx);
}

/* ---------- Transition ---------- */
void sm_transition_init(sm_transition *t, const char *event, int target) {
    strncpy(t->event, event, SM_MAX_EVENT - 1);
    t->target_state_id = target;
    t->guard = NULL;
    t->guard_ctx = NULL;
    t->action = NULL;
    t->action_ctx = NULL;
}

void sm_transition_set_guard(sm_transition *t, sm_guard_fn guard, void *ctx) {
    t->guard = guard;
    t->guard_ctx = ctx;
}

void sm_transition_set_action(sm_transition *t, sm_transition_action_fn action, void *ctx) {
    t->action = action;
    t->action_ctx = ctx;
}

/* ---------- State ---------- */
void sm_state_init(sm_state *s, int id, const char *name, sm_state_type type) {
    s->id = id;
    strncpy(s->name, name, SM_MAX_NAME - 1);
    s->type = type;
    s->entry_count = 0;
    s->exit_count = 0;
    s->do_count = 0;
    s->transition_count = 0;
    s->parent_id = -1;
    s->child_count = 0;
    s->initial_child = -1;
    s->history_child = -1;
    s->ortho_count = 0;
    s->deferred_count = 0;
}

void sm_state_add_entry_action(sm_state *s, sm_action_fn fn, void *ctx) {
    if (s->entry_count >= SM_MAX_ACTIONS) return;
    sm_action_init(&s->entry_actions[s->entry_count++], "entry", fn, ctx);
}

void sm_state_add_exit_action(sm_state *s, sm_action_fn fn, void *ctx) {
    if (s->exit_count >= SM_MAX_ACTIONS) return;
    sm_action_init(&s->exit_actions[s->exit_count++], "exit", fn, ctx);
}

void sm_state_add_do_action(sm_state *s, sm_action_fn fn, void *ctx) {
    if (s->do_count >= SM_MAX_ACTIONS) return;
    sm_action_init(&s->do_actions[s->do_count++], "do", fn, ctx);
}

int sm_state_add_transition(sm_state *s, const sm_transition *t) {
    if (s->transition_count >= SM_MAX_TRANSITIONS) return -1;
    s->transitions[s->transition_count++] = *t;
    return 0;
}

void sm_state_add_child(sm_state *s, int child_id) {
    if (s->child_count >= SM_MAX_CHILDREN) return;
    s->children[s->child_count++] = child_id;
    if (s->initial_child < 0) s->initial_child = child_id;
}

void sm_state_add_deferred_event(sm_state *s, const char *event) {
    if (s->deferred_count >= SM_MAX_DEFERRED) return;
    strncpy(s->deferred_events[s->deferred_count++], event, SM_MAX_EVENT - 1);
}

/* ---------- State Machine ---------- */
void sm_state_machine_init(sm_state_machine *sm, void *user_data) {
    sm->state_count = 0;
    sm->current_state_id = -1;
    sm->previous_state_id = -1;
    sm->active_count = 0;
    sm->user_data = user_data;
}

int sm_state_machine_add_state(sm_state_machine *sm, const sm_state *s) {
    if (sm->state_count >= SM_MAX_STATES) return -1;
    sm->states[sm->state_count++] = *s;
    return 0;
}

sm_state *sm_state_machine_find_state(sm_state_machine *sm, int id) {
    for (size_t i = 0; i < sm->state_count; i++) {
        if (sm->states[i].id == id) return &sm->states[i];
    }
    return NULL;
}

static void _execute_entry_actions_recursive(sm_state_machine *sm, sm_state *s) {
    for (size_t i = 0; i < s->entry_count; i++) sm_action_execute(&s->entry_actions[i]);
    if (s->initial_child >= 0) {
        sm_state *child = sm_state_machine_find_state(sm, s->initial_child);
        if (child) _execute_entry_actions_recursive(sm, child);
    }
}

static void _execute_exit_actions_recursive(sm_state_machine *sm, sm_state *s) {
    /* exit children first */
    for (size_t i = 0; i < s->child_count; i++) {
        sm_state *child = sm_state_machine_find_state(sm, s->children[i]);
        int found = 0;
        for (size_t a = 0; a < sm->active_count; a++) {
            if (sm->active_states[a] && sm->active_states[a]->id == s->children[i]) found = 1;
        }
        if (found && child) _execute_exit_actions_recursive(sm, child);
    }
    for (size_t i = 0; i < s->exit_count; i++) sm_action_execute(&s->exit_actions[i]);
}

static int _find_lca(sm_state_machine *sm, int from_id, int to_id) {
    (void)sm; (void)from_id; (void)to_id;
    return -1; /* simplified: no LCA, just flat */
}

int sm_state_machine_start(sm_state_machine *sm, int state_id) {
    sm_state *s = sm_state_machine_find_state(sm, state_id);
    if (!s) return -1;
    sm->current_state_id = state_id;
    sm->active_count = 1;
    sm->active_states[0] = s;
    _execute_entry_actions_recursive(sm, s);
    return 0;
}

int sm_state_machine_send_event(sm_state_machine *sm, const char *event) {
    if (sm->current_state_id < 0) return -1;
    sm_state *cur = sm_state_machine_find_state(sm, sm->current_state_id);
    if (!cur) return -1;

    /* check deferred events on current state */
    for (size_t d = 0; d < cur->deferred_count; d++) {
        if (strcmp(cur->deferred_events[d], event) == 0) {
            return sm_state_machine_defer_event(sm, event);
        }
    }

    for (size_t i = 0; i < cur->transition_count; i++) {
        sm_transition *t = &cur->transitions[i];
        if (strcmp(t->event, event) == 0) {
            if (t->guard && !t->guard(t->guard_ctx)) continue;
            sm->previous_state_id = sm->current_state_id;
            /* exit current */
            _execute_exit_actions_recursive(sm, cur);
            /* transition action */
            if (t->action) t->action(t->action_ctx, sm->previous_state_id, t->target_state_id);
            /* enter target */
            sm_state *target = sm_state_machine_find_state(sm, t->target_state_id);
            if (target) {
                sm->current_state_id = t->target_state_id;
                sm->active_states[0] = target;
                sm->active_count = 1;
                _execute_entry_actions_recursive(sm, target);
            }
            return 0;
        }
    }
    /* check parent transitions (hierarchical) */
    if (cur->parent_id >= 0) {
        sm_state *parent = sm_state_machine_find_state(sm, cur->parent_id);
        if (parent) {
            for (size_t i = 0; i < parent->transition_count; i++) {
                sm_transition *t = &parent->transitions[i];
                if (strcmp(t->event, event) == 0) {
                    if (t->guard && !t->guard(t->guard_ctx)) continue;
                    sm->previous_state_id = sm->current_state_id;
                    _execute_exit_actions_recursive(sm, parent);
                    if (t->action) t->action(t->action_ctx, sm->previous_state_id, t->target_state_id);
                    sm_state *target = sm_state_machine_find_state(sm, t->target_state_id);
                    if (target) {
                        sm->current_state_id = t->target_state_id;
                        sm->active_states[0] = target;
                        sm->active_count = 1;
                        _execute_entry_actions_recursive(sm, target);
                    }
                    return 0;
                }
            }
        }
    }
    return -1;
}

int sm_state_machine_get_current(const sm_state_machine *sm) {
    return sm->current_state_id;
}

int sm_state_machine_is_in(const sm_state_machine *sm, int state_id) {
    if (sm->current_state_id == state_id) return 1;
    /* check if state_id is ancestor of current */
    sm_state *cur = sm_state_machine_find_state((sm_state_machine *)sm, sm->current_state_id);
    while (cur && cur->parent_id >= 0) {
        if (cur->parent_id == state_id) return 1;
        cur = sm_state_machine_find_state((sm_state_machine *)sm, cur->parent_id);
    }
    return 0;
}

void sm_state_machine_do_actions(sm_state_machine *sm) {
    sm_state *cur = sm_state_machine_find_state(sm, sm->current_state_id);
    if (cur) {
        for (size_t i = 0; i < cur->do_count; i++) sm_action_execute(&cur->do_actions[i]);
    }
}

int sm_state_machine_defer_event(sm_state_machine *sm, const char *event) {
    (void)sm; (void)event;
    return 0; /* simplified */
}

void sm_state_machine_flush_deferred(sm_state_machine *sm) {
    (void)sm;
    /* simplified: iterate deferred queue and re-send events */
}
