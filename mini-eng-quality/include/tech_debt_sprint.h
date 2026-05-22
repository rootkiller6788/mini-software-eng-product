#ifndef TECH_DEBT_SPRINT_H
#define TECH_DEBT_SPRINT_H

#include <stddef.h>
#include <stdint.h>

#define TDS_MAX_DEBT_ITEMS      256
#define TDS_MAX_SPRINTS         32
#define TDS_MAX_FITNESS_FUNCS   16
#define TDS_MAX_OBSERVERS       8
#define TDS_MAX_NAME_LEN        128
#define TDS_MAX_DESC_LEN        512
#define TDS_MAX_OWNER_LEN       64
#define TDS_MAX_TAG_LEN         32

typedef enum {
    TDS_SEVERITY_LOW,
    TDS_SEVERITY_MEDIUM,
    TDS_SEVERITY_HIGH,
    TDS_SEVERITY_CRITICAL
} tds_severity_e;

typedef enum {
    TDS_CATEGORY_ARCHITECTURE,
    TDS_CATEGORY_CODE_QUALITY,
    TDS_CATEGORY_TEST_COVERAGE,
    TDS_CATEGORY_PERFORMANCE,
    TDS_CATEGORY_SECURITY,
    TDS_CATEGORY_DEPENDENCY,
    TDS_CATEGORY_DOCUMENTATION
} tds_category_e;

typedef enum {
    TDS_STATUS_BACKLOG,
    TDS_STATUS_SELECTED,
    TDS_STATUS_IN_PROGRESS,
    TDS_STATUS_DONE,
    TDS_STATUS_DEFERRED
} tds_status_e;

typedef struct {
    int  id;
    char name[TDS_MAX_NAME_LEN];
    char description[TDS_MAX_DESC_LEN];
    char owner[TDS_MAX_OWNER_LEN];
    tds_severity_e severity;
    tds_category_e category;
    tds_status_e status;
    double estimated_hours;
    double actual_hours;
    double interest_rate_percent;
    double principal_hours;
    double interest_hours;
    int    days_outstanding;
    int    sprint_id;
    char   tags[8][TDS_MAX_TAG_LEN];
    int    tag_count;
} tds_debt_item_t;

typedef struct {
    int  id;
    char name[TDS_MAX_NAME_LEN];
    int  total_story_points;
    int  debt_allocation_percent;
    double debt_capacity_hours;
    tds_debt_item_t *items[TDS_MAX_DEBT_ITEMS];
    int  item_count;
    double completed_hours;
    int    is_active;
} tds_sprint_t;

typedef struct {
    tds_debt_item_t items[TDS_MAX_DEBT_ITEMS];
    int  count;
    int  next_id;
} tds_backlog_t;

typedef struct {
    double total_principal_hours;
    double total_interest_hours;
    double total_interest_cost;
    double avg_interest_rate;
    double avg_age_days;
} tds_interest_report_t;

typedef struct {
    double principal_before;
    double principal_after;
    double effort_spent;
    double interest_reduction;
    double roi_ratio;
    int    items_resolved;
} tds_roi_analysis_t;

typedef double (*tds_fitness_func_t)(void *context);

typedef struct {
    char name[TDS_MAX_NAME_LEN];
    tds_fitness_func_t func;
    void *context;
    double threshold;
    double current_value;
    int    passed;
} tds_fitness_function_t;

typedef struct {
    tds_fitness_function_t functions[TDS_MAX_FITNESS_FUNCS];
    int  count;
} tds_fitness_suite_t;

void  tds_backlog_init(tds_backlog_t *bl);
int   tds_add_debt(tds_backlog_t *bl, const char *name,
                   const char *description, const char *owner,
                   tds_severity_e severity, tds_category_e category,
                   double estimated_hours, double interest_rate);
int   tds_remove_debt(tds_backlog_t *bl, int id);
tds_debt_item_t *tds_find_debt(tds_backlog_t *bl, int id);
void  tds_update_debt_status(tds_debt_item_t *item, tds_status_e status);
void  tds_calc_interest(tds_debt_item_t *item, int days_outstanding);
double tds_total_principal(const tds_backlog_t *bl);
double tds_total_interest(const tds_backlog_t *bl);
void  tds_sort_by_interest(tds_backlog_t *bl);
void  tds_sort_by_severity(tds_backlog_t *bl);
void  tds_print_backlog(const tds_backlog_t *bl);
void  tds_sprint_init(tds_sprint_t *sprint, int id, const char *name,
                      int story_points, int debt_pct);
int   tds_sprint_add_item(tds_sprint_t *sprint, tds_debt_item_t *item);
double tds_sprint_capacity_hours(tds_sprint_t *sprint, int sprint_days,
                                 int team_size, double hours_per_day);
void  tds_sprint_print(const tds_sprint_t *sprint);
void  tds_compute_roi(const tds_backlog_t *bl, int resolved_ids[],
                      int resolved_count, double effort_spent,
                      tds_roi_analysis_t *roi);
double tds_debt_index(const tds_backlog_t *bl, double current_velocity);
void  tds_interest_report(const tds_backlog_t *bl,
                          tds_interest_report_t *report);
void  tds_fitness_init(tds_fitness_suite_t *suite);
int   tds_fitness_add(tds_fitness_suite_t *suite, const char *name,
                      tds_fitness_func_t func, void *ctx, double threshold);
void  tds_fitness_evaluate(tds_fitness_suite_t *suite);
int   tds_fitness_all_passed(const tds_fitness_suite_t *suite);
void  tds_fitness_print(const tds_fitness_suite_t *suite);

#endif
