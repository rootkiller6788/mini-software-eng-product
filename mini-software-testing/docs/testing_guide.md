# Mini Software Testing — User Guide

## Overview

`mini-software-testing` is a lightweight, header-only-friendly software testing library written in **C99**.
It provides five core testing modules for comprehensive software quality assurance.

---

## 1. Unit Test Framework (`unit_test_fw`)

### Test Suite & Test Case
```c
#include "include/unit_test_fw.h"

TEST(my_test_name) {
    ASSERT_EQ(2 + 2, 4);
}

int main() {
    utfw_init();
    utfw_suite_t *s = utfw_register_suite("Math", setup_func, teardown_func);
    utfw_register_case(s, "addition", __FILE__, __LINE__, my_test_name);
    utfw_run_all();
    utfw_report();
    return utfw_exit_code();
}
```

### Available Assertions
| Macro | Description |
|-------|-------------|
| `ASSERT_TRUE(expr)` | Asserts expression is true |
| `ASSERT_FALSE(expr)` | Asserts expression is false |
| `ASSERT_EQ(a, b)` | Asserts integer equality |
| `ASSERT_NEQ(a, b)` | Asserts integer inequality |
| `ASSERT_STREQ(a, b)` | Asserts string equality |
| `ASSERT_STRNEQ(a, b)` | Asserts string inequality |
| `ASSERT_NULL(ptr)` | Asserts pointer is NULL |
| `ASSERT_NOT_NULL(ptr)` | Asserts pointer is not NULL |
| `ASSERT_DOUBLE_EQ(a, b, eps)` | Asserts floating-point equality within epsilon |
| `ASSERT_FAIL(msg)` | Unconditional failure |
| `SKIP(reason)` | Skips the current test |

### Fixture Reuse (Setup/Teardown)
```c
static int counter;
static void init() { counter = 0; }
static void cleanup() { counter = -1; }

utfw_suite_t *s = utfw_register_suite("Suite", init, cleanup);
```

### Test Runner Pipeline
`discover` → `execute` → `report`
- Discovers all registered suites and cases
- Executes each case with setup/teardown
- Reports PASS/FAIL/SKIP with timing

---

## 2. Mock & Stub (`mock_stub`)

### Test Doubles

| Type | Purpose | Usage |
|------|---------|-------|
| **Stub** | Returns hardcoded responses | `stub_set_default_int()` |
| **Mock** | Verifies expected calls | `mock_expect_call()` + `mock_verify()` |
| **Fake** | Lightweight working implementation | `fake_put_int()` / `fake_get_int()` |
| **Spy** | Records received calls for later inspection | `spy_record()` / `spy_get_count()` |

### Mock Example
```c
mock_init();

mock_expect_t *exp = mock_expect_call("db_query", 1);
mock_set_return_int(exp, 42);

int result = my_function_that_calls_db_query();
mock_verify();  // checks call count matched

mock_destroy();
```

### Argument Matchers
```c
mock_arg_match_int(10, 10);         // exact int match
mock_arg_match_string("abc", "abc"); // exact string match
mock_arg_match_any();               // matches anything
```

---

## 3. TDD & BDD (`tdd_bdd`)

### TDD Cycle: Red → Green → Refactor
```c
tdd_set_phase(TDD_RED);       // Write a failing test
// ... implement feature ...
tdd_set_phase(TDD_GREEN);     // Write simplest code to pass
// ... make test pass ...
tdd_set_phase(TDD_REFACTOR);  // Clean up code
```

### BDD: Feature & Scenario (Given-When-Then)
```c
tdd_init();

FEATURE("Login", "User authentication system");
SCENARIO("Valid credentials");
    GIVEN("a registered user");
    WHEN("entering correct password");
    THEN("login succeeds");
    AND("dashboard is shown");

SCENARIO("Invalid credentials");
    GIVEN("a registered user");
    WHEN("entering wrong password");
    THEN("error message is shown");

tdd_run_all();
tdd_report();
```

### Living Documentation
```c
tdd_generate_living_doc("features.md");
```
Generates a Markdown file with all features, scenarios, steps, and pass/fail status.

---

## 4. E2E & Contract Testing (`e2e_contract`)

### Page Object Model (Selenium-like)
```c
e2e_init();

e2e_page_t *login = e2e_page_define("login", "https://app.com/login");
e2e_element_t *user = e2e_element_add(login, "username");
e2e_element_set_locator(user, LOCATOR_ID, "user-input");

e2e_navigate("login");
e2e_type("login", "username", "admin");
e2e_type("login", "password", "secret");
e2e_click("login", "login_btn");
e2e_assert_visible("dashboard", "welcome");
```

### Contract Testing (Pact Model)
```c
contract_pact_t *pact = contract_pact_create("ConsumerApp", "ProviderService");

contract_interaction_t *i = contract_add_interaction(pact, "Get user");
contract_upon_receiving(i, CONTRACT_GET, "/users/123");
contract_will_respond_with(i, 200);
contract_with_response_body(i, "{\"id\":123}");

contract_verify(pact);
contract_verifier_report();
```

### Scenario Helpers
- `e2e_scenario_login(username, password)` — full login flow
- `e2e_scenario_search(query)` — search flow
- `e2e_scenario_checkout()` — checkout flow

---

## 5. Mutation Testing (`mutation_testing`)

### Mutant Operators
| Category | Operators |
|----------|-----------|
| Arithmetic | `+`→`-`, `-`→`+`, `*`→`/`, `/`→`*` |
| Relational | `>`→`>=`, `>=`→`>`, `<`→`<=`, `<=`→`<`, `==`→`!=`, `!=`→`==` |
| Logical | `&&`→`||`, `||`→`&&` |
| Conditional | `if(cond)`→`if(true)`, `if(cond)`→`if(false)`, negate `!cond` |

### Usage
```c
mut_init();
mut_config_enable_operator(&g_mut_runner.config, MUT_ARITHMETIC_ADD_TO_SUB);

// Run tests against each mutant
mut_run_tests_on_mutant(&g_mut_runner, mutant, my_test_runner);

mut_calculate_score(&g_mut_runner);
mut_report(&g_mut_runner);
```

### Mutation Score
- **Score = killed / (total - equivalent) * 100%**
- >= 80%: Good test coverage
- >= 60%: Adequate
- < 60%: Insufficient — write more tests

### Mutant Statuses
- **Killed** — Test suite detected the mutation (good!)
- **Survived** — Tests passed despite mutation (bad — need better tests)
- **Equivalent** — Mutation produces identical behavior

---

## Building

```bash
make all        # Build examples and demos
make examples   # Build only examples
make demo       # Build only demos
make clean      # Remove build artifacts
```

## Project Structure

```
mini-software-testing/
├── include/          # Header files
│   ├── unit_test_fw.h
│   ├── mock_stub.h
│   ├── tdd_bdd.h
│   ├── e2e_contract.h
│   └── mutation_testing.h
├── src/              # Implementation files
├── examples/         # Simple usage examples
├── demo/             # Full-featured demos
├── docs/             # Documentation
├── README.md
└── Makefile
```
