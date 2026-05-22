#include "product_kpi.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

void kpi_dashboard_init(KpiDashboard *kd) {
    memset(kd, 0, sizeof(*kd));
}

int kpi_add(KpiDashboard *kd, const char *name, KpiMeasure measure, double target,
            double warn, double crit) {
    if (kd->kpi_count >= 16) return -1;
    ProductKpi *k = &kd->kpis[kd->kpi_count];
    strncpy(k->name, name, 63); k->name[63] = '\0';
    k->measure = measure; k->target = target;
    k->threshold_warning = warn; k->threshold_critical = crit;
    k->current = 0; k->history_count = 0;
    return kd->kpi_count++;
}

void kpi_update(KpiDashboard *kd, const char *name, double value) {
    for (int i = 0; i < kd->kpi_count; i++)
        if (strcmp(kd->kpis[i].name, name) == 0) {
            ProductKpi *k = &kd->kpis[i];
            k->current = value;
            if (k->history_count < MAX_KPI_POINTS)
                k->history[k->history_count++] = value;
            return;
        }
}

double kpi_trend(KpiDashboard *kd, const char *name) {
    for (int i = 0; i < kd->kpi_count; i++)
        if (strcmp(kd->kpis[i].name, name) == 0) {
            ProductKpi *k = &kd->kpis[i];
            if (k->history_count < 2) return 0;
            int n = k->history_count;
            double sum_x = 0, sum_y = 0, sum_xy = 0, sum_x2 = 0;
            for (int j = 0; j < n; j++) {
                sum_x += j; sum_y += k->history[j];
                sum_xy += j * k->history[j]; sum_x2 += j * j;
            }
            double denom = n * sum_x2 - sum_x * sum_x;
            if (fabs(denom) < 1e-9) return 0;
            return (n * sum_xy - sum_x * sum_y) / denom;
        }
    return 0;
}

bool kpi_on_track(KpiDashboard *kd, const char *name) {
    for (int i = 0; i < kd->kpi_count; i++)
        if (strcmp(kd->kpis[i].name, name) == 0) {
            ProductKpi *k = &kd->kpis[i];
            if (k->current < k->threshold_critical) return false;
            if (k->current < k->threshold_warning) return true; /* warn but on track */
            return k->current >= k->target * 0.9;
        }
    return false;
}

double kpi_calc_nps(int promoters, int passives, int detractors) {
    int total = promoters + passives + detractors;
    if (total == 0) return 0;
    return 100.0 * (promoters - detractors) / total;
}

double kpi_calc_retention(int day0_users, int dayN_users) {
    if (day0_users == 0) return 0;
    return 100.0 * dayN_users / day0_users;
}

double kpi_calc_churn(int start_users, int end_users, int new_users) {
    int avg = (start_users + end_users) / 2;
    if (avg == 0) return 0;
    return 100.0 * (start_users - end_users + new_users) / avg;
}

void kpi_dashboard_print(KpiDashboard *kd) {
    const char *ms[] = {"ABS","PCT","RATIO","$"};
    printf("=== KPI Dashboard ===\n");
    for (int i = 0; i < kd->kpi_count; i++) {
        ProductKpi *k = &kd->kpis[i];
        double trend = kpi_trend(kd, k->name);
        printf("  %s: %.2f %s (target=%.2f, trend=%+.2f/wk) %s\n",
               k->name, k->current, ms[k->measure], k->target, trend,
               kpi_on_track(kd, k->name) ? "ON TRACK" : "OFF TRACK");
    }
    printf("  NPS=%.1f D7=%.1f%% D30=%.1f%% MAU=%.0f\n",
           kd->nps, kd->retention_d7, kd->retention_d30, kd->mau);
}
