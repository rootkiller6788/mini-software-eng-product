#include "include/unit_test_fw.h"
#include "include/mock_stub.h"
#include "include/tdd_bdd.h"
#include "include/mutation_testing.h"

static int shared_counter;
static int db_result;
static int send_count;

static int fake_query(const char *q) { (void)q; return db_result; }
static int fake_send(const char *msg) { (void)msg; send_count++; return 0; }

static int divide(int a, int b) { return b != 0 ? a / b : 0; }
static int is_positive(int x) { return x > 0; }
static int max_of_two(int a, int b) { return a > b ? a : b; }
static int is_even(int x) { return x % 2 == 0; }

static void suite_setup(void)   { shared_counter = 0; }
static void suite_teardown(void){ shared_counter = -1; }

TEST(demo_test_addition) {
    ASSERT_EQ(2 + 2, 4);
}

TEST(demo_test_subtraction) {
    ASSERT_EQ(10 - 3, 7);
}

TEST(demo_test_multiplication) {
    ASSERT_EQ(6 * 7, 42);
}

TEST(demo_test_division) {
    ASSERT_EQ(divide(10, 2), 5);
    ASSERT_EQ(divide(10, 0), 0);
}

TEST(demo_test_counter) {
    ASSERT_EQ(shared_counter, 0);
    shared_counter++;
    ASSERT_EQ(shared_counter, 1);
    shared_counter += 5;
    ASSERT_EQ(shared_counter, 6);
}

TEST(demo_test_positive) {
    ASSERT_EQ(is_positive(1), 1);
    ASSERT_EQ(is_positive(0), 0);
    ASSERT_EQ(is_positive(-1), 0);
}

TEST(demo_test_max) {
    ASSERT_EQ(max_of_two(5, 3), 5);
    ASSERT_EQ(max_of_two(-1, 1), 1);
    ASSERT_EQ(max_of_two(10, 10), 10);
}

TEST(demo_test_even) {
    ASSERT_EQ(is_even(2), 1);
    ASSERT_EQ(is_even(3), 0);
    ASSERT_EQ(is_even(0), 1);
}

TEST(demo_test_strings) {
    ASSERT_STREQ("hello world", "hello world");
    ASSERT_STRNEQ("abc", "xyz");
}

TEST(demo_test_pointers) {
    int val = 100;
    int *p = &val;
    ASSERT_NOT_NULL(p);
    ASSERT_EQ(*p, 100);
    int *np = NULL;
    ASSERT_NULL(np);
}

TEST(demo_test_double_precision) {
    ASSERT_DOUBLE_EQ(1.0 / 3.0, 0.333333, 0.0001);
}

TEST(demo_test_skip_demo) {
    SKIP("Feature planned for v2.0");
}

static void demo_mock_tests(void) {
    mock_init();

    mock_expect_t *exp = mock_expect_call("fake_send", 2);
    mock_set_return_int(exp, 0);

    send_count = 0;
    int rc1 = fake_send("hello");
    int rc2 = fake_send("world");

    printf("  fake_send rc1=%d rc2=%d\n", rc1, rc2);

    int failures = mock_verify();
    printf("  Mock verify failures: %d\n", failures);

    spy_record("fake_send", 1, MOCK_ARG_STRING, "hello");
    spy_record("fake_send", 1, MOCK_ARG_STRING, "world");

    int spy_cnt = spy_get_count("fake_send");
    printf("  Spy recorded calls: %d\n", spy_cnt);

    mock_destroy();

    fake_put_int("counter", 42);
    printf("  Fake store int: %d\n", fake_get_int("counter"));
    fake_put_string("name", "test_user");
    printf("  Fake store str: %s\n", fake_get_string("name"));
    fake_clear();
}

static void demo_bdd_workflow(void) {
    tdd_init();

    tdd_set_phase(TDD_RED);
    printf("\n  [TDD RED phase] Write a failing test\n");

    FEATURE("User Authentication", "Allow users to log in with credentials");
    SCENARIO("Successful login with valid credentials");
        GIVEN("a registered user");
        WHEN("the user enters correct username and password");
        THEN("login succeeds");

    SCENARIO("Failed login with invalid password");
        GIVEN("a registered user");
        WHEN("the user enters wrong password");
        THEN("login fails with error message");

    tdd_set_phase(TDD_GREEN);
    printf("\n  [TDD GREEN phase] Write simplest code to pass\n");

    tdd_set_phase(TDD_REFACTOR);
    printf("\n  [TDD REFACTOR phase] Clean up while tests stay green\n");

    tdd_run_all();
    tdd_report();
}

static int mutation_test_runner(void) {
    int result = 0;

    if (divide(4, 2) != 2) result |= 1;
    if (divide(10, 3) != 3) result |= 2;

    if (is_positive(5) != 1) result |= 4;
    if (is_positive(-5) != 0) result |= 8;

    if (max_of_two(3, 7) != 7) result |= 16;
    if (max_of_two(10, 5) != 10) result |= 32;

    if (is_even(4) != 1) result |= 64;
    if (is_even(3) != 0) result |= 128;

    if (10 > 5 != 1) result |= 256;
    if (5 >= 5 != 1) result |= 512;
    if (3 < 7 != 1) result |= 1024;
    if (5 <= 5 != 1) result |= 2048;

    if (5 == 5 != 1) result |= 4096;
    if (5 != 3 != 1) result |= 8192;

    return result;
}

static int mutation_always_pass(void) {
    return 0;
}

static void demo_mutation_testing(void) {
    mut_init();

    mut_config_t *cfg = &g_mut_runner.config;
    mut_config_enable_operator(cfg, MUT_ARITHMETIC_ADD_TO_SUB);
    mut_config_enable_operator(cfg, MUT_ARITHMETIC_DIV_TO_MUL);
    mut_config_enable_operator(cfg, MUT_RELATIONAL_GT_TO_GTE);
    mut_config_enable_operator(cfg, MUT_RELATIONAL_EQ_TO_NEQ);
    mut_config_enable_operator(cfg, MUT_LOGICAL_AND_TO_OR);
    mut_config_enable_operator(cfg, MUT_CONDITIONAL_TRUE);
    mut_config_enable_operator(cfg, MUT_CONDITIONAL_FALSE);

    printf("\n  Enabled mutation operators:\n");
    int i;
    for (i = 0; i < cfg->enabled_count; i++) {
        printf("    - %s: %s\n", mut_operator_name(cfg->enabled_operators[i]),
               mut_operator_description(cfg->enabled_operators[i]));
    }

    mut_mutant_t *m1 = &g_mut_runner.mutants[g_mut_runner.mutant_count++];
    memset(m1, 0, sizeof(mut_mutant_t));
    m1->id = 1;
    m1->operator_type = MUT_ARITHMETIC_ADD_TO_SUB;
    m1->line_number = 10;
    strncpy(m1->description, "Replace + with - in divide function", MUT_NAME_MAX - 1);

    mut_mutant_t *m2 = &g_mut_runner.mutants[g_mut_runner.mutant_count++];
    memset(m2, 0, sizeof(mut_mutant_t));
    m2->id = 2;
    m2->operator_type = MUT_RELATIONAL_GT_TO_GTE;
    m2->line_number = 20;
    strncpy(m2->description, "Replace > with >= in max_of_two", MUT_NAME_MAX - 1);

    mut_mutant_t *m3 = &g_mut_runner.mutants[g_mut_runner.mutant_count++];
    memset(m3, 0, sizeof(mut_mutant_t));
    m3->id = 3;
    m3->operator_type = MUT_CONDITIONAL_FALSE;
    m3->line_number = 15;
    strncpy(m3->description, "Replace condition with false", MUT_NAME_MAX - 1);

    mut_mutant_t *m4 = &g_mut_runner.mutants[g_mut_runner.mutant_count++];
    memset(m4, 0, sizeof(mut_mutant_t));
    m4->id = 4;
    m4->operator_type = MUT_LOGICAL_AND_TO_OR;
    m4->line_number = 25;
    strncpy(m4->description, "Replace && with ||", MUT_NAME_MAX - 1);

    mut_mutant_t *m5 = &g_mut_runner.mutants[g_mut_runner.mutant_count++];
    memset(m5, 0, sizeof(mut_mutant_t));
    m5->id = 5;
    m5->operator_type = MUT_RELATIONAL_EQ_TO_NEQ;
    m5->line_number = 30;
    strncpy(m5->description, "Replace == with !=", MUT_NAME_MAX - 1);

    printf("\n  Running mutation tests...\n");
    printf("  ----------------------------------------\n");

    {
        int r = mut_run_tests_on_mutant(&g_mut_runner, m1, mutation_test_runner);
        printf("  Mutant #1 (ADD_TO_SUB): %s\n", r == 1 ? "KILLED" : "SURVIVED");
    }
    {
        int r = mut_run_tests_on_mutant(&g_mut_runner, m2, mutation_test_runner);
        printf("  Mutant #2 (GT_TO_GTE): %s\n", r == 1 ? "KILLED" : "SURVIVED");
    }
    {
        int r = mut_run_tests_on_mutant(&g_mut_runner, m3, mutation_test_runner);
        printf("  Mutant #3 (CONDITIONAL_FALSE): %s\n", r == 1 ? "KILLED" : "SURVIVED");
    }
    {
        int r = mut_run_tests_on_mutant(&g_mut_runner, m4, mutation_test_runner);
        printf("  Mutant #4 (AND_TO_OR): %s\n", r == 1 ? "KILLED" : "SURVIVED");
    }
    {
        int r = mut_run_tests_on_mutant(&g_mut_runner, m5, mutation_test_runner);
        printf("  Mutant #5 (EQ_TO_NEQ): %s\n", r == 1 ? "KILLED" : "SURVIVED");
    }

    int survive_count = 0;
    for (i = 0; i < g_mut_runner.mutant_count; i++) {
        if (g_mut_runner.mutants[i].survived) survive_count++;
    }

    g_mut_runner.mutants_survived = survive_count;
    mut_report(&g_mut_runner);
    mut_report_detailed(&g_mut_runner);
}

int main(void) {
    printf("========================================\n");
    printf("  mini-software-testing — Full Demo\n");
    printf("========================================\n");

    utfw_init();

    utfw_suite_t *math_suite = utfw_register_suite("Arithmetic", suite_setup, suite_teardown);
    utfw_register_case(math_suite, "addition", __FILE__, __LINE__, demo_test_addition);
    utfw_register_case(math_suite, "subtraction", __FILE__, __LINE__, demo_test_subtraction);
    utfw_register_case(math_suite, "multiplication", __FILE__, __LINE__, demo_test_multiplication);
    utfw_register_case(math_suite, "division", __FILE__, __LINE__, demo_test_division);
    utfw_register_case(math_suite, "counter", __FILE__, __LINE__, demo_test_counter);
    utfw_register_case(math_suite, "positive", __FILE__, __LINE__, demo_test_positive);
    utfw_register_case(math_suite, "max_of_two", __FILE__, __LINE__, demo_test_max);
    utfw_register_case(math_suite, "is_even", __FILE__, __LINE__, demo_test_even);

    utfw_suite_t *util_suite = utfw_register_suite("Utilities", NULL, NULL);
    utfw_register_case(util_suite, "strings", __FILE__, __LINE__, demo_test_strings);
    utfw_register_case(util_suite, "pointers", __FILE__, __LINE__, demo_test_pointers);
    utfw_register_case(util_suite, "double_precision", __FILE__, __LINE__, demo_test_double_precision);
    utfw_register_case(util_suite, "skip_demo", __FILE__, __LINE__, demo_test_skip_demo);

    printf("\n=== 1. Unit Test Framework ===\n");
    utfw_run_all();
    utfw_report();

    printf("\n=== 2. Mock / Stub / Spy / Fake ===\n");
    demo_mock_tests();

    printf("\n=== 3. TDD / BDD Workflow ===\n");
    demo_bdd_workflow();

    printf("\n=== 4. Mutation Testing ===\n");
    demo_mutation_testing();

    printf("\n========================================\n");
    printf("  Demo Complete\n");
    printf("========================================\n");

    return g_ut_runner.total_failed > 0 ? 1 : 0;
}
