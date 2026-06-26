#include "agile_scrum.h"

#include <stdio.h>
#include <time.h>

int main(void)
{
    printf("=== Agile Scrum Example ===\n\n");

    ScrumContext *ctx = scrum_context_create();
    if (!ctx) {
        printf("Failed to create scrum context\n");
        return 1;
    }

    int item1 = scrum_backlog_add(ctx, "User login page", "Implement OAuth2 login flow",
                                   5, SCRUM_PRIORITY_HIGH);
    int item2 = scrum_backlog_add(ctx, "Dashboard widget", "Create summary dashboard",
                                   3, SCRUM_PRIORITY_MEDIUM);
    int item3 = scrum_backlog_add(ctx, "API rate limiting", "Protect API from abuse",
                                   8, SCRUM_PRIORITY_CRITICAL);
    int item4 = scrum_backlog_add(ctx, "Email notifications", "Send email alerts",
                                   2, SCRUM_PRIORITY_LOW);
    int item5 = scrum_backlog_add(ctx, "Search feature", "Full-text search for docs",
                                   5, SCRUM_PRIORITY_MEDIUM);

    printf("Backlog after adding items:\n");
    scrum_backlog_print(ctx);

    ScrumSprint *sprint = scrum_sprint_create(ctx, "Sprint 1",
                                               "Deliver core auth & dashboard",
                                               14);
    if (sprint) {
        int ids[] = { item1, item2, item3 };
        scrum_sprint_plan(ctx, sprint->id, ids, 3);

        printf("\nSprint created:\n");
        scrum_sprint_print(sprint);

        scrum_sprint_start(ctx, sprint->id);
        printf("\nSprint started. Recording standup...\n");

        scrum_standup_record(ctx, sprint->id, 1, "Alice",
                              "Worked on login form UI",
                              "Continue with OAuth integration",
                              "Waiting for OAuth provider credentials");
        scrum_standup_record(ctx, sprint->id, 2, "Bob",
                              "Set up dashboard layout",
                              "Add real data to dashboard",
                              "None");

        printf("\nSprint review:\n");
        scrum_review_conduct(ctx, sprint->id,
                              "Login flow works, dashboard shows mock data",
                              "Stakeholders liked the login flow, want dark mode",
                              2, 1);

        scrum_sprint_complete(ctx, sprint->id);

        ScrumVelocityReport vel = scrum_velocity_calculate(ctx);
        scrum_velocity_print(&vel);

        printf("\nBurndown chart data:\n");
        ScrumBurndownPoint points[14];
        int bd_count = scrum_burndown_generate(ctx, sprint->id, points, 14);
        for (int d = 0; d < bd_count; d++) {
            printf("  Day %d: %d points remaining\n",
                   points[d].day_index, points[d].points_remaining);
        }

        printf("\nSprint health: %.2f\n",
               scrum_sprint_health_score(ctx, sprint->id));

        scrum_backlog_refine(ctx, NULL);
    }

    scrum_context_free(ctx);
    printf("\nDone.\n");
    return 0;
}
