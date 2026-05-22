#ifndef RETROSPECTIVE_H
#define RETROSPECTIVE_H

#include <stddef.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

#define RETRO_MAX_ITEMS 256
#define RETRO_MAX_ACTION_ITEMS 128
#define RETRO_MAX_TEXT 1024
#define RETRO_MAX_PARTICIPANTS 64
#define RETRO_MAX_PARTICIPANT_NAME 128
#define RETRO_MAX_HEALTH_QUESTIONS 20
#define RETRO_MAX_SURVEY_QUESTIONS 16
#define RETRO_MAX_PSYCH_QUESTIONS 12
#define RETRO_MAX_RESPONSES 256

typedef enum {
    RETRO_FORMAT_START_STOP_CONTINUE = 0,
    RETRO_FORMAT_MAD_SAD_GLAD,
    RETRO_FORMAT_FOUR_LS,
    RETRO_FORMAT_SAILBOAT,
    RETRO_FORMAT_KALM_KEEP_ADD_LESS_MORE
} RetroFormat;

typedef enum {
    RETRO_CAT_START = 0,
    RETRO_CAT_STOP,
    RETRO_CAT_CONTINUE
} RetroSSC;

typedef enum {
    RETRO_CAT_MAD = 0,
    RETRO_CAT_SAD,
    RETRO_CAT_GLAD
} RetroMSG;

typedef enum {
    RETRO_CAT_LIKED = 0,
    RETRO_CAT_LEARNED,
    RETRO_CAT_LACKED,
    RETRO_CAT_LONGED_FOR
} Retro4L;

typedef enum {
    ACTION_STATUS_OPEN = 0,
    ACTION_STATUS_IN_PROGRESS,
    ACTION_STATUS_DONE,
    ACTION_STATUS_WONT_DO
} ActionItemStatus;

typedef struct {
    int id;
    char description[RETRO_MAX_TEXT];
    int assignee_id;
    char assignee_name[RETRO_MAX_PARTICIPANT_NAME];
    ActionItemStatus status;
    int sprint_id;
    time_t created_at;
    time_t completed_at;
    int priority;
} ActionItem;

typedef struct {
    int category;
    char text[RETRO_MAX_TEXT];
    char author[RETRO_MAX_PARTICIPANT_NAME];
} RetroCard;

typedef struct {
    int id;
    RetroFormat format;
    int sprint_id;
    time_t date;
    RetroCard items[RETRO_MAX_ITEMS];
    int item_count;
    ActionItem action_items[RETRO_MAX_ACTION_ITEMS];
    int action_item_count;
    int next_action_id;
    double engagement_score;
} RetrospectiveSession;

typedef struct {
    char question[RETRO_MAX_TEXT];
    double score;       /* 1.0-5.0 */
    int response_count;
} HealthCheckQuestion;

typedef struct {
    int sprint_id;
    time_t date;
    HealthCheckQuestion questions[RETRO_MAX_HEALTH_QUESTIONS];
    int question_count;
    double overall_health;
} RetroHealthCheck;

typedef struct {
    char question[RETRO_MAX_TEXT];
    int agree_count;
    int neutral_count;
    int disagree_count;
    int total_responses;
} MoraleSurveyQuestion;

typedef struct {
    time_t date;
    MoraleSurveyQuestion questions[RETRO_MAX_SURVEY_QUESTIONS];
    int question_count;
    double morale_index; /* 0.0-1.0 */
} TeamMoraleSurvey;

typedef struct {
    char statement[RETRO_MAX_TEXT];
    double avg_score;   /* 1.0-5.0 */
    int response_count;
    int safe_zone_count; /* scores >= 4.0 */
} PsychSafetyQuestion;

typedef struct {
    time_t date;
    PsychSafetyQuestion questions[RETRO_MAX_PSYCH_QUESTIONS];
    int question_count;
    double safety_index; /* 0.0-1.0 */
    int safe_participants;
    int total_participants;
} PsychSafetyAssessment;

typedef struct {
    RetrospectiveSession *sessions;
    int session_count;
    int session_capacity;
    ActionItem *pending_actions;
    int pending_action_count;
    int pending_action_capacity;
} RetroContext;

RetroContext *retro_context_create(void);
void retro_context_free(RetroContext *ctx);

RetrospectiveSession *retro_session_create(RetroContext *ctx,
                                            RetroFormat format, int sprint_id);

int retro_add_item_ssc(RetrospectiveSession *session, RetroSSC category,
                        const char *text, const char *author);
int retro_add_item_msg(RetrospectiveSession *session, RetroMSG category,
                        const char *text, const char *author);
int retro_add_item_4ls(RetrospectiveSession *session, Retro4L category,
                        const char *text, const char *author);

int retro_action_item_create(RetrospectiveSession *session,
                              const char *description,
                              const char *assignee_name, int priority);
int retro_action_item_track(RetroContext *ctx, int action_id,
                             ActionItemStatus new_status);
int retro_action_item_find_pending(const RetroContext *ctx,
                                    int *result_ids, int max_results);

RetroHealthCheck *retro_health_check_create(int sprint_id);
int retro_health_question_add(RetroHealthCheck *check, const char *question);
int retro_health_answer_record(RetroHealthCheck *check, int question_index,
                                double score);
double retro_health_score(const RetroHealthCheck *check);

TeamMoraleSurvey *retro_morale_survey_create(void);
int retro_morale_question_add(TeamMoraleSurvey *survey, const char *question);
int retro_morale_answer_record(TeamMoraleSurvey *survey, int question_index,
                                int response_type); /* 0=agree,1=neutral,2=disagree */
double retro_morale_index(const TeamMoraleSurvey *survey);

PsychSafetyAssessment *retro_psych_safety_assess(void);
int retro_psych_question_add(PsychSafetyAssessment *assessment,
                              const char *statement);
int retro_psych_answer_record(PsychSafetyAssessment *assessment,
                               int question_index, double score);
double retro_psych_safety_index(const PsychSafetyAssessment *assessment);
int retro_psych_safe_threshold(const PsychSafetyAssessment *assessment,
                                double threshold);

void retro_session_print(const RetrospectiveSession *session);
void retro_health_print(const RetroHealthCheck *check);
void retro_morale_print(const TeamMoraleSurvey *survey);
void retro_psych_print(const PsychSafetyAssessment *assessment);
void retro_action_print(const ActionItem *action);

const char *retro_format_name(RetroFormat format);

#ifdef __cplusplus
}
#endif

#endif
