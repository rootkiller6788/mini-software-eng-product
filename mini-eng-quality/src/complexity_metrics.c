#include "complexity_metrics.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>

void cm_analysis_init(cm_analysis_t *a) {
    if (!a) return;
    memset(a, 0, sizeof(*a));
}

int cm_calc_cyclomatic(const char *source, int len) {
    int edges = 1, nodes = 1;
    if (!source || len <= 0) return 1;
    for (int i = 0; i < len; i++) {
        switch (source[i]) {
        case '?': case '&':
            if (i + 1 < len && source[i] == '&' && source[i+1] == '&')
                { edges++; nodes++; i++; }
            else if (i + 1 < len && source[i] == '|' && source[i+1] == '|')
                { edges++; nodes++; i++; }
            else edges++;
            break;
        case 'i': if (i + 1 < len && source[i+1] == 'f') edges++; break;
        case 'w': if (i + 4 < len && strncmp(source + i, "while", 5) == 0) edges++; break;
        case 'f': if (i + 2 < len && strncmp(source + i, "for", 3) == 0) edges++; break;
        case 's': if (i + 5 < len && strncmp(source + i, "switch", 6) == 0) edges++; break;
        case 'c':
            if (i + 4 < len && strncmp(source + i, "case", 4) == 0) { edges++; nodes++; }
            else if (i + 4 < len && strncmp(source + i, "catch", 5) == 0) edges++;
            break;
        default: break;
        }
    }
    return edges - nodes + 2;
}

static int is_keyword_start(const char *s, const char *kw, int remaining) {
    size_t kwl = strlen(kw);
    if ((size_t)remaining < kwl) return 0;
    if (strncmp(s, kw, kwl) != 0) return 0;
    if (remaining > (int)kwl && (isalnum(s[kwl]) || s[kwl] == '_')) return 0;
    if (s > (const char *)0 && isalnum(*(s - 1))) return 0;
    return 1;
}

int cm_calc_cognitive(const char *source, int len) {
    int score = 0, nesting = 0;
    if (!source || len <= 0) return 0;
    for (int i = 0; i < len; i++) {
        if (source[i] == '{') nesting++;
        if (source[i] == '}') { if (nesting > 0) nesting--; continue; }
        if (source[i] == 'i' && is_keyword_start(source + i, "if", len - i))
            { score += 1 + nesting; i += 1; }
        else if (source[i] == 'e' && is_keyword_start(source + i, "else", len - i)) {
            if (i + 4 < len && source[i+4] == ' ' && i + 6 < len &&
                strncmp(source + i + 5, "if", 2) == 0)
                { score += nesting; i += 3; }
            else { score += nesting; i += 3; }
        }
        else if (source[i] == 'w' && is_keyword_start(source + i, "while", len - i))
            { score += 1 + nesting; i += 4; }
        else if (source[i] == 'f' && is_keyword_start(source + i, "for", len - i))
            { score += 1 + nesting; i += 2; }
        else if (source[i] == 'd' && is_keyword_start(source + i, "do", len - i))
            { score += 1 + nesting; i += 1; }
        else if (source[i] == 's' && is_keyword_start(source + i, "switch", len - i))
            { score += 1 + nesting; i += 5; }
        else if (source[i] == 'c' && is_keyword_start(source + i, "catch", len - i))
            { score += 1 + nesting; i += 4; }
        else if (source[i] == 'g' && is_keyword_start(source + i, "goto", len - i))
            { score += nesting; i += 3; }
        else if (source[i] == 'b' && is_keyword_start(source + i, "break", len - i)) {
            int nl = 0;
            for (int j = i - 1; j >= 0 && source[j] != '\n'; j--)
                if (source[j] == 's') { nl = 1; break; }
            if (nl) { score += nesting; i += 4; }
        }
        else if (source[i] == '&' && i + 1 < len && source[i+1] == '&')
            { score += nesting; i++; }
        else if (source[i] == '|' && i + 1 < len && source[i+1] == '|')
            { score += nesting; i++; }
    }
    return score;
}

double cm_calc_comment_ratio(int comment_lines, int total_lines) {
    if (total_lines <= 0) return 0.0;
    return (double)comment_lines / (double)total_lines * 100.0;
}

int cm_count_loc(const char *source, int len, int *comments, int *blanks) {
    int total = 0, com = 0, blk = 0, in_block = 0;
    if (!source) return 0;
    for (int i = 0; i < len; i++) {
        if (in_block) {
            if (source[i] == '*' && i + 1 < len && source[i+1] == '/')
                { in_block = 0; i++; }
            continue;
        }
        if (source[i] == '/' && i + 1 < len) {
            if (source[i+1] == '/') { com++; while (i < len && source[i] != '\n') i++; continue; }
            if (source[i+1] == '*') { com++; in_block = 1; i++; continue; }
        }
        if (source[i] == '\n') {
            total++;
            if (i > 0) {
                int blank_line = 1;
                int j = i - 1;
                while (j >= 0 && source[j] != '\n') {
                    if (!isspace(source[j])) { blank_line = 0; break; }
                    j--;
                }
                if (blank_line) blk++;
            }
        }
    }
    if (comments) *comments = com;
    if (blanks) *blanks = blk;
    return total > 0 ? total : 1;
}

double cm_calc_halstead_volume(int operators, int operands,
                               int unique_operators, int unique_operands) {
    int N = operators + operands;
    int n = unique_operators + unique_operands;
    if (n <= 0 || N <= 0) return 0.0;
    return (double)N * log2((double)n);
}

double cm_calc_maintainability(double hv, double cc, double loc,
                               double comment_ratio) {
    double mi = 171.0 - 5.2 * log(fmax(hv, 1.0)) -
                0.23 * cc - 16.2 * log(fmax(loc, 1.0));
    mi += comment_ratio * 0.1;
    if (mi > 100.0) mi = 100.0;
    if (mi < 0.0) mi = 0.0;
    return mi;
}

void cm_assess_risk_function(cm_function_metrics_t *fm) {
    if (!fm) return;
    if (fm->cyclomatic_complexity > 50 || fm->cognitive_complexity > 30)
        fm->risk = CM_SEVERITY_CRITICAL;
    else if (fm->cyclomatic_complexity > 20 || fm->cognitive_complexity > 15)
        fm->risk = CM_SEVERITY_HIGH;
    else if (fm->cyclomatic_complexity > 10 || fm->cognitive_complexity > 8)
        fm->risk = CM_SEVERITY_MEDIUM;
    else
        fm->risk = CM_SEVERITY_LOW;
}

void cm_assess_risk_file(cm_file_metrics_t *fm) {
    if (!fm) return;
    if (fm->avg_cyclomatic > 20.0 || fm->maintainability_index < 20.0)
        fm->risk = CM_SEVERITY_CRITICAL;
    else if (fm->avg_cyclomatic > 10.0 || fm->maintainability_index < 40.0)
        fm->risk = CM_SEVERITY_HIGH;
    else if (fm->avg_cyclomatic > 5.0 || fm->maintainability_index < 65.0)
        fm->risk = CM_SEVERITY_MEDIUM;
    else
        fm->risk = CM_SEVERITY_LOW;
}

int cm_analyze_function(cm_analysis_t *a, const char *name, const char *file,
                        int start, int end, const char *source,
                        int source_len) {
    if (!a || a->function_count >= CM_MAX_FUNCTIONS) return -1;
    cm_function_metrics_t *fm = &a->functions[a->function_count];
    memset(fm, 0, sizeof(*fm));
    snprintf(fm->name, CM_MAX_NAME_LEN, "%s", name);
    snprintf(fm->file, CM_MAX_FILE_PATH, "%s", file);
    fm->start_line = start;
    fm->end_line = end;
    fm->loc = end - start + 1;
    int comments = 0, blanks = 0;
    fm->loc = cm_count_loc(source, source_len, &comments, &blanks);
    fm->comment_lines = comments;
    fm->blank_lines = blanks;
    fm->comment_ratio = cm_calc_comment_ratio(comments, fm->loc);
    fm->cyclomatic_complexity = cm_calc_cyclomatic(source, source_len);
    fm->cognitive_complexity = cm_calc_cognitive(source, source_len);
    fm->nesting_depth = cm_nesting_depth(source, source_len);
    fm->return_points = 0;
    cm_assess_risk_function(fm);
    a->function_count++;
    return 0;
}

int cm_analyze_file(cm_analysis_t *a, const char *path,
                    const char *source, int source_len) {
    if (!a || a->file_count >= CM_MAX_FILES) return -1;
    cm_file_metrics_t *fm = &a->files[a->file_count];
    memset(fm, 0, sizeof(*fm));
    snprintf(fm->path, CM_MAX_FILE_PATH, "%s", path);
    int comments = 0, blanks = 0;
    fm->total_loc = cm_count_loc(source, source_len, &comments, &blanks);
    fm->comment_lines = comments;
    fm->blank_lines = blanks;
    fm->code_lines = fm->total_loc - comments - blanks;
    fm->comment_ratio = cm_calc_comment_ratio(comments, fm->total_loc);
    fm->function_count = a->function_count;
    double sum_cc = 0.0, sum_cog = 0.0;
    for (int i = 0; i < a->function_count; i++) {
        sum_cc += (double)a->functions[i].cyclomatic_complexity;
        sum_cog += (double)a->functions[i].cognitive_complexity;
    }
    if (fm->function_count > 0) {
        fm->avg_cyclomatic = sum_cc / (double)fm->function_count;
        fm->avg_cognitive = sum_cog / (double)fm->function_count;
    }
    double hv = cm_calc_halstead_volume((int)fm->total_loc, 0, 0, 0);
    if (hv < 1.0) hv = fm->total_loc * 3.0;
    fm->maintainability_index =
        cm_calc_maintainability(hv, fm->avg_cyclomatic,
                                fm->total_loc, fm->comment_ratio);
    cm_assess_risk_file(fm);
    a->file_count++;
    return 0;
}

void cm_detect_duplication(const cm_analysis_t *a, cm_file_metrics_t *fm) {
    (void)a;
    if (!fm) return;
    fm->duplication_blocks = 0;
    fm->duplication_density = 0.0;
}

void cm_print_function_metrics(const cm_function_metrics_t *fm) {
    if (!fm) return;
    printf("  Function: %s (%s:%d-%d)\n",
           fm->name, fm->file, fm->start_line, fm->end_line);
    printf("    LOC=%d  CC=%d  Cognitive=%d  Nested=%d\n",
           fm->loc, fm->cyclomatic_complexity,
           fm->cognitive_complexity, fm->nesting_depth);
    printf("    Comment=%.1f%%  Risk=%s\n", fm->comment_ratio,
           fm->risk == CM_SEVERITY_CRITICAL ? "CRITICAL" :
           fm->risk == CM_SEVERITY_HIGH ? "HIGH" :
           fm->risk == CM_SEVERITY_MEDIUM ? "MEDIUM" : "LOW");
}

void cm_print_file_metrics(const cm_file_metrics_t *fm) {
    if (!fm) return;
    printf("  File: %s\n", fm->path);
    printf("    LOC=%d  Comments=%.1f%%  MI=%.1f\n",
           fm->total_loc, fm->comment_ratio, fm->maintainability_index);
    printf("    Avg CC=%.1f  Avg Cog=%.1f  Funcs=%d\n",
           fm->avg_cyclomatic, fm->avg_cognitive, fm->function_count);
    printf("    Risk=%s\n",
           fm->risk == CM_SEVERITY_CRITICAL ? "CRITICAL" :
           fm->risk == CM_SEVERITY_HIGH ? "HIGH" :
           fm->risk == CM_SEVERITY_MEDIUM ? "MEDIUM" : "LOW");
}

int cm_count_branches(const char *source, int len) {
    return cm_calc_cyclomatic(source, len) - 1;
}

int cm_nesting_depth(const char *source, int len) {
    int depth = 0, max_depth = 0;
    if (!source) return 0;
    for (int i = 0; i < len; i++) {
        if (source[i] == '{') {
            depth++;
            if (depth > max_depth) max_depth = depth;
        }
        if (source[i] == '}' && depth > 0) depth--;
    }
    return max_depth;
}

void cm_cognitive_profile(const char *source, int len,
                          cm_cognitive_profile_t *profile) {
    if (!profile) return;
    memset(profile, 0, sizeof(*profile));
    profile->nesting_depth = cm_nesting_depth(source, len);
    profile->functional_branches = cm_count_branches(source, len);
    if (source && len > 0) profile->structural_elements = len / 20;
}

void cm_hotspot_init(cm_hotspot_analysis_t *ha) {
    if (!ha) return;
    memset(ha, 0, sizeof(*ha));
}

int cm_add_hotspot(cm_hotspot_analysis_t *ha, const char *file,
                   int start, int end, int lines, int changes) {
    if (!ha || ha->count >= CM_MAX_FILES) return -1;
    cm_hotspot_t *hs = &ha->hotspots[ha->count];
    memset(hs, 0, sizeof(*hs));
    snprintf(hs->file, CM_MAX_FILE_PATH, "%s", file);
    hs->start_line = start;
    hs->end_line = end;
    hs->line_count = lines;
    hs->change_freq = changes;
    hs->freq_score = (double)changes * log((double)lines + 1.0);
    if (hs->freq_score > 50.0) hs->risk = CM_SEVERITY_CRITICAL;
    else if (hs->freq_score > 30.0) hs->risk = CM_SEVERITY_HIGH;
    else if (hs->freq_score > 10.0) hs->risk = CM_SEVERITY_MEDIUM;
    else hs->risk = CM_SEVERITY_LOW;
    ha->count++;
    return 0;
}

static int cmp_hotspot_desc(const void *a, const void *b) {
    double sa = ((const cm_hotspot_t *)a)->freq_score;
    double sb = ((const cm_hotspot_t *)b)->freq_score;
    if (sa < sb) return 1;
    if (sa > sb) return -1;
    return 0;
}

void cm_sort_hotspots(cm_hotspot_analysis_t *ha) {
    if (!ha) return;
    qsort(ha->hotspots, ha->count, sizeof(cm_hotspot_t), cmp_hotspot_desc);
}

void cm_print_hotspots(const cm_hotspot_analysis_t *ha) {
    if (!ha) return;
    printf("=== Hotspot Analysis ===\n");
    for (int i = 0; i < ha->count; i++) {
        printf("  %s:%d-%d  lines=%d  changes=%d  score=%.1f  risk=%s\n",
               ha->hotspots[i].file, ha->hotspots[i].start_line,
               ha->hotspots[i].end_line, ha->hotspots[i].line_count,
               ha->hotspots[i].change_freq, ha->hotspots[i].freq_score,
               ha->hotspots[i].risk == CM_SEVERITY_CRITICAL ? "CRITICAL" :
               ha->hotspots[i].risk == CM_SEVERITY_HIGH ? "HIGH" :
               ha->hotspots[i].risk == CM_SEVERITY_MEDIUM ? "MEDIUM" : "LOW");
    }
}
