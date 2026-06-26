#include "static_analysis.h"
#include <stdio.h>
#include <string.h>

int main(void) {
    sa_engine_t engine;
    sa_engine_init(&engine);

    const char *bug_tags[] = {"cwe-476", "null-pointer"};
    sa_register_rule_with_tags(&engine, "S101", SA_SEVERITY_BLOCKER,
                               SA_CATEGORY_BUG,
                               "Null pointer dereference detected",
                               30, bug_tags, 2);
    sa_register_rule(&engine, "S102", SA_SEVERITY_CRITICAL,
                     SA_CATEGORY_VULNERABILITY,
                     "Buffer overflow in strcpy usage", 60);
    sa_register_rule(&engine, "S201", SA_SEVERITY_MAJOR,
                     SA_CATEGORY_CODE_SMELL,
                     "Function has too many parameters (>5)", 20);
    sa_register_rule(&engine, "S202", SA_SEVERITY_MAJOR,
                     SA_CATEGORY_CODE_SMELL,
                     "Cyclomatic complexity exceeds threshold", 45);
    sa_register_rule(&engine, "S301", SA_SEVERITY_MINOR,
                     SA_CATEGORY_CODE_SMELL,
                     "Unused variable detected", 10);
    sa_register_rule(&engine, "S302", SA_SEVERITY_MINOR,
                     SA_CATEGORY_PERFORMANCE,
                     "Inefficient string concatenation", 15);
    sa_register_rule(&engine, "S401", SA_SEVERITY_CRITICAL,
                     SA_CATEGORY_SECURITY_HOTSPOT,
                     "Hardcoded credentials detected", 20);
    sa_register_rule(&engine, "S402", SA_SEVERITY_BLOCKER,
                     SA_CATEGORY_VULNERABILITY,
                     "SQL injection via string formatting", 90);

    printf("Registered %d rules\n", engine.rule_count);

    int issues[] = {
        sa_report_issue(&engine, "S101", "src/auth.c", 45, 45,
                        "Pointer 'user' may be NULL at this point"),
        sa_report_issue(&engine, "S101", "src/parser.c", 120, 120,
                        "Potential NULL dereference after malloc"),
        sa_report_issue(&engine, "S102", "src/net.c", 89, 89,
                        "Unbounded strcpy into fixed-size buffer"),
        sa_report_issue(&engine, "S201", "src/handler.c", 15, 75,
                        "Function has 8 parameters, exceeds limit of 5"),
        sa_report_issue(&engine, "S202", "src/compute.c", 200, 350,
                        "Cyclomatic complexity is 42, exceeds limit of 15"),
        sa_report_issue(&engine, "S301", "src/util.c", 33, 33,
                        "Variable 'temp' is declared but never used"),
        sa_report_issue(&engine, "S402", "src/db.c", 67, 67,
                        "User input concatenated directly into SQL query"),
        sa_report_issue(&engine, "S401", "src/config.c", 10, 10,
                        "API key hardcoded in source file"),
    };
    (void)issues;

    printf("Reported %d issues\n", engine.issue_count);

    sa_update_issue_status(&engine, 2, SA_STATUS_CONFIRMED);
    sa_update_issue_status(&engine, 6, SA_STATUS_FALSE_POSITIVE);
    sa_assign_issue(&engine, 1, "dev-team-a");
    sa_assign_issue(&engine, 4, "dev-team-b");

    printf("\nBy Severity:\n");
    printf("  Blockers:    %d\n", sa_count_by_severity(&engine, SA_SEVERITY_BLOCKER));
    printf("  Criticals:   %d\n", sa_count_by_severity(&engine, SA_SEVERITY_CRITICAL));
    printf("  Majors:      %d\n", sa_count_by_severity(&engine, SA_SEVERITY_MAJOR));
    printf("  Minors:      %d\n", sa_count_by_severity(&engine, SA_SEVERITY_MINOR));

    printf("\nBy Category:\n");
    printf("  Bugs:        %d\n", sa_count_by_category(&engine, SA_CATEGORY_BUG));
    printf("  Vulns:       %d\n", sa_count_by_category(&engine, SA_CATEGORY_VULNERABILITY));
    printf("  Code Smells: %d\n", sa_count_by_category(&engine, SA_CATEGORY_CODE_SMELL));

    double remediation = sa_calc_remediation_hours(&engine);
    double dev_cost = sa_estimate_dev_cost(50000.0, 0.05);
    double ratio = sa_calc_tech_debt_ratio(remediation, dev_cost);
    printf("\nTech Debt: %.1fh remediation / %.0f LOC = %.2f%%\n",
           remediation, 50000.0, ratio);

    sa_report_t report;
    sa_generate_report(&engine, &report, 50000.0);

    sa_quality_gate_t gate;
    sa_quality_gate_default(&gate);
    report.gate_result = sa_evaluate_quality_gate(&report, &gate);

    sa_print_report(&report);

    const sa_rule_t *rule = sa_find_rule(&engine, "S101");
    if (rule) printf("\nRule S101: %s (remediation: %dmin)\n",
                      rule->description, rule->remediation_minutes);

    sa_disable_rule(&engine, "S301");
    printf("Disabled rule S301, remaining enabled rules: ");
    sa_enable_rule(&engine, "S301");
    printf("Re-enabled S301\n");

    return 0;
}
