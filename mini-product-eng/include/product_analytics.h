#ifndef PRODUCT_ANALYTICS_H
#define PRODUCT_ANALYTICS_H
#include <stdbool.h>
#include <stdint.h>

#define MAX_COHORTS 12
#define MAX_COHORT_DAYS 30
#define MAX_FUNNEL_STEPS 10

typedef struct {
    char name[32]; int initial_users;
    double retention[MAX_COHORT_DAYS]; /* day-by-day retention */
    int days_tracked;
} Cohort;

typedef struct {
    char step_name[64]; int entered; int completed; double dropoff_pct;
} FunnelStep;

typedef struct {
    char name[64];
    FunnelStep steps[MAX_FUNNEL_STEPS]; int step_count;
    double overall_conversion; /* last_step.completed / first_step.entered */
} Funnel;

typedef struct {
    Cohort cohorts[MAX_COHORTS]; int cohort_count;
    Funnel funnels[8]; int funnel_count;
    /* User segments */
    int total_users; int active_users; int new_users_today;
    double avg_session_duration_sec;
    int sessions_per_user;
} Analytics;

void analytics_init(Analytics *an);
int  analytics_add_cohort(Analytics *an, const char *name, int initial_users);
void analytics_record_retention(Analytics *an, int cohort_idx, int day, int active_users);
double analytics_retention_at(Analytics *an, int cohort_idx, int day);
int  analytics_add_funnel(Analytics *an, const char *name);
int  analytics_funnel_add_step(Analytics *an, int funnel_idx, const char *step_name, int entered, int completed);
void analytics_funnel_calculate(Analytics *an, int funnel_idx);
void analytics_print_cohorts(Analytics *an);
void analytics_print_funnel(Analytics *an, int funnel_idx);
#endif
