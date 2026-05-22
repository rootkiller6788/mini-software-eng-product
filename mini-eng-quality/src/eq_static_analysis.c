#include "eq_static_analysis.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>

void static_count_lines(const char *source, FileStats *fs) {
    memset(fs, 0, sizeof(*fs));
    const char *s = source;
    while (*s) {
        const char *start = s;
        while (*s && *s != '\n') s++;
        fs->total_lines++;
        const char *ns = start; while (*ns == ' ' || *ns == '\t') ns++;
        if (*ns == '\0' || *ns == '\n') fs->blank_lines++;
        else if (*ns == '/' && *(ns+1) == '/') fs->comment_lines++;
        else if (*ns == '/' && *(ns+1) == '*') { fs->comment_lines++; while (*s && !(*s=='*'&&*(s+1)=='/')) s++; if (*s) s++; }
        else fs->code_lines++;
        if (*s) s++;
    }
    fs->comment_ratio = fs->total_lines > 0 ? (double)fs->comment_lines / fs->total_lines : 0;
}

int static_nesting_depth(const char *source, int *max_d, int *avg_d) {
    int depth = 0, max_depth = 0, total = 0, samples = 0;
    for (const char *s = source; *s; s++) {
        if (*s == '{') { depth++; if (depth > max_depth) max_depth = depth; total += depth; samples++; }
        else if (*s == '}') depth--;
    }
    *max_d = max_depth; *avg_d = samples > 0 ? total / samples : 0;
    return max_depth;
}

int static_detect_duplication(const char *source, int min_block_lines) {
    (void)min_block_lines;
    int lines[1024], lc = 0, dups = 0;
    const char *s = source; lines[lc++] = 0;
    while (*s) { if (*s == '\n') lines[lc++] = (int)(s - source); s++; if (lc >= 1024) break; }
    for (int i = 0; i < lc - 3; i++) {
        for (int j = i + 1; j < lc - 3; j++) {
            int match = 0;
            while (i+match < lc && j+match < lc && match < 10 && source[lines[i]+match] == source[lines[j]+match]) match++;
            if (match >= 5) { dups++; break; }
        }
    }
    return dups;
}

void static_analyze(const char *source_code, StaticAnalysisResult *result) {
    memset(result, 0, sizeof(*result));
    static_count_lines(source_code, &result->file_stats);
    static_nesting_depth(source_code, &result->max_nesting_depth, &result->avg_nesting_depth);
    result->duplicated_blocks = static_detect_duplication(source_code, 5);
    result->duplication_pct = (result->file_stats.code_lines > 0 && result->duplicated_blocks > 0) ? (result->duplicated_blocks * 5 * 100) / result->file_stats.code_lines : 0;
}

bool static_is_too_complex(StaticAnalysisResult *r) { return r->max_nesting_depth > 4 || r->file_stats.code_lines > 500; }

void static_print(StaticAnalysisResult *r) {
    printf("=== Static Analysis ===\n");
    printf("  Lines: %d total (%d code, %d comments, %d blank)\n", r->file_stats.total_lines, r->file_stats.code_lines, r->file_stats.comment_lines, r->file_stats.blank_lines);
    printf("  Nesting: max=%d avg=%d\n", r->max_nesting_depth, r->avg_nesting_depth);
    printf("  Duplication: %d%% (%d blocks)\n", r->duplication_pct, r->duplicated_blocks);
}
