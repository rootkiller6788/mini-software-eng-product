#include "../include/mock_stub.h"
#include "../include/unit_test_fw.h"
#include <assert.h>

static int s_db_result = 0;
static int s_send_count = 0;

static int db_query_int(const char *query) {
    return s_db_result;
}

static int send_notification(const char *msg) {
    s_send_count++;
    return 0;
}

static int process_payment(double amount) {
    (void)amount;
    return 1;
}

static int get_user_id(const char *username) {
    if (strcmp(username, "admin") == 0) return 1;
    if (strcmp(username, "guest") == 0) return 2;
    return -1;
}

TEST(test_stub_hardcoded_response) {
    s_db_result = 42;
    int result = db_query_int("SELECT *");
    ASSERT_EQ(result, 42);

    s_db_result = -1;
    result = db_query_int("SELECT *");
    ASSERT_EQ(result, -1);
}

TEST(test_mock_expect_call) {
    mock_init();

    mock_expect_t *exp = mock_expect_call("send_notification", 1);
    mock_set_return_int(exp, 0);

    int rc = send_notification("Hello");
    ASSERT_EQ(rc, 0);

    int failures = mock_verify();
    ASSERT_EQ(failures, 0);

    mock_destroy();
}

TEST(test_mock_wrong_call_count) {
    mock_init();

    mock_expect_call("send_notification", 3);

    send_notification("msg1");
    send_notification("msg2");

    int failures = mock_verify();
    ASSERT_EQ(failures, 1);

    mock_destroy();
}

TEST(test_spy_record_calls) {
    mock_init();

    s_send_count = 0;
    send_notification("alpha");
    send_notification("beta");
    send_notification("gamma");

    spy_record("send_notification", 1, MOCK_ARG_STRING, "alpha");
    spy_record("send_notification", 1, MOCK_ARG_STRING, "beta");
    spy_record("send_notification", 1, MOCK_ARG_STRING, "gamma");

    int count = spy_get_count("send_notification");
    ASSERT_EQ(count, 3);
    ASSERT_EQ(s_send_count, 3);

    mock_destroy();
}

TEST(test_fake_store) {
    fake_put_int("user_count", 100);
    ASSERT_EQ(fake_get_int("user_count"), 100);

    fake_put_int("user_count", 200);
    ASSERT_EQ(fake_get_int("user_count"), 200);

    ASSERT_EQ(fake_get_int("nonexistent"), 0);

    fake_put_string("last_user", "john_doe");
    ASSERT_STREQ(fake_get_string("last_user"), "john_doe");

    fake_clear();
    ASSERT_EQ(fake_get_int("user_count"), 0);
    ASSERT_NULL(fake_get_string("last_user"));
}

TEST(test_argument_matchers) {
    ASSERT_EQ(mock_arg_match_int(10, 10), (int)true);
    ASSERT_EQ(mock_arg_match_int(10, 20), (int)false);

    ASSERT_EQ(mock_arg_match_string("abc", "abc"), (int)true);
    ASSERT_EQ(mock_arg_match_string("abc", "xyz"), (int)false);

    ASSERT_EQ((int)mock_arg_match_any(), (int)true);
}

TEST(test_mock_verify_count) {
    mock_init();

    send_notification("a");
    send_notification("b");

    int cnt = mock_verify_count("send_notification");
    printf("  send_notification called %d times\n", cnt);

    mock_destroy();
}

int main(void) {
    utfw_init();

    utfw_suite_t *suite = utfw_register_suite("MockStub", NULL, NULL);
    utfw_register_case(suite, "stub_hardcoded_response", __FILE__, __LINE__, test_stub_hardcoded_response);
    utfw_register_case(suite, "mock_expect_call", __FILE__, __LINE__, test_mock_expect_call);
    utfw_register_case(suite, "mock_wrong_call_count", __FILE__, __LINE__, test_mock_wrong_call_count);
    utfw_register_case(suite, "spy_record_calls", __FILE__, __LINE__, test_spy_record_calls);
    utfw_register_case(suite, "fake_store", __FILE__, __LINE__, test_fake_store);
    utfw_register_case(suite, "argument_matchers", __FILE__, __LINE__, test_argument_matchers);
    utfw_register_case(suite, "mock_verify_count", __FILE__, __LINE__, test_mock_verify_count);

    utfw_run_all();
    utfw_report();

    return utfw_exit_code();
}
