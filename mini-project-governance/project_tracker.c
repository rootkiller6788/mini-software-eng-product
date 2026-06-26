#include "project_tracker.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static const char *issue_type_names[] = {"Story", "Task", "Bug", "Epic", "Subtask"};
static const char *issue_status_names[] = {
    "Open", "To Do", "In Progress", "In Review", "Testing",
    "Done", "Closed", "Reopened", "Blocked", "Cancelled"
};
static const char *issue_priority_names[] = {
    "Blocker", "Critical", "Major", "Minor", "Trivial"
};

const char *pt_issue_type_name(IssueType type)
{
    if (type >= 0 && type < (int)(sizeof(issue_type_names) / sizeof(issue_type_names[0])))
        return issue_type_names[type];
    return "Unknown";
}

const char *pt_issue_status_name(IssueStatus status)
{
    if (status >= 0 && status < (int)(sizeof(issue_status_names) / sizeof(issue_status_names[0])))
        return issue_status_names[status];
    return "Unknown";
}

const char *pt_issue_priority_name(IssuePriority priority)
{
    if (priority >= 0 && priority < (int)(sizeof(issue_priority_names) / sizeof(issue_priority_names[0])))
        return issue_priority_names[priority];
    return "Unknown";
}

ProjectTracker *pt_tracker_create(void)
{
    ProjectTracker *tracker = (ProjectTracker *)calloc(1, sizeof(ProjectTracker));
    if (!tracker) return NULL;
    tracker->issue_count = 0;
    tracker->next_issue_id = 1;
    tracker->label_count = 0;
    tracker->component_count = 0;
    tracker->sprint_metrics_count = 0;

    IssueStatus default_states[] = {
        ISSUE_STATUS_OPEN, ISSUE_STATUS_IN_PROGRESS, ISSUE_STATUS_DONE
    };
    pt_workflow_customize(tracker, "Default", default_states, 3);
    return tracker;
}

void pt_tracker_free(ProjectTracker *tracker)
{
    if (tracker) free(tracker);
}

int pt_issue_create(ProjectTracker *tracker, IssueType type,
                     const char *title, const char *description,
                     IssuePriority priority)
{
    if (!tracker || tracker->issue_count >= PT_MAX_ISSUES) return -1;

    ProjectIssue *issue = &tracker->issues[tracker->issue_count];
    issue->id = tracker->next_issue_id++;
    issue->type = type;
    strncpy(issue->title, title, PT_MAX_TITLE - 1);
    issue->title[PT_MAX_TITLE - 1] = '\0';
    strncpy(issue->description, description, PT_MAX_DESC - 1);
    issue->description[PT_MAX_DESC - 1] = '\0';
    issue->assignee_id = 0;
    issue->priority = priority;
    issue->status = ISSUE_STATUS_OPEN;
    issue->sprint_id = 0;
    issue->parent_id = 0;
    issue->story_points = 0;
    issue->label_count = 0;
    issue->component_count = 0;
    issue->created_at = time(NULL);
    issue->updated_at = time(NULL);
    issue->resolved_at = 0;
    issue->epic_id = 0;
    issue->blocked = 0;
    issue->block_reason[0] = '\0';
    tracker->issue_count++;
    return issue->id;
}

int pt_issue_delete(ProjectTracker *tracker, int issue_id)
{
    if (!tracker) return -1;
    for (int i = 0; i < tracker->issue_count; i++) {
        if (tracker->issues[i].id == issue_id) {
            if (i < tracker->issue_count - 1) {
                memmove(&tracker->issues[i], &tracker->issues[i + 1],
                        (tracker->issue_count - i - 1) * sizeof(ProjectIssue));
            }
            tracker->issue_count--;
            return 0;
        }
    }
    return -1;
}

int pt_issue_update(ProjectTracker *tracker, int issue_id,
                     IssueStatus new_status)
{
    ProjectIssue *issue = pt_issue_find(tracker, issue_id);
    if (!issue) return -1;
    issue->status = new_status;
    issue->updated_at = time(NULL);
    if (new_status == ISSUE_STATUS_DONE || new_status == ISSUE_STATUS_CLOSED) {
        issue->resolved_at = time(NULL);
    }
    return 0;
}

int pt_issue_assign(ProjectTracker *tracker, int issue_id, int assignee_id)
{
    ProjectIssue *issue = pt_issue_find(tracker, issue_id);
    if (!issue) return -1;
    issue->assignee_id = assignee_id;
    issue->updated_at = time(NULL);
    return 0;
}

int pt_issue_set_sprint(ProjectTracker *tracker, int issue_id, int sprint_id)
{
    ProjectIssue *issue = pt_issue_find(tracker, issue_id);
    if (!issue) return -1;
    issue->sprint_id = sprint_id;
    issue->updated_at = time(NULL);
    return 0;
}

int pt_issue_set_points(ProjectTracker *tracker, int issue_id, int points)
{
    ProjectIssue *issue = pt_issue_find(tracker, issue_id);
    if (!issue) return -1;
    issue->story_points = points;
    issue->updated_at = time(NULL);
    return 0;
}

int pt_issue_block(ProjectTracker *tracker, int issue_id, const char *reason)
{
    ProjectIssue *issue = pt_issue_find(tracker, issue_id);
    if (!issue) return -1;
    issue->blocked = 1;
    issue->status = ISSUE_STATUS_BLOCKED;
    strncpy(issue->block_reason, reason, PT_MAX_DESC - 1);
    issue->block_reason[PT_MAX_DESC - 1] = '\0';
    issue->updated_at = time(NULL);
    return 0;
}

int pt_issue_unblock(ProjectTracker *tracker, int issue_id)
{
    ProjectIssue *issue = pt_issue_find(tracker, issue_id);
    if (!issue) return -1;
    issue->blocked = 0;
    issue->block_reason[0] = '\0';
    issue->status = ISSUE_STATUS_REOPENED;
    issue->updated_at = time(NULL);
    return 0;
}

ProjectIssue *pt_issue_find(ProjectTracker *tracker, int issue_id)
{
    if (!tracker) return NULL;
    for (int i = 0; i < tracker->issue_count; i++) {
        if (tracker->issues[i].id == issue_id) return &tracker->issues[i];
    }
    return NULL;
}

int pt_label_create(ProjectTracker *tracker, const char *name, int color)
{
    if (!tracker || tracker->label_count >= 64) return -1;
    ProjectLabel *label = &tracker->labels[tracker->label_count];
    strncpy(label->name, name, PT_MAX_LABEL - 1);
    label->name[PT_MAX_LABEL - 1] = '\0';
    label->color = color;
    tracker->label_count++;
    return tracker->label_count - 1;
}

int pt_label_assign_issue(ProjectTracker *tracker, int issue_id, int label_id)
{
    ProjectIssue *issue = pt_issue_find(tracker, issue_id);
    if (!issue || label_id < 0 || label_id >= tracker->label_count) return -1;
    if (issue->label_count >= PT_MAX_LABELS_PER_ISSUE) return -1;
    issue->labels[issue->label_count++] = label_id;
    return 0;
}

int pt_component_create(ProjectTracker *tracker, const char *name,
                         const char *description)
{
    if (!tracker || tracker->component_count >= 32) return -1;
    ProjectComponent *comp = &tracker->components[tracker->component_count];
    strncpy(comp->name, name, PT_MAX_COMPONENT - 1);
    comp->name[PT_MAX_COMPONENT - 1] = '\0';
    strncpy(comp->description, description, PT_MAX_DESC - 1);
    comp->description[PT_MAX_DESC - 1] = '\0';
    comp->lead_id = 0;
    tracker->component_count++;
    return tracker->component_count - 1;
}

int pt_component_assign_issue(ProjectTracker *tracker, int issue_id,
                               int component_id)
{
    ProjectIssue *issue = pt_issue_find(tracker, issue_id);
    if (!issue || component_id < 0 || component_id >= tracker->component_count)
        return -1;
    if (issue->component_count >= PT_MAX_COMPONENTS_PER_ISSUE) return -1;
    issue->components[issue->component_count++] = component_id;
    return 0;
}

int pt_workflow_customize(ProjectTracker *tracker, const char *name,
                           const IssueStatus *states, int state_count)
{
    if (!tracker || !states || state_count > PT_MAX_WORKFLOW_STATES) return -1;
    ProjectWorkflow *wf = &tracker->workflow;
    strncpy(wf->name, name, PT_MAX_WORKFLOW_NAME - 1);
    wf->name[PT_MAX_WORKFLOW_NAME - 1] = '\0';
    wf->state_count = state_count;
    memcpy(wf->states, states, state_count * sizeof(IssueStatus));
    for (int i = 0; i < PT_MAX_WORKFLOW_STATES; i++) {
        for (int j = 0; j < PT_MAX_WORKFLOW_STATES; j++) {
            wf->transitions[i][j] = 0;
        }
    }
    for (int i = 0; i < state_count - 1; i++) {
        wf->transitions[i][i + 1] = 1;
    }
    return 0;
}

int pt_workflow_add_transition(ProjectTracker *tracker,
                                IssueStatus from, IssueStatus to)
{
    if (!tracker) return -1;
    if (from >= PT_MAX_WORKFLOW_STATES || to >= PT_MAX_WORKFLOW_STATES) return -1;
    tracker->workflow.transitions[from][to] = 1;
    return 0;
}

static int match_single(const ProjectIssue *issue, const FilterCondition *cond)
{
    if (!issue || !cond) return 0;
    if (strcmp(cond->field, "type") == 0) {
        int val = issue->type;
        int cmp = atoi(cond->value);
        switch (cond->op) {
            case FILTER_EQ: return val == cmp;
            case FILTER_NEQ: return val != cmp;
            default: return 0;
        }
    }
    if (strcmp(cond->field, "status") == 0) {
        int val = issue->status;
        int cmp = atoi(cond->value);
        switch (cond->op) {
            case FILTER_EQ: return val == cmp;
            case FILTER_NEQ: return val != cmp;
            default: return 0;
        }
    }
    if (strcmp(cond->field, "priority") == 0) {
        int val = issue->priority;
        int cmp = atoi(cond->value);
        switch (cond->op) {
            case FILTER_EQ: return val == cmp;
            case FILTER_GT: return val > cmp;
            case FILTER_LT: return val < cmp;
            default: return 0;
        }
    }
    if (strcmp(cond->field, "assignee") == 0) {
        int val = issue->assignee_id;
        int cmp = atoi(cond->value);
        switch (cond->op) {
            case FILTER_EQ: return val == cmp;
            case FILTER_NEQ: return val != cmp;
            default: return 0;
        }
    }
    if (strcmp(cond->field, "sprint") == 0) {
        int val = issue->sprint_id;
        int cmp = atoi(cond->value);
        switch (cond->op) {
            case FILTER_EQ: return val == cmp;
            case FILTER_NEQ: return val != cmp;
            default: return 0;
        }
    }
    if (strcmp(cond->field, "title") == 0) {
        if (cond->op == FILTER_CONTAINS)
            return strstr(issue->title, cond->value) != NULL;
        if (cond->op == FILTER_NOT_CONTAINS)
            return strstr(issue->title, cond->value) == NULL;
    }
    if (strcmp(cond->field, "blocked") == 0) {
        if (cond->op == FILTER_EQ) return issue->blocked == atoi(cond->value);
        if (cond->op == FILTER_IS_EMPTY) return !issue->blocked;
        if (cond->op == FILTER_IS_NOT_EMPTY) return issue->blocked;
    }
    return 0;
}

int pt_filter_jql(ProjectTracker *tracker, const FilterQuery *query,
                   int *result_ids, int max_results)
{
    if (!tracker || !query || !result_ids) return 0;
    int count = 0;
    for (int i = 0; i < tracker->issue_count && count < max_results; i++) {
        int match = 1;
        for (int j = 0; j < query->condition_count; j++) {
            if (!match_single(&tracker->issues[i], &query->conditions[j])) {
                match = 0;
                break;
            }
        }
        if (match) {
            result_ids[count++] = tracker->issues[i].id;
        }
    }
    return count;
}

int pt_board_view(const ProjectTracker *tracker)
{
    if (!tracker) return -1;
    printf("=== Project Board ===\n");
    printf("Issues: %d\n", tracker->issue_count);
    for (int s = 0; s < (int)(sizeof(issue_status_names) / sizeof(issue_status_names[0])); s++) {
        int count = 0;
        for (int i = 0; i < tracker->issue_count; i++) {
            if (tracker->issues[i].status == (IssueStatus)s) count++;
        }
        if (count > 0) {
            printf("  %s: %d\n", issue_status_names[s], count);
        }
    }
    return 0;
}

int pt_list_view(const ProjectTracker *tracker, IssueStatus filter_status)
{
    if (!tracker) return -1;
    for (int i = 0; i < tracker->issue_count; i++) {
        if (tracker->issues[i].status == filter_status ||
            filter_status == (IssueStatus)(-1)) {
            pt_issue_print(&tracker->issues[i]);
        }
    }
    return 0;
}

SprintMetrics pt_sprint_metrics_calculate(ProjectTracker *tracker,
                                           int sprint_id)
{
    SprintMetrics metrics = {0};
    metrics.sprint_id = sprint_id;
    if (!tracker) return metrics;
    for (int i = 0; i < tracker->issue_count; i++) {
        if (tracker->issues[i].sprint_id == sprint_id) {
            metrics.total_issues++;
            metrics.story_points_planned += tracker->issues[i].story_points;
            if (tracker->issues[i].status == ISSUE_STATUS_DONE ||
                tracker->issues[i].status == ISSUE_STATUS_CLOSED) {
                metrics.completed_issues++;
                metrics.story_points_done += tracker->issues[i].story_points;
            }
            if (tracker->issues[i].type == ISSUE_TYPE_BUG) {
                metrics.bugs_found++;
                if (tracker->issues[i].status == ISSUE_STATUS_DONE ||
                    tracker->issues[i].status == ISSUE_STATUS_CLOSED) {
                    metrics.bugs_fixed++;
                }
            }
        }
    }
    if (metrics.total_issues > 0) {
        metrics.completion_rate =
            (double)metrics.completed_issues / metrics.total_issues;
    }
    return metrics;
}

void pt_issue_print(const ProjectIssue *issue)
{
    if (!issue) return;
    printf("[%s-%d] %s\n", pt_issue_type_name(issue->type), issue->id, issue->title);
    printf("  Status: %s | Priority: %s\n",
           pt_issue_status_name(issue->status),
           pt_issue_priority_name(issue->priority));
    if (issue->blocked) printf("  *** BLOCKED: %s ***\n", issue->block_reason);
}

void pt_workflow_print(const ProjectWorkflow *workflow)
{
    if (!workflow) return;
    printf("Workflow: %s (%d states)\n", workflow->name, workflow->state_count);
    for (int i = 0; i < workflow->state_count; i++) {
        printf("  [%d] %s\n", i, pt_issue_status_name(workflow->states[i]));
    }
}

void pt_sprint_metrics_print(const SprintMetrics *metrics)
{
    if (!metrics) return;
    printf("Sprint #%d Metrics:\n", metrics->sprint_id);
    printf("  Issues: %d total / %d completed (%.1f%%)\n",
           metrics->total_issues, metrics->completed_issues,
           metrics->completion_rate * 100);
    printf("  Story Points: %d planned / %d done\n",
           metrics->story_points_planned, metrics->story_points_done);
    printf("  Bugs: %d found / %d fixed\n",
           metrics->bugs_found, metrics->bugs_fixed);
}

void pt_board_print(const ProjectTracker *tracker)
{
    pt_board_view(tracker);
}
