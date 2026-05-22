#ifndef COMPLEXITY_METRICS_H
#define COMPLEXITY_METRICS_H

#include <stddef.h>
#include <stdint.h>

#define CM_MAX_FUNCTIONS     512
#define CM_MAX_FILES         256
#define CM_MAX_BLOCKS        128
#define CM_MAX_TOKENS        8192
#define CM_MAX_NAME_LEN      128
#define CM_MAX_FILE_PATH     256
#define CM_MAX_NESTING       64

typedef enum {
    CM_TOKEN_KEYWORD,
    CM_TOKEN_IDENTIFIER,
    CM_TOKEN_OPERATOR,
    CM_TOKEN_LITERAL,
    CM_TOKEN_COMMENT,
    CM_TOKEN_WHITESPACE,
    CM_TOKEN_NEWLINE,
    CM_TOKEN_OTHER
} cm_token_type_e;

typedef enum {
    CM_SEVERITY_LOW,
    CM_SEVERITY_MEDIUM,
    CM_SEVERITY_HIGH,
    CM_SEVERITY_CRITICAL
} cm_risk_level_e;

typedef struct {
    int  functional_stmts;
    int  functional_branches;
    int  functional_loops;
    int  exceptional_paths;
    int  handler_count;
    int  nesting_depth;
    int  structural_elements;
} cm_cognitive_profile_t;

typedef struct {
    char name[CM_MAX_NAME_LEN];
    char file[CM_MAX_FILE_PATH];
    int  start_line;
    int  end_line;
    int  loc;
    int  comment_lines;
    int  blank_lines;
    int  cyclomatic_complexity;
    int  cognitive_complexity;
    double comment_ratio;
    int  nesting_depth;
    int  parameter_count;
    int  return_points;
    cm_risk_level_e risk;
} cm_function_metrics_t;

typedef struct {
    char path[CM_MAX_FILE_PATH];
    int  total_loc;
    int  comment_lines;
    int  blank_lines;
    int  code_lines;
    double comment_ratio;
    int  function_count;
    double avg_cyclomatic;
    double avg_cognitive;
    double maintainability_index;
    int  duplication_blocks;
    double duplication_density;
    cm_risk_level_e risk;
} cm_file_metrics_t;

typedef struct {
    cm_function_metrics_t functions[CM_MAX_FUNCTIONS];
    int  function_count;
    cm_file_metrics_t files[CM_MAX_FILES];
    int  file_count;
} cm_analysis_t;

typedef struct {
    char file[CM_MAX_FILE_PATH];
    int  start_line;
    int  end_line;
    int  line_count;
    cm_risk_level_e risk;
    double freq_score;
    int  change_freq;
} cm_hotspot_t;

typedef struct {
    cm_hotspot_t hotspots[CM_MAX_FILES];
    int  count;
} cm_hotspot_analysis_t;

void  cm_analysis_init(cm_analysis_t *a);
int   cm_analyze_function(cm_analysis_t *a, const char *name,
                          const char *file, int start, int end,
                          const char *source, int source_len);
int   cm_analyze_file(cm_analysis_t *a, const char *path,
                      const char *source, int source_len);
int   cm_calc_cyclomatic(const char *source, int len);
int   cm_calc_cognitive(const char *source, int len);
double cm_calc_comment_ratio(int comment_lines, int total_lines);
int   cm_count_loc(const char *source, int len, int *comments, int *blanks);
double cm_calc_maintainability(double hv, double cc, double loc,
                               double comment_ratio);
double cm_calc_halstead_volume(int operators, int operands,
                               int unique_operators, int unique_operands);
void  cm_assess_risk_function(cm_function_metrics_t *fm);
void  cm_assess_risk_file(cm_file_metrics_t *fm);
void  cm_print_function_metrics(const cm_function_metrics_t *fm);
void  cm_print_file_metrics(const cm_file_metrics_t *fm);
void  cm_detect_duplication(const cm_analysis_t *a,
                            cm_file_metrics_t *fm);
void  cm_hotspot_init(cm_hotspot_analysis_t *ha);
int   cm_add_hotspot(cm_hotspot_analysis_t *ha, const char *file,
                     int start, int end, int lines, int changes);
void  cm_sort_hotspots(cm_hotspot_analysis_t *ha);
void  cm_print_hotspots(const cm_hotspot_analysis_t *ha);
int   cm_nesting_depth(const char *source, int len);
int   cm_count_branches(const char *source, int len);
void  cm_cognitive_profile(const char *source, int len,
                           cm_cognitive_profile_t *profile);

#endif
