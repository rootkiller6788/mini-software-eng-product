#include "agile_scrum.h"
#include "project_tracker.h"
#include "okr_tracker.h"

#include <stdio.h>
#include <time.h>
#include <string.h>

static void demo_banner(const char *title)
{
    printf("\n");
    printf("============================================================\n");
    printf("  %s\n", title);
    printf("============================================================\n");
}

static void demo_scenario_1(void)
{
    demo_banner("Scenario 1: Full Sprint Lifecycle");

    ScrumContext *scrum = scrum_context_create();

    scrum_backlog_add(scrum, "User registration", "Sign up with email verification", 5, SCRUM_PRIORITY_HIGH);
    scrum_backlog_add(scrum, "User profile page", "View and edit profile", 3, SCRUM_PRIORITY_MEDIUM);
    scrum_backlog_add(scrum, "Password reset flow", "Forgot password email flow", 3, SCRUM_PRIORITY_HIGH);
    scrum_backlog_add(scrum, "Two-factor auth", "TOTP-based 2FA", 8, SCRUM_PRIORITY_MEDIUM);
    scrum_backlog_add(scrum, "Session management", "View and revoke active sessions", 5, SCRUM_PRIORITY_LOW);
    scrum_backlog_add(scrum, "OAuth provider integration", "Google/GitHub OAuth", 5, SCRUM_PRIORITY_MEDIUM);
    scrum_backlog_add(scrum, "Rate limiting", "Per-user API rate limiting", 3, SCRUM_PRIORITY_HIGH);
    scrum_backlog_add(scrum, "Audit logging", "Log all admin actions", 5, SCRUM_PRIORITY_LOW);

    printf("Initial backlog (%d items):\n", scrum->product_backlog.count);
    for (int i = 0; i < scrum->product_backlog.count; i++) {
        printf("  [%d] %s | %d SP | P:%d\n",
               scrum->product_backlog.items[i].id,
               scrum->product_backlog.items[i].title,
               scrum->product_backlog.items[i].story_points,
               scrum->product_backlog.items[i].priority);
    }

    ScrumSprint *s1 = scrum_sprint_create(scrum, "Sprint Alpha",
                                           "Core authentication MVP", 14);
    int plan1[] = {1, 2, 3, 7};
    scrum_sprint_plan(scrum, s1->id, plan1, 4);
    scrum_sprint_start(scrum, s1->id);

    printf("\n--- Sprint Alpha Started ---\n");
    scrum_sprint_print(s1);

    for (int day = 1; day <= 5; day++) {
        scrum_standup_record(scrum, s1->id, 1, "Alice",
                              day == 1 ? "Setup project structure" : "Working on registration form",
                              "Continue registration validation",
                              day == 3 ? "Need design specs for error states" : "None");
        scrum_standup_record(scrum, s1->id, 2, "Bob",
                              day == 1 ? "Research auth libraries" : "Implementing password reset",
                              "Complete password reset flow",
                              "None");
    }

    printf("\nStandup log (%d entries):\n", scrum->standup_count);
    for (int i = 0; i < scrum->standup_count && i < 3; i++) {
        scrum_standup_print(&scrum->standup_log[i]);
        printf("\n");
    }
    if (scrum->standup_count > 3) {
        printf("  ... and %d more entries\n", scrum->standup_count - 3);
    }

    scrum_review_conduct(scrum, s1->id,
                          "Registration, profile, and password reset all functional.\n  Demo: user signs up, edits profile, resets password successfully.",
                          "Stakeholder feedback: great progress! Want OAuth ASAP.\n  Concern: 2FA not started yet.",
                          3, 1);
    s1->story_points_completed = 11;
    scrum_sprint_complete(scrum, s1->id);

    printf("\n--- Sprint Alpha Completed ---\n");
    ScrumVelocityReport vel = scrum_velocity_calculate(scrum);
    scrum_velocity_print(&vel);

    ScrumSprint *s2 = scrum_sprint_create(scrum, "Sprint Beta",
                                           "Security & session features", 14);
    int plan2[] = {4, 5, 8};
    scrum_sprint_plan(scrum, s2->id, plan2, 3);
    scrum_sprint_start(scrum, s2->id);

    s2->story_points_completed = 10;
    scrum_sprint_complete(scrum, s2->id);

    vel = scrum_velocity_calculate(scrum);
    printf("\nUpdated velocity after 2 sprints:\n");
    scrum_velocity_print(&vel);

    printf("\nBurndown for Sprint Alpha:\n");
    ScrumBurndownPoint bd[14];
    int bd_count = scrum_burndown_generate(scrum, 1, bd, 14);
    for (int d = 0; d < bd_count; d++) {
        printf("  Day %2d: ", bd[d].day_index);
        int bars = (int)((double)bd[d].points_remaining / 16.0 * 20);
        for (int b = 0; b < bars; b++) printf("#");
        printf(" %d SP\n", bd[d].points_remaining);
    }

    printf("\nBurnup for Sprint Beta:\n");
    int scope[14], completed[14];
    int bu_count = scrum_burnup_generate(scrum, 2, scope, completed, 14);
    for (int d = 0; d < bu_count; d++) {
        printf("  Day %2d: scope=%d, completed=%d\n", d, scope[d], completed[d]);
    }

    scrum_context_free(scrum);
}

static void demo_scenario_2(void)
{
    demo_banner("Scenario 2: Project Tracker with JQL Filtering");

    ProjectTracker *tracker = pt_tracker_create();

    int i1 = pt_issue_create(tracker, ISSUE_TYPE_STORY, "Shopping cart persistence",
                              "Save cart across sessions", ISSUE_PRIORITY_MAJOR);
    int i2 = pt_issue_create(tracker, ISSUE_TYPE_BUG, "Checkout crash on IE11",
                              "Null pointer in checkout handler", ISSUE_PRIORITY_CRITICAL);
    int i3 = pt_issue_create(tracker, ISSUE_TYPE_TASK, "Set up CI/CD pipeline",
                              "GitHub Actions workflow", ISSUE_PRIORITY_MAJOR);
    int i4 = pt_issue_create(tracker, ISSUE_TYPE_EPIC, "Payment system overhaul",
                              "Integrate Stripe + PayPal", ISSUE_PRIORITY_BLOCKER);
    int i5 = pt_issue_create(tracker, ISSUE_TYPE_STORY, "Product recommendations",
                              "ML-based recommendations", ISSUE_PRIORITY_MINOR);

    pt_issue_assign(tracker, i1, 101);
    pt_issue_assign(tracker, i2, 102);
    pt_issue_assign(tracker, i3, 103);

    pt_issue_update(tracker, i1, ISSUE_STATUS_IN_PROGRESS);
    pt_issue_update(tracker, i2, ISSUE_STATUS_IN_PROGRESS);
    pt_issue_update(tracker, i3, ISSUE_STATUS_DONE);

    pt_issue_set_sprint(tracker, i1, 1);
    pt_issue_set_sprint(tracker, i2, 1);
    pt_issue_set_sprint(tracker, i3, 1);

    pt_issue_set_points(tracker, i1, 5);
    pt_issue_set_points(tracker, i2, 3);
    pt_issue_set_points(tracker, i3, 2);

    pt_label_create(tracker, "frontend", 0xFF0000);
    pt_label_create(tracker, "backend", 0x00FF00);
    pt_label_create(tracker, "bugfix", 0xFF6666);

    pt_label_assign_issue(tracker, i1, 0);
    pt_label_assign_issue(tracker, i2, 0);
    pt_label_assign_issue(tracker, i3, 1);
    pt_label_assign_issue(tracker, i4, 1);

    pt_component_create(tracker, "Checkout", "Shopping cart and payment");
    pt_component_create(tracker, "Auth", "Authentication system");
    pt_component_create(tracker, "Infra", "Infrastructure");

    pt_component_assign_issue(tracker, i2, 0);
    pt_component_assign_issue(tracker, i3, 2);

    pt_issue_block(tracker, i4, "Waiting for vendor agreement");

    printf("Board view:\n");
    pt_board_view(tracker);

    printf("\nList view (In Progress):\n");
    pt_list_view(tracker, ISSUE_STATUS_IN_PROGRESS);

    FilterQuery q1 = {0};
    q1.conditions[0].field[0] = 't'; q1.conditions[0].field[1] = 'y';  q1.conditions[0].field[2] = 'p'; q1.conditions[0].field[3] = 'e';  q1.conditions[0].field[4] = '\0';
    q1.conditions[0].op = FILTER_EQ;
    strcpy(q1.conditions[0].value, "1");
    q1.condition_count = 1;

    int results[16];
    int rc = pt_filter_jql(tracker, &q1, results, 16);
    printf("\nJQL filter 'type=Bug': %d results\n", rc);
    for (int i = 0; i < rc; i++) {
        ProjectIssue *issue = pt_issue_find(tracker, results[i]);
        if (issue) pt_issue_print(issue);
    }

    FilterQuery q2 = {0};
    q2.conditions[0].field[0] = 's'; q2.conditions[0].field[1] = 'p';  q2.conditions[0].field[2] = 'r'; q2.conditions[0].field[3] = 'i';  q2.conditions[0].field[4] = 'n'; q2.conditions[0].field[5] = 't';  q2.conditions[0].field[6] = '\0';
    q2.conditions[0].op = FILTER_EQ;
    strcpy(q2.conditions[0].value, "1");
    q2.condition_count = 1;

    rc = pt_filter_jql(tracker, &q2, results, 16);
    printf("\nJQL filter 'sprint=1': %d results\n", rc);
    for (int i = 0; i < rc; i++) {
        ProjectIssue *issue = pt_issue_find(tracker, results[i]);
        if (issue) pt_issue_print(issue);
    }

    SprintMetrics sm = pt_sprint_metrics_calculate(tracker, 1);
    pt_sprint_metrics_print(&sm);

    pt_tracker_free(tracker);
}

static void demo_scenario_3(void)
{
    demo_banner("Scenario 3: OKR Alignment & CFR");

    OKRContext *okr = okr_context_create();
    time_t q_end = time(NULL) + 90 * 86400;

    int obj_top = okr_objective_create(okr,
        "Double revenue through product-led growth",
        "Transition from sales-led to product-led for scalable growth",
        OKR_LEVEL_COMPANY, -1, q_end);

    int obj_eng = okr_objective_create(okr,
        "Ship delightful features that convert free users",
        "Reduce signup-to-value time, improve NUX",
        OKR_LEVEL_TEAM, obj_top, q_end);

    int obj_pm = okr_objective_create(okr,
        "Identify and prioritize top 3 growth levers",
        "Data-driven experimentation",
        OKR_LEVEL_TEAM, obj_top, q_end);

    int obj_dev1 = okr_objective_create(okr,
        "Build in-app onboarding wizard",
        "Interactive tutorial for new users",
        OKR_LEVEL_INDIVIDUAL, obj_eng, q_end);

    int obj_dev2 = okr_objective_create(okr,
        "Optimize signup flow A/B tests",
        "Run experiments to improve conversion",
        OKR_LEVEL_INDIVIDUAL, obj_eng, q_end);

    okr_key_result_add(okr, obj_top, "Annual Recurring Revenue",
                        1000000, 2000000, "USD", 0.5);
    okr_key_result_add(okr, obj_top, "Net Revenue Retention",
                        85, 110, "%", 0.3);
    okr_key_result_add(okr, obj_top, "Free-to-paid conversion",
                        2, 5, "%", 0.2);

    okr_key_result_add(okr, obj_eng, "Signup-to-value time",
                        30, 5, "minutes", 0.4);
    okr_key_result_add(okr, obj_eng, "NUX completion rate",
                        20, 70, "%", 0.3);
    okr_key_result_add(okr, obj_eng, "p99 page load",
                        3000, 500, "ms", 0.3);

    okr_key_result_add(okr, obj_pm, "Growth experiments run",
                        0, 20, "experiments", 0.5);
    okr_key_result_add(okr, obj_pm, "Feature adoption rate",
                        15, 40, "%", 0.5);

    okr_key_result_add(okr, obj_dev1, "Wizard steps implemented",
                        0, 6, "steps", 0.6);
    okr_key_result_add(okr, obj_dev1, "Wizard completion rate",
                        0, 60, "%", 0.4);

    okr_key_result_add(okr, obj_dev2, "A/B test variants",
                        0, 5, "variants", 0.5);
    okr_key_result_add(okr, obj_dev2, "Signup conversion lift",
                        0, 15, "pp", 0.5);

    printf("OKR alignment check:\n");
    printf("  Company: %d\n", obj_top);
    printf("  Engineering: %d -> parent %d\n", obj_eng, obj_top);
    printf("  Product: %d -> parent %d\n", obj_pm, obj_top);
    printf("  Dev1: %d -> parent %d\n", obj_dev1, obj_eng);
    printf("  Dev2: %d -> parent %d\n", obj_dev2, obj_eng);
    printf("  Alignment issues: %d\n", okr_alignment_validate(okr));

    for (int w = 1; w <= 4; w++) {
        double conf = 0.9 - (w - 1) * 0.1;
        okr_checkin_weekly(okr, obj_dev1, w, conf,
                            w == 1 ? "Starting wizard design" :
                            w == 2 ? "3 steps done, on track" :
                            w == 3 ? "UX review flagged 2 steps for redesign" :
                                     "Reworked steps, need more time");
    }

    OKRGradeBreakdown grade = okr_grade_calculate(okr, obj_dev1);
    printf("\nDev1 Grade after 4 weeks: ");
    okr_grade_print(&grade);
    printf("Confidence trend: %.2f\n", okr_confidence_trend(okr, obj_dev1));

    cfr_conversation_record(okr, obj_dev1, 1,
        "Weekly 1:1: Discussed career goals, wants to lead frontend standardization");
    cfr_feedback_record(okr, obj_dev1, 2, 1,
        "The wizard UX is elegant and intuitive. Great attention to edge cases!");
    cfr_recognition_record(okr, obj_dev1, 3, 1,
        "Shoutout for helping onboarding new team member Sarah this week");

    printf("\nCFR Entries (%d):\n", okr->cfr_count);
    for (int i = 0; i < okr->cfr_count; i++) {
        printf("  [%d] ", i + 1);
        cfr_entry_print(&okr->cfr_entries[i]);
    }

    int at_risk[8];
    int risk_n = okr_objective_find_at_risk(okr, at_risk, 8);
    printf("\nObjectives at risk (<30%%): %d\n", risk_n);

    okr_context_free(okr);
}

int main(void)
{
    printf("=== Project Governance Demo: Scrum + Tracker + OKR ===\n\n");
    printf("This demo showcases:\n");
    printf("  1. Full Scrum sprint lifecycle (backlog, sprint, standup, review, burndown)\n");
    printf("  2. Project issue tracking (JQL filtering, workflows, sprint metrics)\n");
    printf("  3. OKR alignment (company->team->individual, checkins, CFR)\n\n");

    demo_scenario_1();
    demo_scenario_2();
    demo_scenario_3();

    printf("\n============================================================\n");
    printf("  All scenarios complete.\n");
    printf("============================================================\n");
    return 0;
}
