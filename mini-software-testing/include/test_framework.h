#ifndef TEST_FRAMEWORK_H
#define TEST_FRAMEWORK_H
#include <stdbool.h>

#define MAX_SUITES 8
#define MAX_TESTS_PER_SUITE 16
#define MAX_ASSERTS_PER_TEST 16

typedef enum { TRES_PASS, TRES_FAIL, TRES_ERROR, TRES_SKIP } TestResult;

typedef struct {
    int line; char file[64]; char message[256]; bool passed;
} Assertion;

typedef struct {
    char name[64]; TestResult result;
    Assertion assertions[MAX_ASSERTS_PER_TEST]; int assert_count;
    int duration_ms;
} TestCase;

typedef struct {
    char name[64];
    TestCase tests[MAX_TESTS_PER_SUITE]; int test_count;
    void (*setup)(void); void (*teardown)(void);
} TestSuite;

typedef struct {
    TestSuite suites[MAX_SUITES]; int suite_count;
    int total_pass; int total_fail; int total_error; int total_skip;
    int total_duration_ms;
} TestRunner;

/* Assertion macros (mockable via function pointers in runner) */
#define TF_ASSERT(cond, msg) do { \
    if (!(cond)) { fprintf(stderr,"FAIL: %s:%d %s\n",__FILE__,__LINE__,msg); } \
} while(0)

#define TF_ASSERT_EQ(a,b)   TF_ASSERT((a)==(b),"expected equal")
#define TF_ASSERT_NEQ(a,b)  TF_ASSERT((a)!=(b),"expected not equal")
#define TF_ASSERT_TRUE(c)   TF_ASSERT((c),"expected true")
#define TF_ASSERT_FALSE(c)  TF_ASSERT(!(c),"expected false")
#define TF_ASSERT_NULL(p)   TF_ASSERT((p)==NULL,"expected NULL")
#define TF_ASSERT_NOT_NULL(p) TF_ASSERT((p)!=NULL,"expected not NULL")

void runner_init(TestRunner *tr);
int  suite_add(TestRunner *tr, const char *name, void(*setup)(void), void(*teardown)(void));
int  test_add(TestRunner *tr, int suite_idx, const char *name);
void test_record_assert(TestRunner *tr, int suite_idx, int test_idx, int line, const char *file, const char *msg, bool passed);
void test_finish(TestRunner *tr, int suite_idx, int test_idx, TestResult result, int duration_ms);
int  runner_run_all(TestRunner *tr); /* returns 0 if all pass */
void runner_print_summary(TestRunner *tr);
#endif
