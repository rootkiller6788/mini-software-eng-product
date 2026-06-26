#include "gov_governance.h"
#include <stdio.h>

int main(void) {
    printf("=== Example 2: Kanban Board with OKR Tracking ===\n\n");

    GovernanceProgram gp;
    gov_program_init(&gp, "Platform Team", "Maintain and improve developer platform", GOV_KANBAN);

    /* Setup Kanban board */
    kanban_add_column(&gp.kanban_board, "Backlog", 20, false);
    kanban_add_column(&gp.kanban_board, "Analysis", 5, false);
    kanban_add_column(&gp.kanban_board, "Development", 5, false);
    kanban_add_column(&gp.kanban_board, "Code Review", 3, false);
    kanban_add_column(&gp.kanban_board, "Testing", 3, false);
    kanban_add_column(&gp.kanban_board, "Done", 0, true);

    /* Add cards */
    kanban_add_card(&gp.kanban_board, "K-1", "Fix CI pipeline flaky test", COS_EXPEDITE, 5, 0);
    kanban_add_card(&gp.kanban_board, "K-2", "Add code coverage dashboard", COS_STANDARD, 8, 0);
    kanban_add_card(&gp.kanban_board, "K-3", "Upgrade Postgres to v16", COS_FIXED_DATE, 13, 1700000000);
    kanban_add_card(&gp.kanban_board, "K-4", "Improve deployment docs", COS_INTANGIBLE, 3, 0);
    kanban_add_card(&gp.kanban_board, "K-5", "Performance benchmark suite", COS_STANDARD, 8, 0);

    /* Simulate flow */
    kanban_move_card(&gp.kanban_board, "K-1", 1);
    kanban_move_card(&gp.kanban_board, "K-1", 2);
    kanban_move_card(&gp.kanban_board, "K-1", 3);
    kanban_move_card(&gp.kanban_board, "K-2", 1);
    kanban_move_card(&gp.kanban_board, "K-4", 1);
    kanban_move_card(&gp.kanban_board, "K-4", 2);
    kanban_block_card(&gp.kanban_board, "K-2", "Waiting for access to coverage tool");

    kanban_print_board(&gp.kanban_board);
    kanban_print_metrics(&gp.kanban_board);

    /* Setup OKRs for the platform team */
    okr_add_objective(&gp.okr_dashboard, "O1", "Improve Developer Productivity",
                      "Make developers 2x more productive with better tooling",
                      false, 1000000, 2000000);
    okr_add_key_result(&gp.okr_dashboard, "KR1", "Reduce CI build time to <5min",
                       KR_ABSOLUTE, 5, 15, 0, 40);
    okr_add_key_result(&gp.okr_dashboard, "KR2", "Deployment frequency to 10/day",
                       KR_ABSOLUTE, 10, 3, 0, 30);
    okr_add_key_result(&gp.okr_dashboard, "KR3", "Developer satisfaction score >4.2/5",
                       KR_ABSOLUTE, 4.2, 3.5, 0, 30);

    /* Simulate progress */
    okr_update_kr(&gp.okr_dashboard, "KR1", 8);
    okr_update_kr(&gp.okr_dashboard, "KR2", 7);
    okr_update_kr(&gp.okr_dashboard, "KR3", 4.0);

    okr_print_dashboard(&gp.okr_dashboard);

    /* Cumulative flow */
    CumulativeFlowDiagram cfd;
    cfd_init(&cfd);
    for (int d = 0; d < 5; d++) {
        cfd_record(&cfd, &gp.kanban_board, d);
    }
    cfd_print(&cfd);

    /* Add risks */
    gov_add_risk(&gp, "R1", "CI infrastructure instability", 0.4, 7, "DevOps Lead", "Add fallback runners");
    gov_add_risk(&gp, "R2", "Key developer departure", 0.2, 9, "EM", "Document all processes");
    gov_print_risks(&gp);

    return 0;
}