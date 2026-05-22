#include "static_analysis.h"
#include "complexity_metrics.h"
#include "code_review.h"
#include "lint_formatter.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

static void simulate_scan(sa_engine_t *eng);
static void print_separator(const char *title);
static void gate_report(sa_engine_t *eng, double loc);

int main(void) {
    print_separator("MINI-ENG-QUALITY: Quality Gate Demo");

    srand((unsigned)time(NULL));

    sa_engine_t eng;
    sa_engine_init(&eng);

    printf("[1] Registering analysis rules...\n");
    sa_register_rule(&eng, "BUG-001", SA_SEVERITY_BLOCKER, SA_CATEGORY_BUG,
                     "Null pointer dereference", 30);
    sa_register_rule(&eng, "BUG-002", SA_SEVERITY_CRITICAL, SA_CATEGORY_BUG,
                     "Division by zero", 20);
    sa_register_rule(&eng, "BUG-003", SA_SEVERITY_MAJOR, SA_CATEGORY_BUG,
                     "Resource leak (missing free)", 15);
    sa_register_rule(&eng, "VULN-001", SA_SEVERITY_BLOCKER, SA_CATEGORY_VULNERABILITY,
                     "Buffer overflow in string operation", 45);
    sa_register_rule(&eng, "VULN-002", SA_SEVERITY_CRITICAL, SA_CATEGORY_VULNERABILITY,
                     "Format string vulnerability", 30);
    sa_register_rule(&eng, "VULN-003", SA_SEVERITY_MAJOR, SA_CATEGORY_VULNERABILITY,
                     "Insecure random number generation", 25);
    sa_register_rule(&eng, "SMELL-001", SA_SEVERITY_MAJOR, SA_CATEGORY_CODE_SMELL,
                     "Function exceeds 50 lines", 60);
    sa_register_rule(&eng, "SMELL-002", SA_SEVERITY_MAJOR, SA_CATEGORY_CODE_SMELL,
                     "Cyclomatic complexity > 15", 45);
    sa_register_rule(&eng, "SMELL-003", SA_SEVERITY_MINOR, SA_CATEGORY_CODE_SMELL,
                     "Magic number used instead of constant", 10);
    sa_register_rule(&eng, "SMELL-004", SA_SEVERITY_MINOR, SA_CATEGORY_CODE_SMELL,
                     "Unused function parameter", 5);
    sa_register_rule(&eng, "SMELL-005", SA_SEVERITY_MINOR, SA_CATEGORY_CODE_SMELL,
                     "Variable naming convention violation", 5);
    sa_register_rule(&eng, "PERF-001", SA_SEVERITY_MAJOR, SA_CATEGORY_PERFORMANCE,
                     "Inefficient string copy in loop", 20);
    sa_register_rule(&eng, "PERF-002", SA_SEVERITY_MINOR, SA_CATEGORY_PERFORMANCE,
                     "Unnecessary memory allocation", 10);
    sa_register_rule(&eng, "SEC-001", SA_SEVERITY_CRITICAL, SA_CATEGORY_SECURITY_HOTSPOT,
                     "Hardcoded password or token", 15);
    sa_register_rule(&eng, "SEC-002", SA_SEVERITY_MAJOR, SA_CATEGORY_SECURITY_HOTSPOT,
                     "Missing input validation", 20);
    printf("    Registered %d rules.\n", eng.rule_count);

    printf("\n[2] Simulating source code scan...\n");
    simulate_scan(&eng);

    printf("\n[3] Generating analysis report...\n");
    double loc = 85000.0;
    sa_report_t report;
    sa_generate_report(&eng, &report, loc);

    printf("\n[4] Configuring Quality Gate thresholds...\n");
    sa_quality_gate_t gate;
    sa_quality_gate_default(&gate);
    gate.blocker_threshold = 0.0;
    gate.critical_threshold = 5.0;
    gate.major_threshold = 20.0;
    gate.minor_threshold = 50.0;
    printf("    Blocker max: %.0f | Critical max: %.0f | Major max: %.0f | Minor max: %.0f\n",
           gate.blocker_threshold, gate.critical_threshold,
           gate.major_threshold, gate.minor_threshold);

    printf("\n[5] Evaluating against Quality Gate...\n");
    report.gate_result = sa_evaluate_quality_gate(&report, &gate);
    gate_report(&eng, loc);

    printf("\n[6] Computing complexity metrics for key files...\n");
    cm_analysis_t cm;
    cm_analysis_init(&cm);

    const char *hot_func =
        "void process_request(struct request *r) {\n"
        "    if (!r) return;\n"
        "    if (r->type == TYPE_GET) {\n"
        "        handle_get(r);\n"
        "    } else if (r->type == TYPE_POST) {\n"
        "        if (r->body_len > 0) {\n"
        "            if (r->authenticated) {\n"
        "                handle_post_auth(r);\n"
        "            } else {\n"
        "                handle_post_anon(r);\n"
        "            }\n"
        "        } else {\n"
        "            send_error(r, 400);\n"
        "        }\n"
        "    } else if (r->type == TYPE_PUT) {\n"
        "        handle_put(r);\n"
        "    } else {\n"
        "        send_error(r, 405);\n"
        "    }\n"
        "}\n";

    cm_analyze_function(&cm, "process_request", "src/handler.c", 1, 18,
                        hot_func, (int)strlen(hot_func));

    cm_hotspot_analysis_t hotspots;
    cm_hotspot_init(&hotspots);
    cm_add_hotspot(&hotspots, "src/auth.c", 100, 250, 150, 47);
    cm_add_hotspot(&hotspots, "src/handler.c", 1, 400, 400, 82);
    cm_add_hotspot(&hotspots, "src/db.c", 200, 600, 400, 55);
    cm_add_hotspot(&hotspots, "src/parser.c", 50, 300, 250, 38);
    cm_add_hotspot(&hotspots, "include/types.h", 1, 500, 500, 25);
    cm_sort_hotspots(&hotspots);
    cm_print_hotspots(&hotspots);

    printf("\n[7] Running pre-commit lint check...\n");
    lf_engine_t lint;
    lf_engine_init(&lint);
    lf_register_rule(&lint, "no-gets", LF_CATEGORY_SECURITY, LF_SEVERITY_ERROR,
                     "Forbid unsafe gets() usage");
    lf_register_rule(&lint, "no-strcpy", LF_CATEGORY_SECURITY, LF_SEVERITY_ERROR,
                     "Forbid unsafe strcpy() usage");
    lf_register_rule(&lint, "trailing-ws", LF_CATEGORY_STYLE, LF_SEVERITY_WARN,
                     "No trailing whitespace");
    lf_register_rule(&lint, "long-line", LF_CATEGORY_STYLE, LF_SEVERITY_WARN,
                     "Line exceeds 120 characters");

    const char *staged_files[] = {"src/main.c", "src/util.c", "include/types.h"};
    lf_violation_t violations[64];
    for (int i = 0; i < 3; i++) {
        const char *sample = "// sample source\nint main(void) { return 0; } \n";
        int vc = lf_lint_file(&lint, staged_files[i], sample, violations, 64);
        printf("    %s: %d violation(s)\n", staged_files[i], vc);
    }
    lf_print_summary(&lint);

    printf("\n[8] Code review simulation...\n");
    cr_review_t review;
    memset(&review, 0, sizeof(review));
    snprintf(review.title, sizeof(review.title), "Feature: Quality Gate Integration");
    cr_assign_reviewer(&review, "senior-dev");
    cr_assign_reviewer(&review, "security-team");
    cr_init_checklist(&review.checklist);
    cr_add_checklist_item(&review.checklist, "All quality gates pass");
    cr_add_checklist_item(&review.checklist, "No new blockers or criticals");
    cr_add_checklist_item(&review.checklist, "Test coverage maintained");
    cr_add_checklist_item(&review.checklist, "Documentation updated");
    cr_check_item(&review.checklist, 0);
    cr_check_item(&review.checklist, 1);
    cr_check_item(&review.checklist, 2);
    cr_check_item(&review.checklist, 3);
    cr_add_comment(&review, 42, "LGTM, quality gate validation looks solid",
                   CR_SEVERITY_NIT, "reviewer1");

    cr_set_approval_rules(&review.rules, 2, 1);
    cr_compute_stats(&review);
    printf("    Reviewers: %d | Checklist items: %d | All checked: %s\n",
           review.reviewer_count, review.checklist.count,
           cr_all_checked(&review.checklist) ? "YES" : "NO");

    printf("\n[9] Strict quality gate scenario (zero-tolerance)...\n");
    sa_quality_gate_t strict_gate;
    sa_quality_gate_default(&strict_gate);
    strict_gate.blocker_threshold = 0.0;
    strict_gate.critical_threshold = 0.0;
    strict_gate.major_threshold = 5.0;
    strict_gate.minor_threshold = 10.0;
    sa_report_t strict_report;
    sa_generate_report(&eng, &strict_report, loc);
    sa_quality_result_e strict_result =
        sa_evaluate_quality_gate(&strict_report, &strict_gate);
    printf("    Strict gate result: %s (%d blockers, %d critical)\n",
           strict_result == SA_QUALITY_PASS ? "PASS" :
           strict_result == SA_QUALITY_WARN ? "WARN" : "FAIL",
           strict_report.blockers, strict_report.criticals);

    printf("\n[10] Remediation cost planning...\n");
    double total_remediation = sa_calc_remediation_hours(&eng);
    double cost_per_hour = 100.0;
    printf("    Total remediation: %.1f hours\n", total_remediation);
    printf("    Estimated cost:    $%.0f (@$%.0f/h)\n",
           total_remediation * cost_per_hour, cost_per_hour);
    printf("    Dev weeks (40h/wk): %.1f\n", total_remediation / 40.0);

    printf("\n==============================================\n");
    printf("  FINAL QUALITY GATE DECISION: %s\n",
           report.gate_result == SA_QUALITY_PASS ? "PASS" :
           report.gate_result == SA_QUALITY_WARN ? "WARN (review required)" :
           "FAIL (merge blocked)");
    printf("==============================================\n");

    return 0;
}

static void print_separator(const char *title) {
    printf("\n========================================\n");
    printf("  %s\n", title);
    printf("========================================\n\n");
}

static void simulate_scan(sa_engine_t *eng) {
    struct { const char *file; const char *rule; int line;
             const char *msg; } issues[] = {
        {"src/auth.c",   "VULN-001",  45, "Buffer overflow: strcpy into 32-byte array"},
        {"src/auth.c",   "SEC-001",   12, "Hardcoded API token: 'sk-abc123'"},
        {"src/auth.c",   "SMELL-003", 78, "Magic number 4096 used for buffer size"},
        {"src/db.c",     "BUG-003",  120, "Resource leak: db_result not freed on error path"},
        {"src/db.c",     "BUG-001",   56, "Null pointer: conn may be NULL after failed init"},
        {"src/db.c",     "PERF-001", 200, "strcat in loop, consider strncat with pre-allocated buffer"},
        {"src/db.c",     "SMELL-002", 150, "Complexity 22 in query_build function"},
        {"src/net.c",    "VULN-002",  88, "Format string from user input in log function"},
        {"src/net.c",    "VULN-003",  33, "rand() used for session token generation"},
        {"src/net.c",    "SEC-002",   50, "Missing input validation on packet size field"},
        {"src/handler.c","BUG-002",   95, "Potential div-by-zero: denom from user input unchecked"},
        {"src/handler.c","SMELL-001", 10, "handle_request function is 220 lines"},
        {"src/handler.c","SMELL-004", 135,"Unused parameter 'flags' in callback"},
        {"src/handler.c","SMELL-005", 45, "Variable 'tmpBuf' violates snake_case convention"},
        {"src/parser.c", "BUG-001",   30, "NULL deref: parsed node without type check"},
        {"src/parser.c", "SMELL-001",  1, "parse_xml function is 310 lines"},
        {"src/parser.c", "SMELL-005", 67, "Variable 'XMLParser' uses CamelCase"},
        {"src/util.c",   "BUG-003",   15, "Leak: buffer allocated but not freed on early return"},
        {"src/util.c",   "PERF-002",  42, "malloc called inside tight loop"},
        {"src/config.c", "SEC-001",    5, "Database password in source: 'admin123'"},
        {"src/config.c", "SMELL-003", 20, "Magic numbers: timeout 30000, retry 5"},
        {"test/test_auth.c","SMELL-004", 10,"Unused parameter 'ctx' in test helper"},
    };

    for (int i = 0; i < 22; i++) {
        sa_report_issue(eng, issues[i].rule, issues[i].file,
                        issues[i].line, issues[i].line, issues[i].msg);
    }
    printf("    Scanned 9 files, found %d issues.\n", eng->issue_count);

    sa_update_issue_status(eng, 10, SA_STATUS_CONFIRMED);
    sa_update_issue_status(eng, 15, SA_STATUS_FALSE_POSITIVE);
    sa_update_issue_status(eng, 20, SA_STATUS_FALSE_POSITIVE);
    sa_assign_issue(eng, 1, "security-team");
    sa_assign_issue(eng, 4, "backend-team");
    sa_assign_issue(eng, 7, "backend-team");
}

static void gate_report(sa_engine_t *eng, double loc) {
    sa_report_t report;
    sa_generate_report(eng, &report, loc);

    sa_quality_gate_t gate;
    sa_quality_gate_default(&gate);
    report.gate_result = sa_evaluate_quality_gate(&report, &gate);

    sa_print_report(&report);

    double dev_cost = sa_estimate_dev_cost(loc, 0.05);
    double ratio = sa_calc_tech_debt_ratio(
        report.total_remediation_hours, dev_cost);
    printf("\n  Dev Cost:       %.1fh (%.0f LOC @ 0.05h/LOC)\n",
           dev_cost, loc);
    printf("  Tech Debt Ratio: %.2f%%\n", ratio);
    printf("  Estimated days to fix: %.1f\n",
           report.total_remediation_hours / 8.0);
}
