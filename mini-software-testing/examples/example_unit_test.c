#include "../include/unit_test_fw.h"

static int counter;

static void set_up(void) {
    counter = 0;
}

static void tear_down(void) {
    counter = -1;
}

TEST(test_counter_init) {
    ASSERT_EQ(counter, 0);
}

TEST(test_counter_increment) {
    counter++;
    ASSERT_EQ(counter, 1);
    counter++;
    ASSERT_EQ(counter, 2);
}

TEST(test_string_comparison) {
    ASSERT_STREQ("hello", "hello");
}

TEST(test_null_check) {
    int *p = NULL;
    ASSERT_NULL(p);
}

TEST(test_not_null) {
    int x = 42;
    int *p = &x;
    ASSERT_NOT_NULL(p);
    ASSERT_EQ(*p, 42);
}

TEST(test_double_compare) {
    ASSERT_DOUBLE_EQ(3.14, 3.14, 0.001);
}

TEST(test_skip_case) {
    SKIP("Not implemented yet");
}

static void test_fib(int n, int expected) {
    if (n <= 1) {
        ASSERT_EQ(n, expected);
        return;
    }
    int a = 0, b = 1, c = 0, i;
    for (i = 2; i <= n; i++) {
        c = a + b;
        a = b;
        b = c;
    }
    ASSERT_EQ(c, expected);
}

TEST(test_fibonacci) {
    test_fib(0, 0);
    test_fib(1, 1);
    test_fib(2, 1);
    test_fib(5, 5);
    test_fib(10, 55);
}

int main(void) {
    utfw_init();

    utfw_suite_t *math_suite = utfw_register_suite("Math", set_up, tear_down);
    utfw_register_case(math_suite, "counter_init", __FILE__, __LINE__, test_counter_init);
    utfw_register_case(math_suite, "counter_increment", __FILE__, __LINE__, test_counter_increment);
    utfw_register_case(math_suite, "fibonacci", __FILE__, __LINE__, test_fibonacci);

    utfw_suite_t *string_suite = utfw_register_suite("String", NULL, NULL);
    utfw_register_case(string_suite, "string_comparison", __FILE__, __LINE__, test_string_comparison);
    utfw_register_case(string_suite, "null_check", __FILE__, __LINE__, test_null_check);
    utfw_register_case(string_suite, "not_null", __FILE__, __LINE__, test_not_null);
    utfw_register_case(string_suite, "double_compare", __FILE__, __LINE__, test_double_compare);

    utfw_suite_t *skip_suite = utfw_register_suite("SkipDemo", NULL, NULL);
    utfw_register_case(skip_suite, "skip_case", __FILE__, __LINE__, test_skip_case);

    utfw_run_all();
    utfw_report();

    return utfw_exit_code();
}
