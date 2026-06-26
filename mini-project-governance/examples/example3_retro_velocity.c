#include "gov_governance.h"
#include <stdio.h>

int main(void) {
    printf("=== Example 3: Retrospective & Velocity Analysis ===\n\n");

    RetroLog retro_log;
    retro_init(&retro_log);

    /* Sprint 1 retrospective */
    retro_create_session(&retro_log, 1, "Setup CI/CD pipeline", RETRO_START_STOP_CONTINUE, "Alice");
    retro_add_item(&retro_log, 0, "Pair programming sessions were very helpful",
                   "Bob", RETRO_CAT_WHAT_WENT_WELL);
    retro_add_item(&retro_log, 0, "CI pipeline took too long to configure",
                   "Charlie", RETRO_CAT_WHAT_WENT_WRONG);
    retro_add_item(&retro_log, 0, "Need better test data management",
                   "Diana", RETRO_CAT_WHAT_TO_IMPROVE);
    retro_add_item(&retro_log, 0, "Thanks to ops team for quick infra setup",
                   "Eve", RETRO_CAT_APPRECIATION);

    /* Vote on items */
    retro_vote(&retro_log, 0, 0);
    retro_vote(&retro_log, 0, 0);
    retro_vote(&retro_log, 0, 1);
    retro_vote(&retro_log, 0, 1);
    retro_vote(&retro_log, 0, 1);
    retro_vote(&retro_log, 0, 2);

    /* Create action items */
    retro_add_action(&retro_log, 0, "Set up test data factory", "Charlie", 2000000000, 7, 5);
    retro_add_action(&retro_log, 0, "Optimize CI config", "Diana", 2000000000, 8, 3);

    /* Health check */
    retro_add_health(&retro_log, 0, "Alice", 4.0, 4.5, 3.5, 4.0, 4.5, 4.0);
    retro_add_health(&retro_log, 0, "Bob", 3.5, 3.5, 3.0, 3.5, 4.0, 3.5);
    retro_add_health(&retro_log, 0, "Charlie", 4.5, 4.0, 4.0, 4.0, 4.5, 4.5);
    retro_add_health(&retro_log, 0, "Diana", 3.0, 3.0, 3.5, 4.5, 3.5, 3.0);
    retro_add_health(&retro_log, 0, "Eve", 4.0, 4.0, 3.5, 4.5, 4.0, 4.0);

    retro_print_session(&retro_log, 0);

    /* Sprint 2 retrospective */
    retro_create_session(&retro_log, 2, "User auth module", RETRO_GLAD_SAD_MAD, "Alice");
    retro_add_item(&retro_log, 1, "Auth module completed ahead of schedule",
                   "Bob", RETRO_CAT_WHAT_WENT_WELL);
    retro_add_item(&retro_log, 1, "Integration tests broke repeatedly",
                   "Charlie", RETRO_CAT_WHAT_WENT_WRONG);
    retro_add_item(&retro_log, 1, "Adopt test containers for integration tests",
                   "Diana", RETRO_CAT_WHAT_TO_IMPROVE);
    retro_vote(&retro_log, 1, 1); retro_vote(&retro_log, 1, 1); retro_vote(&retro_log, 1, 1);
    retro_vote(&retro_log, 1, 2);

    retro_add_action(&retro_log, 1, "Implement testcontainers for Postgres", "Charlie", 2000000000, 9, 4);

    retro_add_health(&retro_log, 1, "Alice", 4.2, 4.5, 4.0, 4.0, 4.5, 4.2);
    retro_add_health(&retro_log, 1, "Bob", 4.0, 4.0, 3.5, 4.0, 4.5, 4.0);
    retro_add_health(&retro_log, 1, "Charlie", 4.5, 4.2, 4.5, 4.5, 4.5, 4.5);
    retro_add_health(&retro_log, 1, "Diana", 3.5, 3.5, 4.0, 4.5, 4.0, 3.5);
    retro_add_health(&retro_log, 1, "Eve", 4.5, 4.5, 4.0, 4.5, 4.5, 4.5);

    retro_print_session(&retro_log, 1);
    retro_print_summary(&retro_log);
    retro_print_health_trend(&retro_log);

    /* Velocity analysis */
    printf("\n=== Velocity Profile ===\n");
    VelocityProfile vp;
    velocity_init(&vp, "Platform Team", 3);
    velocity_record(&vp, 1, 30, 25, 8, 7, 40, 2, 2);
    velocity_record(&vp, 2, 35, 32, 10, 9, 40, 1, 1);
    velocity_record(&vp, 3, 25, 22, 7, 6, 38, 3, 1);
    velocity_record(&vp, 4, 32, 30, 9, 8, 40, 0, 0);
    velocity_print_profile(&vp);

    /* Forecast remaining work */
    Forecast f;
    forecast_init(&f, FORECAST_ROLLING_AVG);
    forecast_rolling(&vp, &f, 150);
    velocity_print_forecast(&f);

    /* Try weighted forecast */
    Forecast fw;
    forecast_init(&fw, FORECAST_WEIGHTED);
    forecast_weighted(&vp, &fw, 150, 0.7);
    printf("\nWeighted forecast: %.1f points next sprint\n", fw.predicted_points);

    /* 5 Whys analysis */
    printf("\n=== 5 Whys Root Cause Analysis ===\n");
    FiveWhys fw5;
    five_whys_init(&fw5, "Integration tests keep failing randomly");
    five_whys_add(&fw5, 0, "Database state is not cleaned between tests");
    five_whys_add(&fw5, 1, "Tests share a single database instance");
    five_whys_add(&fw5, 2, "We don't use test containers or isolated DBs");
    five_whys_add(&fw5, 3, "Testcontainers integration was never prioritized");
    five_whys_add(&fw5, 4, "No clear ownership for test infrastructure");
    five_whys_solve(&fw5, "No test infrastructure owner",
                    "Assign Charlie as Test Infra Lead, allocate 20% time for testcontainers setup");
    five_whys_print(&fw5);

    return 0;
}