#ifndef PRODUCT_KPI_H
#define PRODUCT_KPI_H
#include <stdbool.h>

#define MAX_KPI_POINTS 52  /* weekly data for a year */

typedef enum { KPIM_ABSOLUTE, KPIM_PERCENTAGE, KPIM_RATIO, KPIM_CURRENCY } KpiMeasure;

typedef struct {
    char name[64]; KpiMeasure measure; double target; double current;
    double history[MAX_KPI_POINTS]; int history_count;
    double threshold_warning; double threshold_critical;
} ProductKpi;

typedef struct {
    ProductKpi kpis[16]; int kpi_count;
    /* Common product KPIs */
    double nps;          /* Net Promoter Score: promoters% - detractors% */
    double retention_d7;  /* Day-7 retention */
    double retention_d30; /* Day-30 retention */
    double mau;           /* Monthly Active Users */
    double conversion_rate; /* funnel conversion % */
    double churn_rate;
    double avg_revenue_per_user;
} KpiDashboard;

void kpi_dashboard_init(KpiDashboard *kd);
int  kpi_add(KpiDashboard *kd, const char *name, KpiMeasure measure, double target, double warn, double crit);
void kpi_update(KpiDashboard *kd, const char *name, double value);
double kpi_trend(KpiDashboard *kd, const char *name); /* simple linear regression slope */
bool kpi_on_track(KpiDashboard *kd, const char *name);
void kpi_dashboard_print(KpiDashboard *kd);

/* NPS calculation */
double kpi_calc_nps(int promoters, int passives, int detractors);
double kpi_calc_retention(int day0_users, int dayN_users);
double kpi_calc_churn(int start_users, int end_users, int new_users);
#endif
