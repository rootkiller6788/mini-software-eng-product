#ifndef E2E_CONTRACT_H
#define E2E_CONTRACT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define E2E_PAGE_MAX     32
#define E2E_ELEM_MAX     128
#define E2E_NAME_MAX     256
#define E2E_SELECTOR_MAX 128
#define E2E_VALUE_MAX    512

typedef enum {
    LOCATOR_ID,
    LOCATOR_CLASS,
    LOCATOR_NAME,
    LOCATOR_TAG,
    LOCATOR_XPATH,
    LOCATOR_CSS,
    LOCATOR_LINK_TEXT,
    LOCATOR_PARTIAL_LINK
} e2e_locator_type_t;

typedef struct e2e_element {
    char              name[E2E_NAME_MAX];
    e2e_locator_type_t locator_type;
    char              locator_value[E2E_SELECTOR_MAX];
    bool              exists;
    bool              visible;
    bool              enabled;
    char              text[E2E_VALUE_MAX];
    char              value[E2E_VALUE_MAX];
} e2e_element_t;

typedef struct e2e_page {
    char          name[E2E_NAME_MAX];
    char          url[E2E_VALUE_MAX];
    e2e_element_t elements[E2E_ELEM_MAX];
    int           element_count;
} e2e_page_t;

typedef struct e2e_browser {
    e2e_page_t       pages[E2E_PAGE_MAX];
    int              page_count;
    e2e_page_t      *current_page;
    char             current_url[E2E_VALUE_MAX];
    bool             running;
} e2e_browser_t;

typedef struct e2e_step_result {
    bool passed;
    char message[E2E_VALUE_MAX];
} e2e_step_result_t;

extern e2e_browser_t g_e2e_browser;

void    e2e_init(void);
void    e2e_destroy(void);

e2e_page_t* e2e_page_define(const char *name, const char *url);
e2e_element_t* e2e_element_add(e2e_page_t *page, const char *name);
void    e2e_element_set_locator(e2e_element_t *elem, e2e_locator_type_t type, const char *value);
void    e2e_element_set_text(e2e_element_t *elem, const char *text);
void    e2e_element_set_value(e2e_element_t *elem, const char *val);
void    e2e_element_set_visible(e2e_element_t *elem, bool vis);
void    e2e_element_set_enabled(e2e_element_t *elem, bool en);

e2e_step_result_t e2e_navigate(const char *page_name);
e2e_step_result_t e2e_click(const char *page_name, const char *elem_name);
e2e_step_result_t e2e_type(const char *page_name, const char *elem_name, const char *text);
e2e_step_result_t e2e_assert_visible(const char *page_name, const char *elem_name);
e2e_step_result_t e2e_assert_text(const char *page_name, const char *elem_name, const char *expected);
e2e_step_result_t e2e_assert_url(const char *expected_url);
e2e_step_result_t e2e_assert_enabled(const char *page_name, const char *elem_name);

int     e2e_scenario_login(const char *username, const char *password);
int     e2e_scenario_search(const char *query);
int     e2e_scenario_checkout(void);

typedef enum {
    CONTRACT_GET,
    CONTRACT_POST,
    CONTRACT_PUT,
    CONTRACT_DELETE,
    CONTRACT_PATCH
} contract_method_t;

typedef struct contract_header {
    char key[E2E_NAME_MAX];
    char value[E2E_VALUE_MAX];
} contract_header_t;

typedef struct contract_interaction {
    char               description[E2E_VALUE_MAX];
    contract_method_t  method;
    char               path[E2E_VALUE_MAX];
    int                status;
    contract_header_t  request_headers[16];
    int                req_header_count;
    char               request_body[E2E_VALUE_MAX];
    contract_header_t  response_headers[16];
    int                resp_header_count;
    char               response_body[E2E_VALUE_MAX];
} contract_interaction_t;

typedef struct contract_pact {
    char                   consumer[E2E_NAME_MAX];
    char                   provider[E2E_NAME_MAX];
    contract_interaction_t interactions[E2E_ELEM_MAX];
    int                    interaction_count;
} contract_pact_t;

typedef struct contract_verifier {
    contract_pact_t       pacts[E2E_PAGE_MAX];
    int                   pact_count;
    int                   verified;
    int                   failed;
} contract_verifier_t;

extern contract_verifier_t g_contract_verifier;

contract_pact_t* contract_pact_create(const char *consumer, const char *provider);
contract_interaction_t* contract_add_interaction(contract_pact_t *pact, const char *desc);
void    contract_given(contract_interaction_t *i, const char *provider_state);
void    contract_upon_receiving(contract_interaction_t *i, contract_method_t method, const char *path);
void    contract_will_respond_with(contract_interaction_t *i, int status);
void    contract_with_request_body(contract_interaction_t *i, const char *body);
void    contract_with_response_body(contract_interaction_t *i, const char *body);
void    contract_with_request_header(contract_interaction_t *i, const char *key, const char *value);
void    contract_with_response_header(contract_interaction_t *i, const char *key, const char *value);

int     contract_verify(contract_pact_t *pact);
int     contract_verifier_run_all(void);
void    contract_verifier_report(void);

bool    contract_schema_validate(const char *json_schema, const char *json_body);
bool    contract_body_matches(const char *expected, const char *actual);

#endif
