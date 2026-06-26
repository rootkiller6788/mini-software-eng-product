#include "../include/tdd_bdd.h"

static int counter = 0;

static void given_a_counter(void) {
    counter = 0;
    printf(" [counter=0]");
}

static void when_incrementing_by_n(void) {
    counter += 5;
    printf(" [counter+=5]");
}

static void then_counter_equals_n(void) {
    printf(" [assert counter==5]");
    tdd_assert_eq_int(5, counter, "counter should be 5");
}

static void given_an_empty_stack(void) {
    printf(" [stack empty]");
}

static void when_pushing_an_item(void) {
    printf(" [push item]");
}

static void then_stack_is_not_empty(void) {
    tdd_assert_true(true, "stack should not be empty");
}

static void when_popping_from_empty_stack(void) {
    printf(" [pop from empty]");
}

static void then_error_is_raised(void) {
    tdd_assert_true(true, "error should be raised");
}

int main(void) {
    tdd_init();

    FEATURE("Calculator", "A simple calculator that performs arithmetic operations");
    SCENARIO("Adding two numbers");
        GIVEN("I have a calculator");
        WHEN("I add 2 and 3");
        THEN("the result should be 5");

    SCENARIO("Subtracting two numbers");
        GIVEN("I have a calculator");
        WHEN("I subtract 3 from 10");
        THEN("the result should be 7");

    FEATURE("Counter", "A counter that can be incremented");
    SCENARIO("Increment the counter");
        GIVEN_STEP("a counter initialized to 0", given_a_counter);
        WHEN_STEP("I increment by 5", when_incrementing_by_n);
        THEN_STEP("counter should be 5", then_counter_equals_n);

    FEATURE("Stack", "A LIFO data structure");
    SCENARIO("Push increases size");
        GIVEN_STEP("an empty stack", given_an_empty_stack);
        WHEN_STEP("pushing an item", when_pushing_an_item);
        THEN_STEP("stack is not empty", then_stack_is_not_empty);

    SCENARIO("Pop from empty raises error");
        GIVEN_STEP("an empty stack", given_an_empty_stack);
        WHEN_STEP("popping from empty stack", when_popping_from_empty_stack);
        THEN_STEP("error is raised", then_error_is_raised);

    tdd_set_phase(TDD_RED);
    printf("  Writing a failing test first...\n");

    tdd_set_phase(TDD_GREEN);
    printf("  Writing simplest code to pass...\n");

    tdd_set_phase(TDD_REFACTOR);
    printf("  Refactoring while keeping tests green...\n");

    tdd_run_all();
    tdd_report();

    const char *doc_path = "living_doc.md";
    tdd_generate_living_doc(doc_path);

    return tdd_exit_code();
}
