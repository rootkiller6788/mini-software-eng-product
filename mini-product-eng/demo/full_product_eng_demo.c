#include "product_lifecycle.h"
#include "roadmap_plan.h"
#include "spec_design.h"
#include "stakeholder_mgmt.h"
#include "release_mgmt.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void separator(const char *title) {
    printf("\n============================================================\n");
    printf("  %s\n", title);
    printf("============================================================\n\n");
}

int main(void) {
    separator("FULL PRODUCT ENGINEERING DEMO — DataViz Pro");

    /* ========== 1. PRODUCT LIFECYCLE ========== */
    separator("1. PRODUCT LIFECYCLE MANAGEMENT");
    ProductLifecycle pl;
    product_lifecycle_init(&pl, "DataViz Pro", "Make data visualization accessible to everyone");

    printf("Product: %s\n", pl.product_name);
    printf("Vision:  %s\n", pl.vision);

    product_lifecycle_add_mvp_feature(&pl, "Chart builder (bar, line, pie, scatter)", MVP_MUST_HAVE);
    product_lifecycle_add_mvp_feature(&pl, "CSV/Excel/JSON import", MVP_MUST_HAVE);
    product_lifecycle_add_mvp_feature(&pl, "Dashboard sharing via link", MVP_MUST_HAVE);
    product_lifecycle_add_mvp_feature(&pl, "Custom color themes", MVP_SHOULD_HAVE);
    product_lifecycle_add_mvp_feature(&pl, "Export to PDF/PNG", MVP_SHOULD_HAVE);
    product_lifecycle_add_mvp_feature(&pl, "Real-time data streaming", MVP_COULD_HAVE);
    product_lifecycle_add_mvp_feature(&pl, "White-label option", MVP_WONT_HAVE);

    printf("MVP Features: %zu\n", pl.mvp_feature_count);
    const char *labels[] = {"Must", "Should", "Could", "Won't"};
    for (size_t i = 0; i < pl.mvp_feature_count; i++) {
        printf("  [%s] %s\n", labels[pl.mvp_features[i].priority], pl.mvp_features[i].feature);
    }

    ProductPhase to_pass[] = {PHASE_DISCOVERY, PHASE_DEFINITION, PHASE_DESIGN,
                               PHASE_DEVELOPMENT, PHASE_TESTING};
    for (int i = 0; i < 5; i++) {
        product_lifecycle_evaluate_gate(&pl, to_pass[i], true);
        product_lifecycle_advance_phase(&pl);
    }
    printf("\nCurrent Phase: %s  MVP Ready: %s  Can Regress: %s\n",
           product_lifecycle_phase_name(pl.current_phase),
           product_lifecycle_is_mvp_ready(&pl) ? "Yes" : "No",
           product_lifecycle_can_regress(&pl) ? "Yes" : "No");

    /* ========== 2. RICE PRIORITIZATION & ROADMAP ========== */
    separator("2. ROADMAP & RICE PRIORITIZATION");
    ProductRoadmap roadmap;
    roadmap_init(&roadmap, 2, 2026);

    roadmap_add_feature(&roadmap, "Interactive chart builder", "15+ chart types, drag-drop config", 2000, 3.0, 0.9, 3.0);
    roadmap_add_feature(&roadmap, "Multi-format data import", "CSV/Excel/JSON/SQL", 1500, 2.5, 1.0, 1.5);
    roadmap_add_feature(&roadmap, "Dashboard sharing", "Share by link/email with permissions", 800, 2.0, 0.8, 2.0);
    roadmap_add_feature(&roadmap, "Real-time streaming", "WebSocket live data feeds", 300, 2.5, 0.6, 5.0);
    roadmap_add_feature(&roadmap, "PDF export engine", "High-fidelity vector PDF output", 1000, 1.5, 0.95, 1.0);
    roadmap_add_feature(&roadmap, "AI trend detection", "Auto-detect anomalies and trends", 500, 3.0, 0.5, 6.0);
    roadmap_add_feature(&roadmap, "Native mobile apps", "iOS and Android clients", 2000, 2.0, 0.7, 8.0);
    roadmap_add_feature(&roadmap, "Enterprise SSO", "SAML/OIDC with Azure AD", 100, 2.0, 0.9, 3.0);
    roadmap_add_feature(&roadmap, "Embedded analytics", "iframe/JS embed SDK", 400, 2.5, 0.75, 3.5);
    roadmap_add_feature(&roadmap, "Custom plugin system", "3rd-party chart plugins", 150, 2.0, 0.5, 5.0);
    roadmap_add_feature(&roadmap, "Multi-language i18n", "10 languages, RTL support", 600, 1.5, 0.8, 2.5);
    roadmap_add_feature(&roadmap, "API rate limiting", "Per-tenant configurable limits", 80, 1.5, 0.95, 0.5);

    roadmap_prioritize_by_rice(&roadmap);

    const char *hz[] = {"NOW", "NEXT", "LATER"};
    printf("Prioritized Backlog (%zu features):\n\n", roadmap.backlog_count);
    printf("%-4s %-35s %8s %8s\n", "#", "Feature", "RICE", "Horizon");
    printf("---------------------------------------------------------\n");
    for (size_t i = 0; i < roadmap.backlog_count; i++) {
        RiceFeature *f = &roadmap.backlog[i];
        printf("%3zu  %-35s %8.2f %8s\n", i + 1, f->feature_name, f->rice_score, hz[f->horizon]);
    }

    printf("\nHorizon Distribution: NOW=%d NEXT=%d LATER=%d\n",
           roadmap_count_by_horizon(&roadmap, TIME_HORIZON_NOW),
           roadmap_count_by_horizon(&roadmap, TIME_HORIZON_NEXT),
           roadmap_count_by_horizon(&roadmap, TIME_HORIZON_LATER));

    roadmap_auto_commit_now(&roadmap, 3);

    roadmap_add_theme(&roadmap, "Core Experience", TIME_HORIZON_NOW);
    const char *kr_core[] = {"WAU > 10,000", "Dashboard load < 2s", "NPS > 45"};
    roadmap_set_okr(&roadmap.themes[0], "Deliver best-in-class charting experience", kr_core, 3);

    roadmap_add_theme(&roadmap, "Enterprise Growth", TIME_HORIZON_NEXT);
    const char *kr_ent[] = {"5 enterprise deals closed", "SSO adoption > 60%", "Churn < 2%"};
    roadmap_set_okr(&roadmap.themes[1], "Win and retain enterprise customers", kr_ent, 3);

    roadmap_add_theme(&roadmap, "Platform Intelligence", TIME_HORIZON_LATER);
    const char *kr_ai[] = {"AI insights used by 30%", ">1000 datasets processed/week"};
    roadmap_set_okr(&roadmap.themes[2], "Leverage AI for proactive analytics", kr_ai, 2);

    printf("\nOpportunity Scores:\n");
    struct { const char *n; double i; double g; } opps[] = {
        {"Onboarding simplification", 4.5, 2.0},
        {"Enterprise security certs", 3.5, 4.0},
        {"Mobile companion app", 5.0, 3.5},
        {"Third-party integrations", 4.0, 2.5},
        {"Collaboration features", 3.0, 4.5},
    };
    for (int i = 0; i < 5; i++) {
        printf("  %-30s score=%.2f (imp=%.1f gap=%.1f)\n",
               opps[i].n,
               roadmap_calc_opportunity_score(opps[i].i, opps[i].g),
               opps[i].i, opps[i].g);
    }

    /* ========== 3. SPEC & DESIGN ========== */
    separator("3. SPECIFICATION & DESIGN");
    PRD prd;
    prd_init(&prd, "DataViz Pro v1.0 PRD",
             "Business users spend >4 hours/week creating charts manually in spreadsheets.");

    strncpy(prd.target_user, "Business analysts, team leads, data engineers",
            sizeof(prd.target_user) - 1);
    strncpy(prd.success_metrics, "Time-to-chart <30s, WAU >10K, NPS >45, Churn <5%",
            sizeof(prd.success_metrics) - 1);
    strncpy(prd.scope, "Chart builder, data import, dashboard sharing, color themes, PDF export",
            sizeof(prd.scope) - 1);
    strncpy(prd.out_of_scope, "White-label, on-premise deploy, custom plugins (v2)",
            sizeof(prd.out_of_scope) - 1);
    strncpy(prd.assumptions, "Users have modern browsers; Backend API v2 complete by Sprint 6",
            sizeof(prd.assumptions) - 1);
    strncpy(prd.risks, "Chart rendering perf at >10K data points; Browser compatibility for Safari <14",
            sizeof(prd.risks) - 1);

    UserStory us1, us2, us3;
    user_story_init(&us1, "business analyst",
                    "drag and drop CSV files to create charts",
                    "I can visualize data in seconds without writing code");
    user_story_add_acceptance(&us1, "a CSV with 3 numeric columns and 1 date column",
                              "I drag it onto the dashboard",
                              "a bar chart auto-generates with correct axes, labels, and title");
    us1.priority = 1; us1.story_points = 8;

    user_story_init(&us2, "team lead",
                    "share dashboards with a view-only link",
                    "my team can see real-time data without creating accounts");
    user_story_add_acceptance(&us2, "a dashboard is saved",
                              "I click Share and copy the generated URL",
                              "opening the URL in incognito shows the dashboard with current data, read-only");
    us2.priority = 1; us2.story_points = 5;

    user_story_init(&us3, "data engineer",
                    "connect to PostgreSQL databases for live data",
                    "dashboards always show fresh data without manual CSV uploads");
    user_story_add_acceptance(&us3, "a PostgreSQL DB connection is configured and tested",
                              "I create a chart from a database table",
                              "the chart refreshes automatically every 5 minutes with latest data");
    us3.priority = 2; us3.story_points = 13;

    prd_add_story(&prd, &us1);
    prd_add_story(&prd, &us2);
    prd_add_story(&prd, &us3);

    prd_print_summary(&prd);
    printf("\nUser Stories:\n");
    for (size_t i = 0; i < prd.story_count; i++) {
        char buf[1024];
        user_story_format(&prd.stories[i], buf, sizeof(buf));
        printf("%s\n", buf);
    }

    TechnicalSpec ts;
    memset(&ts, 0, sizeof(ts));
    tech_spec_add_endpoint(&ts, "POST /api/v1/charts", "POST", 201);
    tech_spec_add_endpoint(&ts, "GET /api/v1/charts/{id}", "GET", 200);
    tech_spec_add_endpoint(&ts, "PUT /api/v1/charts/{id}", "PUT", 200);
    tech_spec_add_endpoint(&ts, "DELETE /api/v1/charts/{id}", "DELETE", 204);
    tech_spec_add_endpoint(&ts, "POST /api/v1/dashboards", "POST", 201);
    tech_spec_add_endpoint(&ts, "GET /api/v1/dashboards/{id}", "GET", 200);
    tech_spec_add_endpoint(&ts, "POST /api/v1/dashboards/{id}/share", "POST", 200);
    tech_spec_add_endpoint(&ts, "POST /api/v1/datasources", "POST", 201);
    tech_spec_add_endpoint(&ts, "GET /api/v1/datasources/{id}/test", "POST", 200);

    tech_spec_add_model(&ts, "Chart");
    tech_spec_add_model(&ts, "Dashboard");
    tech_spec_add_model(&ts, "DataSource");
    tech_spec_add_model(&ts, "ShareLink");
    tech_spec_add_model(&ts, "User");

    tech_spec_add_mockup(&ts, "Dashboard Home", "Responsive grid of chart cards with search bar and + button");
    tech_spec_add_mockup(&ts, "Chart Editor", "Left sidebar config panel + live preview canvas on right");
    tech_spec_add_mockup(&ts, "Share Dialog", "Modal with link display, copy button, email input, and permission dropdown");
    tech_spec_add_mockup(&ts, "Data Import Wizard", "Step-by-step: upload file -> preview -> map columns -> confirm");

    tech_spec_print_summary(&ts);

    DesignReview dr;
    design_review_init(&dr);
    dr.requirements_clarity = true;
    dr.edge_cases_covered = true;
    dr.error_handling_documented = true;
    dr.security_review = true;
    dr.performance_constraints = true;
    dr.dependency_analysis = true;
    dr.backward_compatibility = true;
    dr.monitoring_plan = true;
    dr.accessibility_review = false;
    dr.i18n_considerations = false;

    printf("\nDesign Review: %.0f%% complete  All passed: %s\n",
           design_review_score(&dr) * 100,
           design_review_all_passed(&dr) ? "Yes" : "No");
    char pending[512];
    design_review_list_pending(&dr, pending, sizeof(pending));
    if (pending[0]) printf("Pending items:\n%s", pending);

    /* ========== 4. STAKEHOLDER MANAGEMENT ========== */
    separator("4. STAKEHOLDER MANAGEMENT");

    Stakeholder sh[8];
    stakeholder_init(&sh[0], "CEO", "Chief Executive Officer", 1.0, 0.8);
    stakeholder_init(&sh[1], "CTO", "Chief Technology Officer", 0.9, 0.95);
    stakeholder_init(&sh[2], "VP Product", "VP of Product", 0.85, 0.9);
    stakeholder_init(&sh[3], "Engineering Manager", "Engineering Lead", 0.6, 0.9);
    stakeholder_init(&sh[4], "Customer Support", "Support Team Lead", 0.2, 0.85);
    stakeholder_init(&sh[5], "Legal Team", "Legal Counsel", 0.75, 0.3);
    stakeholder_init(&sh[6], "End Users", "Daily Active Users", 0.1, 0.7);
    stakeholder_init(&sh[7], "Investors", "Board Members", 0.8, 0.35);

    char map[3072];
    stakeholder_map_print(sh, 8, map, sizeof(map));
    printf("Stakeholder Power x Interest Map:\n%s\n", map);

    CommunicationPlan comms[8];
    comm_plan_generate(sh, 8, comms);
    printf("Communication Plans:\n");
    printf("%-25s %-14s %-10s %-15s %s\n", "Stakeholder", "Channel", "Frequency", "Owner", "Content");
    printf("------------------------------------------------------------------------------------------------------\n");
    for (int i = 0; i < 8; i++) {
        printf("%-25s %-14s %-10s %-15s %s\n",
               comms[i].stakeholder_name, comms[i].channel, comms[i].frequency,
               comms[i].owner, comms[i].content_summary);
    }

    RaciMatrix rm;
    raci_matrix_init(&rm);
    raci_add_item(&rm, "Approve Product Requirements", "Product Manager", "VP Product");
    raci_add_consulted(&rm, 0, "CTO"); raci_add_consulted(&rm, 0, "Design Lead");
    raci_add_informed(&rm, 0, "CEO"); raci_add_informed(&rm, 0, "Marketing");

    raci_add_item(&rm, "Define Technical Architecture", "Tech Lead", "CTO");
    raci_add_consulted(&rm, 1, "Security"); raci_add_consulted(&rm, 1, "DevOps");
    raci_add_informed(&rm, 1, "Product Manager");

    raci_add_item(&rm, "Execute Go-to-Market Plan", "Marketing Lead", "VP Marketing");
    raci_add_consulted(&rm, 2, "Product Manager"); raci_add_consulted(&rm, 2, "Sales");
    raci_add_informed(&rm, 2, "CEO"); raci_add_informed(&rm, 2, "Engineering Manager");

    raci_add_item(&rm, "Customer Data Migration", "Support Lead", "Engineering Manager");
    raci_add_consulted(&rm, 3, "DBA"); raci_add_consulted(&rm, 3, "Security");
    raci_add_informed(&rm, 3, "Product Manager"); raci_add_informed(&rm, 3, "VP Product");

    printf("\nRACI Matrix (%zu items):\n", rm.item_count);
    for (size_t i = 0; i < rm.item_count; i++) {
        printf("  %s\n    R: %-15s A: %s\n", rm.items[i].task,
               rm.items[i].responsible, rm.items[i].accountable);
    }

    char gapbuf[512];
    if (raci_has_gaps(&rm, gapbuf, sizeof(gapbuf))) {
        printf("\nRACI Gaps:\n%s", gapbuf);
    }

    Escalation esc;
    escalation_init(&esc, "Scope creep: AI features may delay v1 launch by 4 weeks", 2);
    escalation_print(&esc);
    printf("  Requires attention (threshold=2): %s\n",
           escalation_requires_attention(&esc, 2) ? "Yes" : "No");

    escalation_resolve(&esc, "AI moved to LATER horizon; v1 scope locked to MVP features.");
    printf("  After resolution:\n");
    escalation_print(&esc);

    /* ========== 5. RELEASE MANAGEMENT ========== */
    separator("5. RELEASE MANAGEMENT");

    SemVer v;
    semver_parse("1.0.0", &v);
    char vstr[64];
    semver_to_string(&v, vstr, sizeof(vstr));
    printf("Initial version: %s  stable=%s\n", vstr,
           semver_is_stable(&v) ? "yes" : "no");

    semver_bump_patch(&v);
    semver_to_string(&v, vstr, sizeof(vstr));
    printf("Patch bump: %s\n", vstr);

    semver_bump_minor(&v);
    semver_to_string(&v, vstr, sizeof(vstr));
    printf("Minor bump: %s\n", vstr);

    semver_bump_major(&v);
    semver_to_string(&v, vstr, sizeof(vstr));
    printf("Major bump: %s\n", vstr);

    ReleaseTrain rt;
    release_train_init(&rt, "RT-1: DataViz Pro v1.1", 1, 1, 0, 3);
    release_train_lock(&rt);
    release_train_print(&rt);

    FeatureFlag ff[2];
    feature_flag_init(&ff[0], "new_chart_builder", "alice@eng", "Next-gen chart rendering engine");
    feature_flag_init(&ff[1], "dark_mode", "bob@eng", "System-wide dark theme");

    feature_flag_enable(&ff[0]); feature_flag_set_rollout(&ff[0], 10.0);
    feature_flag_enable(&ff[1]); feature_flag_set_rollout(&ff[1], 100.0);

    printf("\nFeature Flags:\n");
    for (int i = 0; i < 2; i++) {
        printf("  %-20s enabled=%s rollout=%.0f%% active(user42)=%s kill_switch=%s\n",
               ff[i].flag_name, ff[i].enabled ? "yes" : "no",
               ff[i].rollout_percentage,
               feature_flag_is_active_for_user(&ff[i], 42) ? "yes" : "no",
               ff[i].kill_switch_active ? "ON" : "off");
    }

    ReleaseChecklistItem checklist[] = {
        {"Code freeze", true, "tech-lead", ""},
        {"QA regression suite", true, "qa", "247 tests pass"},
        {"Load testing 10K concurrent", true, "sre", "All metrics green"},
        {"Security scan (SAST/DAST)", true, "security", "0 critical"},
        {"DB migration forward/rollback tested", true, "dba", "v15 OK both ways"},
        {"Documentation updated", false, "docs", "API ref section pending"},
        {"Stakeholder sign-off", true, "pm", "VP Product approved"},
        {"Monitoring dashboards ready", true, "devops", "Grafana + PagerDuty"},
    };
    size_t clsz = sizeof(checklist) / sizeof(checklist[0]);
    printf("\nRelease Checklist (%zu items): %.0f%% complete  All done: %s\n",
           clsz, release_checklist_progress(checklist, clsz) * 100,
           release_checklist_all_done(checklist, clsz) ? "Yes" : "No");

    RollbackPlan rp;
    rollback_plan_init(&rp,
        "P99 latency > 500ms OR error rate > 1% for 5 min",
        "1. Activate kill switch on new features\n2. Re-deploy v1.0.9 artifacts\n3. Rollback DB migration v15\n4. Verify smoke tests (health, dashboard load)\n5. Notify on-call via PagerDuty",
        20);
    printf("\nRollback Plan: valid=%s  estimated=%d min\n",
           rollback_plan_validate(&rp) ? "yes" : "no", rp.estimated_minutes);

    printf("\nRelease Stage Progression:\n  %s -> %s -> %s -> %s -> %s (terminal)\n",
           release_stage_name(STAGE_PRE_ALPHA), release_stage_name(STAGE_ALPHA),
           release_stage_name(STAGE_BETA), release_stage_name(STAGE_RC),
           release_stage_name(STAGE_GA));

    Announcement ann;
    announcement_init(&ann, "DataViz Pro v1.1 — Feature Update", "2026-07-01");
    announcement_add_highlight(&ann, "New chart builder with 15+ chart types and live preview");
    announcement_add_highlight(&ann, "Real-time data streaming via WebSocket connections");
    announcement_add_highlight(&ann, "Dashboard sharing with password protection and expiration dates");
    announcement_add_highlight(&ann, "Dark mode for reduced eye strain during long sessions");
    announcement_add_highlight(&ann, "PostgreSQL live data source with auto-refresh");
    strncpy(ann.known_issues, "PDF export: custom fonts may not render on Safari <14.",
            sizeof(ann.known_issues) - 1);
    strncpy(ann.upgrade_guide, "Run 'dviz upgrade' or pull docker.io/dataviz/pro:v1.1. DB migration v15 auto-applies.",
            sizeof(ann.upgrade_guide) - 1);

    char annb[3072];
    announcement_generate(&ann, annb, sizeof(annb));
    printf("\n%s\n", annb);

    /* Cleanup */
    product_lifecycle_reset(&pl);
    roadmap_free(&roadmap);
    spec_free_all(&prd, &ts);
    raci_free(&rm);

    printf("=== Full Product Engineering Demo Complete ===\n");
    return 0;
}
