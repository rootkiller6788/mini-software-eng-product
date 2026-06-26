#ifndef TEST_PERFORMANCE_H
#define TEST_PERFORMANCE_H
#include <stdbool.h>

#define MAX_BENCHMARKS 16
#define MAX_ITERATIONS 10000
#define MAX_PERF_HISTORY 32

typedef enum { BUNIT_NS, BUNIT_US, BUNIT_MS, BUNIT_OPS_SEC } BenchUnit;

typedef struct {
    char name[64]; char description[256];
    int iterations; double total_time_us;
    double min_time_us; double max_time_us; double avg_time_us;
    double p50_us; double p95_us; double p99_us;
    BenchUnit unit;
    double baseline_us; /* previous benchmark result */
} Benchmark;

typedef struct {
    char name[64]; int target_users; double ramp_time_sec;
    double request_rate; /* requests per second */
} LoadProfile;

typedef struct {
    Benchmark benchmarks[MAX_BENCHMARKS]; int bench_count;
    double history[MAX_PERF_HISTORY]; int history_count; /* benchmark results over time */
    double regression_threshold_pct; /* e.g. 10% slower = regression */
} PerfSuite;

void perf_suite_init(PerfSuite *ps, double regression_threshold);
int  perf_add_benchmark(PerfSuite *ps, const char *name, const char *desc);
void perf_record(PerfSuite *ps, int bench_idx, int iterations, double *times_us, int count);
void perf_set_baseline(PerfSuite *ps, int bench_idx);
bool perf_detect_regression(PerfSuite *ps, int bench_idx);
double perf_regression_pct(PerfSuite *ps, int bench_idx); /* positive = slower, negative = faster */
void perf_load_profile_init(LoadProfile *lp, const char *name, int users, double ramp_sec);
int  perf_load_profile_rps(LoadProfile *lp); /* requests per second at steady state */
void perf_print_benchmark(PerfSuite *ps, int bench_idx);
void perf_print_all(PerfSuite *ps);
#endif
