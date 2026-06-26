#ifndef MOCK_STUB_H
#define MOCK_STUB_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdarg.h>

#define MOCK_FUNC_MAX     64
#define MOCK_NAME_MAX     128
#define MOCK_ARGS_MAX     16
#define MOCK_CALLS_MAX    128

typedef enum {
    MOCK_ARG_INT,
    MOCK_ARG_LONG,
    MOCK_ARG_FLOAT,
    MOCK_ARG_DOUBLE,
    MOCK_ARG_STRING,
    MOCK_ARG_PTR,
    MOCK_ARG_BOOL
} mock_arg_type_t;

typedef struct {
    mock_arg_type_t type;
    union {
        int    int_val;
        long   long_val;
        float  float_val;
        double double_val;
        char  *str_val;
        void  *ptr_val;
        bool   bool_val;
    } value;
} mock_arg_t;

typedef struct mock_call {
    char       func_name[MOCK_NAME_MAX];
    mock_arg_t args[MOCK_ARGS_MAX];
    int        arg_count;
} mock_call_t;

typedef struct mock_expect {
    char       func_name[MOCK_NAME_MAX];
    int        expected_calls;
    int        actual_calls;
    mock_arg_t expected_args[MOCK_ARGS_MAX];
    int        expected_arg_count;
    bool       ignore_args;
    bool       has_return;
    union {
        int    int_ret;
        long   long_ret;
        float  float_ret;
        double double_ret;
        char  *str_ret;
        void  *ptr_ret;
        bool   bool_ret;
    } return_value;
    mock_arg_type_t return_type;
    bool  throw_error;
    int   error_code;
} mock_expect_t;

typedef struct spy_call {
    char       func_name[MOCK_NAME_MAX];
    mock_arg_t args[MOCK_ARGS_MAX];
    int        arg_count;
} spy_call_t;

typedef struct mock_ctx {
    mock_expect_t expectations[MOCK_FUNC_MAX];
    int           expect_count;
    mock_call_t   calls[MOCK_CALLS_MAX];
    int           call_count;
    spy_call_t    spy_calls[MOCK_CALLS_MAX];
    int           spy_count;
    bool          fail_on_unexpected;
} mock_ctx_t;

extern mock_ctx_t *g_mock_ctx;

void    mock_init(void);
void    mock_destroy(void);

mock_expect_t* mock_expect_call(const char *func_name, int times);
mock_expect_t* mock_expect_any(const char *func_name);
void    mock_set_return_int(mock_expect_t *exp, int val);
void    mock_set_return_double(mock_expect_t *exp, double val);
void    mock_set_return_string(mock_expect_t *exp, const char *val);
void    mock_set_return_ptr(mock_expect_t *exp, void *val);
void    mock_set_return_bool(mock_expect_t *exp, bool val);
void    mock_set_error(mock_expect_t *exp, int error_code);

void    mock_record_call(const char *func_name, int argc, ...);
int     mock_verify(void);
int     mock_verify_count(const char *func_name);

bool    mock_arg_match_int(int expected, int actual);
bool    mock_arg_match_string(const char *expected, const char *actual);
bool    mock_arg_match_any(void);

typedef int  (*stub_int_func_t)(void);
typedef long (*stub_long_func_t)(void);
typedef char* (*stub_str_func_t)(void);
typedef void* (*stub_ptr_func_t)(void);

typedef struct stub_entry {
    char              func_name[MOCK_NAME_MAX];
    union {
        stub_int_func_t  int_func;
        stub_long_func_t long_func;
        stub_str_func_t  str_func;
        stub_ptr_func_t  ptr_func;
    } stub;
    bool              has_stub;
    int               int_default;
    char             *str_default;
    void             *ptr_default;
} stub_entry_t;

void    stub_register(const char *func_name, void *func_ptr);
void    stub_set_default_int(const char *func_name, int val);
void    stub_set_default_string(const char *func_name, const char *val);
int     stub_call_int(const char *func_name);
char   *stub_call_string(const char *func_name);
void   *stub_call_ptr(const char *func_name);

void    spy_record(const char *func_name, int argc, ...);
int     spy_get_count(const char *func_name);
spy_call_t* spy_get_call(const char *func_name, int index);

typedef struct fake_store {
    char  key[MOCK_NAME_MAX];
    int   int_val;
    char *str_val;
    void *ptr_val;
    bool  has_value;
} fake_store_t;

void    fake_put_int(const char *key, int val);
int     fake_get_int(const char *key);
void    fake_put_string(const char *key, const char *val);
char   *fake_get_string(const char *key);
void    fake_clear(void);

#endif
