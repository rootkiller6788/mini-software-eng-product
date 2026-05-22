#include "arch_metrics.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

void metrics_compute(ModuleMetrics *m, int ca, int ce, int abstract_count, int total_count, int relations) {
    m->afferent_coupling = ca; m->efferent_coupling = ce;
    int denom = ca + ce;
    m->instability = denom > 0 ? (double)ce / denom : 0.0;
    m->abstractness = total_count > 0 ? (double)abstract_count / total_count : 0.0;
    m->distance = fabs(m->abstractness + m->instability - 1.0);
    m->in_safe_zone = m->distance < 0.3;
    m->relational_cohesion = total_count > 1 ? (double)(relations + 1) / total_count : 1.0;
    m->info_flow = (ca + ce) > 0 ? (double)(ca * ce) * (ca * ce) : 0;
}

void metrics_generate_report(MetricsReport *r, ModuleMetrics *metrics, int count) {
    memset(r, 0, sizeof(*r)); r->count = count;
    double sum_i = 0, sum_a = 0, sum_d = 0;
    for (int i = 0; i < count; i++) {
        r->metrics[i] = metrics[i]; sum_i += metrics[i].instability; sum_a += metrics[i].abstractness;
        sum_d += metrics[i].distance;
        if (metrics[i].instability > 0.7 && metrics[i].abstractness < 0.3) r->zone_of_pain_count++;
        if (metrics[i].instability < 0.3 && metrics[i].abstractness > 0.7) r->zone_of_uselessness_count++;
    }
    if (count > 0) { r->avg_instability = sum_i / count; r->avg_abstractness = sum_a / count; r->avg_distance = sum_d / count; }
}

void metrics_detect_violations(ModuleMetrics *m, const char *name, char *warnings, int sz) {
    warnings[0] = '\0';
    if (m->distance > 0.7) snprintf(warnings, sz, "%s: Zone of Pain (I=%.2f A=%.2f D=%.2f)", name, m->instability, m->abstractness, m->distance);
    else if (m->distance > 0.3 && m->distance <= 0.7) snprintf(warnings, sz, "%s: Borderline (D=%.2f)", name, m->distance);
}

void metrics_print_report(MetricsReport *r) {
    printf("=== Architecture Metrics Report ===\n");
    printf("  Avg Instability: %.3f, Avg Abstractness: %.3f\n", r->avg_instability, r->avg_abstractness);
    printf("  Avg Distance: %.3f\n", r->avg_distance);
    printf("  Zone of Pain: %d, Zone of Uselessness: %d\n", r->zone_of_pain_count, r->zone_of_uselessness_count);
    for (int i = 0; i < r->count; i++) { printf("  Module %d: I=%.2f A=%.2f D=%.2f\n", i, r->metrics[i].instability, r->metrics[i].abstractness, r->metrics[i].distance); }
}

void metrics_print_module(ModuleMetrics *m, const char *name) {
    printf("  %s: Ca=%d Ce=%d I=%.3f A=%.3f D=%.3f %s\n", name, m->afferent_coupling, m->efferent_coupling, m->instability, m->abstractness, m->distance, m->in_safe_zone ? "[SAFE]" : "[RISK]");
}
