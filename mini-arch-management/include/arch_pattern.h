#ifndef ARCH_PATTERN_H
#define ARCH_PATTERN_H
#include <stdbool.h>

typedef enum { PAT_LAYERED, PAT_PIPES_FILTERS, PAT_MICROSERVICES, PAT_EVENT_DRIVEN, PAT_MONOLITH, PAT_PLUGIN } ArchPattern;

typedef struct {
    ArchPattern type;
    int layer_count;
    int *module_ids;
    int module_count;
    double confidence; /* 0..1 match score */
} DetectedPattern;

typedef struct {
    int module_count;
    int fan_in[64], fan_out[64];
    int layers[64];
} PatternContext;

void pattern_init_context(PatternContext *ctx, int module_count);
int  pattern_detect_layered(PatternContext *ctx, DetectedPattern *result);
int  pattern_detect_pipes_filters(PatternContext *ctx, DetectedPattern *result);
int  pattern_detect_monolith(PatternContext *ctx, DetectedPattern *result);
bool pattern_match(DetectedPattern *p, ArchPattern expected);
const char *pattern_name(ArchPattern p);
void pattern_print(DetectedPattern *p);
#endif
