#ifndef EQ_STATIC_ANALYSIS_H
#define EQ_STATIC_ANALYSIS_H
#include <stdbool.h>

typedef struct {
    int total_lines; int code_lines; int comment_lines; int blank_lines; double comment_ratio;
} FileStats;

typedef struct {
    int max_nesting_depth; int avg_nesting_depth;
    int function_count; int avg_function_length;
    int duplication_pct; int duplicated_blocks;
    FileStats file_stats;
} StaticAnalysisResult;

void static_analyze(const char *source_code, StaticAnalysisResult *result);
void static_count_lines(const char *source, FileStats *fs);
int  static_detect_duplication(const char *source, int min_block_lines);
int  static_nesting_depth(const char *source, int *max, int *avg);
bool static_is_too_complex(StaticAnalysisResult *r);
void static_print(StaticAnalysisResult *r);
#endif
