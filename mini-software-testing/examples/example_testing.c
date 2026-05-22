#include "test_framework.h"
#include "test_double.h"
#include "test_coverage.h"
#include "test_performance.h"
#include "test_reporting.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main(void) {
    printf("===== Software Testing Demo =====\n\n");

    /* Test Framework */
    printf("--- Test Runner ---\n");
    TestRunner tr;
    runner_init(&tr);
    int si = suite_add(&tr, "Math Tests", NULL, NULL);
    test_add(&tr, si, "test_addition");
    test_add(&tr, si, "test_subtraction");
    test_add(&tr, si, "test_multiplication");
    test_record_assert(&tr, si, 0, __LINE__, __FILE__, "1+1==2", true);
    test_record_assert(&tr, si, 0, __LINE__, __FILE__, "2+2==4", true);
    test_record_assert(&tr, si, 1, __LINE__, __FILE__, "5-3==2", true);
    test_record_assert(&tr, si, 1, __LINE__, __FILE__, "10-5==4", false);
    test_record_assert(&tr, si, 2, __LINE__, __FILE__, "3*4==12", true);
    runner_run_all(&tr);
    runner_print_summary(&tr);
    printf("\n");

    /* Test Doubles */
    printf("--- Test Doubles ---\n");
    DoubleRegistry dr;
    double_registry_init(&dr);
    double_add_stub(&dr, "database_query", 42);
    double_add_mock(&dr, "email_service");
    double_mock_expect(&dr, 1, "send_welcome(user=1)", 0);
    double_add_fake(&dr, "cache", 0);
    double_add_spy(&dr, "logger", 0);
    double_spy_record(&dr, 3, "logger", "INFO: user logged in");
    double_spy_record(&dr, 3, "logger", "INFO: user logged out");
    printf("Spy call count (logger): %d\n", double_spy_call_count(&dr, 3));
    double_print_all(&dr);
    printf("\n");

    /* Code Coverage */
    printf("--- Code Coverage ---\n");
    CoverageData cd;
    coverage_init(&cd);
    coverage_add_file(&cd, "src/math.c");
    coverage_add_line(&cd, 0, 10, false);
    coverage_add_line(&cd, 0, 11, true);
    coverage_add_line(&cd, 0, 12, false);
    coverage_add_line(&cd, 0, 13, true);
    coverage_hit(&cd, 0, 10);
    coverage_hit(&cd, 0, 11);
    coverage_hit(&cd, 0, 11);
    coverage_hit(&cd, 0, 12);
    coverage_branch_hit(&cd, 0, 11, true, true);
    coverage_print_summary(&cd);
    printf("Meets 80%% threshold: %s\n\n",
           coverage_meets_threshold(&cd, 80.0, 50.0) ? "YES" : "NO");

    /* Performance */
    printf("--- Performance Benchmarks ---\n");
    PerfSuite ps;
    perf_suite_init(&ps, 10.0);
    perf_add_benchmark(&ps, "sort_1k", "Sort 1000 integers");
    double times1[] = {12.5, 13.0, 12.8, 12.9, 13.2, 12.7, 12.6, 13.1, 12.9, 12.8,
                       13.0, 12.8, 12.7, 12.9, 13.1, 12.8, 12.9, 13.0, 12.7, 12.8};
    perf_record(&ps, 0, 1000, times1, 20);
    perf_add_benchmark(&ps, "hash_10k", "Hash 10000 strings");
    double times2[] = {45.0, 46.2, 44.8, 45.5, 45.1, 44.9, 45.3, 45.7, 45.2, 44.6};
    perf_record(&ps, 1, 10000, times2, 10);
    perf_print_all(&ps);

    /* Load profile */
    LoadProfile lp;
    perf_load_profile_init(&lp, "API Stress", 1000, 60.0);
    printf("Load profile: %d req/s for %d users over %.0fs\n\n",
           perf_load_profile_rps(&lp), lp.target_users, lp.ramp_time_sec);

    /* Test Reporting */
    printf("--- Test Reporting ---\n");
    TestReport rep;
    report_init(&rep, "mini-testing-demo", 3);
    report_record_run(&rep, &tr);
    report_print_history(&rep);
    report_print_trend(&rep);
    report_export_junit(&rep, &tr, "build/test-results.xml");
    printf("JUnit XML exported to build/test-results.xml\n\n");

    /* Mutation Testing */
    printf("--- Mutation Testing ---\n");
    MutationReport mr;
    mutation_init(&mr);
    mutation_add_mutant(&mr, "MUT-01: flip > to >=");
    mutation_add_mutant(&mr, "MUT-02: remove null check");
    mutation_add_mutant(&mr, "MUT-03: swap && to ||");
    mutation_add_mutant(&mr, "MUT-04: increment to decrement");
    mutation_kill(&mr, "MUT-01: flip > to >=");
    mutation_kill(&mr, "MUT-02: remove null check");
    mutation_kill(&mr, "MUT-03: swap && to ||");
    mutation_print(&mr);

    return 0;
}
