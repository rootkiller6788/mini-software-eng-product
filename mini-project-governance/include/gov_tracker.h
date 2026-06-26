#ifndef GOV_TRACKER_H
#define GOV_TRACKER_H
#include <stdint.h>
#include <stdbool.h>
#include <time.h>

/* ============================================================
 * L1: Issue/Task Tracker — Jira-like Core Definitions
 * L3: Engineering Structure — Workflow State Machine
 * Reference: Jira, Linear, Asana task models
 * ============================================================ */

#define TRACKER_MAX_ISSUES     64
#define TRACKER_MAX_PROJECTS   8
#define TRACKER_MAX_EPICS      32
#define TRACKER_MAX_LABELS     8
#define TRACKER_NAME_LEN       64
#define TRACKER_DESC_LEN       256
#define TRACKER_ID_LEN         32
#define TRACKER_MAX_COMMENTS   16
#define TRACKER_COMMENT_LEN    128
#define TRACKER_MAX_WATCHERS   8
#define TRACKER_MAX_LINKS      8

/* ---- Issue Types ---- */
typedef enum {
    ISSUE_EPIC = 0,
    ISSUE_STORY,
    ISSUE_TASK,
    ISSUE_BUG,
    ISSUE_SUBTASK,
    ISSUE_RISK
} IssueType;

/* ---- Issue Status (workflow states) ---- */
typedef enum {
    STATUS_OPEN = 0,
    STATUS_TODO,
    STATUS_IN_PROGRESS,
    STATUS_IN_REVIEW,
    STATUS_IN_TESTING,
    STATUS_DONE,
    STATUS_CLOSED,
    STATUS_BLOCKED,
    STATUS_CANCELLED,
    STATUS_REOPENED
} IssueStatus;

/* ---- Priority ---- */
typedef enum {
    PRIORITY_TRIVIAL = 0,
    PRIORITY_MINOR,
    PRIORITY_MAJOR,
    PRIORITY_CRITICAL,
    PRIORITY_BLOCKER
} IssuePriority;

/* ---- Comment ---- */
typedef struct {
    char author[TRACKER_NAME_LEN];
    char body[TRACKER_COMMENT_LEN];
    uint32_t timestamp;
} IssueComment;

/* ---- Link ---- */
typedef enum { LINK_BLOCKS, LINK_RELATES, LINK_DUPLICATES, LINK_PARENT } LinkType;

typedef struct {
    char target_id[TRACKER_ID_LEN];
    LinkType type;
} IssueLink;

/* ---- Issue ---- */
typedef struct {
    char id[TRACKER_ID_LEN];
    char title[TRACKER_NAME_LEN];
    char description[TRACKER_DESC_LEN];
    IssueType type;
    IssueStatus status;
    IssuePriority priority;
    int story_points;
    int original_estimate_h;
    int remaining_h;
    int logged_h;
    char assignee[TRACKER_NAME_LEN];
    char reporter[TRACKER_NAME_LEN];
    int epic_id;
    int project_id;
    char labels[TRACKER_MAX_LABELS][TRACKER_NAME_LEN];
    int label_count;
    IssueComment comments[TRACKER_MAX_COMMENTS];
    int comment_count;
    char watchers[TRACKER_MAX_WATCHERS][TRACKER_NAME_LEN];
    int watcher_count;
    IssueLink links[TRACKER_MAX_LINKS];
    int link_count;
    int sprint_id;
    uint32_t created_at;
    uint32_t updated_at;
    uint32_t resolved_at;
    uint32_t due_date;
} Issue;

/* ---- Project ---- */
typedef struct {
    char id[TRACKER_ID_LEN];
    char name[TRACKER_NAME_LEN];
    char description[TRACKER_DESC_LEN];
    int issue_ids[TRACKER_MAX_ISSUES];
    int issue_count;
    uint32_t created_at;
} Project;

/* ---- Workflow Transition Rule (L3: State Machine) ---- */
typedef struct {
    IssueStatus from;
    IssueStatus to;
    bool allowed;
} WorkflowTransition;

#define TRANSITIONS_MAX 32

typedef struct {
    WorkflowTransition transitions[TRANSITIONS_MAX];
    int count;
} WorkflowEngine;

/* ---- Tracker Main ---- */
typedef struct {
    Issue issues[TRACKER_MAX_ISSUES];
    int issue_count;
    Project projects[TRACKER_MAX_PROJECTS];
    int project_count;
    Issue epics[TRACKER_MAX_EPICS];
    int epic_count;
    WorkflowEngine workflow;
    int next_id;
} IssueTracker;

/* ---- API ---- */
void tracker_init(IssueTracker *tr);
int  tracker_add_project(IssueTracker *tr, const char *id, const char *name, const char *desc);

int  tracker_add_issue(IssueTracker *tr, const char *id, const char *title,
                       const char *desc, IssueType type, IssuePriority priority,
                       int points, int estimate);
int  tracker_find_issue(IssueTracker *tr, const char *id);
bool tracker_update_status(IssueTracker *tr, const char *id, IssueStatus new_status);
bool tracker_assign(IssueTracker *tr, const char *id, const char *assignee);

/* Comments & watchers */
bool tracker_add_comment(IssueTracker *tr, const char *id, const char *author, const char *body);
bool tracker_add_watcher(IssueTracker *tr, const char *id, const char *user);
bool tracker_remove_watcher(IssueTracker *tr, const char *id, const char *user);

/* Labels & links */
bool tracker_add_label(IssueTracker *tr, const char *id, const char *label);
bool tracker_link_issues(IssueTracker *tr, const char *from, const char *to, LinkType type);

/* Workflow engine */
void workflow_init(WorkflowEngine *wf);
bool workflow_add_transition(WorkflowEngine *wf, IssueStatus from, IssueStatus to);
bool workflow_is_valid(WorkflowEngine *wf, IssueStatus from, IssueStatus to);
void workflow_setup_default(WorkflowEngine *wf);

/* Query */
int  tracker_issues_by_status(IssueTracker *tr, IssueStatus status, int *results, int max);
int  tracker_issues_by_assignee(IssueTracker *tr, const char *user, int *results, int max);
int  tracker_issues_by_label(IssueTracker *tr, const char *label, int *results, int max);
int  tracker_blocked_issues(IssueTracker *tr, int *results, int max);
int  tracker_overdue_issues(IssueTracker *tr, int *results, int max);

/* Sprint assignment */
void tracker_assign_sprint(IssueTracker *tr, const char *issue_id, int sprint_id);

/* Reports */
void tracker_print_issue(IssueTracker *tr, const char *id);
void tracker_print_board(IssueTracker *tr);
void tracker_print_sprint_summary(IssueTracker *tr, int sprint_id);
void tracker_print_burndown(IssueTracker *tr, int sprint_id, int total_days);

#endif
