#ifndef UNIT_TEST_FW_H
#define UNIT_TEST_FW_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>

#define UTFW_NAME_MAX    256
#define UTFW_MSG_MAX     512
#define UTFW_SUITES_MAX  64
#define UTFW_CASES_MAX   256
#define UTFW_ASSERT_MAX  128

typedef enum {
    UTFW_PASS,
    UTFW_FAIL,
    UTFW_SKIP,
    UTFW_ERROR
} utfw_result_t;

typedef struct utfw_case {
    char          name[UTFW_NAME_MAX];
    char          file[UTFW_NAME_MAX];
    int           line;
    void        (*func)(void);
    utfw_result_t result;
    char          message[UTFW_MSG_MAX];
    double        elapsed_ms;
} utfw_case_t;

typedef struct utfw_suite {
    char        name[UTFW_NAME_MAX];
    void      (*setup)(void);
    void      (*teardown)(void);
    utfw_case_t cases[UTFW_CASES_MAX];
    int         case_count;
    int         passed;
    int         failed;
    int         skipped;
    int         errors;
    double      elapsed_ms;
} utfw_suite_t;

typedef struct utfw_runner {
    utfw_suite_t suites[UTFW_SUITES_MAX];
    int          suite_count;
    utfw_suite_t *current_suite;
    utfw_case_t  *current_case;
    bool         current_failed;
    bool         current_skip;
    int          total_passed;
    int          total_failed;
    int          total_skipped;
    int          total_errors;
    int          total_assertions;
    double       total_elapsed_ms;
} utfw_runner_t;

extern utfw_runner_t g_ut_runner;

void        utfw_init(void);
utfw_suite_t* utfw_register_suite(const char *name, void (*setup)(void), void (*teardown)(void));
utfw_case_t*  utfw_register_case(utfw_suite_t *suite, const char *name, const char *file, int line, void (*func)(void));
void        utfw_run_all(void);
void        utfw_report(void);
int         utfw_exit_code(void);

void        utfw_record_pass(void);
void        utfw_record_fail(const char *file, int line, const char *msg);
void        utfw_record_skip(const char *msg);
void        utfw_record_error(const char *file, int line, const char *msg);

#define TEST(name) \
    static void name(void); \
    void name(void)

#define UTFW_STR_(x) #x
#define UTFW_STR(x)  UTFW_STR_(x)
#define UTFW_CONCAT_(a,b) a##b
#define UTFW_CONCAT(a,b) UTFW_CONCAT_(a,b)

#define ASSERT_TRUE(expr) do { \
    g_ut_runner.total_assertions++; \
    if (!(expr)) { \
        char _utfw_buf[UTFW_MSG_MAX]; \
        snprintf(_utfw_buf, UTFW_MSG_MAX, "ASSERT_TRUE(%s) failed", #expr); \
        utfw_record_fail(__FILE__, __LINE__, _utfw_buf); \
        return; \
    } \
} while(0)

#define ASSERT_FALSE(expr) do { \
    g_ut_runner.total_assertions++; \
    if (expr) { \
        char _utfw_buf[UTFW_MSG_MAX]; \
        snprintf(_utfw_buf, UTFW_MSG_MAX, "ASSERT_FALSE(%s) failed", #expr); \
        utfw_record_fail(__FILE__, __LINE__, _utfw_buf); \
        return; \
    } \
} while(0)

#define ASSERT_NULL(ptr) do { \
    g_ut_runner.total_assertions++; \
    if ((ptr) != NULL) { \
        char _utfw_buf[UTFW_MSG_MAX]; \
        snprintf(_utfw_buf, UTFW_MSG_MAX, "ASSERT_NULL(%s) failed: expected NULL, got %p", #ptr, (void*)(ptr)); \
        utfw_record_fail(__FILE__, __LINE__, _utfw_buf); \
        return; \
    } \
} while(0)

#define ASSERT_NOT_NULL(ptr) do { \
    g_ut_runner.total_assertions++; \
    if ((ptr) == NULL) { \
        char _utfw_buf[UTFW_MSG_MAX]; \
        snprintf(_utfw_buf, UTFW_MSG_MAX, "ASSERT_NOT_NULL(%s) failed: expected non-NULL", #ptr); \
        utfw_record_fail(__FILE__, __LINE__, _utfw_buf); \
        return; \
    } \
} while(0)

#define ASSERT_EQ(a, b) do { \
    g_ut_runner.total_assertions++; \
    if ((a) != (b)) { \
        char _utfw_buf[UTFW_MSG_MAX]; \
        snprintf(_utfw_buf, UTFW_MSG_MAX, "ASSERT_EQ(%s, %s) failed: values differ", #a, #b); \
        utfw_record_fail(__FILE__, __LINE__, _utfw_buf); \
        return; \
    } \
} while(0)

#define ASSERT_NEQ(a, b) do { \
    g_ut_runner.total_assertions++; \
    if ((a) == (b)) { \
        char _utfw_buf[UTFW_MSG_MAX]; \
        snprintf(_utfw_buf, UTFW_MSG_MAX, "ASSERT_NEQ(%s, %s) failed: values equal", #a, #b); \
        utfw_record_fail(__FILE__, __LINE__, _utfw_buf); \
        return; \
    } \
} while(0)

#define ASSERT_STREQ(a, b) do { \
    g_ut_runner.total_assertions++; \
    if (strcmp((a), (b)) != 0) { \
        char _utfw_buf[UTFW_MSG_MAX]; \
        snprintf(_utfw_buf, UTFW_MSG_MAX, "ASSERT_STREQ(%s, %s) failed: \"%s\" != \"%s\"", #a, #b, (const char*)(a), (const char*)(b)); \
        utfw_record_fail(__FILE__, __LINE__, _utfw_buf); \
        return; \
    } \
} while(0)

#define ASSERT_STRNEQ(a, b) do { \
    g_ut_runner.total_assertions++; \
    if (strcmp((a), (b)) == 0) { \
        char _utfw_buf[UTFW_MSG_MAX]; \
        snprintf(_utfw_buf, UTFW_MSG_MAX, "ASSERT_STRNEQ(%s, %s) failed: strings equal", #a, #b); \
        utfw_record_fail(__FILE__, __LINE__, _utfw_buf); \
        return; \
    } \
} while(0)

#define ASSERT_DOUBLE_EQ(a, b, epsilon) do { \
    g_ut_runner.total_assertions++; \
    double _utfw_diff = (a) - (b); \
    if (_utfw_diff < 0) _utfw_diff = -_utfw_diff; \
    if (_utfw_diff > (epsilon)) { \
        char _utfw_buf[UTFW_MSG_MAX]; \
        snprintf(_utfw_buf, UTFW_MSG_MAX, "ASSERT_DOUBLE_EQ(%s, %s, %.9g) failed", #a, #b, (epsilon)); \
        utfw_record_fail(__FILE__, __LINE__, _utfw_buf); \
        return; \
    } \
} while(0)

#define ASSERT_FAIL(msg) do { \
    g_ut_runner.total_assertions++; \
    utfw_record_fail(__FILE__, __LINE__, (msg)); \
    return; \
} while(0)

#define SKIP(reason) do { \
    g_ut_runner.current_skip = true; \
    utfw_record_skip(reason); \
    return; \
} while(0)

#define RUN_TEST(suite_name, case_name, func) do { \
    utfw_suite_t *_s = utfw_register_suite(suite_name, NULL, NULL); \
    utfw_register_case(_s, case_name, __FILE__, __LINE__, func); \
} while(0)

#define SETUP(func)   (void (*)(void))(func)
#define TEARDOWN(func) (void (*)(void))(func)

#endif
