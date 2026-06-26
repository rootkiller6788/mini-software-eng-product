#include "static_analysis.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

void sa_engine_init(sa_engine_t *engine) {
    if (!engine) return;
    memset(engine, 0, sizeof(*engine));
    engine->next_issue_id = 1;
}

int sa_register_rule(sa_engine_t *engine, const char *rule_id,
                     sa_severity_e severity, sa_category_e category,
                     const char *description, int remediation_min) {
    const char *empty_tags[1] = {""};
    return sa_register_rule_with_tags(engine, rule_id, severity, category,
                                      description, remediation_min,
                                      empty_tags, 0);
}

int sa_register_rule_with_tags(sa_engine_t *engine, const char *rule_id,
                               sa_severity_e severity, sa_category_e category,
                               const char *description, int remediation_min,
                               const char *tags[], int tag_count) {
    if (!engine || engine->rule_count >= SA_MAX_RULES) return -1;
    sa_rule_t *r = &engine->rules[engine->rule_count];
    memset(r, 0, sizeof(*r));
    snprintf(r->rule_id, SA_MAX_RULE_ID_LEN, "%s", rule_id);
    r->severity = severity;
    r->category = category;
    r->remediation_minutes = remediation_min;
    r->enabled = 1;
    snprintf(r->description, SA_MAX_MSG_LEN, "%s", description);
    for (int i = 0; i < tag_count && i < SA_MAX_TAGS; i++)
        snprintf(r->tags[i], 64, "%s", tags[i]);
    r->tag_count = tag_count;
    engine->rule_count++;
    return 0;
}

int sa_report_issue(sa_engine_t *engine, const char *rule_id,
                    const char *file_path, int line_start, int line_end,
                    const char *message) {
    return sa_report_issue_detailed(engine, rule_id, file_path,
                                    line_start, line_end, 0, 0, message);
}

int sa_report_issue_detailed(sa_engine_t *engine, const char *rule_id,
                             const char *file_path, int line_start,
                             int line_end, int col_start, int col_end,
                             const char *message) {
    if (!engine || engine->issue_count >= SA_MAX_ISSUES) return -1;
    const sa_rule_t *rule = sa_find_rule(engine, rule_id);
    if (!rule || !rule->enabled) return -1;

    sa_issue_t *iss = &engine->issues[engine->issue_count];
    memset(iss, 0, sizeof(*iss));
    iss->id = engine->next_issue_id++;
    snprintf(iss->rule_id, SA_MAX_RULE_ID_LEN, "%s", rule_id);
    snprintf(iss->file_path, SA_MAX_FILE_PATH, "%s", file_path);
    iss->line_start = line_start;
    iss->line_end = line_end;
    iss->column_start = col_start;
    iss->column_end = col_end;
    iss->severity = rule->severity;
    iss->category = rule->category;
    iss->status = SA_STATUS_OPEN;
    iss->effort_minutes = rule->remediation_minutes;
    snprintf(iss->message, SA_MAX_MSG_LEN, "%s", message);
    engine->issue_count++;
    return iss->id;
}

void sa_update_issue_status(sa_engine_t *engine, int issue_id,
                            sa_issue_status_e status) {
    if (!engine) return;
    for (int i = 0; i < engine->issue_count; i++) {
        if (engine->issues[i].id == issue_id) {
            engine->issues[i].status = status;
            return;
        }
    }
}

void sa_assign_issue(sa_engine_t *engine, int issue_id,
                     const char *assignee) {
    if (!engine || !assignee) return;
    for (int i = 0; i < engine->issue_count; i++) {
        if (engine->issues[i].id == issue_id) {
            snprintf(engine->issues[i].assignee,
                     sizeof(engine->issues[0].assignee), "%s", assignee);
            return;
        }
    }
}

int sa_count_by_severity(const sa_engine_t *engine, sa_severity_e sev) {
    if (!engine) return 0;
    int count = 0;
    for (int i = 0; i < engine->issue_count; i++)
        if (engine->issues[i].severity == sev) count++;
    return count;
}

int sa_count_by_category(const sa_engine_t *engine, sa_category_e cat) {
    if (!engine) return 0;
    int count = 0;
    for (int i = 0; i < engine->issue_count; i++)
        if (engine->issues[i].category == cat) count++;
    return count;
}

void sa_quality_gate_default(sa_quality_gate_t *gate) {
    if (!gate) return;
    memset(gate, 0, sizeof(*gate));
    gate->blocker_threshold = 0.0;
    gate->critical_threshold = 0.0;
    gate->major_threshold = 3.0;
    gate->minor_threshold = 10.0;
    gate->max_new_blockers = 0;
    gate->max_new_criticals = 0;
    gate->max_total_issues = 1000;
    gate->min_remediation_ratio = 1.0;
}

void sa_quality_gate_set(sa_quality_gate_t *gate,
                         double blocker_thresh, double crit_thresh,
                         double major_thresh, double minor_thresh) {
    if (!gate) return;
    gate->blocker_threshold = blocker_thresh;
    gate->critical_threshold = crit_thresh;
    gate->major_threshold = major_thresh;
    gate->minor_threshold = minor_thresh;
}

double sa_calc_tech_debt_ratio(double remediation_hours,
                               double dev_cost_hours) {
    if (dev_cost_hours <= 0.0) return 0.0;
    return (remediation_hours / dev_cost_hours) * 100.0;
}

double sa_estimate_dev_cost(double loc, double cost_per_loc_hours) {
    if (loc <= 0.0 || cost_per_loc_hours <= 0.0) return 0.0;
    return loc * cost_per_loc_hours;
}

double sa_calc_remediation_hours(const sa_engine_t *engine) {
    if (!engine) return 0.0;
    double total = 0.0;
    for (int i = 0; i < engine->issue_count; i++)
        if (engine->issues[i].status == SA_STATUS_OPEN)
            total += (double)engine->issues[i].effort_minutes;
    return total / 60.0;
}

void sa_generate_report(const sa_engine_t *engine, sa_report_t *report,
                        double loc) {
    if (!engine || !report) return;
    memset(report, 0, sizeof(*report));
    report->total_issues = engine->issue_count;
    report->blockers = sa_count_by_severity(engine, SA_SEVERITY_BLOCKER);
    report->criticals = sa_count_by_severity(engine, SA_SEVERITY_CRITICAL);
    report->majors = sa_count_by_severity(engine, SA_SEVERITY_MAJOR);
    report->minors = sa_count_by_severity(engine, SA_SEVERITY_MINOR);
    report->bugs = sa_count_by_category(engine, SA_CATEGORY_BUG);
    report->vulnerabilities = sa_count_by_category(engine, SA_CATEGORY_VULNERABILITY);
    report->code_smells = sa_count_by_category(engine, SA_CATEGORY_CODE_SMELL);
    report->total_remediation_hours = sa_calc_remediation_hours(engine);
    report->lines_of_code = loc;
    double dev_cost = sa_estimate_dev_cost(loc, 0.05);
    report->technical_debt_ratio =
        sa_calc_tech_debt_ratio(report->total_remediation_hours, dev_cost);
    report->files_analyzed = 0;
    report->gate_result = SA_QUALITY_PASS;
}

sa_quality_result_e sa_evaluate_quality_gate(const sa_report_t *report,
                                             const sa_quality_gate_t *gate) {
    if (!report || !gate) return SA_QUALITY_PASS;
    if ((double)report->blockers > gate->blocker_threshold)
        return SA_QUALITY_FAIL;
    if ((double)report->criticals > gate->critical_threshold)
        return SA_QUALITY_FAIL;
    if ((double)report->majors > gate->major_threshold)
        return SA_QUALITY_FAIL;
    if ((double)report->minors > gate->minor_threshold)
        return SA_QUALITY_WARN;
    if (report->technical_debt_ratio > 5.0)
        return SA_QUALITY_WARN;
    return SA_QUALITY_PASS;
}

void sa_print_report(const sa_report_t *report) {
    if (!report) return;
    printf("========================================\n");
    printf("  Static Analysis Quality Report\n");
    printf("========================================\n");
    printf("  Lines of Code:     %.0f\n", report->lines_of_code);
    printf("  Files Analyzed:    %d\n", report->files_analyzed);
    printf("  Total Issues:      %d\n", report->total_issues);
    printf("  Blockers:          %d\n", report->blockers);
    printf("  Criticals:         %d\n", report->criticals);
    printf("  Majors:            %d\n", report->majors);
    printf("  Minors:            %d\n", report->minors);
    printf("  Bugs:              %d\n", report->bugs);
    printf("  Vulnerabilities:   %d\n", report->vulnerabilities);
    printf("  Code Smells:       %d\n", report->code_smells);
    printf("  Tech Debt Ratio:   %.2f%%\n", report->technical_debt_ratio);
    printf("  Remediation Time:  %.1fh\n", report->total_remediation_hours);
    printf("  Gate Result:       %s\n",
           report->gate_result == SA_QUALITY_PASS ? "PASS" :
           report->gate_result == SA_QUALITY_WARN ? "WARN" : "FAIL");
}

int sa_find_issue(const sa_engine_t *engine, const char *rule_id,
                  const char *file_path, int line) {
    if (!engine) return -1;
    for (int i = 0; i < engine->issue_count; i++) {
        if (strcmp(engine->issues[i].rule_id, rule_id) == 0 &&
            strcmp(engine->issues[i].file_path, file_path) == 0 &&
            engine->issues[i].line_start <= line &&
            engine->issues[i].line_end >= line)
            return engine->issues[i].id;
    }
    return -1;
}

const sa_rule_t *sa_find_rule(const sa_engine_t *engine,
                              const char *rule_id) {
    if (!engine || !rule_id) return NULL;
    for (int i = 0; i < engine->rule_count; i++)
        if (strcmp(engine->rules[i].rule_id, rule_id) == 0)
            return &engine->rules[i];
    return NULL;
}

void sa_disable_rule(sa_engine_t *engine, const char *rule_id) {
    if (!engine || !rule_id) return;
    for (int i = 0; i < engine->rule_count; i++)
        if (strcmp(engine->rules[i].rule_id, rule_id) == 0)
            engine->rules[i].enabled = 0;
}

void sa_enable_rule(sa_engine_t *engine, const char *rule_id) {
    if (!engine || !rule_id) return;
    for (int i = 0; i < engine->rule_count; i++)
        if (strcmp(engine->rules[i].rule_id, rule_id) == 0)
            engine->rules[i].enabled = 1;
}
