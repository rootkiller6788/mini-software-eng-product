#include "tech_debt_sprint.h"
#include "complexity_metrics.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

static double fitness_test_coverage(void *ctx);
static double fitness_cyclomatic_limit(void *ctx);
static double fitness_coupling_limit(void *ctx);
static void print_section(const char *title);
static void simulate_sprint_lifecycle(tds_backlog_t *bl);

int main(void) {
    print_section("MINI-ENG-QUALITY: Tech Debt Sprint Management Demo");

    tds_backlog_t backlog;
    tds_backlog_init(&backlog);

    printf("[1] Populating Technical Debt Backlog...\n\n");

    tds_add_debt(&backlog,
        "Monolithic auth module",
        "Authentication logic is tightly coupled across 15 files. "
        "Should be extracted into a standalone service.",
        "alice", TDS_SEVERITY_CRITICAL, TDS_CATEGORY_ARCHITECTURE,
        80.0, 25.0);

    tds_add_debt(&backlog,
        "Cyclic dependency: core <-> utils",
        "Core module depends on utils, utils imports core headers. "
        "Break with dependency inversion.",
        "bob", TDS_SEVERITY_HIGH, TDS_CATEGORY_ARCHITECTURE,
        40.0, 18.0);

    tds_add_debt(&backlog,
        "God class: DataProcessor",
        "DataProcessor class is 2400 lines with 80+ methods. "
        "Split into DataReader, DataWriter, DataValidator.",
        "charlie", TDS_SEVERITY_CRITICAL, TDS_CATEGORY_CODE_QUALITY,
        60.0, 22.0);

    tds_add_debt(&backlog,
        "Missing error handling in payment flow",
        "payment_process() has no error handling for network failures. "
        "Transactions can be lost.",
        "diana", TDS_SEVERITY_CRITICAL, TDS_CATEGORY_CODE_QUALITY,
        24.0, 30.0);

    tds_add_debt(&backlog,
        "Test coverage below 60%",
        "Overall test coverage is 52%. Need to reach 80%. "
        "Focus on auth, payment, and data modules.",
        "eve", TDS_SEVERITY_HIGH, TDS_CATEGORY_TEST_COVERAGE,
        120.0, 15.0);

    tds_add_debt(&backlog,
        "No integration tests for API endpoints",
        "All 45 API endpoints lack integration test coverage. "
        "Write at least smoke tests for critical paths.",
        "eve", TDS_SEVERITY_HIGH, TDS_CATEGORY_TEST_COVERAGE,
        60.0, 12.0);

    tds_add_debt(&backlog,
        "N+1 query problem in product listing",
        "Product listing makes separate DB query for each item's "
        "category. Use JOIN or batch fetch.",
        "frank", TDS_SEVERITY_MAJOR, TDS_CATEGORY_PERFORMANCE,
        16.0, 20.0);

    tds_add_debt(&backlog,
        "Large file upload blocks event loop",
        "Uploading files > 100MB blocks the main thread. "
        "Implement streaming/chunked upload.",
        "frank", TDS_SEVERITY_HIGH, TDS_CATEGORY_PERFORMANCE,
        32.0, 18.0);

    tds_add_debt(&backlog,
        "Hardcoded encryption key",
        "AES-256 key is hardcoded in config.c line 23. "
        "Move to secure key management service.",
        "security-team", TDS_SEVERITY_CRITICAL, TDS_CATEGORY_SECURITY,
        8.0, 35.0);

    tds_add_debt(&backlog,
        "No rate limiting on login endpoint",
        "Login endpoint vulnerable to brute force. "
        "Implement exponential backoff or CAPTCHA.",
        "security-team", TDS_SEVERITY_HIGH, TDS_CATEGORY_SECURITY,
        12.0, 25.0);

    tds_add_debt(&backlog,
        "Dependency on deprecated library libxml v2.3",
        "libxml v2.3 has 4 known CVEs. "
        "Upgrade to v3.1+ or migrate to alternative.",
        "ops-team", TDS_SEVERITY_HIGH, TDS_CATEGORY_DEPENDENCY,
        20.0, 28.0);

    tds_add_debt(&backlog,
        "Outdated OpenSSL 1.1.0",
        "OpenSSL 1.1.0 reached EOL. "
        "Need to upgrade to 3.0 LTS.",
        "ops-team", TDS_SEVERITY_CRITICAL, TDS_CATEGORY_DEPENDENCY,
        35.0, 30.0);

    tds_add_debt(&backlog,
        "Missing API documentation",
        "All v2 endpoints undocumented. "
        "Auto-generate OpenAPI specs from code annotations.",
        "docs-team", TDS_SEVERITY_MEDIUM, TDS_CATEGORY_DOCUMENTATION,
        40.0, 8.0);

    tds_add_debt(&backlog,
        "Onboarding guide stale since v1.5",
        "Developer onboarding guide references old architecture. "
        "Rewrite for v2.x.",
        "docs-team", TDS_SEVERITY_LOW, TDS_CATEGORY_DOCUMENTATION,
        16.0, 5.0);

    tds_print_backlog(&backlog);

    printf("\n[2] Calculating Interest on Outstanding Debt...\n\n");
    int ages[] = {180, 120, 90, 60, 150, 100, 75, 45, 200, 80, 160, 140, 30, 15};
    for (int i = 0; i < backlog.count; i++) {
        tds_calc_interest(&backlog.items[i], ages[i]);
    }

    tds_interest_report_t report;
    tds_interest_report(&backlog, &report);
    printf("  Total Principal:      %.1f hours\n", report.total_principal_hours);
    printf("  Total Interest:       %.1f hours\n", report.total_interest_hours);
    printf("  Interest Cost ($):    $%.0f (@$100/h)\n", report.total_interest_cost);
    printf("  Avg Interest Rate:    %.2f%%\n", report.avg_interest_rate);
    printf("  Avg Age:              %.0f days\n", report.avg_age_days);

    printf("\n[3] Debt Reduction Sprint Planning (20%% Allocation)...\n\n");
    tds_sort_by_interest(&backlog);

    tds_sprint_t sprint;
    tds_sprint_init(&sprint, 42, "Reducing Technical Debt - Sprint 42",
                    80, 20);

    double team_capacity = tds_sprint_capacity_hours(
        &sprint, 10, 6, 6.0);
    double debt_budget = team_capacity * 0.20;
    printf("  Sprint Duration:   10 days\n");
    printf("  Team Size:         6 developers\n");
    printf("  Team Capacity:     %.1f hours\n", team_capacity);
    printf("  Debt Budget (20%%): %.1f hours\n", debt_budget);

    int added_count = 0;
    double allocated = 0.0;
    for (int i = 0; i < backlog.count && allocated < debt_budget; i++) {
        if (backlog.items[i].status == TDS_STATUS_BACKLOG &&
            backlog.items[i].estimated_hours <= (debt_budget - allocated)) {
            if (tds_sprint_add_item(&sprint, &backlog.items[i]) == 0) {
                allocated += backlog.items[i].estimated_hours;
                added_count++;
            }
        }
    }
    printf("  Items Added:       %d (%.1f hours allocated)\n",
           added_count, allocated);

    tds_sprint_print(&sprint);

    printf("\n[4] Simulating Sprint Lifecycle...\n\n");
    simulate_sprint_lifecycle(&backlog);

    printf("\n[5] ROI Analysis of Completed Debt Items...\n\n");
    int resolved[] = {1, 3, 4, 7, 9, 12};
    tds_roi_analysis_t roi;
    tds_compute_roi(&backlog, resolved, 6, 80.0, &roi);
    printf("  Principal Before:  %.1fh\n", roi.principal_before);
    printf("  Principal After:   %.1fh\n", roi.principal_after);
    printf("  Effort Spent:      %.1fh\n", roi.effort_spent);
    printf("  Interest Reduced:  %.1fh\n", roi.interest_reduction);
    printf("  ROI Ratio:         %.2fx\n", roi.roi_ratio);
    printf("  Items Resolved:    %d\n", roi.items_resolved);

    double debt_idx = tds_debt_index(&backlog, 20.0);
    printf("\n  Debt Index:        %.1f sprints to clear backlog\n", debt_idx);

    printf("\n[6] Architecture Fitness Functions...\n\n");

    tds_fitness_suite_t suite;
    tds_fitness_init(&suite);

    double cov_val = 72.5;
    double cc_val = 8.3;
    double coup_val = 0.42;

    tds_fitness_add(&suite, "Test Coverage >= 70%",
                    fitness_test_coverage, &cov_val, 70.0);
    tds_fitness_add(&suite, "Avg Cyclomatic < 10",
                    fitness_cyclomatic_limit, &cc_val, 10.0);
    tds_fitness_add(&suite, "Coupling Factor < 0.5",
                    fitness_coupling_limit, &coup_val, 0.5);

    tds_fitness_evaluate(&suite);
    tds_fitness_print(&suite);

    printf("\n  All fitness functions passed: %s\n",
           tds_fitness_all_passed(&suite) ? "YES" : "NO");

    printf("\n[7] Complexity Hotspot Analysis on Debt-Heavy Files...\n\n");
    cm_hotspot_analysis_t ha;
    cm_hotspot_init(&ha);

    cm_add_hotspot(&ha, "src/auth/auth_module.c",    1, 2400, 2400, 87);
    cm_add_hotspot(&ha, "src/core/data_processor.c", 1, 2400, 2400, 120);
    cm_add_hotspot(&ha, "src/payment/processor.c",   50, 400, 350, 45);
    cm_add_hotspot(&ha, "src/config/config.c",        1, 200, 200, 38);
    cm_add_hotspot(&ha, "src/network/upload.c",       100, 500, 400, 52);
    cm_add_hotspot(&ha, "src/deps/libxml_wrapper.c",  1, 350, 350, 25);
    cm_sort_hotspots(&ha);
    cm_print_hotspots(&ha);

    printf("\n========================================\n");
    printf("  TECH DEBT MANAGEMENT SUMMARY\n");
    printf("========================================\n");
    printf("  Backlog Items:        %d\n", backlog.count);
    printf("  Total Principal:      %.1f hours\n", tds_total_principal(&backlog));
    printf("  Total Interest:       %.1f hours\n", tds_total_interest(&backlog));
    printf("  Sprint Items:         %d (debt sprint)\n", sprint.item_count);
    printf("  Sprint Debt Budget:   %.1f hours\n", debt_budget);
    printf("  ROI Ratio:            %.2fx\n", roi.roi_ratio);
    printf("  Debt Index:           %.1f sprints\n", debt_idx);
    printf("  Fitness Functions:    %s\n",
           tds_fitness_all_passed(&suite) ? "ALL PASSED" : "SOME FAILED");
    printf("========================================\n");

    return 0;
}

static void print_section(const char *title) {
    printf("\n========================================\n");
    printf("  %s\n", title);
    printf("========================================\n\n");
}

static void simulate_sprint_lifecycle(tds_backlog_t *bl) {
    tds_sprint_t sprint;
    tds_sprint_init(&sprint, 43, "Debt Reduction - Sprint 43", 60, 20);
    sprint.is_active = 1;

    int ids[] = {5, 6, 8, 11};
    for (int i = 0; i < 4; i++) {
        tds_debt_item_t *item = tds_find_debt(bl, ids[i]);
        if (item) {
            tds_sprint_add_item(&sprint, item);
            tds_update_debt_status(item, TDS_STATUS_IN_PROGRESS);
            printf("  [Sprint %d] Started: %s (%.1fh)\n",
                   sprint.id, item->name, item->estimated_hours);
        }
    }

    tds_debt_item_t *done1 = tds_find_debt(bl, 5);
    if (done1) {
        tds_update_debt_status(done1, TDS_STATUS_DONE);
        sprint.completed_hours += done1->estimated_hours;
        done1->actual_hours = done1->estimated_hours;
    }

    tds_debt_item_t *done2 = tds_find_debt(bl, 6);
    if (done2) {
        tds_update_debt_status(done2, TDS_STATUS_DONE);
        sprint.completed_hours += done2->estimated_hours;
        done2->actual_hours = done2->estimated_hours;
    }

    tds_debt_item_t *deferred = tds_find_debt(bl, 11);
    if (deferred) {
        tds_update_debt_status(deferred, TDS_STATUS_DEFERRED);
        printf("  [Sprint %d] Deferred: %s (blocked by dependency)\n",
               sprint.id, deferred->name);
    }

    sprint.is_active = 0;

    printf("\n  Sprint %d complete: %.1f/%.1f hours done (%d/%d items)\n",
           sprint.id, sprint.completed_hours,
           sprint.debt_capacity_hours,
           (int)(sprint.completed_hours > 0 ? 2 : 0),
           sprint.item_count);

    tds_sprint_print(&sprint);
}

static double fitness_test_coverage(void *ctx) {
    if (!ctx) return 0.0;
    return *(double *)ctx;
}

static double fitness_cyclomatic_limit(void *ctx) {
    if (!ctx) return 0.0;
    double cc = *(double *)ctx;
    return cc > 0 ? 20.0 - cc : 0.0;
}

static double fitness_coupling_limit(void *ctx) {
    if (!ctx) return 0.0;
    double cp = *(double *)ctx;
    return 1.0 - cp;
}
