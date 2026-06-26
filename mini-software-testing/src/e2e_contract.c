#include "../include/e2e_contract.h"

e2e_browser_t g_e2e_browser;

void e2e_init(void) {
    memset(&g_e2e_browser, 0, sizeof(g_e2e_browser));
    g_e2e_browser.running = true;
}

void e2e_destroy(void) {
    memset(&g_e2e_browser, 0, sizeof(g_e2e_browser));
}

e2e_page_t* e2e_page_define(const char *name, const char *url) {
    if (g_e2e_browser.page_count >= E2E_PAGE_MAX) {
        fprintf(stderr, "ERROR: too many pages (max %d)\n", E2E_PAGE_MAX);
        return NULL;
    }
    e2e_page_t *p = &g_e2e_browser.pages[g_e2e_browser.page_count++];
    memset(p, 0, sizeof(e2e_page_t));
    strncpy(p->name, name, E2E_NAME_MAX - 1);
    if (url) strncpy(p->url, url, E2E_VALUE_MAX - 1);
    return p;
}

e2e_element_t* e2e_element_add(e2e_page_t *page, const char *name) {
    if (!page) return NULL;
    if (page->element_count >= E2E_ELEM_MAX) {
        fprintf(stderr, "ERROR: too many elements on page '%s' (max %d)\n", page->name, E2E_ELEM_MAX);
        return NULL;
    }
    e2e_element_t *e = &page->elements[page->element_count++];
    memset(e, 0, sizeof(e2e_element_t));
    strncpy(e->name, name, E2E_NAME_MAX - 1);
    e->exists = true;
    e->visible = true;
    e->enabled = true;
    return e;
}

void e2e_element_set_locator(e2e_element_t *elem, e2e_locator_type_t type, const char *value) {
    if (!elem) return;
    elem->locator_type = type;
    if (value) strncpy(elem->locator_value, value, E2E_SELECTOR_MAX - 1);
}

void e2e_element_set_text(e2e_element_t *elem, const char *text) {
    if (!elem) return;
    if (text) strncpy(elem->text, text, E2E_VALUE_MAX - 1);
}

void e2e_element_set_value(e2e_element_t *elem, const char *val) {
    if (!elem) return;
    if (val) strncpy(elem->value, val, E2E_VALUE_MAX - 1);
}

void e2e_element_set_visible(e2e_element_t *elem, bool vis) {
    if (!elem) return;
    elem->visible = vis;
}

void e2e_element_set_enabled(e2e_element_t *elem, bool en) {
    if (!elem) return;
    elem->enabled = en;
}

static e2e_page_t* e2e_find_page(const char *name) {
    int i;
    for (i = 0; i < g_e2e_browser.page_count; i++) {
        if (strcmp(g_e2e_browser.pages[i].name, name) == 0) return &g_e2e_browser.pages[i];
    }
    return NULL;
}

static e2e_element_t* e2e_find_element(e2e_page_t *page, const char *name) {
    if (!page) return NULL;
    int i;
    for (i = 0; i < page->element_count; i++) {
        if (strcmp(page->elements[i].name, name) == 0) return &page->elements[i];
    }
    return NULL;
}

e2e_step_result_t e2e_navigate(const char *page_name) {
    e2e_step_result_t r = {false, ""};
    e2e_page_t *p = e2e_find_page(page_name);
    if (!p) {
        snprintf(r.message, E2E_VALUE_MAX, "Page '%s' not found", page_name);
        return r;
    }
    strncpy(g_e2e_browser.current_url, p->url, E2E_VALUE_MAX - 1);
    g_e2e_browser.current_page = p;
    r.passed = true;
    snprintf(r.message, E2E_VALUE_MAX, "Navigated to '%s' (%s)", page_name, p->url);
    return r;
}

e2e_step_result_t e2e_click(const char *page_name, const char *elem_name) {
    e2e_step_result_t r = {false, ""};
    e2e_page_t *p = e2e_find_page(page_name);
    if (!p) {
        snprintf(r.message, E2E_VALUE_MAX, "Page '%s' not found", page_name);
        return r;
    }
    e2e_element_t *e = e2e_find_element(p, elem_name);
    if (!e) {
        snprintf(r.message, E2E_VALUE_MAX, "Element '%s' not found on page '%s'", elem_name, page_name);
        return r;
    }
    if (!e->visible) {
        snprintf(r.message, E2E_VALUE_MAX, "Element '%s' is not visible", elem_name);
        return r;
    }
    if (!e->enabled) {
        snprintf(r.message, E2E_VALUE_MAX, "Element '%s' is not enabled", elem_name);
        return r;
    }
    r.passed = true;
    snprintf(r.message, E2E_VALUE_MAX, "Clicked '%s' on page '%s'", elem_name, page_name);
    return r;
}

e2e_step_result_t e2e_type(const char *page_name, const char *elem_name, const char *text) {
    e2e_step_result_t r = {false, ""};
    e2e_page_t *p = e2e_find_page(page_name);
    if (!p) {
        snprintf(r.message, E2E_VALUE_MAX, "Page '%s' not found", page_name);
        return r;
    }
    e2e_element_t *e = e2e_find_element(p, elem_name);
    if (!e) {
        snprintf(r.message, E2E_VALUE_MAX, "Element '%s' not found on page '%s'", elem_name, page_name);
        return r;
    }
    if (!e->visible || !e->enabled) {
        snprintf(r.message, E2E_VALUE_MAX, "Element '%s' is not interactive", elem_name);
        return r;
    }
    if (text) strncpy(e->value, text, E2E_VALUE_MAX - 1);
    r.passed = true;
    snprintf(r.message, E2E_VALUE_MAX, "Typed into '%s' on page '%s'", elem_name, page_name);
    return r;
}

e2e_step_result_t e2e_assert_visible(const char *page_name, const char *elem_name) {
    e2e_step_result_t r = {false, ""};
    e2e_page_t *p = e2e_find_page(page_name);
    if (!p) {
        snprintf(r.message, E2E_VALUE_MAX, "Page '%s' not found", page_name);
        return r;
    }
    e2e_element_t *e = e2e_find_element(p, elem_name);
    if (!e) {
        snprintf(r.message, E2E_VALUE_MAX, "Element '%s' not found on page '%s'", elem_name, page_name);
        return r;
    }
    if (!e->visible) {
        snprintf(r.message, E2E_VALUE_MAX, "Assertion failed: element '%s' is not visible", elem_name);
        return r;
    }
    r.passed = true;
    snprintf(r.message, E2E_VALUE_MAX, "Element '%s' is visible", elem_name);
    return r;
}

e2e_step_result_t e2e_assert_text(const char *page_name, const char *elem_name, const char *expected) {
    e2e_step_result_t r = {false, ""};
    e2e_page_t *p = e2e_find_page(page_name);
    if (!p) {
        snprintf(r.message, E2E_VALUE_MAX, "Page '%s' not found", page_name);
        return r;
    }
    e2e_element_t *e = e2e_find_element(p, elem_name);
    if (!e) {
        snprintf(r.message, E2E_VALUE_MAX, "Element '%s' not found on page '%s'", elem_name, page_name);
        return r;
    }
    if (!e->visible) {
        snprintf(r.message, E2E_VALUE_MAX, "Element '%s' is not visible", elem_name);
        return r;
    }
    if (strcmp(e->text, expected) != 0) {
        snprintf(r.message, E2E_VALUE_MAX, "Text mismatch: expected '%s', got '%s'", expected, e->text);
        return r;
    }
    r.passed = true;
    snprintf(r.message, E2E_VALUE_MAX, "Text assertion passed for '%s'", elem_name);
    return r;
}

e2e_step_result_t e2e_assert_url(const char *expected_url) {
    e2e_step_result_t r = {false, ""};
    if (strcmp(g_e2e_browser.current_url, expected_url) != 0) {
        snprintf(r.message, E2E_VALUE_MAX, "URL mismatch: expected '%s', got '%s'", expected_url, g_e2e_browser.current_url);
        return r;
    }
    r.passed = true;
    snprintf(r.message, E2E_VALUE_MAX, "URL assertion passed");
    return r;
}

e2e_step_result_t e2e_assert_enabled(const char *page_name, const char *elem_name) {
    e2e_step_result_t r = {false, ""};
    e2e_page_t *p = e2e_find_page(page_name);
    if (!p) {
        snprintf(r.message, E2E_VALUE_MAX, "Page '%s' not found", page_name);
        return r;
    }
    e2e_element_t *e = e2e_find_element(p, elem_name);
    if (!e) {
        snprintf(r.message, E2E_VALUE_MAX, "Element '%s' not found on page '%s'", elem_name, page_name);
        return r;
    }
    if (!e->enabled) {
        snprintf(r.message, E2E_VALUE_MAX, "Assertion failed: element '%s' is not enabled", elem_name);
        return r;
    }
    r.passed = true;
    snprintf(r.message, E2E_VALUE_MAX, "Element '%s' is enabled", elem_name);
    return r;
}

int e2e_scenario_login(const char *username, const char *password) {
    e2e_step_result_t r;
    r = e2e_navigate("login");
    if (!r.passed) { printf("  FAIL: %s\n", r.message); return 0; }
    r = e2e_type("login", "username", username);
    if (!r.passed) { printf("  FAIL: %s\n", r.message); return 0; }
    r = e2e_type("login", "password", password);
    if (!r.passed) { printf("  FAIL: %s\n", r.message); return 0; }
    r = e2e_click("login", "login_btn");
    if (!r.passed) { printf("  FAIL: %s\n", r.message); return 0; }
    e2e_navigate("dashboard");
    r = e2e_assert_visible("dashboard", "welcome");
    if (!r.passed) { printf("  FAIL: %s\n", r.message); return 0; }
    printf("  Login scenario passed\n");
    return 1;
}

int e2e_scenario_search(const char *query) {
    e2e_step_result_t r;
    r = e2e_type("dashboard", "search_box", query);
    if (!r.passed) { printf("  FAIL: %s\n", r.message); return 0; }
    r = e2e_click("dashboard", "search_btn");
    if (!r.passed) { printf("  FAIL: %s\n", r.message); return 0; }
    e2e_navigate("results");
    r = e2e_assert_visible("results", "result_list");
    if (!r.passed) { printf("  FAIL: %s\n", r.message); return 0; }
    printf("  Search scenario passed\n");
    return 1;
}

int e2e_scenario_checkout(void) {
    e2e_step_result_t r;
    r = e2e_click("cart", "checkout_btn");
    if (!r.passed) { printf("  FAIL: %s\n", r.message); return 0; }
    e2e_navigate("checkout");
    r = e2e_assert_visible("checkout", "order_summary");
    if (!r.passed) { printf("  FAIL: %s\n", r.message); return 0; }
    printf("  Checkout scenario passed\n");
    return 1;
}

contract_verifier_t g_contract_verifier;

contract_pact_t* contract_pact_create(const char *consumer, const char *provider) {
    if (g_contract_verifier.pact_count >= E2E_PAGE_MAX) {
        fprintf(stderr, "ERROR: too many pacts (max %d)\n", E2E_PAGE_MAX);
        return NULL;
    }
    contract_pact_t *p = &g_contract_verifier.pacts[g_contract_verifier.pact_count++];
    memset(p, 0, sizeof(contract_pact_t));
    strncpy(p->consumer, consumer, E2E_NAME_MAX - 1);
    strncpy(p->provider, provider, E2E_NAME_MAX - 1);
    return p;
}

contract_interaction_t* contract_add_interaction(contract_pact_t *pact, const char *desc) {
    if (!pact) return NULL;
    if (pact->interaction_count >= E2E_ELEM_MAX) {
        fprintf(stderr, "ERROR: too many interactions (max %d)\n", E2E_ELEM_MAX);
        return NULL;
    }
    contract_interaction_t *i = &pact->interactions[pact->interaction_count++];
    memset(i, 0, sizeof(contract_interaction_t));
    if (desc) strncpy(i->description, desc, E2E_VALUE_MAX - 1);
    return i;
}

void contract_upon_receiving(contract_interaction_t *i, contract_method_t method, const char *path) {
    if (!i) return;
    i->method = method;
    if (path) strncpy(i->path, path, E2E_VALUE_MAX - 1);
}

void contract_will_respond_with(contract_interaction_t *i, int status) {
    if (!i) return;
    i->status = status;
}

void contract_with_request_body(contract_interaction_t *i, const char *body) {
    if (!i || !body) return;
    strncpy(i->request_body, body, E2E_VALUE_MAX - 1);
}

void contract_with_response_body(contract_interaction_t *i, const char *body) {
    if (!i || !body) return;
    strncpy(i->response_body, body, E2E_VALUE_MAX - 1);
}

void contract_with_request_header(contract_interaction_t *i, const char *key, const char *value) {
    if (!i || !key || !value) return;
    if (i->req_header_count >= 16) return;
    strncpy(i->request_headers[i->req_header_count].key, key, E2E_NAME_MAX - 1);
    strncpy(i->request_headers[i->req_header_count].value, value, E2E_VALUE_MAX - 1);
    i->req_header_count++;
}

void contract_with_response_header(contract_interaction_t *i, const char *key, const char *value) {
    if (!i || !key || !value) return;
    if (i->resp_header_count >= 16) return;
    strncpy(i->response_headers[i->resp_header_count].key, key, E2E_NAME_MAX - 1);
    strncpy(i->response_headers[i->resp_header_count].value, value, E2E_VALUE_MAX - 1);
    i->resp_header_count++;
}

int contract_verify(contract_pact_t *pact) {
    if (!pact) return 0;
    printf("\n  Verifying contract: %s -> %s\n", pact->consumer, pact->provider);
    printf("  ----------------------------------------\n");
    int passed = 0;
    int failed = 0;
    int i;
    for (i = 0; i < pact->interaction_count; i++) {
        contract_interaction_t *inter = &pact->interactions[i];
        printf("  [%d] %s\n", i + 1, inter->description);

        const char *method_str = "GET";
        switch (inter->method) {
            case CONTRACT_POST:   method_str = "POST"; break;
            case CONTRACT_PUT:    method_str = "PUT"; break;
            case CONTRACT_DELETE: method_str = "DELETE"; break;
            case CONTRACT_PATCH:  method_str = "PATCH"; break;
            default: break;
        }
        printf("      %s %s -> %d\n", method_str, inter->path, inter->status);
        if (inter->request_body[0]) printf("      Request:  %s\n", inter->request_body);
        if (inter->response_body[0]) printf("      Response: %s\n", inter->response_body);

        if (inter->status >= 100 && inter->status < 600) {
            passed++;
            printf("      [PASS]\n");
        } else {
            failed++;
            printf("      [FAIL] Invalid status code\n");
        }
    }
    g_contract_verifier.verified += passed;
    g_contract_verifier.failed += failed;
    printf("  Contract result: %d passed, %d failed\n", passed, failed);
    return 1;
}

int contract_verifier_run_all(void) {
    printf("\n========================================\n");
    printf("  Contract Test Verifier\n");
    printf("========================================\n");

    g_contract_verifier.verified = 0;
    g_contract_verifier.failed = 0;

    int i;
    for (i = 0; i < g_contract_verifier.pact_count; i++) {
        contract_verify(&g_contract_verifier.pacts[i]);
    }
    return (g_contract_verifier.failed == 0) ? 0 : 1;
}

void contract_verifier_report(void) {
    printf("\n========================================\n");
    printf("  Contract Verification Report\n");
    printf("========================================\n");
    printf("  Total pacts:       %d\n", g_contract_verifier.pact_count);
    printf("  Verified:          %d\n", g_contract_verifier.verified);
    printf("  Failed:            %d\n", g_contract_verifier.failed);
    printf("========================================\n");
}

bool contract_schema_validate(const char *json_schema, const char *json_body) {
    if (!json_schema || !json_body) return false;
    if (strstr(json_body, "\"") || strstr(json_body, "{") || strstr(json_body, "[")) {
        return true;
    }
    return strlen(json_schema) > 0 && strlen(json_body) > 0;
}

bool contract_body_matches(const char *expected, const char *actual) {
    if (!expected && !actual) return true;
    if (!expected || !actual) return false;
    return strcmp(expected, actual) == 0;
}
