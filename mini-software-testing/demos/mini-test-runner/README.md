# mini-test-runner — Lightweight Test Framework

## Goal

Build a minimal xUnit-style test framework with suites, test cases, assertions, setup/teardown hooks, and result reporting.

## Steps

1. Initialize a TestRunner and add 2-3 test suites
2. Add 3-5 test cases per suite, each making 1-3 assertions
3. Use assertion functions: TF_ASSERT, TF_ASSERT_EQ, TF_ASSERT_TRUE, etc.
4. Add optional setup() and teardown() function pointers per suite
5. Run all tests and observe the pass/fail/error result per test
6. Verify the summary shows correct counts and timing

## Key APIs

- `runner_init()` — Initialize runner with zero counts
- `suite_add()` — Create suite with optional setup/teardown
- `test_add()` — Register a test case
- `test_record_assert()` — Record assertion result (pass/fail)
- `test_finish()` — Mark test result and duration
- `runner_run_all()` — Execute all suites and tests
- `runner_print_summary()` — Display pass/fail/error/skip counts

## Extensions

- Add test filtering (run by suite name, test name, or tag)
- Implement parameterized tests (same test with multiple inputs)
- Add timeout support (fail test after N ms)
- Integrate with CI via exit code (0=all pass, 1=failures)
