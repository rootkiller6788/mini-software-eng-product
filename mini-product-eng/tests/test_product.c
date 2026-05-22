#include "product_roadmap.h"
#include "product_backlog.h"
#include "product_kpi.h"
#include "product_experiment.h"
#include "product_analytics.h"
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <assert.h>

static int tests = 0, passed = 0;
#define TEST(name) do { tests++; printf("  TEST %s... ", name); } while(0)
#define OK() do { passed++; printf("OK\n"); } while(0)

int main(void) {
    printf("=== Product Engineering Tests ===\n\n");

    /* Roadmap */
    printf("roadmap:\n");
    TEST("init"); { ProductRoadmap pr;
      roadmap_init(&pr, "Vision", 2026, 2);
      assert(pr.current_year == 2026); assert(pr.current_quarter == 2); OK(); }
    TEST("add theme"); { ProductRoadmap pr; roadmap_init(&pr,"V",2026,1);
      assert(roadmap_add_theme(&pr,"AI","desc") == 0);
      assert(pr.theme_count == 1); OK(); }
    TEST("add item"); { ProductRoadmap pr; roadmap_init(&pr,"V",2026,1);
      roadmap_add_theme(&pr,"AI","desc");
      assert(roadmap_add_item(&pr,0,"AI-01","Feature X",RITEM_FEATURE,2,4) == 0);
      assert(pr.themes[0].item_count == 1); OK(); }
    TEST("update status"); { ProductRoadmap pr; roadmap_init(&pr,"V",2026,1);
      roadmap_add_theme(&pr,"AI","desc");
      roadmap_add_item(&pr,0,"AI-01","X",RITEM_FEATURE,2,4);
      assert(roadmap_update_status(&pr,"AI-01",RSTATUS_IN_PROGRESS));
      assert(pr.themes[0].items[0].status == RSTATUS_IN_PROGRESS); OK(); }
    TEST("items in quarter"); { ProductRoadmap pr; roadmap_init(&pr,"V",2026,1);
      roadmap_add_theme(&pr,"AI","desc");
      roadmap_add_item(&pr,0,"A","X",RITEM_FEATURE,2,4);
      roadmap_add_item(&pr,0,"B","Y",RITEM_STORY,3,2);
      assert(roadmap_items_in_quarter(&pr,2) == 1);
      assert(roadmap_total_effort(&pr,2) == 4); OK(); }
    TEST("max themes"); { ProductRoadmap pr; roadmap_init(&pr,"V",2026,1);
      int last = -1;
      for (int i = 0; i < MAX_THEMES; i++) last = roadmap_add_theme(&pr,"T","");
      assert(last == MAX_THEMES - 1);
      assert(roadmap_add_theme(&pr,"overflow","") == -1); OK(); }

    /* Backlog */
    printf("backlog:\n");
    TEST("init"); { Backlog bl; backlog_init(&bl, 20, 10);
      assert(bl.velocity == 20); assert(bl.sprint_length_days == 10); OK(); }
    TEST("add"); { Backlog bl; backlog_init(&bl, 20, 10);
      assert(backlog_add(&bl,"US-01","Login",PRIO_HIGH,BLITEM_STORY,5,8) == 0);
      assert(bl.total_points == 5); OK(); }
    TEST("sort by priority"); { Backlog bl; backlog_init(&bl,20,10);
      backlog_add(&bl,"A","Low",PRIO_LOW,BLITEM_STORY,3,5);
      backlog_add(&bl,"B","High",PRIO_HIGH,BLITEM_STORY,3,5);
      backlog_add(&bl,"C","Crit",PRIO_CRITICAL,BLITEM_STORY,3,5);
      backlog_sort_by_priority(&bl);
      assert(bl.items[0].priority == PRIO_CRITICAL); OK(); }
    TEST("sort by value/effort"); { Backlog bl; backlog_init(&bl,20,10);
      backlog_add(&bl,"A","Low ROI",PRIO_MEDIUM,BLITEM_STORY,10,1);
      backlog_add(&bl,"B","High ROI",PRIO_MEDIUM,BLITEM_STORY,1,10);
      backlog_sort_by_value_effort(&bl);
      assert(bl.items[0].business_value == 10); OK(); }
    TEST("remove"); { Backlog bl; backlog_init(&bl,20,10);
      backlog_add(&bl,"US-01","X",PRIO_HIGH,BLITEM_STORY,5,8);
      assert(backlog_remove(&bl,"US-01"));
      assert(bl.item_count == 0); assert(bl.total_points == 0); OK(); }
    TEST("sprint capacity"); { Backlog bl; backlog_init(&bl,15,10);
      backlog_add(&bl,"A","",PRIO_CRITICAL,BLITEM_STORY,8,5);
      backlog_add(&bl,"B","",PRIO_HIGH,BLITEM_STORY,5,5);
      backlog_add(&bl,"C","",PRIO_MEDIUM,BLITEM_STORY,5,5);
      assert(backlog_sprint_capacity(&bl) == 2); OK(); }
    TEST("accept"); { Backlog bl; backlog_init(&bl,20,10);
      backlog_add(&bl,"US-01","X",PRIO_HIGH,BLITEM_STORY,5,8);
      backlog_accept(&bl,"US-01",120);
      assert(bl.items[0].accepted); assert(bl.items[0].actual_hours == 120); OK(); }
    TEST("max items"); { Backlog bl; backlog_init(&bl,20,10);
      for (int i = 0; i < MAX_BACKLOG_ITEMS; i++) backlog_add(&bl,"X","",PRIO_LOW,BLITEM_TASK,1,1);
      assert(backlog_add(&bl,"overflow","",PRIO_LOW,BLITEM_TASK,1,1) == -1); OK(); }

    /* KPI */
    printf("kpi:\n");
    TEST("calc nps"); {
      double nps = kpi_calc_nps(400, 300, 100);
      assert(nps > 0); /* 400-100=300 promoters net, /800 = 37.5 */
      assert(fabs(nps - 37.5) < 0.1); OK(); }
    TEST("calc retention"); {
      assert(fabs(kpi_calc_retention(1000, 600) - 60.0) < 0.01); OK(); }
    TEST("calc churn"); {
      double c = kpi_calc_churn(1000, 900, 50);
      assert(c > 0); OK(); }
    TEST("add and update"); { KpiDashboard kd; kpi_dashboard_init(&kd);
      kpi_add(&kd,"Test",KPIM_ABSOLUTE,100,50,25);
      kpi_update(&kd,"Test",75); kpi_update(&kd,"Test",80);
      assert(kd.kpis[0].history_count == 2);
      assert(fabs(kd.kpis[0].current - 80.0) < 0.01); OK(); }
    TEST("on track"); { KpiDashboard kd; kpi_dashboard_init(&kd);
      kpi_add(&kd,"Test",KPIM_ABSOLUTE,100,50,25);
      kpi_update(&kd,"Test",95);
      assert(kpi_on_track(&kd,"Test")); OK(); }
    TEST("trend positive"); { KpiDashboard kd; kpi_dashboard_init(&kd);
      kpi_add(&kd,"Test",KPIM_ABSOLUTE,100,50,25);
      for (int j = 0; j < 5; j++) kpi_update(&kd,"Test", 10.0 + j * 2);
      assert(kpi_trend(&kd,"Test") > 0); OK(); }

    /* Experiment */
    printf("experiment:\n");
    TEST("init"); { Experiment exp; experiment_init(&exp,"E1","H");
      assert(exp.significance_level == 0.05); OK(); }
    TEST("add variant"); { Experiment exp; experiment_init(&exp,"E1","H");
      assert(experiment_add_variant(&exp,"Ctrl") == 0);
      assert(experiment_add_variant(&exp,"VarA") == 1); OK(); }
    TEST("assign users"); { Experiment exp; experiment_init(&exp,"E1","H");
      experiment_add_variant(&exp,"Ctrl"); experiment_add_variant(&exp,"VarA");
      experiment_assign_users(&exp, 0, 100, 10);
      experiment_assign_users(&exp, 1, 100, 15);
      assert(fabs(exp.variants[0].conversion_rate - 0.1) < 0.01);
      assert(fabs(exp.variants[1].conversion_rate - 0.15) < 0.01); OK(); }
    TEST("not significant small sample"); { Experiment exp; experiment_init(&exp,"E1","H");
      experiment_add_variant(&exp,"Ctrl"); experiment_add_variant(&exp,"VarA");
      experiment_assign_users(&exp,0,10,1); experiment_assign_users(&exp,1,10,2);
      assert(!experiment_is_significant(&exp)); OK(); }
    TEST("significant large sample"); { Experiment exp; experiment_init(&exp,"E1","H");
      experiment_add_variant(&exp,"Ctrl"); experiment_add_variant(&exp,"VarA");
      experiment_assign_users(&exp,0,5000,500); experiment_assign_users(&exp,1,5000,600);
      assert(experiment_is_significant(&exp)); OK(); }
    TEST("winning variant"); { Experiment exp; experiment_init(&exp,"E1","H");
      experiment_add_variant(&exp,"Ctrl"); experiment_add_variant(&exp,"VarA");
      experiment_set_control(&exp,0);
      experiment_assign_users(&exp,0,5000,500); experiment_assign_users(&exp,1,5000,600);
      assert(experiment_winning_variant(&exp) == 1); OK(); }

    /* Analytics */
    printf("analytics:\n");
    TEST("add cohort"); { Analytics an; analytics_init(&an);
      assert(analytics_add_cohort(&an,"C1",500) == 0); OK(); }
    TEST("record retention"); { Analytics an; analytics_init(&an);
      analytics_add_cohort(&an,"C1",500);
      analytics_record_retention(&an,0,7,200);
      assert(fabs(analytics_retention_at(&an,0,7) - 40.0) < 0.01); OK(); }
    TEST("funnel"); { Analytics an; analytics_init(&an);
      analytics_add_funnel(&an,"Signup");
      analytics_funnel_add_step(&an,0,"Step1",100,80);
      analytics_funnel_add_step(&an,0,"Step2",80,50);
      analytics_funnel_calculate(&an,0);
      assert(fabs(an.funnels[0].overall_conversion - 50.0) < 0.01); OK(); }
    TEST("funnel dropoff"); { Analytics an; analytics_init(&an);
      analytics_add_funnel(&an,"F");
      analytics_funnel_add_step(&an,0,"S",100,60);
      assert(fabs(an.funnels[0].steps[0].dropoff_pct - 40.0) < 0.01); OK(); }

    printf("\n=== Results: %d/%d passed ===\n", passed, tests);
    return passed == tests ? 0 : 1;
}
