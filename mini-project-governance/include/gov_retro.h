#ifndef GOV_RETRO_H
#define GOV_RETRO_H
#include <stdint.h>
#include <stdbool.h>
#include <time.h>

/* ============================================================
 * L1: Retrospective System — Core Definitions
 * Reference: Agile Retrospectives (Derby & Larsen)
 * L3: Structured Retrospective Formats
 * ============================================================ */

#define RETRO_MAX_SPRINTS      16
#define RETRO_MAX_ITEMS        64
#define RETRO_MAX_ACTIONS      32
#define RETRO_MAX_PARTICIPANTS 16
#define RETRO_NAME_LEN         64
#define RETRO_DESC_LEN         256
#define RETRO_ID_LEN           32

/* ---- Retrospective Formats ---- */
typedef enum {
    RETRO_START_STOP_CONTINUE = 0,
    RETRO_GLAD_SAD_MAD,
    RETRO_4L,              /* Liked, Learned, Lacked, Longed for */
    RETRO_SAILBOAT,        /* Wind, Anchor, Island, Rocks */
    RETRO_KALM,             /* Keep, Add, Less, More */
    RETRO_DROWNED_SAVED,
    RETRO_5WHYS
} RetroFormat;

/* ---- Retro Item Category ---- */
typedef enum {
    RETRO_CAT_WHAT_WENT_WELL = 0,
    RETRO_CAT_WHAT_WENT_WRONG,
    RETRO_CAT_WHAT_TO_IMPROVE,
    RETRO_CAT_APPRECIATION,
    RETRO_CAT_ACTION_ITEM,
    RETRO_CAT_PUZZLE           /* things we don't understand */
} RetroCategory;

/* ---- Retro Item ---- */
typedef struct {
    char id[RETRO_ID_LEN];
    char content[RETRO_DESC_LEN];
    char author[RETRO_NAME_LEN];
    RetroCategory category;
    int votes;                 /* dot-voting */
    bool discussed;
    bool has_action_item;
    int action_item_id;        /* -1 if none */
} RetroItem;

/* ---- Action Item ---- */
typedef enum {
    ACTION_OPEN = 0,
    ACTION_IN_PROGRESS,
    ACTION_DONE,
    ACTION_BLOCKED,
    ACTION_ABANDONED
} ActionItemStatus;

typedef struct {
    char id[RETRO_ID_LEN];
    char description[RETRO_DESC_LEN];
    char owner[RETRO_NAME_LEN];
    ActionItemStatus status;
    int sprint_created;
    int sprint_completed;
    uint32_t created_at;
    uint32_t due_date;
    uint32_t completed_at;
    int retry_count;           /* number of times reopened */
    double impact_score;       /* 1-10, team-estimated */
    double effort_score;       /* 1-10 */
} ActionItem;

/* ---- Team Health Check ---- */
typedef struct {
    char participant[RETRO_NAME_LEN];
    double happiness;          /* 1-5 scale */
    double engagement;
    double clarity;
    double autonomy;
    double mastery;
    double team_trust;
    uint32_t timestamp;
} HealthCheck;

/* ---- Retrospective Session ---- */
typedef struct {
    int sprint_number;
    char sprint_goal[RETRO_DESC_LEN];
    RetroFormat format;
    RetroItem items[RETRO_MAX_ITEMS];
    int item_count;
    ActionItem actions[RETRO_MAX_ACTIONS];
    int action_count;
    HealthCheck health_checks[RETRO_MAX_PARTICIPANTS];
    int health_count;
    double avg_happiness;
    double avg_engagement;
    int total_votes_cast;
    uint32_t session_date;
    bool completed;
    char facilitator[RETRO_NAME_LEN];
} RetroSession;

/* ---- Retrospective Log ---- */
typedef struct {
    RetroSession sessions[RETRO_MAX_SPRINTS];
    int session_count;
    ActionItem open_actions[RETRO_MAX_ACTIONS];
    int open_action_count;
    double team_happiness_trend[RETRO_MAX_SPRINTS];
    int trend_points;
} RetroLog;

/* ---- API ---- */
void retro_init(RetroLog *log);
int  retro_create_session(RetroLog *log, int sprint_number, const char *goal,
                          RetroFormat format, const char *facilitator);

int  retro_add_item(RetroLog *log, int session_idx, const char *content,
                    const char *author, RetroCategory cat);
void retro_vote(RetroLog *log, int session_idx, int item_idx);
int  retro_top_voted(RetroLog *log, int session_idx, int *results, int max);

int  retro_add_action(RetroLog *log, int session_idx, const char *desc,
                      const char *owner, uint32_t due_date, double impact, double effort);
bool retro_complete_action(RetroLog *log, const char *action_id);
bool retro_reopen_action(RetroLog *log, const char *action_id);
int  retro_pending_actions(RetroLog *log, int *results, int max);

/* Health checks */
void retro_add_health(RetroLog *log, int session_idx, const char *participant,
                      double happiness, double engagement, double clarity,
                      double autonomy, double mastery, double team_trust);
void retro_calc_health_avg(RetroLog *log, int session_idx);
void retro_update_happiness_trend(RetroLog *log);
bool retro_health_declining(RetroLog *log, int window);

/* L8: 5 Whys Root Cause Analysis */
typedef struct {
    char problem[RETRO_DESC_LEN];
    char why_answers[5][RETRO_DESC_LEN];
    char root_cause[RETRO_DESC_LEN];
    char countermeasure[RETRO_DESC_LEN];
    int depth_reached;
} FiveWhys;

void five_whys_init(FiveWhys *fw, const char *problem);
bool five_whys_add(FiveWhys *fw, int level, const char *because);
void five_whys_solve(FiveWhys *fw, const char *root_cause, const char *countermeasure);

/* Print */
void retro_print_session(RetroLog *log, int session_idx);
void retro_print_summary(RetroLog *log);
void retro_print_health_trend(RetroLog *log);
void five_whys_print(FiveWhys *fw);

#endif
