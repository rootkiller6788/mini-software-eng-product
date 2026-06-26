#include "gov_governance.h"
#include <stdio.h>

int main(void) {
    printf("=== Example 1: Full Scrum Sprint Simulation ===\n\n");

    GovernanceProgram gp;
    gov_program_init(&gp, "MiniApp v2.0", "Launch mobile app with AI features", GOV_SCRUM);

    /* Setup team */
    scrum_add_member(&gp.scrum_board.team, "Alice", ROLE_SCRUM_MASTER, 100);
    scrum_add_member(&gp.scrum_board.team, "Bob", ROLE_PRODUCT_OWNER, 100);
    scrum_add_member(&gp.scrum_board.team, "Charlie", ROLE_DEVELOPER, 100);
    scrum_add_member(&gp.scrum_board.team, "Diana", ROLE_DEVELOPER, 100);
    scrum_add_member(&gp.scrum_board.team, "Eve", ROLE_DEVELOPER, 80);

    /* Create product backlog */
    pbi_add(&gp.scrum_board.backlog, "US-01", "User Login with OAuth", PBI_USER_STORY, 8, 8, 40);
    pbi_add(&gp.scrum_board.backlog, "US-02", "User Profile Page", PBI_USER_STORY, 5, 6, 25);
    pbi_add(&gp.scrum_board.backlog, "US-03", "Dashboard with Charts", PBI_FEATURE, 13, 9, 80);
    pbi_add(&gp.scrum_board.backlog, "US-04", "Push Notifications", PBI_FEATURE, 8, 7, 40);
    pbi_add(&gp.scrum_board.backlog, "US-05", "Search Functionality", PBI_FEATURE, 13, 8, 65);
    pbi_add(&gp.scrum_board.backlog, "US-06", "Settings Page", PBI_USER_STORY, 3, 4, 15);
    pbi_add(&gp.scrum_board.backlog, "BUG-01", "Fix memory leak in image upload", PBI_BUG, 5, 10, 25);
    pbi_add(&gp.scrum_board.backlog, "TD-01", "Refactor auth module", PBI_TECH_DEBT, 8, 3, 32);

    /* Setup dependencies */
    pbi_add_dependency(&gp.scrum_board.backlog, "US-02", "US-01");
    pbi_add_dependency(&gp.scrum_board.backlog, "US-03", "US-01");

    /* Sort by WSJF */
    pbi_sort_by_wsjf(&gp.scrum_board.backlog);

    printf("Backlog (sorted by WSJF):\n");
    for (int i = 0; i < gp.scrum_board.backlog.count; i++) {
        ProductBacklogItem *p = &gp.scrum_board.backlog.items[i];
        printf("  %s: %s [%d pts, value=%d, WSJF=%.2f]\n",
               p->id, p->title, p->story_points, p->business_value, p->wsjf_score);
    }

    /* Create and run sprints */
    printf("\n--- Sprint 1 ---\n");
    sprint_create(&gp.scrum_board, "Core auth + profile", 10);
    sprint_plan(&gp.scrum_board, 0);
    sprint_start(&gp.scrum_board, 0);
    /* Simulate progress */
    gp.scrum_board.backlog.items[0].state = PBI_IN_PROGRESS;
    gp.scrum_board.sprints[0].completed_points = 13;
    gp.scrum_board.sprints[0].completed_items = 2;
    sprint_record_burndown(&gp.scrum_board.sprints[0], 0, 13);
    sprint_record_burndown(&gp.scrum_board.sprints[0], 5, 5);
    sprint_record_burndown(&gp.scrum_board.sprints[0], 9, 0);
    sprint_finish(&gp.scrum_board, 0);
    scrum_print_sprint_report(&gp.scrum_board, 0);

    printf("\n--- Sprint 2 ---\n");
    sprint_create(&gp.scrum_board, "Dashboard + notifications", 10);
    sprint_plan(&gp.scrum_board, 1);
    sprint_start(&gp.scrum_board, 1);
    gp.scrum_board.sprints[1].completed_points = 18;
    gp.scrum_board.sprints[1].completed_items = 2;
    sprint_finish(&gp.scrum_board, 1);
    scrum_print_sprint_report(&gp.scrum_board, 1);

    /* Predict completion */
    int remaining = 0;
    for (int i = 0; i < gp.scrum_board.backlog.count; i++) {
        if (gp.scrum_board.backlog.items[i].state != PBI_ACCEPTED)
            remaining += gp.scrum_board.backlog.items[i].story_points;
    }
    printf("\n--- Forecast ---\n");
    printf("  Remaining points: %d\n", remaining);
    printf("  Avg velocity: %.1f\n", gp.scrum_board.avg_velocity);
    printf("  Estimated sprints to complete: %.1f\n",
           scrum_predict_completion(&gp.scrum_board, remaining));

    /* Velocity chart */
    scrum_print_velocity_chart(&gp.scrum_board);

    /* Scorecard */
    GovernanceScorecard sc;
    gov_calc_scorecard(&gp, &sc);
    gov_print_scorecard(&sc);

    return 0;
}