#include "include/e2e_contract.h"
#include "include/unit_test_fw.h"

static void build_login_page(void) {
    e2e_page_t *page = e2e_page_define("login", "https://app.example.com/login");
    e2e_element_t *username = e2e_element_add(page, "username");
    e2e_element_set_locator(username, LOCATOR_ID, "user-input");
    e2e_element_set_visible(username, true);
    e2e_element_set_enabled(username, true);

    e2e_element_t *password = e2e_element_add(page, "password");
    e2e_element_set_locator(password, LOCATOR_ID, "pass-input");
    e2e_element_set_visible(password, true);
    e2e_element_set_enabled(password, true);

    e2e_element_t *login_btn = e2e_element_add(page, "login_btn");
    e2e_element_set_locator(login_btn, LOCATOR_ID, "login-button");
    e2e_element_set_visible(login_btn, true);
    e2e_element_set_enabled(login_btn, true);

    e2e_element_t *error_msg = e2e_element_add(page, "error_msg");
    e2e_element_set_locator(error_msg, LOCATOR_CLASS, "error");
    e2e_element_set_visible(error_msg, false);
}

static void build_dashboard_page(void) {
    e2e_page_t *page = e2e_page_define("dashboard", "https://app.example.com/dashboard");
    e2e_element_t *welcome = e2e_element_add(page, "welcome");
    e2e_element_set_locator(welcome, LOCATOR_ID, "welcome-msg");
    e2e_element_set_text(welcome, "Welcome, User!");
    e2e_element_set_visible(welcome, true);
    e2e_element_set_enabled(welcome, true);

    e2e_element_t *search_box = e2e_element_add(page, "search_box");
    e2e_element_set_locator(search_box, LOCATOR_ID, "search-input");
    e2e_element_set_visible(search_box, true);
    e2e_element_set_enabled(search_box, true);

    e2e_element_t *search_btn = e2e_element_add(page, "search_btn");
    e2e_element_set_locator(search_btn, LOCATOR_ID, "search-btn");
    e2e_element_set_visible(search_btn, true);
    e2e_element_set_enabled(search_btn, true);

    e2e_element_t *logout_btn = e2e_element_add(page, "logout_btn");
    e2e_element_set_locator(logout_btn, LOCATOR_ID, "logout-btn");
    e2e_element_set_visible(logout_btn, true);
    e2e_element_set_enabled(logout_btn, true);
}

static void build_results_page(void) {
    e2e_page_t *page = e2e_page_define("results", "https://app.example.com/results");
    e2e_element_t *result_list = e2e_element_add(page, "result_list");
    e2e_element_set_locator(result_list, LOCATOR_ID, "result-list");
    e2e_element_set_visible(result_list, true);
    e2e_element_set_enabled(result_list, true);

    e2e_element_t *no_results = e2e_element_add(page, "no_results");
    e2e_element_set_locator(no_results, LOCATOR_CLASS, "empty-state");
    e2e_element_set_text(no_results, "No results found");
    e2e_element_set_visible(no_results, false);
}

static void build_checkout_page(void) {
    e2e_page_t *page = e2e_page_define("checkout", "https://app.example.com/checkout");
    e2e_element_t *order_summary = e2e_element_add(page, "order_summary");
    e2e_element_set_locator(order_summary, LOCATOR_ID, "order-summary");
    e2e_element_set_visible(order_summary, true);
    e2e_element_set_enabled(order_summary, true);

    e2e_element_t *pay_btn = e2e_element_add(page, "pay_btn");
    e2e_element_set_locator(pay_btn, LOCATOR_ID, "pay-button");
    e2e_element_set_visible(pay_btn, true);
    e2e_element_set_enabled(pay_btn, true);
}

static void build_cart_page(void) {
    e2e_page_t *page = e2e_page_define("cart", "https://app.example.com/cart");
    e2e_element_t *checkout_btn = e2e_element_add(page, "checkout_btn");
    e2e_element_set_locator(checkout_btn, LOCATOR_ID, "checkout-btn");
    e2e_element_set_visible(checkout_btn, true);
    e2e_element_set_enabled(checkout_btn, true);
}

TEST(test_e2e_navigation) {
    e2e_init();

    build_login_page();
    build_dashboard_page();

    e2e_step_result_t r = e2e_navigate("login");
    ASSERT_EQ((int)r.passed, (int)true);

    r = e2e_assert_url("https://app.example.com/login");
    ASSERT_EQ((int)r.passed, (int)true);

    e2e_destroy();
}

TEST(test_e2e_element_visibility) {
    e2e_init();

    build_dashboard_page();
    e2e_navigate("dashboard");

    e2e_step_result_t r = e2e_assert_visible("dashboard", "welcome");
    ASSERT_EQ((int)r.passed, (int)true);

    r = e2e_assert_enabled("dashboard", "welcome");
    ASSERT_EQ((int)r.passed, (int)true);

    e2e_destroy();
}

TEST(test_e2e_type_and_click) {
    e2e_init();

    build_login_page();
    e2e_navigate("login");

    e2e_step_result_t r = e2e_type("login", "username", "admin");
    ASSERT_EQ((int)r.passed, (int)true);

    r = e2e_type("login", "password", "secret123");
    ASSERT_EQ((int)r.passed, (int)true);

    r = e2e_click("login", "login_btn");
    ASSERT_EQ((int)r.passed, (int)true);

    e2e_destroy();
}

TEST(test_e2e_login_scenario) {
    e2e_init();

    build_login_page();
    build_dashboard_page();

    int result = e2e_scenario_login("admin", "password123");
    ASSERT_EQ(result, 1);

    e2e_destroy();
}

TEST(test_e2e_search_scenario) {
    e2e_init();

    build_dashboard_page();
    build_results_page();

    e2e_navigate("dashboard");
    int result = e2e_scenario_search("test query");
    ASSERT_EQ(result, 1);

    e2e_destroy();
}

TEST(test_e2e_checkout_flow) {
    e2e_init();

    build_dashboard_page();
    build_cart_page();
    build_checkout_page();

    e2e_navigate("dashboard");
    e2e_navigate("cart");
    int result = e2e_scenario_checkout();
    ASSERT_EQ(result, 1);

    e2e_destroy();
}

TEST(test_e2e_assert_text) {
    e2e_init();

    build_dashboard_page();
    e2e_navigate("dashboard");

    e2e_step_result_t r = e2e_assert_text("dashboard", "welcome", "Welcome, User!");
    ASSERT_EQ((int)r.passed, (int)true);

    e2e_destroy();
}

TEST(test_e2e_invisible_element) {
    e2e_init();

    build_login_page();
    e2e_navigate("login");

    e2e_step_result_t r = e2e_assert_visible("login", "error_msg");
    ASSERT_EQ((int)r.passed, (int)false);

    e2e_destroy();
}

TEST(test_contract_consumer_definition) {
    g_contract_verifier.pact_count = 0;

    contract_pact_t *pact = contract_pact_create("ConsumerApp", "UserService");
    ASSERT_NOT_NULL(pact);

    contract_interaction_t *i1 = contract_add_interaction(pact, "Get user by ID");
    ASSERT_NOT_NULL(i1);
    contract_upon_receiving(i1, CONTRACT_GET, "/users/123");
    contract_will_respond_with(i1, 200);
    contract_with_response_body(i1, "{\"id\":123,\"name\":\"John\"}");
    contract_with_response_header(i1, "Content-Type", "application/json");

    contract_interaction_t *i2 = contract_add_interaction(pact, "Create a new user");
    ASSERT_NOT_NULL(i2);
    contract_upon_receiving(i2, CONTRACT_POST, "/users");
    contract_with_request_body(i2, "{\"name\":\"Jane\"}");
    contract_will_respond_with(i2, 201);
    contract_with_response_body(i2, "{\"id\":456,\"name\":\"Jane\"}");

    ASSERT_STREQ(pact->consumer, "ConsumerApp");
    ASSERT_STREQ(pact->provider, "UserService");
    ASSERT_EQ(pact->interaction_count, 2);
}

TEST(test_contract_verify) {
    g_contract_verifier.pact_count = 0;

    contract_pact_t *pact = contract_pact_create("MobileApp", "PaymentAPI");
    ASSERT_NOT_NULL(pact);

    contract_interaction_t *i = contract_add_interaction(pact, "Process payment");
    contract_upon_receiving(i, CONTRACT_POST, "/payments");
    contract_with_request_body(i, "{\"amount\":99.99,\"currency\":\"USD\"}");
    contract_will_respond_with(i, 200);
    contract_with_response_body(i, "{\"status\":\"success\"}");

    int result = contract_verify(pact);
    ASSERT_EQ(result, 1);
}

TEST(test_contract_multiple_pacts) {
    g_contract_verifier.pact_count = 0;

    contract_pact_t *p1 = contract_pact_create("WebApp", "AuthService");
    contract_interaction_t *i1 = contract_add_interaction(p1, "Login");
    contract_upon_receiving(i1, CONTRACT_POST, "/auth/login");
    contract_will_respond_with(i1, 200);

    contract_pact_t *p2 = contract_pact_create("MobileApp", "AuthService");
    contract_interaction_t *i2 = contract_add_interaction(p2, "Refresh token");
    contract_upon_receiving(i2, CONTRACT_POST, "/auth/refresh");
    contract_will_respond_with(i2, 200);

    ASSERT_EQ(g_contract_verifier.pact_count, 2);
    ASSERT_STREQ(p1->consumer, "WebApp");
    ASSERT_STREQ(p2->consumer, "MobileApp");
}

TEST(test_contract_methods) {
    g_contract_verifier.pact_count = 0;
    contract_pact_t *p = contract_pact_create("Tester", "API");

    contract_interaction_t *get = contract_add_interaction(p, "GET resource");
    contract_upon_receiving(get, CONTRACT_GET, "/resource");
    contract_will_respond_with(get, 200);
    ASSERT_EQ(get->method, CONTRACT_GET);

    contract_interaction_t *post = contract_add_interaction(p, "POST resource");
    contract_upon_receiving(post, CONTRACT_POST, "/resource");
    contract_will_respond_with(post, 201);
    ASSERT_EQ(post->method, CONTRACT_POST);

    contract_interaction_t *put = contract_add_interaction(p, "PUT resource");
    contract_upon_receiving(put, CONTRACT_PUT, "/resource/1");
    contract_will_respond_with(put, 200);
    ASSERT_EQ(put->method, CONTRACT_PUT);

    contract_interaction_t *del = contract_add_interaction(p, "DELETE resource");
    contract_upon_receiving(del, CONTRACT_DELETE, "/resource/1");
    contract_will_respond_with(del, 204);
    ASSERT_EQ(del->method, CONTRACT_DELETE);

    ASSERT_EQ(p->interaction_count, 4);

    contract_verifier_run_all();
    contract_verifier_report();
}

TEST(test_schema_validation) {
    const char *schema = "{\"type\":\"object\",\"properties\":{\"name\":{\"type\":\"string\"}}}";
    const char *valid_body = "{\"name\":\"John\"}";
    bool valid = contract_schema_validate(schema, valid_body);
    ASSERT_EQ((int)valid, (int)true);

    bool invalid = contract_schema_validate("", "");
    ASSERT_EQ((int)invalid, (int)false);
}

TEST(test_body_matching) {
    const char *expected = "{\"status\":\"ok\"}";
    const char *actual_same = "{\"status\":\"ok\"}";
    const char *actual_diff = "{\"status\":\"error\"}";

    ASSERT_EQ((int)contract_body_matches(expected, actual_same), (int)true);
    ASSERT_EQ((int)contract_body_matches(expected, actual_diff), (int)false);
    ASSERT_EQ((int)contract_body_matches(NULL, NULL), (int)true);
}

int main(void) {
    utfw_init();

    utfw_suite_t *e2e = utfw_register_suite("E2E_Testing", NULL, NULL);
    utfw_register_case(e2e, "e2e_navigation", __FILE__, __LINE__, test_e2e_navigation);
    utfw_register_case(e2e, "e2e_element_visibility", __FILE__, __LINE__, test_e2e_element_visibility);
    utfw_register_case(e2e, "e2e_type_and_click", __FILE__, __LINE__, test_e2e_type_and_click);
    utfw_register_case(e2e, "e2e_login_scenario", __FILE__, __LINE__, test_e2e_login_scenario);
    utfw_register_case(e2e, "e2e_search_scenario", __FILE__, __LINE__, test_e2e_search_scenario);
    utfw_register_case(e2e, "e2e_checkout_flow", __FILE__, __LINE__, test_e2e_checkout_flow);
    utfw_register_case(e2e, "e2e_assert_text", __FILE__, __LINE__, test_e2e_assert_text);
    utfw_register_case(e2e, "e2e_invisible_element", __FILE__, __LINE__, test_e2e_invisible_element);

    utfw_suite_t *contract = utfw_register_suite("Contract_Testing", NULL, NULL);
    utfw_register_case(contract, "consumer_definition", __FILE__, __LINE__, test_contract_consumer_definition);
    utfw_register_case(contract, "contract_verify", __FILE__, __LINE__, test_contract_verify);
    utfw_register_case(contract, "multiple_pacts", __FILE__, __LINE__, test_contract_multiple_pacts);
    utfw_register_case(contract, "contract_methods", __FILE__, __LINE__, test_contract_methods);
    utfw_register_case(contract, "schema_validation", __FILE__, __LINE__, test_schema_validation);
    utfw_register_case(contract, "body_matching", __FILE__, __LINE__, test_body_matching);

    utfw_run_all();
    utfw_report();

    return utfw_exit_code();
}
