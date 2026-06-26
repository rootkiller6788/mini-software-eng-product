#ifndef ARCH_METRICS_H
#define ARCH_METRICS_H
#include <stdbool.h>

#define METRICS_MAX_MODULES 64

typedef struct {
    int afferent_coupling;   /* Ca: incoming dependencies */
    int efferent_coupling;   /* Ce: outgoing dependencies */
    double instability;      /* I = Ce / (Ca + Ce) */
    double abstractness;     /* A = abstract_classes / total_classes */
    double distance;         /* D = |A + I - 1| */
    bool in_safe_zone;       /* D < 0.3 */
    int relational_cohesion; /* H = (R + 1) / N */
    double info_flow;        /* IF = (fan_in * fan_out)^2 */
} ModuleMetrics;

typedef struct {
    ModuleMetrics metrics[METRICS_MAX_MODULES]; int count;
    double avg_instability;
    double avg_abstractness;
    double avg_distance;
    int zone_of_pain_count;
    int zone_of_uselessness_count;
} MetricsReport;

void metrics_compute(ModuleMetrics *m, int ca, int ce, int abstract_count, int total_count, int relations);
void metrics_generate_report(MetricsReport *r, ModuleMetrics *metrics, int count);
void metrics_detect_violations(ModuleMetrics *m, const char *name, char *warnings, int warn_buf_size);
void metrics_print_report(MetricsReport *r);
void metrics_print_module(ModuleMetrics *m, const char *name);
#endif
