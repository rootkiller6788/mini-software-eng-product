#include "tech_debt_sprint.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

void tds_backlog_init(tds_backlog_t *bl) {
    if (!bl) return;
    memset(bl, 0, sizeof(*bl));
    bl->next_id = 1;
}

int tds_add_debt(tds_backlog_t *bl, const char *name,
                 const char *description, const char *owner,
                 tds_severity_e severity, tds_category_e category,
                 double estimated_hours, double interest_rate) {
    if (!bl || bl->count >= TDS_MAX_DEBT_ITEMS) return -1;
    tds_debt_item_t *item = &bl->items[bl->count];
    memset(item, 0, sizeof(*item));
    item->id = bl->next_id++;
    snprintf(item->name, TDS_MAX_NAME_LEN, "%s", name);
    snprintf(item->description, TDS_MAX_DESC_LEN, "%s", description);
    snprintf(item->owner, TDS_MAX_OWNER_LEN, "%s", owner);
    item->severity = severity;
    item->category = category;
    item->status = TDS_STATUS_BACKLOG;
    item->estimated_hours = estimated_hours;
    item->principal_hours = estimated_hours;
    item->interest_rate_percent = interest_rate;
    item->sprint_id = -1;
    bl->count++;
    return item->id;
}

int tds_remove_debt(tds_backlog_t *bl, int id) {
    if (!bl || id <= 0) return -1;
    int idx = -1;
    for (int i = 0; i < bl->count; i++)
        if (bl->items[i].id == id) { idx = i; break; }
    if (idx < 0) return -1;
    if (idx < bl->count - 1)
        memmove(&bl->items[idx], &bl->items[idx + 1],
                (bl->count - idx - 1) * sizeof(tds_debt_item_t));
    bl->count--;
    return 0;
}

tds_debt_item_t *tds_find_debt(tds_backlog_t *bl, int id) {
    if (!bl) return NULL;
    for (int i = 0; i < bl->count; i++)
        if (bl->items[i].id == id) return &bl->items[i];
    return NULL;
}

void tds_update_debt_status(tds_debt_item_t *item, tds_status_e status) {
    if (!item) return;
    item->status = status;
}

void tds_calc_interest(tds_debt_item_t *item, int days_outstanding) {
    if (!item) return;
    item->days_outstanding = days_outstanding;
    double years = (double)days_outstanding / 365.0;
    item->interest_hours =
        item->principal_hours * (item->interest_rate_percent / 100.0) * years;
}

double tds_total_principal(const tds_backlog_t *bl) {
    if (!bl) return 0.0;
    double total = 0.0;
    for (int i = 0; i < bl->count; i++)
        total += bl->items[i].principal_hours;
    return total;
}

double tds_total_interest(const tds_backlog_t *bl) {
    if (!bl) return 0.0;
    double total = 0.0;
    for (int i = 0; i < bl->count; i++)
        total += bl->items[i].interest_hours;
    return total;
}

static int cmp_interest_desc(const void *a, const void *b) {
    double ia = ((const tds_debt_item_t *)a)->interest_hours;
    double ib = ((const tds_debt_item_t *)b)->interest_hours;
    if (ia < ib) return 1;
    if (ia > ib) return -1;
    return 0;
}

void tds_sort_by_interest(tds_backlog_t *bl) {
    if (!bl) return;
    qsort(bl->items, bl->count, sizeof(tds_debt_item_t), cmp_interest_desc);
}

static int cmp_severity_desc(const void *a, const void *b) {
    int sa = (int)((const tds_debt_item_t *)a)->severity;
    int sb = (int)((const tds_debt_item_t *)b)->severity;
    if (sa < sb) return 1;
    if (sa > sb) return -1;
    return 0;
}

void tds_sort_by_severity(tds_backlog_t *bl) {
    if (!bl) return;
    qsort(bl->items, bl->count, sizeof(tds_debt_item_t), cmp_severity_desc);
}

static const char *severity_names[] = {
    "LOW", "MEDIUM", "HIGH", "CRITICAL"
};
static const char *category_names[] = {
    "ARCHITECTURE", "CODE_QUALITY", "TEST_COVERAGE",
    "PERFORMANCE", "SECURITY", "DEPENDENCY", "DOCUMENTATION"
};
static const char *status_names[] = {
    "BACKLOG", "SELECTED", "IN_PROGRESS", "DONE", "DEFERRED"
};

void tds_print_backlog(const tds_backlog_t *bl) {
    if (!bl) return;
    printf("=== Tech Debt Backlog (%d items) ===\n", bl->count);
    printf("Total Principal: %.1fh | Total Interest: %.1fh\n",
           tds_total_principal(bl), tds_total_interest(bl));
    for (int i = 0; i < bl->count; i++) {
        tds_debt_item_t *d = &bl->items[i];
        printf("  #%d [%s|%s] %s - %s (%.1fh, +%.2f%%)\n",
               d->id,
               severity_names[d->severity],
               category_names[d->category],
               d->name, status_names[d->status],
               d->principal_hours, d->interest_rate_percent);
    }
}

void tds_sprint_init(tds_sprint_t *sprint, int id, const char *name,
                     int story_points, int debt_pct) {
    if (!sprint) return;
    memset(sprint, 0, sizeof(*sprint));
    sprint->id = id;
    snprintf(sprint->name, TDS_MAX_NAME_LEN, "%s", name);
    sprint->total_story_points = story_points;
    sprint->debt_allocation_percent = debt_pct;
    sprint->is_active = 0;
}

int tds_sprint_add_item(tds_sprint_t *sprint, tds_debt_item_t *item) {
    if (!sprint || !item || sprint->item_count >= TDS_MAX_DEBT_ITEMS)
        return -1;
    sprint->items[sprint->item_count++] = item;
    item->sprint_id = sprint->id;
    item->status = TDS_STATUS_SELECTED;
    sprint->debt_capacity_hours += item->estimated_hours;
    return 0;
}

double tds_sprint_capacity_hours(tds_sprint_t *sprint, int sprint_days,
                                 int team_size, double hours_per_day) {
    (void)sprint;
    double total = (double)sprint_days * (double)team_size * hours_per_day;
    return total * 0.8;
}

void tds_sprint_print(const tds_sprint_t *sprint) {
    if (!sprint) return;
    printf("=== Sprint #%d: %s ===\n", sprint->id, sprint->name);
    printf("  Story Points: %d | Debt Allocation: %d%%\n",
           sprint->total_story_points, sprint->debt_allocation_percent);
    printf("  Debt Capacity: %.1fh | Items: %d | Done: %.1fh\n",
           sprint->debt_capacity_hours, sprint->item_count,
           sprint->completed_hours);
    for (int i = 0; i < sprint->item_count; i++) {
        tds_debt_item_t *d = sprint->items[i];
        printf("    #%d %s [%s] %.1fh\n",
               d->id, d->name, status_names[d->status], d->estimated_hours);
    }
}

void tds_compute_roi(const tds_backlog_t *bl, int resolved_ids[],
                     int resolved_count, double effort_spent,
                     tds_roi_analysis_t *roi) {
    if (!bl || !roi) return;
    memset(roi, 0, sizeof(*roi));
    roi->effort_spent = effort_spent;
    roi->items_resolved = resolved_count;

    for (int i = 0; i < resolved_count; i++) {
        tds_debt_item_t *item = tds_find_debt((tds_backlog_t *)bl,
                                              resolved_ids[i]);
        if (item) {
            roi->principal_before += item->principal_hours;
            roi->interest_reduction += item->interest_hours;
            if (item->status == TDS_STATUS_DONE)
                roi->principal_after += 0.0;
            else
                roi->principal_after += item->principal_hours;
        }
    }
    roi->principal_after = roi->principal_before;
    if (effort_spent > 0.0)
        roi->roi_ratio = roi->interest_reduction / effort_spent;
}

double tds_debt_index(const tds_backlog_t *bl, double current_velocity) {
    if (!bl || current_velocity <= 0.0) return 0.0;
    double total_remediation = tds_total_principal(bl);
    return total_remediation / current_velocity;
}

void tds_interest_report(const tds_backlog_t *bl,
                         tds_interest_report_t *report) {
    if (!bl || !report) return;
    memset(report, 0, sizeof(*report));
    for (int i = 0; i < bl->count; i++) {
        report->total_principal_hours += bl->items[i].principal_hours;
        report->total_interest_hours += bl->items[i].interest_hours;
        report->avg_age_days += (double)bl->items[i].days_outstanding;
    }
    if (bl->count > 0) {
        report->avg_interest_rate /= (double)bl->count;
        report->avg_age_days /= (double)bl->count;
    }
    report->total_interest_cost =
        report->total_interest_hours * 100.0;
}

void tds_fitness_init(tds_fitness_suite_t *suite) {
    if (!suite) return;
    memset(suite, 0, sizeof(*suite));
}

int tds_fitness_add(tds_fitness_suite_t *suite, const char *name,
                    tds_fitness_func_t func, void *ctx, double threshold) {
    if (!suite || suite->count >= TDS_MAX_FITNESS_FUNCS) return -1;
    tds_fitness_function_t *f = &suite->functions[suite->count];
    memset(f, 0, sizeof(*f));
    snprintf(f->name, TDS_MAX_NAME_LEN, "%s", name);
    f->func = func;
    f->context = ctx;
    f->threshold = threshold;
    suite->count++;
    return 0;
}

void tds_fitness_evaluate(tds_fitness_suite_t *suite) {
    if (!suite) return;
    for (int i = 0; i < suite->count; i++) {
        tds_fitness_function_t *f = &suite->functions[i];
        if (f->func) {
            f->current_value = f->func(f->context);
            f->passed = (f->current_value >= f->threshold);
        }
    }
}

int tds_fitness_all_passed(const tds_fitness_suite_t *suite) {
    if (!suite) return 1;
    for (int i = 0; i < suite->count; i++)
        if (!suite->functions[i].passed) return 0;
    return 1;
}

void tds_fitness_print(const tds_fitness_suite_t *suite) {
    if (!suite) return;
    printf("=== Architecture Fitness Functions ===\n");
    for (int i = 0; i < suite->count; i++) {
        printf("  %s: %.3f (threshold=%.3f) %s\n",
               suite->functions[i].name,
               suite->functions[i].current_value,
               suite->functions[i].threshold,
               suite->functions[i].passed ? "[PASS]" : "[FAIL]");
    }
}
