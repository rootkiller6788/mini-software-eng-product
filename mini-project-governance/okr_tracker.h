#ifndef OKR_TRACKER_H
#define OKR_TRACKER_H

#include <stddef.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

#define OKR_MAX_OBJECTIVES 256
#define OKR_MAX_KEY_RESULTS 1024
#define OKR_MAX_TITLE 256
#define OKR_MAX_DESC 1024
#define OKR_MAX_CHECKINS 52
#define OKR_MAX_CFR_ENTRIES 512
#define OKR_MAX_UNIT 32
#define OKR_MAX_LEVELS 5

typedef enum {
    OKR_LEVEL_COMPANY = 0,
    OKR_LEVEL_DEPARTMENT,
    OKR_LEVEL_TEAM,
    OKR_LEVEL_INDIVIDUAL
} OKRLevel;

typedef enum {
    OKR_STATUS_DRAFT = 0,
    OKR_STATUS_ACTIVE,
    OKR_STATUS_COMPLETED,
    OKR_STATUS_AT_RISK,
    OKR_STATUS_ABANDONED
} OKRStatus;

typedef enum {
    CFR_TYPE_CONVERSATION = 0,
    CFR_TYPE_FEEDBACK,
    CFR_TYPE_RECOGNITION
} CFRType;

typedef struct {
    int id;
    int objective_id;
    char title[OKR_MAX_TITLE];
    double start_value;
    double target_value;
    double current_value;
    char unit[OKR_MAX_UNIT];
    double weight; /* 0.0-1.0 weight within its objective */
} OKRKeyResult;

typedef struct {
    int id;
    char title[OKR_MAX_TITLE];
    char description[OKR_MAX_DESC];
    OKRLevel level;
    OKRStatus status;
    int parent_objective_id; /* for alignment, -1 if top level */
    time_t start_date;
    time_t end_date;
    int key_result_ids[8];
    int key_result_count;
    double overall_grade; /* 0.0-1.0, calculated from KRs */
} OKRObjective;

typedef struct {
    int id;
    int objective_id;
    int key_result_id;
    int week_number;
    time_t checkin_date;
    double confidence; /* 0.0-1.0: how confident to meet target */
    char notes[OKR_MAX_DESC];
} OKRCheckin;

typedef struct {
    int id;
    int objective_id;
    CFRType type;
    int participant_id;
    int target_participant_id;
    char content[OKR_MAX_DESC];
    time_t date;
} CFREntry;

typedef struct {
    double overall;
    double achievement;
    double progress;
    double learning;
} OKRGradeBreakdown;

typedef struct {
    OKRObjective *objectives;
    int objective_count;
    int objective_capacity;
    OKRKeyResult *key_results;
    int key_result_count;
    int key_result_capacity;
    OKRCheckin checkins[OKR_MAX_CHECKINS * 4]; /* per objective * 4 checkins */
    int checkin_count;
    CFREntry cfr_entries[OKR_MAX_CFR_ENTRIES];
    int cfr_count;
    int next_id;
} OKRContext;

OKRContext *okr_context_create(void);
void okr_context_free(OKRContext *ctx);

int okr_objective_create(OKRContext *ctx, const char *title,
                          const char *description, OKRLevel level,
                          int parent_objective_id, time_t end_date);

int okr_key_result_add(OKRContext *ctx, int objective_id,
                        const char *title, double start_value,
                        double target_value, const char *unit,
                        double weight);

int okr_key_result_update(OKRContext *ctx, int key_result_id,
                           double new_value);

OKRGradeBreakdown okr_grade_calculate(const OKRContext *ctx,
                                       int objective_id);

int okr_checkin_weekly(OKRContext *ctx, int objective_id,
                        int week_number, double confidence,
                        const char *notes);

int okr_alignment_link(OKRContext *ctx, int child_objective_id,
                        int parent_objective_id);

int okr_alignment_validate(const OKRContext *ctx);
int okr_objective_find_at_risk(OKRContext *ctx, int *result_ids, int max_results);

int cfr_conversation_record(OKRContext *ctx, int objective_id,
                             int participant_id, const char *content);
int cfr_feedback_record(OKRContext *ctx, int objective_id,
                         int giver_id, int receiver_id,
                         const char *content);
int cfr_recognition_record(OKRContext *ctx, int objective_id,
                            int giver_id, int receiver_id,
                            const char *content);

double okr_confidence_trend(const OKRContext *ctx, int objective_id);

void okr_objective_print(const OKRObjective *obj);
void okr_key_result_print(const OKRKeyResult *kr);
void okr_grade_print(const OKRGradeBreakdown *grade);
void okr_checkin_print(const OKRCheckin *checkin);
void cfr_entry_print(const CFREntry *entry);
void okr_context_summary(const OKRContext *ctx);

#ifdef __cplusplus
}
#endif

#endif
