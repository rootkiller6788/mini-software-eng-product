#include "kanban_board.h"
#include "retrospective.h"
#include "project_tracker.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static void demo_banner(const char *title)
{
    printf("\n");
    printf("============================================================\n");
    printf("  %s\n", title);
    printf("============================================================\n");
}

static void demo_kanban_full_workflow(void)
{
    demo_banner("Part 1: Kanban Board — Full Workflow Simulation");

    KanbanBoard *board = kanban_board_create("Product Engineering");

    kanban_wip_limit_set(board, KANBAN_COL_TODO, 6);
    kanban_wip_limit_set(board, KANBAN_COL_IN_PROGRESS, 4);
    kanban_wip_limit_set(board, KANBAN_COL_REVIEW, 3);

    const char *tasks[] = {
        "Implement dark mode toggle",
        "Fix memory leak in renderer",
        "Add keyboard shortcut editor",
        "Optimize database query for search",
        "Write API documentation",
        "Set up error monitoring (Sentry)",
        "Migrate CSS to CSS Modules",
        "Add PWA offline support",
        "Refactor state management to Zustand",
        "Create onboarding tutorial"
    };
    int priorities[] = {2, 1, 3, 1, 4, 2, 3, 3, 2, 4};
    int card_ids[10];

    printf("Creating %d cards on the board...\n", (int)(sizeof(tasks) / sizeof(tasks[0])));
    for (int i = 0; i < 10; i++) {
        card_ids[i] = kanban_card_create(board, tasks[i],
                                          "Implementation task", priorities[i]);
        printf("  Card #%d: %s (P:%d)\n", card_ids[i], tasks[i], priorities[i]);
    }

    printf("\nInitial state:\n");
    kanban_board_print(board);

    kanban_card_move(board, card_ids[0], KANBAN_COL_TODO);
    kanban_card_move(board, card_ids[1], KANBAN_COL_TODO);
    kanban_card_move(board, card_ids[2], KANBAN_COL_TODO);
    kanban_card_move(board, card_ids[3], KANBAN_COL_TODO);
    kanban_card_move(board, card_ids[4], KANBAN_COL_TODO);
    kanban_card_move(board, card_ids[5], KANBAN_COL_TODO);

    kanban_card_move(board, card_ids[0], KANBAN_COL_IN_PROGRESS);
    kanban_card_move(board, card_ids[1], KANBAN_COL_IN_PROGRESS);
    kanban_card_move(board, card_ids[2], KANBAN_COL_IN_PROGRESS);
    kanban_card_move(board, card_ids[3], KANBAN_COL_IN_PROGRESS);

    kanban_card_move(board, card_ids[0], KANBAN_COL_REVIEW);
    kanban_card_move(board, card_ids[1], KANBAN_COL_REVIEW);

    kanban_card_block(board, card_ids[2], "Waiting on design team");

    kanban_card_move(board, card_ids[0], KANBAN_COL_DONE);

    kanban_card_move(board, card_ids[4], KANBAN_COL_IN_PROGRESS);
    kanban_card_move(board, card_ids[5], KANBAN_COL_IN_PROGRESS);

    kanban_cfd_snapshot(board);

    kanban_card_move(board, card_ids[1], KANBAN_COL_DONE);
    kanban_card_move(board, card_ids[3], KANBAN_COL_REVIEW);
    kanban_card_move(board, card_ids[3], KANBAN_COL_DONE);

    kanban_cfd_snapshot(board);

    kanban_card_unblock(board, card_ids[2]);
    kanban_card_move(board, card_ids[2], KANBAN_COL_REVIEW);
    kanban_card_move(board, card_ids[2], KANBAN_COL_DONE);

    kanban_card_move(board, card_ids[4], KANBAN_COL_DONE);
    kanban_card_move(board, card_ids[5], KANBAN_COL_REVIEW);
    kanban_card_move(board, card_ids[5], KANBAN_COL_DONE);

    kanban_cfd_snapshot(board);

    printf("\nAfter workflow simulation:\n");
    kanban_board_print(board);

    printf("\n--- Flow Metrics ---\n");
    KanbanFlowMetrics metrics = kanban_flow_metrics_calculate(board);
    kanban_metrics_print(&metrics);

    printf("\n--- Bottleneck Analysis ---\n");
    KanbanBottleneckReport bottlenecks[8];
    int bn_count = kanban_bottleneck_detect(board, bottlenecks, 8);
    kanban_bottleneck_print(bottlenecks, bn_count);

    printf("\n--- CFD (Cumulative Flow Diagram) ---\n");
    printf("Date       | Backlog | Todo | InProgress | Review | Done\n");
    printf("-----------+---------+------+------------+--------+------\n");
    KanbanCFDPoint cfd[32];
    int cfd_count;
    kanban_cfd_generate(board, cfd, 32, &cfd_count);
    for (int i = 0; i < cfd_count; i++) {
        printf("Snapshot %2d | %7d | %4d | %10d | %6d | %4d\n",
               i,
               cfd[i].column_counts[KANBAN_COL_BACKLOG],
               cfd[i].column_counts[KANBAN_COL_TODO],
               cfd[i].column_counts[KANBAN_COL_IN_PROGRESS],
               cfd[i].column_counts[KANBAN_COL_REVIEW],
               cfd[i].column_counts[KANBAN_COL_DONE]);
    }

    printf("\n--- Card Details ---\n");
    for (int i = 0; i < 6; i++) {
        KanbanCard *card = kanban_card_find(board, card_ids[i]);
        if (card) {
            double cycle = kanban_cycle_time_calculate(card);
            double lead = kanban_lead_time_calculate(card);
            printf("  #%d %s | Cycle:%.1fd | Lead:%.1fd | %s\n",
                   card->id, card->title, cycle, lead,
                   kanban_column_type_name(card->column));
        }
    }

    kanban_board_free(board);
}

static void demo_retrospective_full_workflow(void)
{
    demo_banner("Part 2: Retrospective — Full Workflow");

    RetroContext *ctx = retro_context_create();

    printf("--- Retrospective #1: Start/Stop/Continue ---\n");
    RetrospectiveSession *ret1 = retro_session_create(ctx,
        RETRO_FORMAT_START_STOP_CONTINUE, 1);

    retro_add_item_ssc(ret1, RETRO_CAT_START, "Start writing unit tests before merging",
                        "Alice");
    retro_add_item_ssc(ret1, RETRO_CAT_START, "Start daily design reviews",
                        "Bob");
    retro_add_item_ssc(ret1, RETRO_CAT_STOP, "Stop merging PRs without review",
                        "Charlie");
    retro_add_item_ssc(ret1, RETRO_CAT_STOP, "Stop skipping standup",
                        "Alice");
    retro_add_item_ssc(ret1, RETRO_CAT_CONTINUE, "Continue pair programming sessions",
                        "Bob");
    retro_add_item_ssc(ret1, RETRO_CAT_CONTINUE, "Continue weekly knowledge sharing",
                        "Diana");

    retro_action_item_create(ret1, "Set up pre-commit hook for unit tests",
                              "Alice", 1);
    retro_action_item_create(ret1, "Add CODEOWNERS file for mandatory reviews",
                              "Bob", 1);
    retro_action_item_create(ret1, "Schedule recurring design review meeting",
                              "Charlie", 2);

    retro_session_print(ret1);

    printf("\n--- Retrospective #2: Mad/Sad/Glad ---\n");
    RetrospectiveSession *ret2 = retro_session_create(ctx,
        RETRO_FORMAT_MAD_SAD_GLAD, 2);

    retro_add_item_msg(ret2, RETRO_CAT_MAD, "Production deploy broke on Friday night",
                        "Alice");
    retro_add_item_msg(ret2, RETRO_CAT_MAD, "Unclear requirements wasted 2 days",
                        "Bob");
    retro_add_item_msg(ret2, RETRO_CAT_SAD, "Team member leaving the project",
                        "Charlie");
    retro_add_item_msg(ret2, RETRO_CAT_SAD, "Tech debt slowing feature delivery",
                        "Diana");
    retro_add_item_msg(ret2, RETRO_CAT_GLAD, "New CI pipeline cut deploy time by 50%",
                        "Alice");
    retro_add_item_msg(ret2, RETRO_CAT_GLAD, "Customer feedback is very positive",
                        "Bob");
    retro_add_item_msg(ret2, RETRO_CAT_GLAD, "Team successfully shipped 3 features",
                        "Diana");

    int a1 = retro_action_item_create(ret2, "No Friday deploys: update release policy",
                                       "Charlie", 1);
    retro_action_item_create(ret2, "Schedule tech debt sprint for Q2",
                              "Bob", 2);

    retro_session_print(ret2);

    printf("\n--- Retrospective #3: 4Ls ---\n");
    RetrospectiveSession *ret3 = retro_session_create(ctx,
        RETRO_FORMAT_FOUR_LS, 3);

    retro_add_item_4ls(ret3, RETRO_CAT_LIKED, "New code review process is efficient",
                        "Alice");
    retro_add_item_4ls(ret3, RETRO_CAT_LIKED, "Team retro is always honest and productive",
                        "Bob");
    retro_add_item_4ls(ret3, RETRO_CAT_LEARNED, "Learned that writing RFCs saves time",
                        "Charlie");
    retro_add_item_4ls(ret3, RETRO_CAT_LEARNED, "Learned to estimate better with planning poker",
                        "Diana");
    retro_add_item_4ls(ret3, RETRO_CAT_LACKED, "Lacked clear ownership boundaries",
                        "Alice");
    retro_add_item_4ls(ret3, RETRO_CAT_LACKED, "Lacked integration test coverage",
                        "Bob");
    retro_add_item_4ls(ret3, RETRO_CAT_LONGED_FOR, "Longed for better monitoring dashboards",
                        "Charlie");
    retro_add_item_4ls(ret3, RETRO_CAT_LONGED_FOR, "Longed for in-person team meetup",
                        "Diana");

    retro_session_print(ret3);

    retro_action_item_track(ctx, a1, ACTION_STATUS_IN_PROGRESS);
    printf("\nTracking action items...\n");
    printf("  Action #%d: In Progress\n", a1);

    int pending[32];
    int pend_count = retro_action_item_find_pending(ctx, pending, 32);
    printf("\nPending action items: %d\n", pend_count);

    printf("\n--- Health Check ---\n");
    RetroHealthCheck *health = retro_health_check_create(3);

    const char *hq[] = {
        "How clear was the sprint goal?",
        "How effective was sprint planning?",
        "How well did the team collaborate?",
        "How manageable was the workload?",
        "How satisfied are you with the delivered quality?"
    };
    for (int i = 0; i < 5; i++) {
        retro_health_question_add(health, hq[i]);
    }

    double scores[] = {4.2, 3.8, 4.5, 3.0, 4.1};
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 6; j++) {
            retro_health_answer_record(health, i, scores[i] + (j - 2.5) * 0.4);
        }
    }

    health->overall_health = retro_health_score(health);
    retro_health_print(health);

    printf("\n--- Team Morale Survey ---\n");
    TeamMoraleSurvey *morale = retro_morale_survey_create();

    const char *mq[] = {
        "I feel motivated at work",
        "My work is valued by the team",
        "I have growth opportunities",
        "Work-life balance is healthy",
        "I trust my team members",
        "Communication is open and honest"
    };
    for (int i = 0; i < 6; i++) {
        retro_morale_question_add(morale, mq[i]);
    }

    int responses[6][3] = {
        {7, 2, 1}, {8, 1, 1}, {5, 3, 2},
        {6, 2, 2}, {9, 1, 0}, {5, 4, 1}
    };
    for (int i = 0; i < 6; i++) {
        for (int t = 0; t < 3; t++) {
            for (int r = 0; r < responses[i][t]; r++) {
                retro_morale_answer_record(morale, i, t);
            }
        }
    }

    morale->morale_index = retro_morale_index(morale);
    retro_morale_print(morale);

    printf("\n--- Psychological Safety Assessment ---\n");
    PsychSafetyAssessment *psych = retro_psych_safety_assess();

    const char *pq[] = {
        "I can openly discuss mistakes without fear",
        "Team members value my ideas and opinions",
        "Disagreements are resolved constructively",
        "I feel safe asking for help",
        "Risks and concerns are shared freely",
        "Different perspectives are welcomed",
        "It is safe to experiment and fail",
        "I can be my authentic self at work"
    };
    for (int i = 0; i < 8; i++) {
        retro_psych_question_add(psych, pq[i]);
    }

    double psych_scores[] = {4.5, 4.2, 3.8, 4.7, 3.5, 4.0, 3.2, 4.8};
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 10; j++) {
            retro_psych_answer_record(psych, i, psych_scores[i]);
        }
    }

    psych->safety_index = retro_psych_safety_index(psych);
    psych->safe_participants = retro_psych_safe_threshold(psych, 4.0);
    psych->total_participants = 10;
    retro_psych_print(psych);

    printf("\nSafe threshold >= 4.0: %d/%d areas\n",
           psych->safe_participants, psych->question_count);

    free(health);
    free(morale);
    free(psych);
    retro_context_free(ctx);
}

static void demo_combined_insights(void)
{
    demo_banner("Part 3: Combined Flow & Retro Insights");

    printf("Flow efficiency improvement tracking:\n\n");

    printf("  Sprint 1 Retro actions:\n");
    printf("   - Added CODEOWNERS review policy  -> Reduced review cycle time\n");
    printf("   - Pre-commit hooks for tests     -> Fewer bugs reaching review\n");
    printf("   - Design review meetings         -> Fewer reworks in progress\n");

    printf("\n  Sprint 2 Retro actions:\n");
    printf("   - No Friday deploys policy       -> Reduced production incidents\n");
    printf("   - Tech debt sprint planned       -> Expected 30%% velocity improvement\n");

    printf("\n  Impact on Kanban Flow:\n");
    printf("   Cycle time: 3.2d -> 1.8d (44%% improvement)\n");
    printf("   WIP: 4 items -> 3 items (better flow)\n");
    printf("   Throughput: 4.5/wk -> 7.2/wk (60%% improvement)\n");

    printf("\n  Psychological Safety Trend:\n");
    printf("   Sprint 1: 0.72\n");
    printf("   Sprint 3: 0.80 (+11%%)\n");

    printf("\n  Morale Index:\n");
    printf("   Current: 0.72\n");
    printf("   Action: Schedule team building event\n");
    printf("           Address work-life balance concerns\n");
}

int main(void)
{
    printf("=== Project Governance Demo: Kanban + Retrospective ===\n\n");
    printf("This demo showcases:\n");
    printf("  1. Kanban board with WIP limits, CFD, flow metrics, bottleneck detection\n");
    printf("  2. Three retrospective formats (Start/Stop/Continue, Mad/Sad/Glad, 4Ls)\n");
    printf("  3. Action item tracking, health checks, morale surveys, psych safety\n\n");

    demo_kanban_full_workflow();
    demo_retrospective_full_workflow();
    demo_combined_insights();

    printf("\n============================================================\n");
    printf("  All demonstrations complete.\n");
    printf("============================================================\n");
    return 0;
}
