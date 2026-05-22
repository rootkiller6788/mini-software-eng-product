#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================
 * State Machine — Harel statecharts (C99)
 * ============================================================ */

#define SM_MAX_NAME       64
#define SM_MAX_STATES     64
#define SM_MAX_TRANSITIONS 128
#define SM_MAX_ACTIONS    8
#define SM_MAX_CHILDREN   16
#define SM_MAX_DEFERRED   8
#define SM_MAX_ORTHO      8
#define SM_MAX_EVENT      32

/* ---------- Action ---------- */
typedef void (*sm_action_fn)(void *ctx);

typedef struct {
    char        name[SM_MAX_NAME];
    sm_action_fn fn;
    void       *ctx;
} sm_action;

void sm_action_init(sm_action *a, const char *name, sm_action_fn fn, void *ctx);
void sm_action_execute(const sm_action *a);

/* ---------- Transition ---------- */
typedef int (*sm_guard_fn)(void *ctx);
typedef void (*sm_transition_action_fn)(void *ctx, int from_state_id, int to_state_id);

typedef struct {
    char                      event[SM_MAX_EVENT];
    sm_guard_fn               guard;
    void                     *guard_ctx;
    sm_transition_action_fn   action;
    void                     *action_ctx;
    int                       target_state_id;
} sm_transition;

void sm_transition_init(sm_transition *t, const char *event, int target);
void sm_transition_set_guard(sm_transition *t, sm_guard_fn guard, void *ctx);
void sm_transition_set_action(sm_transition *t, sm_transition_action_fn action, void *ctx);

/* ---------- State ---------- */
typedef enum {
    SM_STATE_NORMAL,
    SM_STATE_INITIAL,
    SM_STATE_FINAL,
    SM_STATE_HISTORY_SHALLOW,
    SM_STATE_HISTORY_DEEP
} sm_state_type;

typedef struct sm_state {
    int           id;
    char          name[SM_MAX_NAME];
    sm_state_type type;
    /* actions */
    sm_action     entry_actions[SM_MAX_ACTIONS];
    size_t        entry_count;
    sm_action     exit_actions[SM_MAX_ACTIONS];
    size_t        exit_count;
    sm_action     do_actions[SM_MAX_ACTIONS];
    size_t        do_count;
    /* transitions */
    sm_transition transitions[SM_MAX_TRANSITIONS];
    size_t        transition_count;
    /* hierarchy */
    int           parent_id;        /* -1 if top-level */
    int           children[SM_MAX_CHILDREN];
    size_t        child_count;
    int           initial_child;    /* for compound states */
    int           history_child;    /* last active child for history */
    /* orthogonal regions */
    int           orthogonal[SM_MAX_ORTHO];
    size_t        ortho_count;
    /* deferred events */
    char          deferred_events[SM_MAX_DEFERRED][SM_MAX_EVENT];
    size_t        deferred_count;
} sm_state;

void sm_state_init(sm_state *s, int id, const char *name, sm_state_type type);
void sm_state_add_entry_action(sm_state *s, sm_action_fn fn, void *ctx);
void sm_state_add_exit_action(sm_state *s, sm_action_fn fn, void *ctx);
void sm_state_add_do_action(sm_state *s, sm_action_fn fn, void *ctx);
int  sm_state_add_transition(sm_state *s, const sm_transition *t);
void sm_state_add_child(sm_state *s, int child_id);
void sm_state_add_deferred_event(sm_state *s, const char *event);

/* ---------- State Machine ---------- */
typedef struct {
    sm_state   states[SM_MAX_STATES];
    size_t     state_count;
    int        current_state_id;
    int        previous_state_id;
    sm_state  *active_states[SM_MAX_STATES]; /* for parallel */
    size_t     active_count;
    void      *user_data;
} sm_state_machine;

void sm_state_machine_init(sm_state_machine *sm, void *user_data);
int  sm_state_machine_add_state(sm_state_machine *sm, const sm_state *s);
sm_state *sm_state_machine_find_state(sm_state_machine *sm, int id);
int  sm_state_machine_start(sm_state_machine *sm, int state_id);
int  sm_state_machine_send_event(sm_state_machine *sm, const char *event);
int  sm_state_machine_get_current(const sm_state_machine *sm);
int  sm_state_machine_is_in(const sm_state_machine *sm, int state_id);
void sm_state_machine_do_actions(sm_state_machine *sm);
int  sm_state_machine_defer_event(sm_state_machine *sm, const char *event);
void sm_state_machine_flush_deferred(sm_state_machine *sm);

#ifdef __cplusplus
}
#endif

#endif /* STATE_MACHINE_H */
