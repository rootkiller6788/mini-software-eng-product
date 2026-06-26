#include "gov_governance.h"
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <assert.h>

static int tests = 0, passed = 0;
#define TEST(name) do { tests++; printf("  TEST %s... ", name); } while(0)
#define OK() do { passed++; printf("OK\n"); } while(0)
#define FAIL(msg) do { printf("FAIL: %s\n", msg); } while(0)

int main(void) {
    printf("=== Project Governance Tests ===\n\n");

    /* ---- Scrum Tests ---- */
    printf("scrum:\n");
    TEST("scrum_init"); {
        ScrumBoard sb;
        scrum_init(&sb);
        assert(sb.sprint_count == 0);
        assert(sb.current_sprint == -1);
        assert(sb.avg_velocity == 0.0);
        OK();
    }
    TEST("add_team_member"); {
        ScrumTeam t;
        scrum_team_init(&t);
        assert(scrum_add_member(&t, "Alice", ROLE_DEVELOPER, 100) == 0);
        assert(t.members[0].active);
        assert(scrum_add_member(&t, "Bob", ROLE_SCRUM_MASTER, 50) == 1);
        assert(t.count == 2);
        assert(scrum_add_member(&t, "BadCapacity", ROLE_DEVELOPER, 101) == -1);
        assert(scrum_add_member(&t, "BadCapacity2", ROLE_DEVELOPER, -1) == -1);
        OK();
    }
    TEST("pbi_add"); {
        ScrumBoard sb;
        scrum_init(&sb);
        assert(pbi_add(&sb.backlog, "US-1", "Login", PBI_USER_STORY, 5, 8, 40) == 0);
        assert(sb.backlog.total_points == 5);
        assert(pbi_add(&sb.backlog, "US-2", "Logout", PBI_USER_STORY, 3, 5, 20) == 1);
        assert(sb.backlog.count == 2);
        assert(pbi_add(&sb.backlog, "Bad", "Zero", PBI_TASK, 0, 1, 1) == -1);
        OK();
    }
    TEST("pbi_find"); {
        ScrumBoard sb;
        scrum_init(&sb);
        pbi_add(&sb.backlog, "US-1", "A", PBI_USER_STORY, 3, 5, 10);
        pbi_add(&sb.backlog, "US-2", "B", PBI_TASK, 2, 3, 5);
        assert(pbi_find(&sb.backlog, "US-1") == 0);
        assert(pbi_find(&sb.backlog, "US-2") == 1);
        assert(pbi_find(&sb.backlog, "NOPE") == -1);
        OK();
    }
    TEST("pbi_move_state"); {
        ScrumBoard sb;
        scrum_init(&sb);
        pbi_add(&sb.backlog, "US-1", "A", PBI_USER_STORY, 3, 5, 10);
        assert(pbi_move_state(&sb.backlog, "US-1", PBI_IN_PROGRESS));
        assert(sb.backlog.items[0].state == PBI_IN_PROGRESS);
        assert(pbi_move_state(&sb.backlog, "US-1", PBI_DONE));
        assert(pbi_move_state(&sb.backlog, "US-1", PBI_ACCEPTED));
        assert(sb.backlog.items[0].state == PBI_ACCEPTED);
        OK();
    }
    TEST("pbi_sort_priority"); {
        ScrumBoard sb;
        scrum_init(&sb);
        pbi_add(&sb.backlog, "A", "Low", PBI_TASK, 3, 5, 5);
        pbi_add(&sb.backlog, "B", "High", PBI_TASK, 3, 5, 5);
        pbi_add(&sb.backlog, "C", "Mid", PBI_TASK, 3, 5, 5);
        sb.backlog.items[0].priority = 50;
        sb.backlog.items[1].priority = 10;
        sb.backlog.items[2].priority = 30;
        pbi_sort_by_priority(&sb.backlog);
        assert(sb.backlog.items[0].priority == 10);
        assert(sb.backlog.items[2].priority == 50);
        OK();
    }
    TEST("pbi_wsjf"); {
        ScrumBoard sb;
        scrum_init(&sb);
        pbi_add(&sb.backlog, "A", "FeatureA", PBI_FEATURE, 13, 10, 80);
        pbi_add(&sb.backlog, "B", "BugFix", PBI_BUG, 3, 8, 10);
        pbi_calc_wsjf(&sb.backlog);
        assert(sb.backlog.items[0].wsjf_score > 0);
        assert(sb.backlog.items[1].wsjf_score > sb.backlog.items[0].wsjf_score);
        OK();
    }
    TEST("pbi_dependency"); {
        ScrumBoard sb;
        scrum_init(&sb);
        pbi_add(&sb.backlog, "A", "First", PBI_TASK, 5, 5, 5);
        pbi_add(&sb.backlog, "B", "Second", PBI_TASK, 3, 5, 5);
        pbi_add_dependency(&sb.backlog, "B", "A");
        assert(sb.backlog.items[1].dep_count == 1);
        assert(sb.backlog.items[1].dependencies[0] == 0);
        OK();
    }
    TEST("sprint_create"); {
        ScrumBoard sb;
        scrum_init(&sb);
        assert(sprint_create(&sb, "Sprint 1 Goal", 10) == 0);
        assert(sb.sprints[0].sprint_number == 1);
        assert(sb.sprints[0].duration_days == 10);
        assert(sb.sprints[0].state == SPRINT_PLANNING);
        assert(sprint_create(&sb, "Bad", 0) == -1);
        OK();
    }
    TEST("sprint_start_finish"); {
        ScrumBoard sb;
        scrum_init(&sb);
        sprint_create(&sb, "Goal", 10);
        pbi_add(&sb.backlog, "A", "Item", PBI_TASK, 8, 5, 5);
        pbi_add(&sb.backlog, "B", "Item2", PBI_TASK, 5, 5, 5);
        sprint_load_items(&sb, 0, 10);
        assert(sprint_start(&sb, 0));
        sb.sprints[0].completed_points = 8;
        sb.sprints[0].completed_items = 1;
        assert(sprint_finish(&sb, 0));
        assert(sb.sprints[0].state == SPRINT_REVIEW);
        assert(sb.velocity_samples == 1);
        assert(sb.avg_velocity > 0);
        OK();
    }
    TEST("sprint_plan"); {
        ScrumBoard sb;
        scrum_init(&sb);
        sprint_create(&sb, "Goal", 10);
        pbi_add(&sb.backlog, "A", "ItemA", PBI_TASK, 5, 3, 10);
        pbi_add(&sb.backlog, "B", "ItemB", PBI_TASK, 3, 3, 5);
        assert(sprint_plan(&sb, 0));
        assert(sb.sprints[0].planned_points > 0);
        OK();
    }
    TEST("burndown"); {
        Sprint sp;
        memset(&sp, 0, sizeof(sp));
        sp.planned_points = 30;
        sp.duration_days = 10;
        sprint_record_burndown(&sp, 0, 30);
        sprint_record_burndown(&sp, 3, 18);
        sprint_record_burndown(&sp, 5, 10);
        assert(sp.burndown_ideal[0] > 28);
        assert(sp.burndown_actual[3] == 18);
        assert(sp.burndown_days == 6);
        OK();
    }
    TEST("velocity_prediction"); {
        ScrumBoard sb;
        scrum_init(&sb);
        sprint_create(&sb, "S1", 10);
        sb.sprints[0].completed_points = 20;
        sb.sprints[0].velocity = 20;
        sb.avg_velocity = 20;
        double sprints = scrum_predict_completion(&sb, 100);
        assert(fabs(sprints - 5.0) < 0.1);
        OK();
    }

    /* ---- Kanban Tests ---- */
    printf("kanban:\n");
    TEST("kanban_init"); {
        KanbanBoard kb;
        kanban_init(&kb);
        assert(kb.card_count == 0);
        assert(kb.column_count == 0);
        OK();
    }
    TEST("kanban_add_column"); {
        KanbanBoard kb;
        kanban_init(&kb);
        assert(kanban_add_column(&kb, "Backlog", 10, false) == 0);
        assert(kanban_add_column(&kb, "In Progress", 5, false) == 1);
        assert(kanban_add_column(&kb, "Done", 0, true) == 2);
        assert(kb.columns[1].wip_limit == 5);
        OK();
    }
    TEST("kanban_add_card"); {
        KanbanBoard kb;
        kanban_init(&kb);
        kanban_add_column(&kb, "Backlog", 10, false);
        kanban_add_column(&kb, "Done", 0, true);
        assert(kanban_add_card(&kb, "C-1", "Task 1", COS_STANDARD, 3, 0) == 0);
        assert(kb.columns[0].card_count == 1);
        OK();
    }
    TEST("kanban_move_card"); {
        KanbanBoard kb;
        kanban_init(&kb);
        kanban_add_column(&kb, "Backlog", 10, false);
        kanban_add_column(&kb, "In Progress", 5, false);
        kanban_add_column(&kb, "Done", 0, true);
        kanban_add_card(&kb, "C-1", "Task", COS_STANDARD, 3, 0);
        assert(kanban_move_card(&kb, "C-1", 1));
        assert(kb.columns[0].card_count == 0);
        assert(kb.columns[1].card_count == 1);
        assert(kanban_move_card(&kb, "C-1", 2));
        assert(kb.columns[2].card_count == 1);
        assert(kb.total_completed == 1);
        OK();
    }
    TEST("kanban_wip_limit"); {
        KanbanBoard kb;
        kanban_init(&kb);
        kanban_add_column(&kb, "Col1", 1, false);
        kanban_add_column(&kb, "Col2", 1, false);
        kanban_add_column(&kb, "Done", 0, true);
        kanban_add_card(&kb, "C-1", "A", COS_STANDARD, 3, 0);
        kanban_add_card(&kb, "C-2", "B", COS_STANDARD, 3, 0);
        kanban_move_card(&kb, "C-1", 1);
        assert(!kanban_move_card(&kb, "C-2", 1));
        OK();
    }
    TEST("kanban_expedite_bypass_wip"); {
        KanbanBoard kb;
        kanban_init(&kb);
        kanban_add_column(&kb, "Backlog", 10, false);
        kanban_add_column(&kb, "In Progress", 1, false);
        kanban_add_column(&kb, "Done", 0, true);
        kanban_add_card(&kb, "C-1", "Normal", COS_STANDARD, 3, 0);
        kanban_add_card(&kb, "C-2", "Urgent", COS_EXPEDITE, 3, 0);
        kanban_move_card(&kb, "C-1", 1);
        assert(kanban_move_card(&kb, "C-2", 1));
        OK();
    }
    TEST("kanban_block_card"); {
        KanbanBoard kb;
        kanban_init(&kb);
        kanban_add_column(&kb, "Col1", 10, false);
        kanban_add_card(&kb, "C-1", "A", COS_STANDARD, 3, 0);
        assert(kanban_block_card(&kb, "C-1", "Waiting for API"));
        assert(kb.cards[0].blocked);
        assert(strcmp(kb.cards[0].block_reason, "Waiting for API") == 0);
        assert(kanban_unblock_card(&kb, "C-1"));
        assert(!kb.cards[0].blocked);
        OK();
    }
    TEST("kanban_littles_law"); {
        KanbanBoard kb;
        kanban_init(&kb);
        kanban_add_column(&kb, "Backlog", 10, false);
        kanban_add_column(&kb, "Progress", 5, false);
        kanban_add_column(&kb, "Done", 0, true);
        for (int i = 0; i < 10; i++) {
            char id[16];
            snprintf(id, sizeof(id), "C-%d", i);
            kanban_add_card(&kb, id, "Task", COS_STANDARD, 3, 0);
            kanban_move_card(&kb, id, 1);
            kanban_move_card(&kb, id, 2);
        }
        kanban_calc_flow_metrics(&kb);
        assert(kb.total_completed == 10);
        OK();
    }
    TEST("kanban_bottleneck"); {
        KanbanBoard kb;
        kanban_init(&kb);
        kanban_add_column(&kb, "Col1", 10, false);
        kanban_add_column(&kb, "Col2", 2, false);
        kanban_add_column(&kb, "Done", 0, true);
        kanban_add_card(&kb, "C-1", "A", COS_STANDARD, 3, 0);
        kanban_add_card(&kb, "C-2", "B", COS_STANDARD, 3, 0);
        kanban_move_card(&kb, "C-1", 1);
        kanban_move_card(&kb, "C-2", 1);
        int bn = kanban_bottleneck_column(&kb);
        assert(bn == 1);
        OK();
    }

    /* ---- OKR Tests ---- */
    printf("okr:\n");
    TEST("okr_init"); {
        OkrDashboard dash;
        okr_init(&dash, 2026, 1);
        assert(dash.year == 2026);
        assert(dash.quarter_number == 1);
        OK();
    }
    TEST("okr_add_objective"); {
        OkrDashboard dash;
        okr_init(&dash, 2026, 1);
        assert(okr_add_objective(&dash, "O1", "Growth", "Grow revenue", true, 1000, 2000) == 0);
        assert(dash.obj_count == 1);
        assert(dash.objectives[0].is_company_level);
        OK();
    }
    TEST("okr_add_kr"); {
        OkrDashboard dash;
        okr_init(&dash, 2026, 1);
        okr_add_objective(&dash, "O1", "Growth", "", true, 1000, 2000);
        assert(okr_add_key_result(&dash, "KR1", "Revenue to $10M", KR_CURRENCY, 10000000, 5000000, 0, 50) == 0);
        assert(okr_add_key_result(&dash, "KR2", "Users to 100K", KR_ABSOLUTE, 100000, 50000, 0, 50) == 1);
        assert(dash.objectives[0].kr_count == 2);
        OK();
    }
    TEST("okr_update_kr"); {
        OkrDashboard dash;
        okr_init(&dash, 2026, 1);
        okr_add_objective(&dash, "O1", "Growth", "", true, 1000, 2000);
        okr_add_key_result(&dash, "KR1", "Revenue", KR_CURRENCY, 10000000, 5000000, 0, 100);
        assert(okr_update_kr(&dash, "KR1", 7500000));
        assert(dash.key_results[0].current == 7500000);
        assert(dash.objectives[0].progress > 0);
        OK();
    }
    TEST("okr_confidence"); {
        assert(okr_assess_confidence(0.9, 0.3) == CONF_ON_TRACK);
        assert(okr_assess_confidence(0.3, 0.8) == CONF_OFF_TRACK);
        assert(okr_assess_confidence(0.5, 0.8) == CONF_AT_RISK);
        assert(okr_assess_confidence(1.5, 0.5) == CONF_COMPLETED);
        OK();
    }
    TEST("okr_company_score"); {
        OkrDashboard dash;
        okr_init(&dash, 2026, 1);
        okr_add_objective(&dash, "O1", "Growth", "", true, 1000, 2000);
        okr_add_key_result(&dash, "KR1", "Rev", KR_CURRENCY, 1000, 0, 0, 100);
        okr_update_kr(&dash, "KR1", 500);
        double score = okr_company_score(&dash);
        assert(score > 0.4 && score < 0.6);
        OK();
    }
    TEST("okr_align"); {
        OkrDashboard dash;
        okr_init(&dash, 2026, 1);
        okr_add_objective(&dash, "O1", "Company", "", true, 1000, 2000);
        okr_add_objective(&dash, "O2", "Team", "", false, 1000, 2000);
        assert(okr_align(&dash, "O1", "O2"));
        assert(dash.objectives[0].alignment_count == 1);
        OK();
    }
    TEST("okr_tree"); {
        OkrDashboard dash;
        okr_init(&dash, 2026, 1);
        okr_add_objective(&dash, "O1", "Company OKR", "", true, 1000, 2000);
        okr_add_key_result(&dash, "KR1", "Metric", KR_ABSOLUTE, 100, 0, 0, 100);
        okr_add_objective(&dash, "O2", "Team OKR", "", false, 1000, 2000);
        okr_align(&dash, "O1", "O2");
        OkrTree *tree = okr_build_tree(&dash);
        assert(tree != NULL);
        assert(tree->root != NULL);
        okr_tree_score(tree);
        okr_tree_free(tree);
        OK();
    }

    /* ---- Tracker Tests ---- */
    printf("tracker:\n");
    TEST("tracker_init"); {
        IssueTracker tr;
        tracker_init(&tr);
        assert(tr.issue_count == 0);
        OK();
    }
    TEST("tracker_add_issue"); {
        IssueTracker tr;
        tracker_init(&tr);
        assert(tracker_add_issue(&tr, "ISS-1", "Bug", "desc", ISSUE_BUG, PRIORITY_MAJOR, 5, 20) == 0);
        assert(tr.issue_count == 1);
        assert(tr.issues[0].status == STATUS_OPEN);
        OK();
    }
    TEST("tracker_update_status"); {
        IssueTracker tr;
        tracker_init(&tr);
        tracker_add_issue(&tr, "ISS-1", "Bug", "", ISSUE_BUG, PRIORITY_MAJOR, 5, 20);
        assert(tracker_update_status(&tr, "ISS-1", STATUS_TODO));
        assert(tracker_update_status(&tr, "ISS-1", STATUS_IN_PROGRESS));
        assert(tr.issues[0].status == STATUS_IN_PROGRESS);
        OK();
    }
    TEST("tracker_invalid_transition"); {
        IssueTracker tr;
        tracker_init(&tr);
        tracker_add_issue(&tr, "ISS-1", "Bug", "", ISSUE_BUG, PRIORITY_MAJOR, 5, 20);
        assert(!tracker_update_status(&tr, "ISS-1", STATUS_DONE));
        OK();
    }
    TEST("tracker_assign"); {
        IssueTracker tr;
        tracker_init(&tr);
        tracker_add_issue(&tr, "ISS-1", "Task", "", ISSUE_TASK, PRIORITY_MAJOR, 3, 10);
        assert(tracker_assign(&tr, "ISS-1", "Alice"));
        assert(strcmp(tr.issues[0].assignee, "Alice") == 0);
        OK();
    }
    TEST("tracker_comment"); {
        IssueTracker tr;
        tracker_init(&tr);
        tracker_add_issue(&tr, "ISS-1", "Task", "", ISSUE_TASK, PRIORITY_MINOR, 2, 5);
        assert(tracker_add_comment(&tr, "ISS-1", "Bob", "Looks good"));
        assert(tr.issues[0].comment_count == 1);
        OK();
    }
    TEST("tracker_query"); {
        IssueTracker tr;
        tracker_init(&tr);
        tracker_add_issue(&tr, "A", "T1", "", ISSUE_STORY, PRIORITY_CRITICAL, 8, 40);
        tracker_add_issue(&tr, "B", "T2", "", ISSUE_TASK, PRIORITY_MINOR, 2, 5);
        tracker_update_status(&tr, "A", STATUS_TODO);
        tracker_update_status(&tr, "A", STATUS_IN_PROGRESS);
        int results[10];
        int cnt = tracker_issues_by_status(&tr, STATUS_IN_PROGRESS, results, 10);
        assert(cnt == 1);
        OK();
    }

    /* ---- Velocity Tests ---- */
    printf("velocity:\n");
    TEST("velocity_init"); {
        VelocityProfile vp;
        velocity_init(&vp, "TeamA", 3);
        assert(strcmp(vp.team_name, "TeamA") == 0);
        assert(vp.rolling_window == 3);
        OK();
    }
    TEST("velocity_record"); {
        VelocityProfile vp;
        velocity_init(&vp, "TeamA", 3);
        assert(velocity_record(&vp, 1, 25, 20, 8, 7, 40, 2, 2) == 0);
        assert(velocity_record(&vp, 2, 30, 28, 10, 9, 40, 1, 1) == 1);
        assert(velocity_record(&vp, 3, 20, 18, 7, 6, 38, 3, 1) == 2);
        assert(velocity_record(&vp, 4, 28, 25, 9, 8, 40, 0, 0) == 3);
        assert(vp.record_count == 4);
        double avg = velocity_rolling_avg(&vp);
        assert(avg > 0);
        OK();
    }
    TEST("velocity_stats"); {
        VelocityProfile vp;
        velocity_init(&vp, "TeamB", 2);
        velocity_record(&vp, 1, 30, 25, 10, 8, 40, 0, 0);
        velocity_record(&vp, 2, 35, 30, 12, 10, 40, 0, 0);
        velocity_record(&vp, 3, 20, 15, 8, 6, 40, 0, 0);
        assert(vp.best_sprint == 30);
        assert(vp.worst_sprint == 15);
        assert(vp.all_time_median > 0);
        double vol = velocity_volatility(&vp);
        assert(vol >= 0);
        OK();
    }
    TEST("velocity_trend"); {
        VelocityProfile vp;
        velocity_init(&vp, "TeamC", 3);
        velocity_record(&vp, 1, 20, 15, 5, 4, 40, 0, 0);
        velocity_record(&vp, 2, 20, 18, 5, 5, 40, 0, 0);
        velocity_record(&vp, 3, 20, 20, 5, 5, 40, 0, 0);
        velocity_record(&vp, 4, 20, 22, 5, 5, 40, 0, 0);
        assert(vp.improving);
        OK();
    }
    TEST("forecast_rolling"); {
        VelocityProfile vp;
        velocity_init(&vp, "T", 3);
        velocity_record(&vp, 1, 20, 20, 5, 5, 40, 0, 0);
        velocity_record(&vp, 2, 20, 20, 5, 5, 40, 0, 0);
        velocity_record(&vp, 3, 20, 20, 5, 5, 40, 0, 0);
        Forecast f;
        forecast_rolling(&vp, &f, 100);
        assert(f.predicted_points > 0);
        assert(f.confidence_50 > 0);
        OK();
    }
    TEST("forecast_monte_carlo"); {
        VelocityProfile vp;
        velocity_init(&vp, "TM", 3);
        velocity_record(&vp, 1, 25, 20, 8, 7, 40, 0, 0);
        velocity_record(&vp, 2, 25, 22, 8, 8, 40, 0, 0);
        velocity_record(&vp, 3, 25, 25, 8, 8, 40, 0, 0);
        Forecast f;
        MonteCarloConfig mc = {1000, 20, NULL, 3};
        int hist[] = {20, 22, 25};
        mc.historical_throughput = hist;
        forecast_monte_carlo(&vp, &f, 50, &mc);
        assert(f.confidence_50 > 0);
        OK();
    }
    TEST("cone_of_uncertainty"); {
        double c1 = cone_of_uncertainty(0.0);
        double c2 = cone_of_uncertainty(0.5);
        double c3 = cone_of_uncertainty(1.0);
        assert(c1 > c2);
        assert(c2 > c3);
        assert(c1 > 3.0);
        OK();
    }
    TEST("parkinsons_law"); {
        double expansion = parkinsons_law_expansion(10.0, 15.0);
        assert(fabs(expansion - 1.5) < 0.01);
        OK();
    }
    TEST("brooks_law"); {
        double delay1 = brooks_law_delay(5, 0, 100);
        assert(fabs(delay1 - 100) < 0.01);
        double delay2 = brooks_law_delay(5, 2, 100);
        assert(delay2 > 100);
        OK();
    }

    /* ---- Retro Tests ---- */
    printf("retro:\n");
    TEST("retro_init"); {
        RetroLog log;
        retro_init(&log);
        assert(log.session_count == 0);
        OK();
    }
    TEST("retro_create_session"); {
        RetroLog log;
        retro_init(&log);
        assert(retro_create_session(&log, 1, "Sprint 1", RETRO_START_STOP_CONTINUE, "Alice") == 0);
        assert(log.sessions[0].sprint_number == 1);
        OK();
    }
    TEST("retro_add_item"); {
        RetroLog log;
        retro_init(&log);
        retro_create_session(&log, 1, "Goal", RETRO_GLAD_SAD_MAD, "Fac");
        assert(retro_add_item(&log, 0, "CI is slow", "Bob", RETRO_CAT_WHAT_TO_IMPROVE) == 0);
        assert(retro_add_item(&log, 0, "Great teamwork", "Alice", RETRO_CAT_WHAT_WENT_WELL) == 1);
        assert(log.sessions[0].item_count == 2);
        OK();
    }
    TEST("retro_vote"); {
        RetroLog log;
        retro_init(&log);
        retro_create_session(&log, 1, "G", RETRO_START_STOP_CONTINUE, "F");
        retro_add_item(&log, 0, "Issue A", "Alice", RETRO_CAT_WHAT_WENT_WRONG);
        retro_add_item(&log, 0, "Issue B", "Bob", RETRO_CAT_WHAT_WENT_WRONG);
        retro_vote(&log, 0, 0);
        retro_vote(&log, 0, 0);
        retro_vote(&log, 0, 1);
        assert(log.sessions[0].items[0].votes == 2);
        assert(log.sessions[0].items[1].votes == 1);
        OK();
    }
    TEST("retro_top_voted"); {
        RetroLog log;
        retro_init(&log);
        retro_create_session(&log, 1, "G", RETRO_4L, "F");
        retro_add_item(&log, 0, "Low", "A", RETRO_CAT_PUZZLE);
        retro_add_item(&log, 0, "High", "B", RETRO_CAT_PUZZLE);
        retro_vote(&log, 0, 0);
        retro_vote(&log, 0, 1);
        retro_vote(&log, 0, 1);
        retro_vote(&log, 0, 1);
        int results[10];
        int cnt = retro_top_voted(&log, 0, results, 10);
        assert(cnt >= 2);
        assert(results[0] == 1);
        OK();
    }
    TEST("retro_add_action"); {
        RetroLog log;
        retro_init(&log);
        retro_create_session(&log, 1, "G", RETRO_SAILBOAT, "F");
        assert(retro_add_action(&log, 0, "Fix CI pipeline", "Alice", 2000000000, 8, 5) == 0);
        assert(log.sessions[0].action_count == 1);
        assert(log.open_action_count == 1);
        OK();
    }
    TEST("retro_complete_action"); {
        RetroLog log;
        retro_init(&log);
        retro_create_session(&log, 1, "G", RETRO_KALM, "F");
        retro_add_action(&log, 0, "Fix bug", "Bob", 2000000000, 7, 3);
        char *aid = log.open_actions[0].id;
        assert(retro_complete_action(&log, aid));
        assert(log.open_action_count == 0);
        OK();
    }
    TEST("retro_health"); {
        RetroLog log;
        retro_init(&log);
        retro_create_session(&log, 1, "G", RETRO_START_STOP_CONTINUE, "F");
        retro_add_health(&log, 0, "Alice", 4.0, 4.0, 3.5, 4.5, 4.0, 4.0);
        retro_add_health(&log, 0, "Bob", 3.5, 3.5, 3.0, 4.0, 3.5, 3.5);
        retro_calc_health_avg(&log, 0);
        assert(log.sessions[0].avg_happiness > 3.5);
        OK();
    }
    TEST("five_whys"); {
        FiveWhys fw;
        five_whys_init(&fw, "Production outage");
        assert(five_whys_add(&fw, 0, "Server crashed"));
        assert(five_whys_add(&fw, 1, "Out of memory"));
        assert(five_whys_add(&fw, 2, "Memory leak in new feature"));
        assert(five_whys_add(&fw, 3, "No load testing before deploy"));
        assert(five_whys_add(&fw, 4, "No load testing in CI pipeline"));
        assert(fw.depth_reached == 5);
        five_whys_solve(&fw, "Missing CI load test stage", "Add k6 tests to CI");
        assert(strlen(fw.root_cause) > 0);
        OK();
    }

    /* ---- Governance Tests ---- */
    printf("governance:\n");
    TEST("gov_program_init"); {
        GovernanceProgram gp;
        gov_program_init(&gp, "Project X", "Build platform", GOV_SCRUM);
        assert(strcmp(gp.name, "Project X") == 0);
        assert(gp.framework == GOV_SCRUM);
        assert(gp.active);
        OK();
    }
    TEST("gov_add_risk"); {
        GovernanceProgram gp;
        gov_program_init(&gp, "P", "M", GOV_SCRUM);
        assert(gov_add_risk(&gp, "R1", "Key person risk", 0.5, 8, "PM", "Cross-train") == 0);
        assert(gov_add_risk(&gp, "R2", "Vendor lock-in", 0.3, 6, "Architect", "Open standards") == 1);
        assert(gp.risk_count == 2);
        assert(gp.risks[0].exposure > 3.0);
        OK();
    }
    TEST("gov_risk_exposure"); {
        GovernanceProgram gp;
        gov_program_init(&gp, "P", "M", GOV_SCRUM);
        gov_add_risk(&gp, "R1", "Risk A", 0.8, 9, "A", "Fix");
        gov_add_risk(&gp, "R2", "Risk B", 0.2, 2, "B", "Fix");
        double total = gov_total_risk_exposure(&gp);
        assert(total > 0);
        int top[5];
        int cnt = gov_top_risks(&gp, top, 5);
        assert(cnt == 2);
        assert(top[0] == 0);
        OK();
    }
    TEST("gov_stakeholder"); {
        GovernanceProgram gp;
        gov_program_init(&gp, "P", "M", GOV_SCRUM);
        assert(gov_add_stakeholder(&gp, "CEO", STAKE_RACI_A, 10, 8) == 0);
        assert(gov_add_stakeholder(&gp, "User", STAKE_RACI_I, 3, 9) == 1);
        assert(gp.stakeholder_count == 2);
        OK();
    }
    TEST("evm"); {
        EarnedValue ev;
        evm_init(&ev, 100000.0);
        evm_update(&ev, 50000.0, 45000.0, 48000.0);
        assert(ev.sv < 0);
        assert(ev.cv < 0);
        assert(ev.spi < 1.0);
        assert(!evm_on_schedule(&ev));
        assert(!evm_on_budget(&ev));
        evm_update(&ev, 50000.0, 52000.0, 50000.0);
        assert(ev.sv > 0);
        assert(ev.cv > 0);
        assert(evm_on_schedule(&ev));
        OK();
    }
    TEST("sprint_sim"); {
        SprintSimConfig cfg;
        sprint_sim_init(&cfg);
        cfg.num_sprints = 3;
        SprintSimResult results[10];
        sprint_sim_run(&cfg, results, 10);
        assert(results[0].planned > 0);
        assert(results[0].completed >= 0);
        OK();
    }
    TEST("gov_scorecard"); {
        GovernanceProgram gp;
        gov_program_init(&gp, "Proj", "Mission", GOV_SCRUM);
        GovernanceScorecard sc;
        gov_calc_scorecard(&gp, &sc);
        assert(sc.budget_adherence_pct >= 0);
        OK();
    }

    printf("\n=== Results: %d/%d passed ===\n", passed, tests);
    return passed == tests ? 0 : 1;
}