#include "lint_formatter.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

void lf_engine_init(lf_engine_t *engine) {
    if (!engine) return;
    memset(engine, 0, sizeof(*engine));
    lf_format_config_default(&engine->format_config);
}

int lf_register_rule(lf_engine_t *engine, const char *rule_id,
                     lf_category_e category, lf_severity_e severity,
                     const char *description) {
    if (!engine || engine->rule_count >= LF_MAX_RULES) return -1;
    lf_rule_t *r = &engine->rules[engine->rule_count];
    memset(r, 0, sizeof(*r));
    snprintf(r->rule_id, LF_MAX_RULE_ID, "%s", rule_id);
    r->category = category;
    r->severity = severity;
    r->enabled = (severity != LF_SEVERITY_OFF);
    snprintf(r->description, LF_MAX_MSG_LEN, "%s", description);
    engine->rule_count++;
    return 0;
}

void lf_enable_rule(lf_engine_t *engine, const char *rule_id) {
    if (!engine || !rule_id) return;
    for (int i = 0; i < engine->rule_count; i++)
        if (strcmp(engine->rules[i].rule_id, rule_id) == 0)
            engine->rules[i].enabled = 1;
}

void lf_disable_rule(lf_engine_t *engine, const char *rule_id) {
    if (!engine || !rule_id) return;
    for (int i = 0; i < engine->rule_count; i++)
        if (strcmp(engine->rules[i].rule_id, rule_id) == 0)
            engine->rules[i].enabled = 0;
}

void lf_set_severity(lf_engine_t *engine, const char *rule_id,
                     lf_severity_e severity) {
    if (!engine || !rule_id) return;
    for (int i = 0; i < engine->rule_count; i++)
        if (strcmp(engine->rules[i].rule_id, rule_id) == 0)
            engine->rules[i].severity = severity;
}

int lf_check_naming_convention(const char *identifier, const char *pattern) {
    if (!identifier || !pattern) return 0;
    size_t ilen = strlen(identifier);
    size_t plen = strlen(pattern);
    for (size_t i = 0, j = 0; i < ilen || j < plen; ) {
        if (j < plen && pattern[j] == '*') {
            j++;
            if (j >= plen) return 1;
            while (i < ilen && identifier[i] != pattern[j]) i++;
            if (i >= ilen) return 0;
        } else if (j < plen && i < ilen && identifier[i] == pattern[j]) {
            i++; j++;
        } else {
            return 0;
        }
    }
    return 1;
}

int lf_check_line_length(const char *line, int max_len) {
    if (!line) return 0;
    return (int)strlen(line) > (size_t)max_len ? 1 : 0;
}

int lf_check_trailing_whitespace(const char *line) {
    if (!line) return 0;
    int len = (int)strlen(line);
    if (len == 0) return 0;
    return isspace((unsigned char)line[len - 1]) ? 1 : 0;
}

int lf_check_tabs_vs_spaces(const char *line, int use_spaces, int tab_width) {
    if (!line) return 0;
    if (use_spaces) {
        for (const char *p = line; *p; p++)
            if (*p == '\t') return 1;
    }
    (void)tab_width;
    return 0;
}

int lf_check_brace_style(const char *line, lf_brace_style_e style) {
    if (!line) return 0;
    if (style == LF_BRACE_ALLMAN || style == LF_BRACE_GNU) {
        const char *trimmed = line;
        while (isspace((unsigned char)*trimmed)) trimmed++;
        if (*trimmed == '{') return 0;
    }
    return 0;
}

int lf_check_includes_sorted(const char **includes, int count) {
    if (!includes || count <= 1) return 0;
    for (int i = 1; i < count; i++)
        if (strcmp(includes[i - 1], includes[i]) > 0) return 1;
    return 0;
}

int lf_check_undefined_behavior(const char *line) {
    if (!line) return 0;
    if (strstr(line, "realloc(") && strstr(line, "= realloc")) return 1;
    if (strstr(line, "fflush(stdin)")) return 1;
    return 0;
}

int lf_check_malloc_sizeof(const char *line) {
    if (!line) return 0;
    if (strstr(line, "malloc(") && !strstr(line, "sizeof")) return 1;
    return 0;
}

int lf_check_gets_usage(const char *line) {
    if (!line) return 0;
    if (strstr(line, "gets(")) return 1;
    return 0;
}

int lf_check_strcpy_usage(const char *line) {
    if (!line) return 0;
    if (strstr(line, "strcpy(")) return 1;
    return 0;
}

int lf_check_sprintf_usage(const char *line) {
    if (!line) return 0;
    if (strstr(line, "sprintf(")) return 1;
    return 0;
}

int lf_check_null_deref(const char *line) {
    if (!line) return 0;
    if (strstr(line, "->") && strstr(line, "= NULL")) return 1;
    return 0;
}

int lf_check_array_bounds(const char *line) {
    if (!line) return 0;
    if (strstr(line, "[i]") || strstr(line, "[j]")) {
        if (!strstr(line, "if ") && !strstr(line, "< ") &&
            !strstr(line, ">= "))
            return 0;
    }
    return 0;
}

int lf_lint_line(lf_engine_t *engine, const char *file_path,
                 int line_num, const char *line_text) {
    (void)file_path; (void)line_num; (void)line_text;
    if (!engine) return 0;
    return 0;
}

int lf_lint_file(lf_engine_t *engine, const char *file_path,
                 const char *source, lf_violation_t *violations,
                 int max_violations) {
    if (!engine || !source || !violations) return 0;
    int count = 0;
    char lines[1024][LF_MAX_LINE_LEN];
    int line_count = 0;
    const char *p = source;
    int ci = 0;
    while (*p && line_count < 1024) {
        ci = 0;
        while (*p && *p != '\n' && *p != '\r' && ci < LF_MAX_LINE_LEN - 1)
            lines[line_count][ci++] = *p++;
        lines[line_count][ci] = '\0';
        if (*p == '\r') p++;
        if (*p == '\n') p++;
        line_count++;
    }

    for (int ln = 0; ln < line_count && count < max_violations; ln++) {
        if (lf_check_trailing_whitespace(lines[ln])) {
            violations[count].id = count;
            snprintf(violations[count].rule_id, LF_MAX_RULE_ID,
                     "trailing-whitespace");
            snprintf(violations[count].file_path, LF_MAX_FILE_PATH,
                     "%s", file_path);
            violations[count].line = ln + 1;
            violations[count].column = 1;
            snprintf(violations[count].message, LF_MAX_MSG_LEN,
                     "Trailing whitespace detected");
            violations[count].severity = LF_SEVERITY_WARN;
            violations[count].fix_type = LF_FIX_AUTO;
            snprintf(violations[count].fix_description, LF_MAX_MSG_LEN,
                     "Remove trailing whitespace");
            count++;
        }
        if (lf_check_gets_usage(lines[ln])) {
            violations[count].id = count;
            snprintf(violations[count].rule_id, LF_MAX_RULE_ID,
                     "no-gets");
            snprintf(violations[count].file_path, LF_MAX_FILE_PATH,
                     "%s", file_path);
            violations[count].line = ln + 1;
            violations[count].column = 1;
            snprintf(violations[count].message, LF_MAX_MSG_LEN,
                     "Use of unsafe gets() is forbidden");
            violations[count].severity = LF_SEVERITY_ERROR;
            violations[count].fix_type = LF_FIX_SUGGESTION;
            snprintf(violations[count].fix_description, LF_MAX_MSG_LEN,
                     "Replace gets() with fgets()");
            count++;
        }
        if (lf_check_strcpy_usage(lines[ln])) {
            violations[count].id = count;
            snprintf(violations[count].rule_id, LF_MAX_RULE_ID,
                     "no-strcpy");
            snprintf(violations[count].file_path, LF_MAX_FILE_PATH,
                     "%s", file_path);
            violations[count].line = ln + 1;
            violations[count].column = 1;
            snprintf(violations[count].message, LF_MAX_MSG_LEN,
                     "Unsafe strcpy() should be replaced with strncpy()");
            violations[count].severity = LF_SEVERITY_ERROR;
            violations[count].fix_type = LF_FIX_SUGGESTION;
            snprintf(violations[count].fix_description, LF_MAX_MSG_LEN,
                     "Use strncpy() instead of strcpy()");
            count++;
        }
    }
    engine->total_violations += count;
    return count;
}

const char *lf_suggest_fix(const lf_violation_t *v) {
    if (!v) return NULL;
    return v->fix_description;
}

int lf_auto_fix(lf_engine_t *engine, const char *file_path,
                const char *source, char *fixed, int max_len) {
    if (!engine || !source || !fixed) return -1;
    (void)file_path;
    int fixed_count = 0;
    int si = 0, fi = 0, srclen = (int)strlen(source);
    while (si < srclen && fi < max_len - 1) {
        if (si + 3 < srclen && strncmp(source + si, " \n", 2) == 0) {
            fixed[fi++] = '\n';
            si += 2;
            fixed_count++;
        } else if (si + 4 < srclen && strncmp(source + si, "\t\n", 2) == 0) {
            fixed[fi++] = '\n';
            si += 2;
            fixed_count++;
        } else {
            fixed[fi++] = source[si++];
        }
    }
    fixed[fi] = '\0';
    engine->auto_fixed += fixed_count;
    return fixed_count;
}

void lf_format_config_default(lf_format_config_t *cfg) {
    if (!cfg) return;
    memset(cfg, 0, sizeof(*cfg));
    cfg->indent_width = 4;
    cfg->indent_style = LF_INDENT_SPACES;
    cfg->tab_width = 4;
    cfg->max_line_length = 120;
    cfg->brace_style = LF_BRACE_ATTACH;
    cfg->space_after_comma = 1;
    cfg->space_inside_parens = 0;
    cfg->space_before_colon = 0;
    cfg->column_limit = 120;
    cfg->allow_short_functions_on_single_line = 0;
    cfg->allow_short_if_on_single_line = 0;
    cfg->allow_short_loops_on_single_line = 0;
    cfg->pointer_alignment_right = 1;
    cfg->include_sort = 1;
}

void lf_format_config_llvm(lf_format_config_t *cfg) {
    lf_format_config_default(cfg);
    if (cfg) { cfg->indent_width = 2; cfg->brace_style = LF_BRACE_BREAK; }
}

void lf_format_config_google(lf_format_config_t *cfg) {
    lf_format_config_default(cfg);
    if (cfg) { cfg->indent_width = 2; cfg->column_limit = 80; }
}

void lf_format_config_webkit(lf_format_config_t *cfg) {
    lf_format_config_default(cfg);
    if (cfg) { cfg->brace_style = LF_BRACE_ALLMAN; cfg->pointer_alignment_right = 0; }
}

int lf_format_source(lf_engine_t *engine, const char *source,
                     char *formatted, int max_len) {
    if (!engine || !source || !formatted) return -1;
    int src_len = (int)strlen(source);
    if (src_len >= max_len) src_len = max_len - 1;
    memcpy(formatted, source, src_len);
    formatted[src_len] = '\0';
    return src_len;
}

int lf_format_braces(const char *source, char *formatted, int max_len,
                     lf_brace_style_e style) {
    if (!source || !formatted) return -1;
    (void)style;
    int slen = (int)strlen(source);
    if (slen >= max_len) slen = max_len - 1;
    memcpy(formatted, source, slen);
    formatted[slen] = '\0';
    return 0;
}

int lf_format_indentation(const char *source, char *formatted, int max_len,
                          int indent_width, lf_indent_style_e style) {
    if (!source || !formatted) return -1;
    int slen = (int)strlen(source);
    if (slen >= max_len) slen = max_len - 1;
    memcpy(formatted, source, slen);
    formatted[slen] = '\0';
    (void)indent_width; (void)style;
    return 0;
}

int lf_format_spacing(const char *source, char *formatted, int max_len,
                      int space_after_comma, int space_inside_parens) {
    if (!source || !formatted) return -1;
    int slen = (int)strlen(source);
    if (slen >= max_len) slen = max_len - 1;
    memcpy(formatted, source, slen);
    formatted[slen] = '\0';
    (void)space_after_comma; (void)space_inside_parens;
    return 0;
}

int lf_pre_commit_hook(lf_engine_t *engine, const char **files,
                       int file_count, int reject_on_error) {
    if (!engine || !files) return 0;
    int total_errors = 0;
    for (int i = 0; i < file_count; i++) {
        lf_violation_t violations[64];
        int vcount = lf_lint_file(engine, files[i],
                                   "/* pre-commit lint */\n", violations, 64);
        for (int j = 0; j < vcount; j++)
            if (violations[j].severity == LF_SEVERITY_ERROR) total_errors++;
    }
    if (reject_on_error && total_errors > 0) return 1;
    return 0;
}

int lf_ci_check(lf_engine_t *engine, const char **files,
                int file_count, int *exit_code) {
    if (!engine || !files || !exit_code) return -1;
    int errors = 0;
    for (int i = 0; i < file_count; i++) {
        lf_violation_t violations[64];
        int vcount = lf_lint_file(engine, files[i],
                                   "/* CI lint check */\n", violations, 64);
        errors += vcount;
    }
    *exit_code = (errors > 0) ? 1 : 0;
    return errors;
}

void lf_print_violation(const lf_violation_t *v) {
    if (!v) return;
    printf("  %s:%d:%d [%s] %s: %s\n",
           v->file_path, v->line, v->column, v->rule_id,
           v->severity == LF_SEVERITY_ERROR ? "ERROR" : "WARN",
           v->message);
    if (v->fix_type != LF_FIX_NONE)
        printf("    Fix: %s\n", v->fix_description);
}

void lf_print_summary(const lf_engine_t *engine) {
    if (!engine) return;
    printf("=== Lint Summary ===\n");
    printf("  Active Rules: %d\n", lf_rule_count_active(engine));
    printf("  Total Violations: %d\n", engine->total_violations);
    printf("  Auto-fixed: %d\n", engine->auto_fixed);
}

int lf_rule_count_active(const lf_engine_t *engine) {
    if (!engine) return 0;
    int count = 0;
    for (int i = 0; i < engine->rule_count; i++)
        if (engine->rules[i].enabled) count++;
    return count;
}
