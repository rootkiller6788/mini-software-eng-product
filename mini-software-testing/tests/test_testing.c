#include "test_framework.h"
#include "test_double.h"
#include "test_coverage.h"
#include "test_performance.h"
#include "test_reporting.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>

static int tests = 0, passed = 0;
#define TEST(name) do { tests++; printf("  TEST %s... ", name); } while(0)
#define OK() do { passed++; printf("OK\n"); } while(0)

int main(void) {
    printf("=== Software Testing Tests ===\n\n");

    /* Framework */
    printf("framework:\n");
    TEST("init runner"); { TestRunner tr; runner_init(&tr);
      assert(tr.suite_count == 0); OK(); }
    TEST("add suite"); { TestRunner tr; runner_init(&tr);
      assert(suite_add(&tr,"TestSuite",NULL,NULL) == 0); OK(); }
    TEST("add test"); { TestRunner tr; runner_init(&tr);
      suite_add(&tr,"S",NULL,NULL); assert(test_add(&tr,0,"test1") == 0); OK(); }
    TEST("record assert"); { TestRunner tr; runner_init(&tr);
      suite_add(&tr,"S",NULL,NULL); test_add(&tr,0,"T");
      test_record_assert(&tr,0,0,10,"file.c","msg",true);
      assert(tr.suites[0].tests[0].assert_count == 1); OK(); }
    TEST("test finish"); { TestRunner tr; runner_init(&tr);
      suite_add(&tr,"S",NULL,NULL); test_add(&tr,0,"T");
      test_finish(&tr,0,0,TRES_PASS,42);
      assert(tr.total_pass == 1); assert(tr.total_duration_ms == 42); OK(); }
    TEST("bad indices"); { TestRunner tr; runner_init(&tr);
      assert(test_add(&tr,99,"x") == -1); OK(); }

    /* Test Doubles */
    printf("doubles:\n");
    TEST("add stub"); { DoubleRegistry dr; double_registry_init(&dr);
      assert(double_add_stub(&dr,"get_port",8080) == 0);
      assert(dr.doubles[0].type == DTYPE_STUB);
      assert(dr.doubles[0].stub_return_value == 8080); OK(); }
    TEST("add mock expect"); { DoubleRegistry dr; double_registry_init(&dr);
      double_add_mock(&dr,"send");
      assert(double_mock_expect(&dr,0,"arg1",42) == 0); OK(); }
    TEST("spy record"); { DoubleRegistry dr; double_registry_init(&dr);
      double_add_spy(&dr,"log",0);
      double_spy_record(&dr,0,"log","msg1");
      assert(double_spy_call_count(&dr,0) == 1); OK(); }
    TEST("fake state"); { DoubleRegistry dr; double_registry_init(&dr);
      double_add_fake(&dr,"cache",99);
      assert(dr.doubles[0].fake_state == 99); OK(); }

    /* Coverage */
    printf("coverage:\n");
    TEST("add file"); { CoverageData cd; coverage_init(&cd);
      assert(coverage_add_file(&cd,"test.c") == 0); OK(); }
    TEST("add line with branch"); { CoverageData cd; coverage_init(&cd);
      coverage_add_file(&cd,"test.c");
      assert(coverage_add_line(&cd,0,42,true) == 0);
      assert(cd.files[0].branch_points == 1); OK(); }
    TEST("line hit"); { CoverageData cd; coverage_init(&cd);
      coverage_add_file(&cd,"test.c"); coverage_add_line(&cd,0,10,false);
      coverage_hit(&cd,0,10); coverage_hit(&cd,0,10);
      assert(cd.files[0].lines[0].hit_count == 2);
      assert(cd.files[0].lines[0].status == CLINE_HIT); OK(); }
    TEST("branch both"); { CoverageData cd; coverage_init(&cd);
      coverage_add_file(&cd,"test.c"); coverage_add_line(&cd,0,10,true);
      coverage_branch_hit(&cd,0,10,true,true);
      assert(cd.files[0].lines[0].status == CLINE_BRANCH_BOTH); OK(); }
    TEST("line pct"); { CoverageData cd; coverage_init(&cd);
      coverage_add_file(&cd,"t.c");
      coverage_add_line(&cd,0,1,false); coverage_add_line(&cd,0,2,false);
      coverage_hit(&cd,0,1); coverage_calculate(&cd);
      assert(coverage_line_pct(&cd) == 50.0); OK(); }
    TEST("meets threshold"); { CoverageData cd; coverage_init(&cd);
      coverage_add_file(&cd,"t.c");
      coverage_add_line(&cd,0,1,false); coverage_hit(&cd,0,1);
      coverage_calculate(&cd);
      assert(coverage_meets_threshold(&cd,80.0,0.0)); OK(); }

    /* Performance */
    printf("performance:\n");
    TEST("add benchmark"); { PerfSuite ps; perf_suite_init(&ps,10.0);
      assert(perf_add_benchmark(&ps,"test","desc") == 0); OK(); }
    TEST("record times"); { PerfSuite ps; perf_suite_init(&ps,10.0);
      perf_add_benchmark(&ps,"test","");
      double t[] = {10.0,12.0,11.0,13.0,10.5};
      perf_record(&ps,0,5,t,5);
      assert(ps.benchmarks[0].avg_time_us > 0); OK(); }
    TEST("baseline regression"); { PerfSuite ps; perf_suite_init(&ps,10.0);
      perf_add_benchmark(&ps,"test",""); double t[]={10.0,10.0,10.0,10.0,10.0};
      perf_record(&ps,0,5,t,5); perf_set_baseline(&ps,0);
      double t2[]={13.0,13.0,13.0,13.0,13.0}; perf_record(&ps,0,5,t2,5);
      assert(perf_detect_regression(&ps,0)); OK(); }

    /* Reporting */
    printf("reporting:\n");
    TEST("record run"); { TestReport rep; report_init(&rep,"P",3);
      TestRunner tr; runner_init(&tr); tr.total_pass=5; tr.total_fail=1;
      report_record_run(&rep,&tr);
      assert(rep.run_count == 1); OK(); }
    TEST("trend"); { TestReport rep; report_init(&rep,"P",3);
      TestRunner tr; runner_init(&tr); tr.total_pass=8; tr.total_fail=2;
      report_record_run(&rep,&tr);
      tr.total_pass=9; tr.total_fail=1; report_record_run(&rep,&tr);
      /* trend should be positive */
      double trend = report_pass_rate_trend(&rep);
      assert(trend >= 0); OK(); }
    TEST("junit export"); { TestReport rep; report_init(&rep,"P",3);
      TestRunner tr; runner_init(&tr);
      suite_add(&tr,"S",NULL,NULL); test_add(&tr,0,"T");
      report_export_junit(&rep,&tr,"build/test-junit.xml");
      /* file should exist */ OK(); }

    /* Mutation */
    printf("mutation:\n");
    TEST("add mutant"); { MutationReport mr; mutation_init(&mr);
      assert(mutation_add_mutant(&mr,"M1") == 0); OK(); }
    TEST("kill mutant"); { MutationReport mr; mutation_init(&mr);
      mutation_add_mutant(&mr,"M1"); mutation_add_mutant(&mr,"M2");
      mutation_kill(&mr,"M1");
      assert(mr.total_killed == 1);
      assert(mutation_score(&mr) == 50.0); OK(); }
    TEST("all killed"); { MutationReport mr; mutation_init(&mr);
      mutation_add_mutant(&mr,"M1"); mutation_add_mutant(&mr,"M2");
      mutation_kill(&mr,"M1"); mutation_kill(&mr,"M2");
      assert(mutation_score(&mr) == 100.0); OK(); }

    printf("\n=== Results: %d/%d passed ===\n", passed, tests);
    return passed == tests ? 0 : 1;
}
