#ifndef LINT_FORMATTER_H
#define LINT_FORMATTER_H

#include <stddef.h>
#include <stdint.h>

#define LF_MAX_RULES          256
#define LF_MAX_FIXES          128
#define LF_MAX_VIOLATIONS     1024
#define LF_MAX_FILE_PATH      256
#define LF_MAX_RULE_ID        64
#define LF_MAX_MSG_LEN        512
#define LF_MAX_LINE_LEN       4096

typedef enum {
    LF_CATEGORY_NAMING,
    LF_CATEGORY_STYLE,
    LF_CATEGORY_SECURITY,
    LF_CATEGORY_PERFORMANCE,
    LF_CATEGORY_BEST_PRACTICE,
    LF_CATEGORY_MEMORY,
    LF_CATEGORY_CONCURRENCY
} lf_category_e;

typedef enum {
    LF_SEVERITY_OFF,
    LF_SEVERITY_WARN,
    LF_SEVERITY_ERROR
} lf_severity_e;

typedef enum {
    LF_BRACE_ATTACH,
    LF_BRACE_BREAK,
    LF_BRACE_LINUX,
    LF_BRACE_STROUSTRUP,
    LF_BRACE_ALLMAN,
    LF_BRACE_GNU
} lf_brace_style_e;

typedef enum {
    LF_INDENT_TABS,
    LF_INDENT_SPACES
} lf_indent_style_e;

typedef enum {
    LF_FIX_NONE,
    LF_FIX_SUGGESTION,
    LF_FIX_AUTO,
    LF_FIX_MANUAL
} lf_fix_type_e;

typedef struct {
    char rule_id[LF_MAX_RULE_ID];
    lf_category_e category;
    lf_severity_e severity;
    char description[LF_MAX_MSG_LEN];
    int  enabled;
} lf_rule_t;

typedef struct {
    int  id;
    char rule_id[LF_MAX_RULE_ID];
    char file_path[LF_MAX_FILE_PATH];
    int  line;
    int  column;
    char message[LF_MAX_MSG_LEN];
    lf_severity_e severity;
    lf_fix_type_e fix_type;
    char fix_description[LF_MAX_MSG_LEN];
    char fix_snippet[LF_MAX_LINE_LEN];
} lf_violation_t;

typedef struct {
    int  indent_width;
    lf_indent_style_e indent_style;
    int  tab_width;
    int  max_line_length;
    lf_brace_style_e brace_style;
    int  space_inside_parens;
    int  space_after_comma;
    int  space_before_colon;
    int  align_consecutive_assignments;
    int  align_consecutive_declarations;
    int  break_before_braces;
    int  column_limit;
    int  allow_short_functions_on_single_line;
    int  allow_short_if_on_single_line;
    int  allow_short_loops_on_single_line;
    int  pointer_alignment_right;
    int  include_sort;
    int  sort_using_declarations;
} lf_format_config_t;

typedef struct {
    lf_rule_t rules[LF_MAX_RULES];
    int  rule_count;
    lf_format_config_t format_config;
    int  total_violations;
    int  auto_fixed;
} lf_engine_t;

void  lf_engine_init(lf_engine_t *engine);
int   lf_register_rule(lf_engine_t *engine, const char *rule_id,
                       lf_category_e category, lf_severity_e severity,
                       const char *description);
void  lf_enable_rule(lf_engine_t *engine, const char *rule_id);
void  lf_disable_rule(lf_engine_t *engine, const char *rule_id);
void  lf_set_severity(lf_engine_t *engine, const char *rule_id,
                      lf_severity_e severity);

int   lf_check_naming_convention(const char *identifier, const char *pattern);
int   lf_check_line_length(const char *line, int max_len);
int   lf_check_trailing_whitespace(const char *line);
int   lf_check_tabs_vs_spaces(const char *line, int use_spaces, int tab_width);
int   lf_check_brace_style(const char *line, lf_brace_style_e style);
int   lf_check_includes_sorted(const char **includes, int count);
int   lf_check_undefined_behavior(const char *line);
int   lf_check_malloc_sizeof(const char *line);
int   lf_check_gets_usage(const char *line);
int   lf_check_strcpy_usage(const char *line);
int   lf_check_sprintf_usage(const char *line);
int   lf_check_null_deref(const char *line);
int   lf_check_array_bounds(const char *line);

int   lf_lint_file(lf_engine_t *engine, const char *file_path,
                   const char *source, lf_violation_t *violations,
                   int max_violations);
int   lf_lint_line(lf_engine_t *engine, const char *file_path,
                   int line_num, const char *line_text);
const char *lf_suggest_fix(const lf_violation_t *v);
int   lf_auto_fix(lf_engine_t *engine, const char *file_path,
                  const char *source, char *fixed, int max_len);
void  lf_format_config_default(lf_format_config_t *cfg);
void  lf_format_config_llvm(lf_format_config_t *cfg);
void  lf_format_config_google(lf_format_config_t *cfg);
void  lf_format_config_webkit(lf_format_config_t *cfg);
int   lf_format_source(lf_engine_t *engine, const char *source,
                       char *formatted, int max_len);
int   lf_format_braces(const char *source, char *formatted, int max_len,
                       lf_brace_style_e style);
int   lf_format_indentation(const char *source, char *formatted, int max_len,
                            int indent_width, lf_indent_style_e style);
int   lf_format_spacing(const char *source, char *formatted, int max_len,
                        int space_after_comma, int space_inside_parens);
int   lf_pre_commit_hook(lf_engine_t *engine, const char **files,
                         int file_count, int reject_on_error);
int   lf_ci_check(lf_engine_t *engine, const char **files,
                  int file_count, int *exit_code);
void  lf_print_violation(const lf_violation_t *v);
void  lf_print_summary(const lf_engine_t *engine);
int   lf_rule_count_active(const lf_engine_t *engine);

#endif
