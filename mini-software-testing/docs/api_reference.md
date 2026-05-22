# API Reference

## unit_test_fw.h — Unit Test Framework

### Types

| Type | Description |
|------|-------------|
| `utfw_result_t` | Enum: `UTFW_PASS`, `UTFW_FAIL`, `UTFW_SKIP`, `UTFW_ERROR` |
| `utfw_case_t` | Test case struct (name, file, line, func, result, message, elapsed_ms) |
| `utfw_suite_t` | Test suite struct (name, setup, teardown, cases[], passed, failed, skipped) |
| `utfw_runner_t` | Global test runner (suites[], counts, timing) |

### Global State

```c
extern utfw_runner_t g_ut_runner;
```

### Functions

```c
void            utfw_init(void);
utfw_suite_t*   utfw_register_suite(const char *name, void (*setup)(), void (*teardown)());
utfw_case_t*    utfw_register_case(utfw_suite_t *suite, const char *name, const char *file, int line, void (*func)());
void            utfw_run_all(void);
void            utfw_report(void);
int             utfw_exit_code(void);
void            utfw_record_pass(void);
void            utfw_record_fail(const char *file, int line, const char *msg);
void            utfw_record_skip(const char *msg);
void            utfw_record_error(const char *file, int line, const char *msg);
```

### Macros

```c
TEST(name)                          // Define a test function
ASSERT_TRUE(expr)                   // Assert boolean true
ASSERT_FALSE(expr)                  // Assert boolean false
ASSERT_NULL(ptr)                    // Assert pointer is NULL
ASSERT_NOT_NULL(ptr)                // Assert pointer is not NULL
ASSERT_EQ(a, b)                     // Assert integer equality
ASSERT_NEQ(a, b)                    // Assert integer inequality
ASSERT_STREQ(a, b)                  // Assert string equality
ASSERT_STRNEQ(a, b)                 // Assert string inequality
ASSERT_DOUBLE_EQ(a, b, epsilon)     // Assert double near-equality
ASSERT_FAIL(msg)                    // Unconditional failure
SKIP(reason)                        // Skip current test
RUN_TEST(suite, name, func)         // Register and run a test
SETUP(func)                         // Cast setup function
TEARDOWN(func)                      // Cast teardown function
```

---

## mock_stub.h — Test Doubles

### Types

| Type | Description |
|------|-------------|
| `mock_arg_type_t` | Enum: `MOCK_ARG_INT`, `MOCK_ARG_LONG`, `MOCK_ARG_FLOAT`, `MOCK_ARG_DOUBLE`, `MOCK_ARG_STRING`, `MOCK_ARG_PTR`, `MOCK_ARG_BOOL` |
| `mock_arg_t` | Argument wrapper (type + union value) |
| `mock_call_t` | Recorded function call |
| `mock_expect_t` | Expected call definition |
| `spy_call_t` | Spy-recorded call |
| `mock_ctx_t` | Mock context (expectations, calls, spies) |
| `stub_entry_t` | Stub entry |
| `fake_store_t` | Fake data store entry |

### Global State

```c
extern mock_ctx_t *g_mock_ctx;
```

### Mock Functions

```c
void            mock_init(void);
void            mock_destroy(void);
mock_expect_t*  mock_expect_call(const char *func_name, int times);
mock_expect_t*  mock_expect_any(const char *func_name);
void            mock_set_return_int(mock_expect_t *exp, int val);
void            mock_set_return_double(mock_expect_t *exp, double val);
void            mock_set_return_string(mock_expect_t *exp, const char *val);
void            mock_set_return_ptr(mock_expect_t *exp, void *val);
void            mock_set_return_bool(mock_expect_t *exp, bool val);
void            mock_set_error(mock_expect_t *exp, int error_code);
void            mock_record_call(const char *func_name, int argc, ...);
int             mock_verify(void);
int             mock_verify_count(const char *func_name);
bool            mock_arg_match_int(int expected, int actual);
bool            mock_arg_match_string(const char *expected, const char *actual);
bool            mock_arg_match_any(void);
```

### Stub Functions

```c
void            stub_register(const char *func_name, void *func_ptr);
void            stub_set_default_int(const char *func_name, int val);
void            stub_set_default_string(const char *func_name, const char *val);
int             stub_call_int(const char *func_name);
char*           stub_call_string(const char *func_name);
void*           stub_call_ptr(const char *func_name);
```

### Spy Functions

```c
void            spy_record(const char *func_name, int argc, ...);
int             spy_get_count(const char *func_name);
spy_call_t*     spy_get_call(const char *func_name, int index);
```

### Fake Functions

```c
void            fake_put_int(const char *key, int val);
int             fake_get_int(const char *key);
void            fake_put_string(const char *key, const char *val);
char*           fake_get_string(const char *key);
void            fake_clear(void);
```

---

## tdd_bdd.h — TDD & BDD

### Types

| Type | Description |
|------|-------------|
| `tdd_phase_t` | TDD phases: `TDD_RED`, `TDD_GREEN`, `TDD_REFACTOR` |
| `tdd_step_type_t` | Step types: `TDD_STEP_GIVEN`, `TDD_STEP_WHEN`, `TDD_STEP_THEN`, `TDD_STEP_AND`, `TDD_STEP_BUT` |
| `tdd_step_t` | Step struct (type, description, action) |
| `tdd_scenario_t` | Scenario struct (name, steps, passed flag) |
| `tdd_feature_t` | Feature struct (name, description, scenarios) |
| `tdd_runner_t` | BDD runner (features, phase, counts) |

### Global State

```c
extern tdd_runner_t g_tdd_runner;
```

### Functions

```c
void            tdd_init(void);
tdd_feature_t*  tdd_feature(const char *name, const char *description);
tdd_scenario_t* tdd_scenario(const char *name);
void            tdd_given(const char *description);
void            tdd_when(const char *description);
void            tdd_then(const char *description);
void            tdd_and(const char *description);
void            tdd_but(const char *description);
void            tdd_step_given(const char *description, void (*action)());
void            tdd_step_when(const char *description, void (*action)());
void            tdd_step_then(const char *description, void (*action)());
void            tdd_step_and(const char *description, void (*action)());
void            tdd_step_but(const char *description, void (*action)());
void            tdd_assert_true(bool condition, const char *msg);
void            tdd_assert_eq_int(int expected, int actual, const char *msg);
void            tdd_assert_eq_str(const char *expected, const char *actual, const char *msg);
void            tdd_set_phase(tdd_phase_t phase);
tdd_phase_t     tdd_get_phase(void);
void            tdd_run_all(void);
void            tdd_report(void);
int             tdd_exit_code(void);
void            tdd_generate_living_doc(const char *filepath);
```

### Macros

```c
FEATURE(name, desc)         // Define a feature
SCENARIO(name)              // Define a scenario
GIVEN(desc)                 // Given step (no action)
WHEN(desc)                  // When step (no action)
THEN(desc)                  // Then step (no action)
AND(desc)                   // And step (no action)
BUT(desc)                   // But step (no action)
GIVEN_STEP(desc, action)    // Given step with action function
WHEN_STEP(desc, action)     // When step with action function
THEN_STEP(desc, action)     // Then step with action function
AND_STEP(desc, action)      // And step with action function
BUT_STEP(desc, action)      // But step with action function
SHOW(expr)                  // Print expression value
PENDING(msg)                // Mark as pending
```

---

## e2e_contract.h — E2E & Contract Testing

### Types (E2E)

| Type | Description |
|------|-------------|
| `e2e_locator_type_t` | `LOCATOR_ID`, `LOCATOR_CLASS`, `LOCATOR_NAME`, `LOCATOR_TAG`, `LOCATOR_XPATH`, `LOCATOR_CSS`, `LOCATOR_LINK_TEXT`, `LOCATOR_PARTIAL_LINK` |
| `e2e_element_t` | Page element (name, locator, text, value, visible, enabled) |
| `e2e_page_t` | Page (name, url, elements) |
| `e2e_browser_t` | Browser context (pages, current page, url) |
| `e2e_step_result_t` | Step result (passed, message) |

### Types (Contract)

| Type | Description |
|------|-------------|
| `contract_method_t` | `CONTRACT_GET`, `CONTRACT_POST`, `CONTRACT_PUT`, `CONTRACT_DELETE`, `CONTRACT_PATCH` |
| `contract_header_t` | HTTP header key-value |
| `contract_interaction_t` | Contract interaction (method, path, status, bodies, headers) |
| `contract_pact_t` | Pact (consumer, provider, interactions) |
| `contract_verifier_t` | Verifier (pacts, verified/failed counts) |

### Global State

```c
extern e2e_browser_t        g_e2e_browser;
extern contract_verifier_t  g_contract_verifier;
```

### E2E Functions

```c
void            e2e_init(void);
void            e2e_destroy(void);
e2e_page_t*     e2e_page_define(const char *name, const char *url);
e2e_element_t*  e2e_element_add(e2e_page_t *page, const char *name);
void            e2e_element_set_locator(e2e_element_t *elem, e2e_locator_type_t type, const char *value);
void            e2e_element_set_text(e2e_element_t *elem, const char *text);
void            e2e_element_set_value(e2e_element_t *elem, const char *val);
void            e2e_element_set_visible(e2e_element_t *elem, bool vis);
void            e2e_element_set_enabled(e2e_element_t *elem, bool en);
e2e_step_result_t e2e_navigate(const char *page_name);
e2e_step_result_t e2e_click(const char *page_name, const char *elem_name);
e2e_step_result_t e2e_type(const char *page_name, const char *elem_name, const char *text);
e2e_step_result_t e2e_assert_visible(const char *page_name, const char *elem_name);
e2e_step_result_t e2e_assert_text(const char *page_name, const char *elem_name, const char *expected);
e2e_step_result_t e2e_assert_url(const char *expected_url);
e2e_step_result_t e2e_assert_enabled(const char *page_name, const char *elem_name);
int             e2e_scenario_login(const char *username, const char *password);
int             e2e_scenario_search(const char *query);
int             e2e_scenario_checkout(void);
```

### Contract Functions

```c
contract_pact_t*        contract_pact_create(const char *consumer, const char *provider);
contract_interaction_t* contract_add_interaction(contract_pact_t *pact, const char *desc);
void            contract_upon_receiving(contract_interaction_t *i, contract_method_t method, const char *path);
void            contract_will_respond_with(contract_interaction_t *i, int status);
void            contract_with_request_body(contract_interaction_t *i, const char *body);
void            contract_with_response_body(contract_interaction_t *i, const char *body);
void            contract_with_request_header(contract_interaction_t *i, const char *key, const char *value);
void            contract_with_response_header(contract_interaction_t *i, const char *key, const char *value);
int             contract_verify(contract_pact_t *pact);
int             contract_verifier_run_all(void);
void            contract_verifier_report(void);
bool            contract_schema_validate(const char *json_schema, const char *json_body);
bool            contract_body_matches(const char *expected, const char *actual);
```

---

## mutation_testing.h — Mutation Testing

### Types

| Type | Description |
|------|-------------|
| `mut_operator_t` | 32 mutation operators (arithmetic, relational, logical, conditional, etc.) |
| `mut_source_line_t` | Source code line |
| `mut_mutant_t` | Mutant (id, operator, original/mutated text, killed/survived/equiv) |
| `mut_config_t` | Configuration (enabled operators, source files) |
| `mut_runner_t` | Runner (config, mutants, score) |

### Global State

```c
extern mut_runner_t g_mut_runner;
```

### Functions

```c
void            mut_init(void);
void            mut_config_init(mut_config_t *config);
void            mut_config_enable_operator(mut_config_t *config, mut_operator_t op);
void            mut_config_add_source(mut_config_t *config, const char *filepath);
void            mut_generate_mutants(mut_runner_t *runner);
int             mut_run_tests_on_mutant(mut_runner_t *runner, mut_mutant_t *mutant, int (*test_func)());
void            mut_record_killed(mut_runner_t *runner, int mutant_id);
void            mut_record_survived(mut_runner_t *runner, int mutant_id);
void            mut_record_equivalent(mut_runner_t *runner, int mutant_id);
double          mut_calculate_score(mut_runner_t *runner);
void            mut_report(mut_runner_t *runner);
void            mut_report_detailed(mut_runner_t *runner);
const char*     mut_operator_name(mut_operator_t op);
const char*     mut_operator_description(mut_operator_t op);
bool            mut_is_equivalent(int mutant_id);
mut_mutant_t*   mut_find_mutant(mut_runner_t *runner, int id);
```

### All Mutation Operators

| Constant | Description |
|----------|-------------|
| `MUT_ARITHMETIC_ADD_TO_SUB` | `+` → `-` |
| `MUT_ARITHMETIC_SUB_TO_ADD` | `-` → `+` |
| `MUT_ARITHMETIC_MUL_TO_DIV` | `*` → `/` |
| `MUT_ARITHMETIC_DIV_TO_MUL` | `/` → `*` |
| `MUT_ARITHMETIC_MOD_TO_MUL` | `%` → `*` |
| `MUT_RELATIONAL_GT_TO_GTE` | `>` → `>=` |
| `MUT_RELATIONAL_GTE_TO_GT` | `>=` → `>` |
| `MUT_RELATIONAL_LT_TO_LTE` | `<` → `<=` |
| `MUT_RELATIONAL_LTE_TO_LT` | `<=` → `<` |
| `MUT_RELATIONAL_EQ_TO_NEQ` | `==` → `!=` |
| `MUT_RELATIONAL_NEQ_TO_EQ` | `!=` → `==` |
| `MUT_LOGICAL_AND_TO_OR` | `&&` → `\|\|` |
| `MUT_LOGICAL_OR_TO_AND` | `\|\|` → `&&` |
| `MUT_CONDITIONAL_TRUE` | `if(cond)` → `if(true)` |
| `MUT_CONDITIONAL_FALSE` | `if(cond)` → `if(false)` |
| `MUT_INCREMENT_TO_DECREMENT` | `++` → `--` |
| `MUT_DECREMENT_TO_INCREMENT` | `--` → `++` |
| `MUT_ASSIGNMENT_ADD_TO_SUB` | `+=` → `-=` |
| `MUT_ASSIGNMENT_SUB_TO_ADD` | `-=` → `+=` |
| `MUT_RETURN_REMOVE` | Remove return statement |
| `MUT_CONSTANT_ZERO` | Replace constant with 0 |
| `MUT_CONSTANT_ONE` | Replace constant with 1 |
| `MUT_CONSTANT_NEG_ONE` | Replace constant with -1 |
| `MUT_NEGATE_CONDITIONAL` | `!` negation |
| `MUT_BITWISE_AND_TO_OR` | `&` → `\|` |
| `MUT_BITWISE_OR_TO_AND` | `\|` → `&` |
| `MUT_BITWISE_XOR_TO_AND` | `^` → `&` |
| `MUT_SHIFT_LEFT_TO_RIGHT` | `<<` → `>>` |
| `MUT_SHIFT_RIGHT_TO_LEFT` | `>>` → `<<` |
| `MUT_REMOVE_VOID_CALL` | Remove void function call |
| `MUT_SWAP_BRANCH` | Swap if/else branches |
