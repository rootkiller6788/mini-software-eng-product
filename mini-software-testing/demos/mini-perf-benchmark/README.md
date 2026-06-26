# mini-perf-benchmark — Performance Benchmarking

## Goal

Record performance benchmarks with statistical distribution (p50/p95/p99), baseline comparison, and regression detection.

## Steps

1. Initialize performance suite with regression threshold (e.g., 10%)
2. Add benchmarks for key operations (sort, hash, parse, serialize)
3. Record multiple timing samples for each benchmark
4. Set baseline from first run
5. Re-run benchmarks and detect regressions (>threshold% slower)
6. Create load profiles (users/ramp time/request rate)
7. Print benchmark results with percentile distributions

## Key APIs

- `perf_suite_init()` — Set regression threshold
- `perf_add_benchmark()` — Register benchmark
- `perf_record()` — Record timing samples, auto-compute stats
- `perf_set_baseline()` — Freeze current as baseline
- `perf_detect_regression()` — Above threshold?
- `perf_regression_pct()` — How much slower/faster
- `perf_load_profile_init()` — Define load scenario

## Extensions

- Add throughput benchmarks (ops/sec instead of latency)
- Implement paired benchmarking (before/after code change)
- Generate flamegraph data (call tree timing)
- Support warmup iterations (discard first N runs)
