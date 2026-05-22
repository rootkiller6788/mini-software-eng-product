#ifndef PROJECT_TRACKER_H
#define PROJECT_TRACKER_H

#include <stddef.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PT_MAX_ISSUES 4096
#define PT_MAX_TITLE 256
#define PT_MAX_DESC 2048
#define PT_MAX_ASSIGNEE 128
#define PT_MAX_LABEL 64
#define PT_MAX_COMPONENT 64
#define PT_MAX_WORKFLOW_STATES 16
#define PT_MAX_WORKFLOW_NAME 128
#define PT_MAX_FILTER_CONDITIONS 32
#define PT_MAX_LABELS_PER_ISSUE 8
#define PT_MAX_COMPONENTS_PER_ISSUE 4
#define PT_MAX_SPRINTS 64

typedef enum {
    ISSUE_TYPE_STORY = 0,
    ISSUE_TYPE_TASK,
    ISSUE_TYPE_BUG,
    ISSUE_TYPE_EPIC,
    ISSUE_TYPE_SUBTASK
} IssueType;

typedef enum {
    ISSUE_STATUS_OPEN = 0,
    ISSUE_STATUS_TODO,
    ISSUE_STATUS_IN_PROGRESS,
    ISSUE_STATUS_IN_REVIEW,
    ISSUE_STATUS_TESTING,
    ISSUE_STATUS_DONE,
    ISSUE_STATUS_CLOSED,
    ISSUE_STATUS_REOPENED,
    ISSUE_STATUS_BLOCKED,
    ISSUE_STATUS_CANCELLED
} IssueStatus;

typedef enum {
    ISSUE_PRIORITY_BLOCKER = 0,
    ISSUE_PRIORITY_CRITICAL,
    ISSUE_PRIORITY_MAJOR,
    ISSUE_PRIORITY_MINOR,
    ISSUE_PRIORITY_TRIVIAL
} IssuePriority;

typedef enum {
    FILTER_EQ = 0,
    FILTER_NEQ,
    FILTER_GT,
    FILTER_LT,
    FILTER_GTE,
    FILTER_LTE,
    FILTER_IN,
    FILTER_CONTAINS,
    FILTER_NOT_CONTAINS,
    FILTER_IS_EMPTY,
    FILTER_IS_NOT_EMPTY
} FilterOperator;

typedef struct {
    char name[PT_MAX_LABEL];
    int color; /* 0xRRGGBB */
} ProjectLabel;

typedef struct {
    char name[PT_MAX_COMPONENT];
    char description[PT_MAX_DESC];
    int lead_id;
} ProjectComponent;

typedef struct {
    int id;
    IssueType type;
    char title[PT_MAX_TITLE];
    char description[PT_MAX_DESC];
    int assignee_id;
    IssuePriority priority;
    IssueStatus status;
    int sprint_id;
    int parent_id;  /* parent epic/story for subtasks */
    int story_points;
    int labels[PT_MAX_LABELS_PER_ISSUE];
    int label_count;
    int components[PT_MAX_COMPONENTS_PER_ISSUE];
    int component_count;
    time_t created_at;
    time_t updated_at;
    time_t resolved_at;
    int epic_id;    /* epic this issue belongs to */
    int blocked;
    char block_reason[PT_MAX_DESC];
} ProjectIssue;

typedef struct {
    char name[PT_MAX_WORKFLOW_NAME];
    IssueStatus states[PT_MAX_WORKFLOW_STATES];
    int state_count;
    int transitions[PT_MAX_WORKFLOW_STATES][PT_MAX_WORKFLOW_STATES];
} ProjectWorkflow;

typedef struct {
    char field[32];
    FilterOperator op;
    char value[PT_MAX_TITLE];
} FilterCondition;

typedef struct {
    FilterCondition conditions[PT_MAX_FILTER_CONDITIONS];
    int condition_count;
} FilterQuery;

typedef struct {
    int sprint_id;
    int total_issues;
    int completed_issues;
    int story_points_planned;
    int story_points_done;
    double completion_rate;
    int bugs_found;
    int bugs_fixed;
    double avg_cycle_time_days;
} SprintMetrics;

typedef struct {
    ProjectIssue issues[PT_MAX_ISSUES];
    int issue_count;
    int next_issue_id;
    ProjectLabel labels[64];
    int label_count;
    ProjectComponent components[32];
    int component_count;
    ProjectWorkflow workflow;
    SprintMetrics sprint_metrics[PT_MAX_SPRINTS];
    int sprint_metrics_count;
} ProjectTracker;

ProjectTracker *pt_tracker_create(void);
void pt_tracker_free(ProjectTracker *tracker);

int pt_issue_create(ProjectTracker *tracker, IssueType type,
                     const char *title, const char *description,
                     IssuePriority priority);
int pt_issue_delete(ProjectTracker *tracker, int issue_id);
int pt_issue_update(ProjectTracker *tracker, int issue_id,
                     IssueStatus new_status);
int pt_issue_assign(ProjectTracker *tracker, int issue_id, int assignee_id);
int pt_issue_set_sprint(ProjectTracker *tracker, int issue_id, int sprint_id);
int pt_issue_set_points(ProjectTracker *tracker, int issue_id, int points);
int pt_issue_block(ProjectTracker *tracker, int issue_id, const char *reason);
int pt_issue_unblock(ProjectTracker *tracker, int issue_id);
ProjectIssue *pt_issue_find(ProjectTracker *tracker, int issue_id);

int pt_label_create(ProjectTracker *tracker, const char *name, int color);
int pt_label_assign_issue(ProjectTracker *tracker, int issue_id, int label_id);

int pt_component_create(ProjectTracker *tracker, const char *name,
                         const char *description);
int pt_component_assign_issue(ProjectTracker *tracker, int issue_id,
                               int component_id);

int pt_workflow_customize(ProjectTracker *tracker, const char *name,
                           const IssueStatus *states, int state_count);
int pt_workflow_add_transition(ProjectTracker *tracker,
                                IssueStatus from, IssueStatus to);

int pt_filter_jql(ProjectTracker *tracker, const FilterQuery *query,
                   int *result_ids, int max_results);

int pt_board_view(const ProjectTracker *tracker);
int pt_list_view(const ProjectTracker *tracker, IssueStatus filter_status);

SprintMetrics pt_sprint_metrics_calculate(ProjectTracker *tracker,
                                           int sprint_id);

void pt_issue_print(const ProjectIssue *issue);
void pt_workflow_print(const ProjectWorkflow *workflow);
void pt_sprint_metrics_print(const SprintMetrics *metrics);
void pt_board_print(const ProjectTracker *tracker);

const char *pt_issue_type_name(IssueType type);
const char *pt_issue_status_name(IssueStatus status);
const char *pt_issue_priority_name(IssuePriority priority);

#ifdef __cplusplus
}
#endif

#endif
