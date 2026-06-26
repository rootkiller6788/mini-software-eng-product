#ifndef GOV_SCRUM_H
#define GOV_SCRUM_H
#include <stdint.h>
#include <stdbool.h>
#include <time.h>

/* ============================================================
 * L1: Scrum Framework — Core Definitions
 * Reference: Scrum Guide (Schwaber & Sutherland)
 * ============================================================ */

#define SCRUM_MAX_SPRINTS      32
#define SCRUM_MAX_TEAM_MEMBERS 16
#define SCRUM_MAX_PBI          128
#define SCRUM_NAME_LEN         64
#define SCRUM_DESC_LEN         256
#define SCRUM_ID_LEN           32

/* ---- Scrum Roles ---- */
typedef enum {
    ROLE_SCRUM_MASTER = 0,
    ROLE_PRODUCT_OWNER,
    ROLE_DEVELOPER,
    ROLE_STAKEHOLDER
} ScrumRole;

typedef struct {
    char name[SCRUM_NAME_LEN];
    ScrumRole role;
    int capacity_pct;
    double hourly_cost;
    bool active;
} TeamMember;

typedef struct {
    TeamMember members[SCRUM_MAX_TEAM_MEMBERS];
    int count;
    int total_capacity;
} ScrumTeam;

/* ---- Product Backlog Item ---- */
typedef enum {
    PBI_EPIC = 0,
    PBI_FEATURE,
    PBI_USER_STORY,
    PBI_TASK,
    PBI_BUG,
    PBI_SPIKE,
    PBI_TECH_DEBT
} PBIType;

typedef enum {
    PBI_NEW = 0,
    PBI_REFINED,
    PBI_READY,
    PBI_IN_PROGRESS,
    PBI_DONE,
    PBI_ACCEPTED,
    PBI_BLOCKED
} PBIState;

typedef struct {
    char id[SCRUM_ID_LEN];
    char title[SCRUM_NAME_LEN];
    char description[SCRUM_DESC_LEN];
    PBIType type;
    PBIState state;
    int story_points;
    int business_value;
    int effort_hours;
    int actual_hours;
    int priority;
    double wsjf_score;
    int sprint_number;
    int dependencies[8];
    int dep_count;
    uint32_t created_at;
    uint32_t completed_at;
} ProductBacklogItem;

typedef struct {
    ProductBacklogItem items[SCRUM_MAX_PBI];
    int count;
    int total_points;
    int total_value;
} ProductBacklog;

/* ---- Sprint ---- */
typedef enum {
    SPRINT_PLANNING = 0,
    SPRINT_ACTIVE,
    SPRINT_REVIEW,
    SPRINT_RETROSPECTIVE,
    SPRINT_CLOSED
} SprintState;

typedef struct {
    int sprint_number;
    char goal[SCRUM_DESC_LEN];
    SprintState state;
    uint32_t start_date;
    uint32_t end_date;
    int duration_days;
    int planned_points;
    int completed_points;
    int planned_items;
    int completed_items;
    int blocked_count;
    double velocity;
    double burndown_ideal[30];
    double burndown_actual[30];
    int burndown_days;
    bool goal_met;
} Sprint;

/* ---- Scrum Board ---- */
typedef struct {
    Sprint sprints[SCRUM_MAX_SPRINTS];
    int sprint_count;
    int current_sprint;
    ProductBacklog backlog;
    ScrumTeam team;
    double avg_velocity;
    double velocity_stdev;
    int velocity_samples;
} ScrumBoard;

/* ---- API ---- */
void scrum_init(ScrumBoard *sb);
void scrum_team_init(ScrumTeam *t);
int  scrum_add_member(ScrumTeam *t, const char *name, ScrumRole role, int capacity);
void scrum_team_capacity(ScrumTeam *t);

int  pbi_add(ProductBacklog *pb, const char *id, const char *title,
             PBIType type, int points, int value, int hours);
bool pbi_move_state(ProductBacklog *pb, const char *id, PBIState new_state);
int  pbi_find(ProductBacklog *pb, const char *id);
void pbi_sort_by_priority(ProductBacklog *pb);
void pbi_sort_by_wsjf(ProductBacklog *pb);
void pbi_calc_wsjf(ProductBacklog *pb);
void pbi_add_dependency(ProductBacklog *pb, const char *from, const char *to);

int  sprint_create(ScrumBoard *sb, const char *goal, int duration_days);
bool sprint_start(ScrumBoard *sb, int sprint_idx);
bool sprint_finish(ScrumBoard *sb, int sprint_idx);
bool sprint_plan(ScrumBoard *sb, int sprint_idx);
void sprint_record_burndown(Sprint *sp, int day, double remaining);
void sprint_calc_velocity(Sprint *sp);
int  sprint_load_items(ScrumBoard *sb, int sprint_idx, int max_points);

int  scrum_wip_count(ScrumBoard *sb);
void scrum_print_sprint_report(ScrumBoard *sb, int sprint_idx);
void scrum_print_velocity_chart(ScrumBoard *sb);
double scrum_predict_completion(ScrumBoard *sb, int remaining_points);

#endif
