#ifndef AGILE_SCRUM_H
#define AGILE_SCRUM_H

#include <stddef.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SCRUM_MAX_SPRINT_NAME 128
#define SCRUM_MAX_GOAL_TEXT 512
#define SCRUM_MAX_ITEM_TITLE 256
#define SCRUM_MAX_ITEM_DESC 1024
#define SCRUM_MAX_PARTICIPANT_NAME 128
#define SCRUM_MAX_BLOCKER_TEXT 512
#define SCRUM_MAX_FEEDBACK_TEXT 1024
#define SCRUM_MAX_ACTION_ITEM 256
#define SCRUM_MAX_BACKLOG_ITEMS 1024
#define SCRUM_MAX_SPRINT_ITEMS 256
#define SCRUM_MAX_STANDUP_ENTRIES 512
#define SCRUM_MAX_RETRO_ITEMS 128

typedef enum {
    SCRUM_STATUS_PLANNING = 0,
    SCRUM_STATUS_ACTIVE,
    SCRUM_STATUS_COMPLETED,
    SCRUM_STATUS_CANCELLED
} ScrumSprintStatus;

typedef enum {
    SCRUM_ITEM_TODO = 0,
    SCRUM_ITEM_IN_PROGRESS,
    SCRUM_ITEM_DONE,
    SCRUM_ITEM_REMOVED
} ScrumItemStatus;

typedef enum {
    SCRUM_PRIORITY_CRITICAL = 1,
    SCRUM_PRIORITY_HIGH = 2,
    SCRUM_PRIORITY_MEDIUM = 3,
    SCRUM_PRIORITY_LOW = 4
} ScrumPriority;

typedef struct {
    int id;
    char title[SCRUM_MAX_ITEM_TITLE];
    char description[SCRUM_MAX_ITEM_DESC];
    int story_points;
    ScrumPriority priority;
    ScrumItemStatus status;
    int assignee_id;
    time_t created_at;
    time_t updated_at;
} ScrumBacklogItem;

typedef struct {
    int id;
    char name[SCRUM_MAX_SPRINT_NAME];
    char goal[SCRUM_MAX_GOAL_TEXT];
    time_t start_date;
    time_t end_date;
    int duration_days;
    ScrumSprintStatus status;
    int story_points_planned;
    int story_points_completed;
    int backlog_item_ids[SCRUM_MAX_SPRINT_ITEMS];
    int backlog_item_count;
    double velocity;
} ScrumSprint;

typedef struct {
    int sprint_id;
    int participant_id;
    char participant_name[SCRUM_MAX_PARTICIPANT_NAME];
    time_t standup_date;
    char yesterday[SCRUM_MAX_ITEM_DESC];
    char today[SCRUM_MAX_ITEM_DESC];
    char blockers[SCRUM_MAX_BLOCKER_TEXT];
} ScrumStandup;

typedef struct {
    int sprint_id;
    time_t review_date;
    char demo_notes[SCRUM_MAX_ITEM_DESC];
    char stakeholder_feedback[SCRUM_MAX_FEEDBACK_TEXT];
    int items_accepted;
    int items_rejected;
    int items_total;
} ScrumSprintReview;

typedef struct {
    char item[SCRUM_MAX_ACTION_ITEM];
    int category;
    int action_item_id;
} ScrumRetroItem;

typedef struct {
    int sprint_id;
    time_t retro_date;
    ScrumRetroItem items[SCRUM_MAX_RETRO_ITEMS];
    int item_count;
    int action_item_ids[SCRUM_MAX_RETRO_ITEMS];
    int action_item_count;
} ScrumRetrospective;

typedef struct {
    int sprint_id;
    int day_index;
    int points_remaining;
} ScrumBurndownPoint;

typedef struct {
    int sprint_id;
    double avg_velocity;
    double last_sprint_velocity;
    int total_sprints;
    int total_points_completed;
    int total_points_planned;
} ScrumVelocityReport;

typedef struct {
    ScrumBacklogItem *items;
    int count;
    int capacity;
} ScrumBacklog;

typedef struct {
    ScrumBacklogItem items[SCRUM_MAX_BACKLOG_ITEMS];
    int count;
} ScrumProductBacklog;

typedef struct {
    int sprint_count;
    ScrumSprint sprints[64];
    ScrumProductBacklog product_backlog;
    ScrumStandup standup_log[SCRUM_MAX_STANDUP_ENTRIES];
    int standup_count;
    double team_velocity_avg;
} ScrumContext;

ScrumContext *scrum_context_create(void);
void scrum_context_free(ScrumContext *ctx);

ScrumSprint *scrum_sprint_create(ScrumContext *ctx, const char *name,
                                  const char *goal, int duration_days);
int scrum_sprint_start(ScrumContext *ctx, int sprint_id);
int scrum_sprint_complete(ScrumContext *ctx, int sprint_id);

int scrum_backlog_add(ScrumContext *ctx, const char *title, const char *desc,
                       int story_points, ScrumPriority priority);
int scrum_backlog_remove(ScrumContext *ctx, int item_id);
int scrum_backlog_refine(ScrumContext *ctx, const char *output_path);

int scrum_sprint_plan(ScrumContext *ctx, int sprint_id,
                       const int *item_ids, int item_count);
ScrumSprint *scrum_sprint_find(ScrumContext *ctx, int sprint_id);

int scrum_standup_record(ScrumContext *ctx, int sprint_id, int participant_id,
                          const char *name, const char *yesterday,
                          const char *today, const char *blockers);

ScrumSprintReview *scrum_review_conduct(ScrumContext *ctx, int sprint_id,
                                         const char *demo_notes,
                                         const char *feedback,
                                         int accepted, int rejected);

ScrumRetrospective *scrum_retrospective_create(ScrumContext *ctx, int sprint_id);
int scrum_retrospective_add_item(ScrumRetrospective *retro,
                                  const char *text, int category);
int scrum_retrospective_add_action(ScrumRetrospective *retro, int action_id);

ScrumVelocityReport scrum_velocity_calculate(const ScrumContext *ctx);

int scrum_burndown_generate(const ScrumContext *ctx, int sprint_id,
                             ScrumBurndownPoint *points, int max_points);
int scrum_burnup_generate(const ScrumContext *ctx, int sprint_id,
                           int *scope_points, int *completed_points,
                           int max_points);

double scrum_sprint_health_score(const ScrumContext *ctx, int sprint_id);

void scrum_sprint_print(const ScrumSprint *sprint);
void scrum_backlog_print(const ScrumContext *ctx);
void scrum_standup_print(const ScrumStandup *standup);
void scrum_velocity_print(const ScrumVelocityReport *report);

#ifdef __cplusplus
}
#endif

#endif
