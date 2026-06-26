#include "gov_tracker.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

/* ============================================================
 * L2: Issue Tracker Implementation — Jira-like system
 * L3: Workflow State Machine — transition validation
 * Core concepts: Issue lifecycle, workflow engine, query/filter
 * ============================================================ */

void tracker_init(IssueTracker *tr) {
    memset(tr, 0, sizeof(*tr));
    tr->next_id = 1;
    workflow_init(&tr->workflow);
    workflow_setup_default(&tr->workflow);
}

int tracker_add_project(IssueTracker *tr, const char *id, const char *name, const char *desc) {
    if (tr->project_count >= TRACKER_MAX_PROJECTS) return -1;
    Project *p = &tr->projects[tr->project_count];
    memset(p, 0, sizeof(*p));
    strncpy(p->id, id, TRACKER_ID_LEN - 1); p->id[TRACKER_ID_LEN - 1] = '\0';
    strncpy(p->name, name, TRACKER_NAME_LEN - 1); p->name[TRACKER_NAME_LEN - 1] = '\0';
    strncpy(p->description, desc, TRACKER_DESC_LEN - 1); p->description[TRACKER_DESC_LEN - 1] = '\0';
    p->created_at = (uint32_t)time(NULL);
    return tr->project_count++;
}

int tracker_add_issue(IssueTracker *tr, const char *id, const char *title,
                      const char *desc, IssueType type, IssuePriority priority,
                      int points, int estimate) {
    if (tr->issue_count >= TRACKER_MAX_ISSUES) return -1;
    Issue *is = &tr->issues[tr->issue_count];
    memset(is, 0, sizeof(*is));
    strncpy(is->id, id, TRACKER_ID_LEN - 1); is->id[TRACKER_ID_LEN - 1] = '\0';
    strncpy(is->title, title, TRACKER_NAME_LEN - 1); is->title[TRACKER_NAME_LEN - 1] = '\0';
    strncpy(is->description, desc, TRACKER_DESC_LEN - 1); is->description[TRACKER_DESC_LEN - 1] = '\0';
    is->type = type;
    is->status = STATUS_OPEN;
    is->priority = priority;
    is->story_points = points;
    is->original_estimate_h = estimate;
    is->remaining_h = estimate;
    is->logged_h = 0;
    is->created_at = (uint32_t)time(NULL);
    is->updated_at = is->created_at;
    is->epic_id = -1;
    is->project_id = -1;
    is->sprint_id = -1;
    strncpy(is->reporter, "system", TRACKER_NAME_LEN - 1);
    return tr->issue_count++;
}

int tracker_find_issue(IssueTracker *tr, const char *id) {
    for (int i = 0; i < tr->issue_count; i++) {
        if (strcmp(tr->issues[i].id, id) == 0) return i;
    }
    return -1;
}

/* L3: Workflow state machine with transition validation */
bool tracker_update_status(IssueTracker *tr, const char *id, IssueStatus new_status) {
    int idx = tracker_find_issue(tr, id);
    if (idx < 0) return false;
    Issue *is = &tr->issues[idx];
    if (!workflow_is_valid(&tr->workflow, is->status, new_status)) return false;
    is->status = new_status;
    is->updated_at = (uint32_t)time(NULL);
    if (new_status == STATUS_DONE || new_status == STATUS_CLOSED) {
        is->resolved_at = is->updated_at;
    }
    return true;
}

bool tracker_assign(IssueTracker *tr, const char *id, const char *assignee) {
    int idx = tracker_find_issue(tr, id);
    if (idx < 0) return false;
    strncpy(tr->issues[idx].assignee, assignee, TRACKER_NAME_LEN - 1);
    tr->issues[idx].assignee[TRACKER_NAME_LEN - 1] = '\0';
    tr->issues[idx].updated_at = (uint32_t)time(NULL);
    return true;
}

bool tracker_add_comment(IssueTracker *tr, const char *id, const char *author, const char *body) {
    int idx = tracker_find_issue(tr, id);
    if (idx < 0) return false;
    Issue *is = &tr->issues[idx];
    if (is->comment_count >= TRACKER_MAX_COMMENTS) return false;
    IssueComment *c = &is->comments[is->comment_count];
    strncpy(c->author, author, TRACKER_NAME_LEN - 1);
    c->author[TRACKER_NAME_LEN - 1] = '\0';
    strncpy(c->body, body, TRACKER_COMMENT_LEN - 1);
    c->body[TRACKER_COMMENT_LEN - 1] = '\0';
    c->timestamp = (uint32_t)time(NULL);
    is->comment_count++;
    is->updated_at = c->timestamp;
    return true;
}

bool tracker_add_watcher(IssueTracker *tr, const char *id, const char *user) {
    int idx = tracker_find_issue(tr, id);
    if (idx < 0) return false;
    Issue *is = &tr->issues[idx];
    if (is->watcher_count >= TRACKER_MAX_WATCHERS) return false;
    /* Check duplicate */
    for (int i = 0; i < is->watcher_count; i++) {
        if (strcmp(is->watchers[i], user) == 0) return false;
    }
    strncpy(is->watchers[is->watcher_count], user, TRACKER_NAME_LEN - 1);
    is->watchers[is->watcher_count][TRACKER_NAME_LEN - 1] = '\0';
    is->watcher_count++;
    return true;
}

bool tracker_remove_watcher(IssueTracker *tr, const char *id, const char *user) {
    int idx = tracker_find_issue(tr, id);
    if (idx < 0) return false;
    Issue *is = &tr->issues[idx];
    for (int i = 0; i < is->watcher_count; i++) {
        if (strcmp(is->watchers[i], user) == 0) {
            memmove(is->watchers[i], is->watchers[i + 1],
                    (is->watcher_count - i - 1) * TRACKER_NAME_LEN);
            is->watcher_count--;
            return true;
        }
    }
    return false;
}

bool tracker_add_label(IssueTracker *tr, const char *id, const char *label) {
    int idx = tracker_find_issue(tr, id);
    if (idx < 0) return false;
    Issue *is = &tr->issues[idx];
    if (is->label_count >= TRACKER_MAX_LABELS) return false;
    for (int i = 0; i < is->label_count; i++) {
        if (strcmp(is->labels[i], label) == 0) return false;
    }
    strncpy(is->labels[is->label_count], label, TRACKER_NAME_LEN - 1);
    is->labels[is->label_count][TRACKER_NAME_LEN - 1] = '\0';
    is->label_count++;
    return true;
}

bool tracker_link_issues(IssueTracker *tr, const char *from, const char *to, LinkType type) {
    int fi = tracker_find_issue(tr, from);
    int ti = tracker_find_issue(tr, to);
    if (fi < 0 || ti < 0 || fi == ti) return false;
    Issue *f = &tr->issues[fi];
    if (f->link_count >= TRACKER_MAX_LINKS) return false;
    f->links[f->link_count].type = type;
    strncpy(f->links[f->link_count].target_id, to, TRACKER_ID_LEN - 1);
    f->links[f->link_count].target_id[TRACKER_ID_LEN - 1] = '\0';
    f->link_count++;
    return true;
}

/* ---- Workflow Engine ---- */
void workflow_init(WorkflowEngine *wf) {
    memset(wf, 0, sizeof(*wf));
}

bool workflow_add_transition(WorkflowEngine *wf, IssueStatus from, IssueStatus to) {
    if (wf->count >= TRANSITIONS_MAX) return false;
    /* Check duplicate */
    for (int i = 0; i < wf->count; i++) {
        if (wf->transitions[i].from == from && wf->transitions[i].to == to) return false;
    }
    wf->transitions[wf->count].from = from;
    wf->transitions[wf->count].to = to;
    wf->transitions[wf->count].allowed = true;
    wf->count++;
    return true;
}

bool workflow_is_valid(WorkflowEngine *wf, IssueStatus from, IssueStatus to) {
    if (from == to) return true; /* Same status allowed (e.g., comment update) */
    for (int i = 0; i < wf->count; i++) {
        if (wf->transitions[i].from == from && wf->transitions[i].to == to)
            return wf->transitions[i].allowed;
    }
    return false;
}

/* L3: Default workflow (simplified Jira workflow) */
void workflow_setup_default(WorkflowEngine *wf) {
    workflow_init(wf);
    /* OPEN transitions */
    workflow_add_transition(wf, STATUS_OPEN, STATUS_TODO);
    workflow_add_transition(wf, STATUS_OPEN, STATUS_CANCELLED);
    /* Transitions from TODO status */
    workflow_add_transition(wf, STATUS_TODO, STATUS_IN_PROGRESS);
    workflow_add_transition(wf, STATUS_TODO, STATUS_CANCELLED);
    /* IN_PROGRESS transitions */
    workflow_add_transition(wf, STATUS_IN_PROGRESS, STATUS_IN_REVIEW);
    workflow_add_transition(wf, STATUS_IN_PROGRESS, STATUS_BLOCKED);
    workflow_add_transition(wf, STATUS_IN_PROGRESS, STATUS_TODO);
    /* IN_REVIEW transitions */
    workflow_add_transition(wf, STATUS_IN_REVIEW, STATUS_IN_TESTING);
    workflow_add_transition(wf, STATUS_IN_REVIEW, STATUS_IN_PROGRESS);
    /* IN_TESTING transitions */
    workflow_add_transition(wf, STATUS_IN_TESTING, STATUS_DONE);
    workflow_add_transition(wf, STATUS_IN_TESTING, STATUS_IN_PROGRESS);
    /* DONE transitions */
    workflow_add_transition(wf, STATUS_DONE, STATUS_CLOSED);
    workflow_add_transition(wf, STATUS_DONE, STATUS_REOPENED);
    /* CLOSED transitions */
    workflow_add_transition(wf, STATUS_CLOSED, STATUS_REOPENED);
    /* REOPENED transitions */
    workflow_add_transition(wf, STATUS_REOPENED, STATUS_TODO);
    /* BLOCKED transitions */
    workflow_add_transition(wf, STATUS_BLOCKED, STATUS_IN_PROGRESS);
    workflow_add_transition(wf, STATUS_BLOCKED, STATUS_TODO);
}

/* ---- Query Functions ---- */
int tracker_issues_by_status(IssueTracker *tr, IssueStatus status, int *results, int max) {
    int count = 0;
    for (int i = 0; i < tr->issue_count && count < max; i++) {
        if (tr->issues[i].status == status) results[count++] = i;
    }
    return count;
}

int tracker_issues_by_assignee(IssueTracker *tr, const char *user, int *results, int max) {
    int count = 0;
    for (int i = 0; i < tr->issue_count && count < max; i++) {
        if (strcmp(tr->issues[i].assignee, user) == 0) results[count++] = i;
    }
    return count;
}

int tracker_issues_by_label(IssueTracker *tr, const char *label, int *results, int max) {
    int count = 0;
    for (int i = 0; i < tr->issue_count && count < max; i++) {
        for (int j = 0; j < tr->issues[i].label_count; j++) {
            if (strcmp(tr->issues[i].labels[j], label) == 0) {
                results[count++] = i;
                break;
            }
        }
    }
    return count;
}

int tracker_blocked_issues(IssueTracker *tr, int *results, int max) {
    return tracker_issues_by_status(tr, STATUS_BLOCKED, results, max);
}

int tracker_overdue_issues(IssueTracker *tr, int *results, int max) {
    int count = 0;
    uint32_t now = (uint32_t)time(NULL);
    for (int i = 0; i < tr->issue_count && count < max; i++) {
        if (tr->issues[i].due_date > 0 && tr->issues[i].due_date < now &&
            tr->issues[i].status != STATUS_DONE && tr->issues[i].status != STATUS_CLOSED) {
            results[count++] = i;
        }
    }
    return count;
}

void tracker_assign_sprint(IssueTracker *tr, const char *issue_id, int sprint_id) {
    int idx = tracker_find_issue(tr, issue_id);
    if (idx >= 0) tr->issues[idx].sprint_id = sprint_id;
}

void tracker_print_issue(IssueTracker *tr, const char *id) {
    int idx = tracker_find_issue(tr, id);
    if (idx < 0) { printf("Issue not found: %s\n", id); return; }
    Issue *is = &tr->issues[idx];
    printf("=== %s: %s ===\n", is->id, is->title);
    printf("  Type: %d | Status: %d | Priority: %d\n", is->type, is->status, is->priority);
    printf("  Points: %d | Est: %dh | Remaining: %dh | Logged: %dh\n",
           is->story_points, is->original_estimate_h, is->remaining_h, is->logged_h);
    printf("  Assignee: %s | Reporter: %s\n", is->assignee, is->reporter);
    if (is->label_count > 0) {
        printf("  Labels: ");
        for (int i = 0; i < is->label_count; i++) printf("[%s] ", is->labels[i]);
        printf("\n");
    }
    printf("  Comments: %d | Watchers: %d\n", is->comment_count, is->watcher_count);
}

void tracker_print_board(IssueTracker *tr) {
    printf("=== Issue Board ===\n");
    const char *status_names[] = {"OPEN","TODO","IN_PROGRESS","IN_REVIEW","IN_TESTING",
                                   "DONE","CLOSED","BLOCKED","CANCELLED","REOPENED"};
    for (int s = 0; s <= STATUS_REOPENED; s++) {
        int results[TRACKER_MAX_ISSUES];
        int cnt = tracker_issues_by_status(tr, (IssueStatus)s, results, TRACKER_MAX_ISSUES);
        if (cnt > 0) {
            printf("\n  [%s] (%d):\n", status_names[s], cnt);
            for (int i = 0; i < cnt; i++) {
                Issue *is = &tr->issues[results[i]];
                printf("    - %s: %s [P%d]\n", is->id, is->title, is->priority);
            }
        }
    }
}

void tracker_print_sprint_summary(IssueTracker *tr, int sprint_id) {
    int total = 0, done = 0, in_progress = 0;
    int total_pts = 0, done_pts = 0;
    for (int i = 0; i < tr->issue_count; i++) {
        if (tr->issues[i].sprint_id == sprint_id) {
            total++;
            total_pts += tr->issues[i].story_points;
            if (tr->issues[i].status == STATUS_DONE || tr->issues[i].status == STATUS_CLOSED) {
                done++;
                done_pts += tr->issues[i].story_points;
            } else if (tr->issues[i].status == STATUS_IN_PROGRESS) {
                in_progress++;
            }
        }
    }
    printf("=== Sprint %d Summary ===\n", sprint_id);
    printf("  Total: %d issues (%d pts)\n", total, total_pts);
    printf("  Done: %d (%d pts)\n", done, done_pts);
    printf("  In Progress: %d\n", in_progress);
    printf("  Completion: %.0f%%\n", total > 0 ? (double)done / total * 100 : 0);
}