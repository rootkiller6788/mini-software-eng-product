#include "stakeholder_mgmt.h"
#include "spec_design.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void header(const char *t) {
    printf("\n============================================================\n");
    printf("  %s\n", t);
    printf("============================================================\n\n");
}

int main(void) {
    header("STAKEHOLDER & SPECIFICATION MANAGEMENT DEEP DIVE");

    /* ========== 1. STAKEHOLDER MAP ========== */
    header("1. STAKEHOLDER MAP — Power x Interest Matrix");

    Stakeholder sh[10];
    stakeholder_init(&sh[0], "CEO", "Chief Executive Officer", 1.0, 0.8);
    stakeholder_init(&sh[1], "CTO", "Chief Technology Officer", 0.9, 0.95);
    stakeholder_init(&sh[2], "VP Product", "VP of Product Management", 0.85, 0.9);
    stakeholder_init(&sh[3], "VP Engineering", "VP of Engineering", 0.8, 0.85);
    stakeholder_init(&sh[4], "Engineering Manager", "Senior Engineering Manager", 0.6, 0.9);
    stakeholder_init(&sh[5], "Customer Support", "Support Team Lead", 0.2, 0.85);
    stakeholder_init(&sh[6], "Legal Team", "Corporate Legal Counsel", 0.75, 0.3);
    stakeholder_init(&sh[7], "End Users", "Daily Active Users (1000+)", 0.1, 0.7);
    stakeholder_init(&sh[8], "Investors", "Board Members & VCs", 0.85, 0.4);
    stakeholder_init(&sh[9], "IT Security", "CISO Office", 0.65, 0.5);

    printf("%-25s %-25s %6s %8s %-18s\n", "Name", "Title", "Power", "Interest", "Quadrant");
    printf("------------------------------------------------------------------------------------------\n");
    for (int i = 0; i < 10; i++) {
        const char *q = stakeholder_quadrant(sh[i].power, sh[i].interest);
        printf("%-25s %-25s %5.2f  %5.2f    %-18s\n", sh[i].name, sh[i].title,
               sh[i].power, sh[i].interest, q);
    }

    char map_buf[4096];
    stakeholder_map_print(sh, 10, map_buf, sizeof(map_buf));
    printf("\n%s", map_buf);

    printf("Engagement Strategies:\n");
    printf("%-25s %s\n", "Stakeholder", "Strategy");
    printf("------------------------------------------------------------------------------------------\n");
    for (int i = 0; i < 10; i++) {
        printf("%-25s %s\n", sh[i].name, sh[i].engagement_strategy);
    }

    /* ========== 2. RACI MATRIX ========== */
    header("2. RACI MATRIX — Roles & Responsibilities");

    RaciMatrix rm;
    raci_matrix_init(&rm);

    struct { const char *task; const char *r; const char *a; } raci_data[] = {
        {"Define product vision & strategy", "Product Manager", "VP Product"},
        {"Approve annual budget & resources", "VP Product", "CEO"},
        {"Prioritize feature backlog (sprint)", "Product Manager", "VP Product"},
        {"Design system architecture", "Tech Lead", "CTO"},
        {"Write code & unit tests", "Senior Engineers", "Engineering Manager"},
        {"Conduct code reviews", "All Engineers", "Tech Lead"},
        {"Execute QA & regression testing", "QA Engineers", "QA Lead"},
        {"Approve security compliance", "CISO", "CTO"},
        {"Publish release notes & docs", "Tech Writer", "Product Manager"},
        {"Train customer support team", "Support Lead", "VP Product"},
        {"Manage vendor relationships", "Procurement", "VP Engineering"},
        {"Run incident post-mortems", "SRE Lead", "CTO"},
    };
    size_t raci_n = sizeof(raci_data) / sizeof(raci_data[0]);

    for (size_t i = 0; i < raci_n; i++) {
        raci_add_item(&rm, raci_data[i].task, raci_data[i].r, raci_data[i].a);
    }

    raci_add_consulted(&rm, 0, "CTO"); raci_add_consulted(&rm, 0, "CEO");
    raci_add_consulted(&rm, 2, "Tech Lead"); raci_add_consulted(&rm, 2, "Design Lead");
    raci_add_consulted(&rm, 3, "Product Manager"); raci_add_consulted(&rm, 3, "Security");
    raci_add_consulted(&rm, 4, "Tech Lead");
    raci_add_consulted(&rm, 5, "Tech Lead");
    raci_add_consulted(&rm, 6, "Tech Lead"); raci_add_consulted(&rm, 6, "Product Manager");
    raci_add_consulted(&rm, 7, "Legal"); raci_add_consulted(&rm, 7, "DevOps");
    raci_add_consulted(&rm, 9, "Product Manager"); raci_add_consulted(&rm, 9, "Engineering Manager");

    raci_add_informed(&rm, 0, "All Stakeholders");
    raci_add_informed(&rm, 1, "Board"); raci_add_informed(&rm, 1, "All Hands");
    raci_add_informed(&rm, 2, "Engineering Manager"); raci_add_informed(&rm, 2, "Support Lead");
    raci_add_informed(&rm, 3, "Product Manager");
    raci_add_informed(&rm, 4, "Product Manager"); raci_add_informed(&rm, 4, "QA Lead");
    raci_add_informed(&rm, 7, "Product Manager"); raci_add_informed(&rm, 7, "CEO");
    raci_add_informed(&rm, 9, "Product Manager"); raci_add_informed(&rm, 9, "Engineering Manager");
    raci_add_informed(&rm, 11, "Product Manager"); raci_add_informed(&rm, 11, "VP Engineering");

    printf("%-45s %-15s %-15s %-25s %-20s\n", "Task", "Responsible", "Accountable", "Consulted", "Informed");
    printf("--------------------------------------------------------------------------------------------------------------------------\n");
    for (size_t i = 0; i < rm.item_count; i++) {
        RacItem *item = &rm.items[i];
        printf("%-45s %-15s %-15s ", item->task, item->responsible, item->accountable);
        for (int j = 0; j < item->consulted_count; j++) {
            printf("%s%s", item->consulted[j], j < item->consulted_count - 1 ? "," : "");
        }
        if (item->consulted_count == 0) printf("-");
        printf("   ");
        for (int j = 0; j < item->informed_count; j++) {
            printf("%s%s", item->informed[j], j < item->informed_count - 1 ? "," : "");
        }
        if (item->informed_count == 0) printf("-");
        printf("\n");
    }

    char gapbuf[1024];
    if (raci_has_gaps(&rm, gapbuf, sizeof(gapbuf))) {
        printf("\nRACI Coverage Gaps:\n%s", gapbuf);
    } else {
        printf("\nRACI: All items have C or I assignments.\n");
    }

    /* ========== 3. COMMUNICATION PLAN ========== */
    header("3. COMMUNICATION PLAN");

    CommunicationPlan plans[10];
    comm_plan_generate(sh, 10, plans);

    printf("%-25s %-18s %-12s %-18s\n", "Stakeholder", "Channel", "Frequency", "Owner");
    printf("-------------------------------------------------------------------------------\n");
    for (int i = 0; i < 10; i++) {
        printf("%-25s %-18s %-12s %-18s\n",
               plans[i].stakeholder_name, plans[i].channel,
               plans[i].frequency, plans[i].owner);
    }

    printf("\nDetailed Communication Content:\n");
    for (int i = 0; i < 10; i++) {
        printf("  [%s] %s: %s\n", plans[i].stakeholder_name,
               plans[i].channel, plans[i].content_summary);
    }

    /* ========== 4. ESCALATION MANAGEMENT ========== */
    header("4. ESCALATION MANAGEMENT");

    Escalation escalations[] = {
        {{0}}, {{0}}, {{0}}, {{0}}, {{0}}
    };
    escalation_init(&escalations[0], "Budget overrun by 15% in Q2", 1);
    escalation_init(&escalations[1], "Key senior engineer resigning mid-sprint", 3);
    escalation_init(&escalations[2], "Critical CVE in dependency library", 3);
    escalation_init(&escalations[3], "Launch date slipping by 3 weeks due to QA backlog", 1);
    escalation_init(&escalations[4], "Customer data deletion incident (GDPR)", 3);

    printf("%-55s Sev  Resolved\n", "Issue");
    printf("-----------------------------------------------------------------\n");

    escalation_resolve(&escalations[0], "Approved reallocation from Q3 contingency buffer (VP Product)");
    escalation_resolve(&escalations[2], "Upgraded library to patched version v2.7.1; security scan clean");
    escalation_resolve(&escalations[4], "Restored from backup within 4 hours; informed DPO; updated incident report");

    for (int i = 0; i < 5; i++) {
        printf("%-55s %2d   %s\n", escalations[i].issue,
               escalations[i].severity,
               escalations[i].resolved ? "YES" : "NO");
        if (!escalations[i].resolved && escalation_requires_attention(&escalations[i], 2)) {
            printf("  *** REQUIRES IMMEDIATE ATTENTION ***\n");
        }
    }

    /* ========== 5. PRD & USER STORIES ========== */
    header("5. PRD — PRODUCT REQUIREMENTS DOCUMENT");

    PRD prd;
    prd_init(&prd, "Enterprise Dashboard v2 PRD",
             "Enterprise customers report dashboard customization is too limited and load times exceed 30s.");

    strncpy(prd.target_user, "Data analysts, business managers, and team leads in enterprise organizations",
            sizeof(prd.target_user) - 1);
    strncpy(prd.success_metrics, "Dashboard load <2s, widget customization adoption >60%, NPS improvement +10pts, churn <3%",
            sizeof(prd.success_metrics) - 1);
    strncpy(prd.scope, "Custom widgets, template library, role-based access control, audit logging",
            sizeof(prd.scope) - 1);
    strncpy(prd.out_of_scope, "White-labeling, on-premise deployment, custom plugin marketplace",
            sizeof(prd.out_of_scope) - 1);
    strncpy(prd.assumptions, "Backend API v2 complete by Sprint 8; Design system tokens approved; Users have modern browsers",
            sizeof(prd.assumptions) - 1);
    strncpy(prd.risks, "Performance at >10K concurrent widgets may require DB sharding; Safari <14 widget render compatibility",
            sizeof(prd.risks) - 1);

    UserStory stories[6];
    user_story_init(&stories[0], "dashboard user",
                    "add, remove, and resize custom widgets on my dashboard",
                    "I can personalize my view for different workflows and data needs");
    user_story_add_acceptance(&stories[0], "I am on the dashboard page",
                              "I click the '+ Add Widget' button", "a catalog appears with available widgets");
    user_story_add_acceptance(&stories[0], "a widget is on my dashboard",
                              "I drag its corner handle", "it resizes responsively");
    stories[0].priority = 1; stories[0].story_points = 8;

    user_story_init(&stories[1], "dashboard user",
                    "save multiple dashboard layouts as named templates",
                    "I can switch between my daily ops view, weekly review, and project-specific dashboards");
    user_story_add_acceptance(&stories[1], "I have a customized dashboard",
                              "I click 'Save as template' and name it",
                              "the layout appears in my template switcher dropdown");
    stories[1].priority = 1; stories[1].story_points = 5;

    user_story_init(&stories[2], "admin",
                    "configure role-based access control on dashboards",
                    "only authorized team members can view or edit sensitive financial data");
    user_story_add_acceptance(&stories[2], "I open dashboard permissions panel",
                              "I assign 'Finance' group to 'View only' role",
                              "members of Finance see the dashboard but cannot modify or export it");
    stories[2].priority = 2; stories[2].story_points = 13;

    user_story_init(&stories[3], "admin",
                    "view an audit log of all dashboard changes",
                    "I can prove compliance and trace who made what change and when");
    user_story_add_acceptance(&stories[3], "I open Audit Log from admin panel",
                              "I filter by date range and user",
                              "all modifications are listed with timestamp, user, action, and diff summary");
    stories[3].priority = 2; stories[3].story_points = 8;

    user_story_init(&stories[4], "new user",
                    "browse and apply pre-built dashboard templates from a gallery",
                    "I can start with a professional dashboard instead of building from scratch");
    user_story_add_acceptance(&stories[4], "I land on the empty dashboard page",
                              "I click 'Browse templates'",
                              "a gallery of 20+ templates appears, selecting one populates my dashboard");
    stories[4].priority = 3; stories[4].story_points = 5;

    user_story_init(&stories[5], "dashboard user",
                    "export a dashboard as a PDF or PNG file",
                    "I can include static snapshots in presentations and reports without granting access");
    user_story_add_acceptance(&stories[5], "I click 'Export' on a dashboard",
                              "I select PDF format and A4 landscape",
                              "a PDF downloads with all visible widgets rendered at high resolution");
    stories[5].priority = 3; stories[5].story_points = 3;

    for (int i = 0; i < 6; i++) { prd_add_story(&prd, &stories[i]); }

    prd_print_summary(&prd);

    printf("\nDetailed User Stories:\n\n");
    for (size_t i = 0; i < prd.story_count; i++) {
        UserStory *s = &prd.stories[i];
        printf("US%zu [P%d | %d pts]: As a %s,\n", i + 1, s->priority, s->story_points, s->role);
        printf("    I want %s,\n", s->want);
        printf("    so that %s\n", s->so_that);
        printf("    Acceptance Criteria (%d):\n", s->ac_count);
        for (int j = 0; j < s->ac_count; j++) {
            printf("      %d. %s\n", j + 1, s->acceptance_criteria[j]);
        }
    }

    int total_sp = prd_total_story_points(&prd);
    printf("\nTotal Story Points: %d (across %zu stories)\n", total_sp, prd.story_count);

    /* ========== 6. TECHNICAL SPEC ========== */
    header("6. TECHNICAL SPECIFICATION");

    TechnicalSpec ts;
    memset(&ts, 0, sizeof(ts));

    tech_spec_add_endpoint(&ts, "GET /api/v2/dashboards", "GET", 200);
    tech_spec_add_endpoint(&ts, "POST /api/v2/dashboards", "POST", 201);
    tech_spec_add_endpoint(&ts, "PUT /api/v2/dashboards/{id}", "PUT", 200);
    tech_spec_add_endpoint(&ts, "DELETE /api/v2/dashboards/{id}", "DELETE", 204);
    tech_spec_add_endpoint(&ts, "GET /api/v2/dashboards/{id}/widgets", "GET", 200);
    tech_spec_add_endpoint(&ts, "POST /api/v2/dashboards/{id}/widgets", "POST", 201);
    tech_spec_add_endpoint(&ts, "PUT /api/v2/widgets/{id}", "PUT", 200);
    tech_spec_add_endpoint(&ts, "DELETE /api/v2/widgets/{id}", "DELETE", 204);
    tech_spec_add_endpoint(&ts, "GET /api/v2/templates", "GET", 200);
    tech_spec_add_endpoint(&ts, "POST /api/v2/templates", "POST", 201);
    tech_spec_add_endpoint(&ts, "GET /api/v2/audit-logs", "GET", 200);
    tech_spec_add_endpoint(&ts, "POST /api/v2/dashboards/{id}/export", "POST", 200);

    tech_spec_add_model(&ts, "Dashboard");
    tech_spec_add_model(&ts, "Widget");
    tech_spec_add_model(&ts, "Template");
    tech_spec_add_model(&ts, "AuditLog");
    tech_spec_add_model(&ts, "Role");
    tech_spec_add_model(&ts, "ExportJob");

    tech_spec_add_mockup(&ts, "Dashboard Grid", "12-column responsive grid with drag-to-resize widgets and contextual toolbar");
    tech_spec_add_mockup(&ts, "Widget Catalog", "Searchable sidebar panel with category filters, preview thumbnails, and descriptions");
    tech_spec_add_mockup(&ts, "Template Gallery", "Card grid of templates with screenshot, name, widget count, and 'Use Template' button");
    tech_spec_add_mockup(&ts, "Role Manager", "Table UI with role name, description, add/remove permission checkboxes per module");
    tech_spec_add_mockup(&ts, "Audit Log Viewer", "Paginated table with date, user avatar, action, and expandable diff panel");

    tech_spec_print_summary(&ts);

    /* ========== 7. DESIGN REVIEW ========== */
    header("7. DESIGN REVIEW CHECKLIST");

    DesignReview dr;
    design_review_init(&dr);

    dr.requirements_clarity = true;
    dr.edge_cases_covered = true;
    dr.error_handling_documented = true;
    dr.security_review = true;
    dr.accessibility_review = true;
    dr.i18n_considerations = true;
    dr.performance_constraints = true;
    dr.dependency_analysis = true;
    dr.backward_compatibility = true;
    dr.monitoring_plan = true;

    const char *check_names[] = {
        "Requirements clarity", "Edge cases covered", "Error handling documented",
        "Security review", "Accessibility review", "i18n considerations",
        "Performance constraints", "Dependency analysis", "Backward compatibility",
        "Monitoring plan"
    };
    const bool *check_vals[] = {
        &dr.requirements_clarity, &dr.edge_cases_covered, &dr.error_handling_documented,
        &dr.security_review, &dr.accessibility_review, &dr.i18n_considerations,
        &dr.performance_constraints, &dr.dependency_analysis, &dr.backward_compatibility,
        &dr.monitoring_plan
    };

    for (int i = 0; i < 10; i++) {
        printf("  [%c] %s\n", *check_vals[i] ? 'x' : ' ', check_names[i]);
    }
    printf("\n  Review Score: %.0f%%  All Passed: %s\n",
           design_review_score(&dr) * 100,
           design_review_all_passed(&dr) ? "YES" : "NO");

    /* Cleanup */
    raci_free(&rm);
    spec_free_all(&prd, &ts);

    header("DEEP DIVE COMPLETE");
    return 0;
}
