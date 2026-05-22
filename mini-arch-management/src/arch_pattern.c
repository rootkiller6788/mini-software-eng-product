#include "arch_pattern.h"
#include <stdio.h>
#include <string.h>

const char *pattern_name(ArchPattern p) {
    switch (p) { case PAT_LAYERED: return "Layered"; case PAT_PIPES_FILTERS: return "Pipes & Filters"; case PAT_MICROSERVICES: return "Microservices"; case PAT_EVENT_DRIVEN: return "Event-Driven"; case PAT_MONOLITH: return "Monolith"; case PAT_PLUGIN: return "Plugin"; default: return "?"; }
}

void pattern_init_context(PatternContext *ctx, int module_count) {
    memset(ctx, 0, sizeof(*ctx)); ctx->module_count = module_count;
}

int pattern_detect_layered(PatternContext *ctx, DetectedPattern *result) {
    int max_layer = 0, layer_sizes[10] = {0};
    for (int i = 0; i < ctx->module_count; i++) { int l = ctx->layers[i]; if (l > max_layer) max_layer = l; if (l < 10) layer_sizes[l]++; }
    result->type = PAT_LAYERED; result->layer_count = max_layer + 1;
    result->confidence = max_layer >= 2 ? 0.8 : 0.3;
    result->module_count = ctx->module_count;
    return result->confidence > 0.5 ? 1 : 0;
}

int pattern_detect_pipes_filters(PatternContext *ctx, DetectedPattern *result) {
    int seq = 0;
    for (int i = 0; i < ctx->module_count - 1; i++) { if (ctx->fan_out[i] <= 1 && ctx->fan_in[i+1] <= 1) seq++; }
    result->type = PAT_PIPES_FILTERS; result->confidence = (double)seq / (ctx->module_count - 1 + 1);
    result->module_count = ctx->module_count;
    return result->confidence > 0.5 ? 1 : 0;
}

int pattern_detect_monolith(PatternContext *ctx, DetectedPattern *result) {
    double avg_fan = 0;
    for (int i = 0; i < ctx->module_count; i++) avg_fan += ctx->fan_in[i] + ctx->fan_out[i];
    avg_fan /= (ctx->module_count * 2 + 1);
    result->type = PAT_MONOLITH; result->confidence = avg_fan > 3 ? 0.9 : 0.2;
    result->module_count = ctx->module_count;
    return result->confidence > 0.5 ? 1 : 0;
}

bool pattern_match(DetectedPattern *p, ArchPattern expected) { return p->type == expected && p->confidence > 0.5; }

void pattern_print(DetectedPattern *p) {
    printf("  Detected: %s (confidence=%.2f, layers=%d)\n", pattern_name(p->type), p->confidence, p->layer_count);
}
