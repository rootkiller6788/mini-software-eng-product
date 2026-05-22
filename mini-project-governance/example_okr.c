#include "okr_tracker.h"

#include <stdio.h>
#include <time.h>

int main(void)
{
    printf("=== OKR Tracker Example ===\n\n");

    OKRContext *ctx = okr_context_create();
    if (!ctx) {
        printf("Failed to create OKR context\n");
        return 1;
    }

    time_t q1_end = time(NULL) + 90 * 86400;
    int obj_company = okr_objective_create(ctx,
        "Become the market leader in developer tools",
        "Accelerate product adoption and customer satisfaction",
        OKR_LEVEL_COMPANY, -1, q1_end);

    int obj_eng = okr_objective_create(ctx,
        "Deliver a world-class developer experience",
        "High performance, reliable, and delightful to use",
        OKR_LEVEL_TEAM, obj_company, q1_end);

    int obj_individual = okr_objective_create(ctx,
        "Ship 3 major features with zero critical bugs",
        "Focus on quality and velocity",
        OKR_LEVEL_INDIVIDUAL, obj_eng, q1_end);

    okr_key_result_add(ctx, obj_company, "Monthly active users",
                        10000, 50000, "users", 0.4);
    okr_key_result_add(ctx, obj_company, "Net Promoter Score",
                        30, 60, "NPS", 0.3);
    okr_key_result_add(ctx, obj_company, "Revenue",
                        500000, 2000000, "USD", 0.3);

    okr_key_result_add(ctx, obj_eng, "p99 API latency",
                        500, 50, "ms", 0.4);
    okr_key_result_add(ctx, obj_eng, "Test coverage",
                        45, 90, "%", 0.3);
    okr_key_result_add(ctx, obj_eng, "Developer onboarding time",
                        7, 1, "days", 0.3);

    okr_key_result_add(ctx, obj_individual, "Feature A shipped",
                        0, 1, "done", 0.35);
    okr_key_result_add(ctx, obj_individual, "Feature B shipped",
                        0, 1, "done", 0.35);
    okr_key_result_add(ctx, obj_individual, "Feature C shipped",
                        0, 1, "done", 0.3);

    printf("Objectives created:\n");
    okr_context_summary(ctx);

    okr_key_result_update(ctx, 1, 25000);
    okr_key_result_update(ctx, 2, 42);
    okr_key_result_update(ctx, 3, 800000);

    okr_key_result_update(ctx, 4, 120);
    okr_key_result_update(ctx, 5, 65);
    okr_key_result_update(ctx, 6, 3);

    okr_key_result_update(ctx, 7, 1);
    okr_key_result_update(ctx, 8, 0);
    okr_key_result_update(ctx, 9, 0);

    OKRGradeBreakdown g_company = okr_grade_calculate(ctx, obj_company);
    OKRGradeBreakdown g_eng = okr_grade_calculate(ctx, obj_eng);
    OKRGradeBreakdown g_ind = okr_grade_calculate(ctx, obj_individual);

    printf("\nGrades:\n");
    printf("Company: "); okr_grade_print(&g_company);
    printf("Engineering: "); okr_grade_print(&g_eng);
    printf("Individual: "); okr_grade_print(&g_ind);

    okr_checkin_weekly(ctx, obj_individual, 1, 0.8,
                        "Feature A is on track, Feature B starting this week");
    okr_checkin_weekly(ctx, obj_individual, 2, 0.7,
                        "Feature B delayed due to dependency, Feature A done");
    okr_checkin_weekly(ctx, obj_individual, 3, 0.6,
                        "Feature C blocked on API changes");

    double trend = okr_confidence_trend(ctx, obj_individual);
    printf("\nConfidence trend: %.2f\n", trend);

    cfr_conversation_record(ctx, obj_individual, 1,
        "Had a great 1:1 discussing career growth opportunities");
    cfr_feedback_record(ctx, obj_individual, 2, 1,
        "Great work on Feature A - clean code and well tested!");
    cfr_recognition_record(ctx, obj_individual, 3, 1,
        "Alice went above and beyond helping the QA team");

    printf("\nCFR Entries:\n");
    for (int i = 0; i < ctx->cfr_count; i++) {
        cfr_entry_print(&ctx->cfr_entries[i]);
    }

    int at_risk[16] = {0};
    int risk_count = okr_objective_find_at_risk(ctx, at_risk, 16);
    printf("\nAt-risk objectives: %d\n", risk_count);

    int alignment_issues = okr_alignment_validate(ctx);
    printf("Alignment issues: %d\n", alignment_issues);

    okr_context_free(ctx);
    printf("\nDone.\n");
    return 0;
}
