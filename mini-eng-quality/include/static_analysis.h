#ifndef STATIC_ANALYSIS_H
#define STATIC_ANALYSIS_H

#include <stddef.h>
#include <stdint.h>

#define SA_MAX_RULES         256
#define SA_MAX_ISSUES        1024
#define SA_MAX_TAGS          16
#define SA_MAX_RULE_ID_LEN   64
#define SA_MAX_MSG_LEN       512
#define SA_MAX_FILE_PATH     256
#define SA_MAX_LINE_LEN      4096

typedef enum {
    SA_SEVERITY_INFO,
    SA_SEVERITY_MINOR,
    SA_SEVERITY_MAJOR,
    SA_SEVERITY_CRITICAL,
    SA_SEVERITY_BLOCKER
} sa_severity_e;

typedef enum {
    SA_CATEGORY_BUG,
    SA_CATEGORY_VULNERABILITY,
    SA_CATEGORY_CODE_SMELL,
    SA_CATEGORY_PERFORMANCE,
    SA_CATEGORY_SECURITY_HOTSPOT
} sa_category_e;

typedef enum {
    SA_STATUS_OPEN,
    SA_STATUS_CONFIRMED,
    SA_STATUS_FALSE_POSITIVE,
    SA_STATUS_WONT_FIX,
    SA_STATUS_FIXED
} sa_issue_status_e;

typedef enum {
    SA_QUALITY_PASS,
    SA_QUALITY_FAIL,
    SA_QUALITY_WARN
} sa_quality_result_e;

typedef struct {
    char rule_id[SA_MAX_RULE_ID_LEN];
    sa_severity_e severity;
    sa_category_e category;
    char description[SA_MAX_MSG_LEN];
    int  remediation_minutes;
    char tags[SA_MAX_TAGS][64];
    int  tag_count;
    int  enabled;
} sa_rule_t;

typedef struct {
    int  id;
    char rule_id[SA_MAX_RULE_ID_LEN];
    char file_path[SA_MAX_FILE_PATH];
    int  line_start;
    int  line_end;
    int  column_start;
    int  column_end;
    sa_severity_e severity;
    sa_category_e category;
    char message[SA_MAX_MSG_LEN];
    sa_issue_status_e status;
    int  effort_minutes;
    char assignee[64];
} sa_issue_t;

typedef struct {
    double blocker_threshold;
    double critical_threshold;
    double major_threshold;
    double minor_threshold;
    int    max_new_blockers;
    int    max_new_criticals;
    int    max_total_issues;
    double min_remediation_ratio;
} sa_quality_gate_t;

typedef struct {
    int  total_issues;
    int  blockers;
    int  criticals;
    int  majors;
    int  minors;
    int  bugs;
    int  vulnerabilities;
    int  code_smells;
    double total_remediation_hours;
    double new_tech_debt_hours;
    double technical_debt_ratio;
    sa_quality_result_e gate_result;
    double lines_of_code;
    int  files_analyzed;
} sa_report_t;

typedef struct {
    sa_rule_t  rules[SA_MAX_RULES];
    int        rule_count;
    sa_issue_t issues[SA_MAX_ISSUES];
    int        issue_count;
    int        next_issue_id;
} sa_engine_t;

void  sa_engine_init(sa_engine_t *engine);
int   sa_register_rule(sa_engine_t *engine, const char *rule_id,
                       sa_severity_e severity, sa_category_e category,
                       const char *description, int remediation_min);
int   sa_register_rule_with_tags(sa_engine_t *engine, const char *rule_id,
                                 sa_severity_e severity, sa_category_e category,
                                 const char *description, int remediation_min,
                                 const char *tags[], int tag_count);
int   sa_report_issue(sa_engine_t *engine, const char *rule_id,
                      const char *file_path, int line_start, int line_end,
                      const char *message);
int   sa_report_issue_detailed(sa_engine_t *engine, const char *rule_id,
                               const char *file_path, int line_start,
                               int line_end, int col_start, int col_end,
                               const char *message);
void  sa_update_issue_status(sa_engine_t *engine, int issue_id,
                             sa_issue_status_e status);
void  sa_assign_issue(sa_engine_t *engine, int issue_id,
                      const char *assignee);
int   sa_count_by_severity(const sa_engine_t *engine, sa_severity_e sev);
int   sa_count_by_category(const sa_engine_t *engine, sa_category_e cat);
void  sa_quality_gate_default(sa_quality_gate_t *gate);
void  sa_quality_gate_set(sa_quality_gate_t *gate,
                          double blocker_thresh, double crit_thresh,
                          double major_thresh, double minor_thresh);
double sa_calc_tech_debt_ratio(double remediation_hours,
                               double dev_cost_hours);
double sa_estimate_dev_cost(double loc, double cost_per_loc_hours);
double sa_calc_remediation_hours(const sa_engine_t *engine);
void  sa_generate_report(const sa_engine_t *engine,
                         sa_report_t *report,
                         double loc);
sa_quality_result_e sa_evaluate_quality_gate(const sa_report_t *report,
                                             const sa_quality_gate_t *gate);
void  sa_print_report(const sa_report_t *report);
int   sa_find_issue(const sa_engine_t *engine, const char *rule_id,
                    const char *file_path, int line);
const sa_rule_t *sa_find_rule(const sa_engine_t *engine,
                              const char *rule_id);
void  sa_disable_rule(sa_engine_t *engine, const char *rule_id);
void  sa_enable_rule(sa_engine_t *engine, const char *rule_id);

#endif
