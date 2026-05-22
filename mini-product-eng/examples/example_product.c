#include "product_roadmap.h"
#include "product_backlog.h"
#include "product_kpi.h"
#include "product_experiment.h"
#include "product_analytics.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

int main(void) {
    printf("===== Product Engineering Demo =====\n\n");

    /* Roadmap */
    printf("--- Product Roadmap ---\n");
    ProductRoadmap pr;
    roadmap_init(&pr, "Be the #1 developer productivity platform", 2026, 2);
    roadmap_add_theme(&pr, "AI Assistant", "LLM-powered code suggestions");
    roadmap_add_theme(&pr, "Performance", "Reduce latency and resource usage");
    roadmap_add_item(&pr, 0, "AI-001", "Code completion v2", RITEM_FEATURE, 2, 4);
    roadmap_add_item(&pr, 0, "AI-002", "Chat-based refactoring", RITEM_FEATURE, 3, 6);
    roadmap_add_item(&pr, 1, "PERF-001", "Lazy-load modules", RITEM_STORY, 2, 2);
    roadmap_add_item(&pr, 1, "PERF-002", "Tree-shaking bundler", RITEM_EPIC, 3, 8);
    roadmap_update_status(&pr, "AI-001", RSTATUS_IN_PROGRESS);
    roadmap_print(&pr);
    printf("Q2 items: %d, effort: %dw\n\n",
           roadmap_items_in_quarter(&pr, 2), roadmap_total_effort(&pr, 2));

    /* Backlog */
    printf("--- Backlog Management ---\n");
    Backlog bl;
    backlog_init(&bl, 20, 10);
    backlog_add(&bl, "US-01", "User login", PRIO_CRITICAL, BLITEM_STORY, 5, 10);
    backlog_add(&bl, "US-02", "Dashboard view", PRIO_HIGH, BLITEM_STORY, 8, 8);
    backlog_add(&bl, "US-03", "Settings page", PRIO_MEDIUM, BLITEM_STORY, 3, 5);
    backlog_add(&bl, "BUG-01", "Login crash on empty", PRIO_CRITICAL, BLITEM_BUG, 2, 9);
    backlog_add(&bl, "SPK-01", "GraphQL vs REST research", PRIO_LOW, BLITEM_SPIKE, 5, 3);
    backlog_sort_by_priority(&bl);
    backlog_print(&bl);
    printf("Sprint capacity: %d items fit in %d pts\n\n",
           backlog_sprint_capacity(&bl), bl.velocity);

    /* KPIs */
    printf("--- KPI Dashboard ---\n");
    KpiDashboard kd;
    kpi_dashboard_init(&kd);
    kpi_add(&kd, "User Growth", KPIM_ABSOLUTE, 10000, 5000, 2000);
    kpi_add(&kd, "NPS Score", KPIM_ABSOLUTE, 50, 30, 10);
    kpi_add(&kd, "Retention D7", KPIM_PERCENTAGE, 60, 40, 25);
    kpi_update(&kd, "User Growth", 7800);
    kpi_update(&kd, "NPS Score", 42);
    kpi_update(&kd, "Retention D7", 55);
    kd.nps = kpi_calc_nps(350, 200, 100);
    kd.retention_d7 = kpi_calc_retention(1000, 600);
    kd.retention_d30 = kpi_calc_retention(1000, 300);
    kd.mau = 15420;
    kd.churn_rate = kpi_calc_churn(15000, 15420, 1200);
    kpi_dashboard_print(&kd);
    printf("\n");

    /* A/B Experiment */
    printf("--- A/B Experiment ---\n");
    Experiment exp;
    experiment_init(&exp, "EXP-01", "New signup flow increases conversion by 10%");
    experiment_add_variant(&exp, "Control");
    experiment_add_variant(&exp, "Variant A");
    experiment_set_control(&exp, 0);
    experiment_assign_users(&exp, 0, 1000, 120);
    experiment_assign_users(&exp, 1, 1000, 155);
    experiment_print(&exp);
    printf("Significant: %s, Winning: %d\n\n",
           experiment_is_significant(&exp) ? "YES" : "NO",
           experiment_winning_variant(&exp));

    /* Analytics */
    printf("--- User Analytics ---\n");
    Analytics an;
    analytics_init(&an);
    analytics_add_cohort(&an, "2026-W01", 500);
    analytics_record_retention(&an, 0, 0, 500);
    analytics_record_retention(&an, 0, 1, 350);
    analytics_record_retention(&an, 0, 2, 280);
    analytics_record_retention(&an, 0, 6, 150);
    analytics_add_cohort(&an, "2026-W02", 600);
    analytics_record_retention(&an, 1, 0, 600);
    analytics_record_retention(&an, 1, 1, 420);
    analytics_print_cohorts(&an);

    analytics_add_funnel(&an, "Signup Funnel");
    analytics_funnel_add_step(&an, 0, "Visit landing", 10000, 8000);
    analytics_funnel_add_step(&an, 0, "Click signup", 8000, 3000);
    analytics_funnel_add_step(&an, 0, "Complete form", 3000, 1500);
    analytics_funnel_add_step(&an, 0, "Verify email", 1500, 1200);
    analytics_funnel_calculate(&an, 0);
    analytics_print_funnel(&an, 0);

    return 0;
}
