#include "test_performance.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

static int cmp_double(const void *a, const void *b) {
    double da = *(const double*)a, db = *(const double*)b;
    return (da > db) - (da < db);
}

void perf_suite_init(PerfSuite *ps, double regression_threshold) {
    memset(ps, 0, sizeof(*ps));
    ps->regression_threshold_pct = regression_threshold;
}

int perf_add_benchmark(PerfSuite *ps, const char *name, const char *desc) {
    if (ps->bench_count >= MAX_BENCHMARKS) return -1;
    Benchmark *b = &ps->benchmarks[ps->bench_count];
    strncpy(b->name, name, 63); b->name[63] = '\0';
    strncpy(b->description, desc, 255); b->description[255] = '\0';
    b->iterations = 0; b->total_time_us = 0;
    b->min_time_us = 1e18; b->max_time_us = 0; b->avg_time_us = 0;
    b->unit = BUNIT_US; b->baseline_us = 0;
    return ps->bench_count++;
}

void perf_record(PerfSuite *ps, int bench_idx, int iterations, double *times_us, int count) {
    if (bench_idx < 0 || bench_idx >= ps->bench_count) return;
    Benchmark *b = &ps->benchmarks[bench_idx];
    b->iterations = iterations;

    double sorted[MAX_ITERATIONS];
    int n = count < MAX_ITERATIONS ? count : MAX_ITERATIONS;
    double sum = 0, min_val = 1e18, max_val = 0;
    for (int i = 0; i < n; i++) {
        sorted[i] = times_us[i];
        sum += times_us[i];
        if (times_us[i] < min_val) min_val = times_us[i];
        if (times_us[i] > max_val) max_val = times_us[i];
    }
    qsort(sorted, n, sizeof(double), cmp_double);

    b->total_time_us = sum; b->avg_time_us = sum / n;
    b->min_time_us = min_val; b->max_time_us = max_val;
    b->p50_us = sorted[n/2];
    b->p95_us = sorted[(int)(n * 0.95)];
    b->p99_us = sorted[(int)(n * 0.99)];
    if (b->baseline_us == 0) b->baseline_us = b->avg_time_us;
}

void perf_set_baseline(PerfSuite *ps, int bench_idx) {
    if (bench_idx >= 0 && bench_idx < ps->bench_count)
        ps->benchmarks[bench_idx].baseline_us = ps->benchmarks[bench_idx].avg_time_us;
}

bool perf_detect_regression(PerfSuite *ps, int bench_idx) {
    return perf_regression_pct(ps, bench_idx) > ps->regression_threshold_pct;
}

double perf_regression_pct(PerfSuite *ps, int bench_idx) {
    if (bench_idx < 0 || bench_idx >= ps->bench_count) return 0;
    Benchmark *b = &ps->benchmarks[bench_idx];
    if (b->baseline_us < 1e-9) return 0;
    return 100.0 * (b->avg_time_us - b->baseline_us) / b->baseline_us;
}

void perf_load_profile_init(LoadProfile *lp, const char *name, int users, double ramp_sec) {
    strncpy(lp->name, name, 63); lp->name[63] = '\0';
    lp->target_users = users; lp->ramp_time_sec = ramp_sec;
    lp->request_rate = 0;
}

int perf_load_profile_rps(LoadProfile *lp) {
    if (lp->ramp_time_sec < 1e-9) return 0;
    return (int)(lp->target_users / lp->ramp_time_sec);
}

void perf_print_benchmark(PerfSuite *ps, int bench_idx) {
    if (bench_idx < 0 || bench_idx >= ps->bench_count) return;
    Benchmark *b = &ps->benchmarks[bench_idx];
    const char *us[] = {"ns","us","ms","ops/s"};
    printf("  %s: avg=%.2f%s p50=%.2f p95=%.2f p99=%.2f (n=%d)",
           b->name, b->avg_time_us, us[b->unit], b->p50_us, b->p95_us, b->p99_us, b->iterations);
    if (b->baseline_us > 0) printf(" baseline=%.2f regr=%.1f%%", b->baseline_us, perf_regression_pct(ps, bench_idx));
    printf("\n");
}

void perf_print_all(PerfSuite *ps) {
    printf("=== Performance Benchmarks ===\n");
    for (int i = 0; i < ps->bench_count; i++) perf_print_benchmark(ps, i);
}
