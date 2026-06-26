#include "product_analytics.h"
#include <stdio.h>
#include <string.h>

void analytics_init(Analytics *an) {
    memset(an, 0, sizeof(*an));
}

int analytics_add_cohort(Analytics *an, const char *name, int initial_users) {
    if (an->cohort_count >= MAX_COHORTS) return -1;
    Cohort *c = &an->cohorts[an->cohort_count];
    strncpy(c->name, name, 31); c->name[31] = '\0';
    c->initial_users = initial_users; c->days_tracked = 0;
    memset(c->retention, 0, sizeof(c->retention));
    return an->cohort_count++;
}

void analytics_record_retention(Analytics *an, int cohort_idx, int day, int active_users) {
    if (cohort_idx < 0 || cohort_idx >= an->cohort_count) return;
    if (day < 0 || day >= MAX_COHORT_DAYS) return;
    Cohort *c = &an->cohorts[cohort_idx];
    c->retention[day] = c->initial_users > 0 ? 100.0 * active_users / c->initial_users : 0;
    if (day >= c->days_tracked) c->days_tracked = day + 1;
}

double analytics_retention_at(Analytics *an, int cohort_idx, int day) {
    if (cohort_idx < 0 || cohort_idx >= an->cohort_count) return 0;
    if (day < 0 || day >= MAX_COHORT_DAYS) return 0;
    return an->cohorts[cohort_idx].retention[day];
}

int analytics_add_funnel(Analytics *an, const char *name) {
    if (an->funnel_count >= 8) return -1;
    Funnel *f = &an->funnels[an->funnel_count];
    strncpy(f->name, name, 63); f->name[63] = '\0';
    f->step_count = 0; f->overall_conversion = 0;
    return an->funnel_count++;
}

int analytics_funnel_add_step(Analytics *an, int funnel_idx, const char *step_name,
                              int entered, int completed) {
    if (funnel_idx < 0 || funnel_idx >= an->funnel_count) return -1;
    Funnel *f = &an->funnels[funnel_idx];
    if (f->step_count >= MAX_FUNNEL_STEPS) return -1;
    FunnelStep *fs = &f->steps[f->step_count];
    strncpy(fs->step_name, step_name, 63); fs->step_name[63] = '\0';
    fs->entered = entered; fs->completed = completed;
    fs->dropoff_pct = entered > 0 ? 100.0 * (entered - completed) / entered : 0;
    return f->step_count++;
}

void analytics_funnel_calculate(Analytics *an, int funnel_idx) {
    if (funnel_idx < 0 || funnel_idx >= an->funnel_count) return;
    Funnel *f = &an->funnels[funnel_idx];
    if (f->step_count == 0) { f->overall_conversion = 0; return; }
    int first = f->steps[0].entered;
    int last = f->steps[f->step_count - 1].completed;
    f->overall_conversion = first > 0 ? 100.0 * last / first : 0;
}

void analytics_print_cohorts(Analytics *an) {
    printf("=== Cohorts (%d) ===\n", an->cohort_count);
    for (int i = 0; i < an->cohort_count; i++) {
        Cohort *c = &an->cohorts[i];
        printf("  %s (n=%d): ", c->name, c->initial_users);
        for (int d = 0; d < c->days_tracked && d < 7; d++)
            printf("D%d=%.0f%% ", d+1, c->retention[d]);
        printf("\n");
    }
}

void analytics_print_funnel(Analytics *an, int funnel_idx) {
    if (funnel_idx < 0 || funnel_idx >= an->funnel_count) return;
    Funnel *f = &an->funnels[funnel_idx];
    printf("=== Funnel: %s (%.1f%% conversion) ===\n", f->name, f->overall_conversion);
    for (int i = 0; i < f->step_count; i++)
        printf("  %s: %d -> %d (%.1f%% dropoff)\n",
               f->steps[i].step_name, f->steps[i].entered,
               f->steps[i].completed, f->steps[i].dropoff_pct);
}
