#include "../include/mock_stub.h"

static mock_ctx_t s_mock_ctx;
mock_ctx_t *g_mock_ctx = NULL;

void mock_init(void) {
    g_mock_ctx = &s_mock_ctx;
    memset(&s_mock_ctx, 0, sizeof(s_mock_ctx));
    s_mock_ctx.fail_on_unexpected = true;
}

void mock_destroy(void) {
    int i;
    for (i = 0; i < s_mock_ctx.expect_count; i++) {
        mock_expect_t *e = &s_mock_ctx.expectations[i];
        if (e->return_type == MOCK_ARG_STRING && e->has_return) {
            free(e->return_value.str_ret);
            e->return_value.str_ret = NULL;
        }
    }
    memset(&s_mock_ctx, 0, sizeof(s_mock_ctx));
    g_mock_ctx = NULL;
}

mock_expect_t* mock_expect_call(const char *func_name, int times) {
    if (s_mock_ctx.expect_count >= MOCK_FUNC_MAX) {
        fprintf(stderr, "ERROR: too many mock expectations (max %d)\n", MOCK_FUNC_MAX);
        return NULL;
    }
    mock_expect_t *exp = &s_mock_ctx.expectations[s_mock_ctx.expect_count++];
    memset(exp, 0, sizeof(mock_expect_t));
    strncpy(exp->func_name, func_name, MOCK_NAME_MAX - 1);
    exp->expected_calls = times;
    exp->ignore_args = true;
    exp->has_return = false;
    return exp;
}

mock_expect_t* mock_expect_any(const char *func_name) {
    return mock_expect_call(func_name, -1);
}

void mock_set_return_int(mock_expect_t *exp, int val) {
    if (!exp) return;
    exp->has_return = true;
    exp->return_type = MOCK_ARG_INT;
    exp->return_value.int_ret = val;
}

void mock_set_return_double(mock_expect_t *exp, double val) {
    if (!exp) return;
    exp->has_return = true;
    exp->return_type = MOCK_ARG_DOUBLE;
    exp->return_value.double_ret = val;
}

void mock_set_return_string(mock_expect_t *exp, const char *val) {
    if (!exp) return;
    exp->has_return = true;
    exp->return_type = MOCK_ARG_STRING;
    if (val) {
        exp->return_value.str_ret = malloc(strlen(val) + 1);
        if (exp->return_value.str_ret) strcpy(exp->return_value.str_ret, val);
    }
}

void mock_set_return_ptr(mock_expect_t *exp, void *val) {
    if (!exp) return;
    exp->has_return = true;
    exp->return_type = MOCK_ARG_PTR;
    exp->return_value.ptr_ret = val;
}

void mock_set_return_bool(mock_expect_t *exp, bool val) {
    if (!exp) return;
    exp->has_return = true;
    exp->return_type = MOCK_ARG_BOOL;
    exp->return_value.bool_ret = val;
}

void mock_set_error(mock_expect_t *exp, int error_code) {
    if (!exp) return;
    exp->throw_error = true;
    exp->error_code = error_code;
}

void mock_record_call(const char *func_name, int argc, ...) {
    if (!g_mock_ctx) return;
    if (s_mock_ctx.call_count >= MOCK_CALLS_MAX) {
        fprintf(stderr, "WARNING: too many mock calls (max %d)\n", MOCK_CALLS_MAX);
        return;
    }
    mock_call_t *call = &s_mock_ctx.calls[s_mock_ctx.call_count++];
    memset(call, 0, sizeof(mock_call_t));
    strncpy(call->func_name, func_name, MOCK_NAME_MAX - 1);
    call->arg_count = argc;

    va_list args;
    va_start(args, argc);
    int i;
    for (i = 0; i < argc && i < MOCK_ARGS_MAX; i++) {
        call->args[i].type = va_arg(args, mock_arg_type_t);
        switch (call->args[i].type) {
            case MOCK_ARG_INT:    call->args[i].value.int_val = va_arg(args, int); break;
            case MOCK_ARG_LONG:   call->args[i].value.long_val = va_arg(args, long); break;
            case MOCK_ARG_FLOAT:  call->args[i].value.float_val = (float)va_arg(args, double); break;
            case MOCK_ARG_DOUBLE: call->args[i].value.double_val = va_arg(args, double); break;
            case MOCK_ARG_STRING: call->args[i].value.str_val = va_arg(args, char*); break;
            case MOCK_ARG_PTR:    call->args[i].value.ptr_val = va_arg(args, void*); break;
            case MOCK_ARG_BOOL:   call->args[i].value.bool_val = (bool)va_arg(args, int); break;
        }
    }
    va_end(args);

    int j;
    for (j = 0; j < s_mock_ctx.expect_count; j++) {
        mock_expect_t *exp = &s_mock_ctx.expectations[j];
        if (strcmp(exp->func_name, func_name) == 0) {
            exp->actual_calls++;
            break;
        }
    }
}

int mock_verify(void) {
    int failures = 0;
    int i;
    for (i = 0; i < s_mock_ctx.expect_count; i++) {
        mock_expect_t *exp = &s_mock_ctx.expectations[i];
        if (exp->expected_calls >= 0 && exp->actual_calls != exp->expected_calls) {
            printf("  [VERIFY FAIL] %s: expected %d calls, got %d\n",
                   exp->func_name, exp->expected_calls, exp->actual_calls);
            failures++;
        }
    }
    if (failures == 0) {
        printf("  [VERIFY PASS] All mock expectations met\n");
    }
    return failures;
}

int mock_verify_count(const char *func_name) {
    int i;
    for (i = 0; i < s_mock_ctx.call_count; i++) {
        if (strcmp(s_mock_ctx.calls[i].func_name, func_name) == 0) {
            int count = 1;
            int j;
            for (j = i + 1; j < s_mock_ctx.call_count; j++) {
                if (strcmp(s_mock_ctx.calls[j].func_name, func_name) == 0) count++;
            }
            return count;
        }
    }
    return 0;
}

bool mock_arg_match_int(int expected, int actual) {
    return expected == actual;
}

bool mock_arg_match_string(const char *expected, const char *actual) {
    if (!expected && !actual) return true;
    if (!expected || !actual) return false;
    return strcmp(expected, actual) == 0;
}

bool mock_arg_match_any(void) {
    return true;
}

static stub_entry_t s_stubs[MOCK_FUNC_MAX];
static int s_stub_count = 0;

void stub_register(const char *func_name, void *func_ptr) {
    if (s_stub_count >= MOCK_FUNC_MAX) {
        fprintf(stderr, "ERROR: too many stubs (max %d)\n", MOCK_FUNC_MAX);
        return;
    }
    stub_entry_t *st = &s_stubs[s_stub_count++];
    memset(st, 0, sizeof(stub_entry_t));
    strncpy(st->func_name, func_name, MOCK_NAME_MAX - 1);
    st->stub.ptr_func = (stub_ptr_func_t)func_ptr;
    st->has_stub = true;
}

void stub_set_default_int(const char *func_name, int val) {
    int i;
    for (i = 0; i < s_stub_count; i++) {
        if (strcmp(s_stubs[i].func_name, func_name) == 0) {
            s_stubs[i].int_default = val;
            return;
        }
    }
    if (s_stub_count < MOCK_FUNC_MAX) {
        stub_entry_t *st = &s_stubs[s_stub_count++];
        memset(st, 0, sizeof(stub_entry_t));
        strncpy(st->func_name, func_name, MOCK_NAME_MAX - 1);
        st->int_default = val;
    }
}

void stub_set_default_string(const char *func_name, const char *val) {
    int i;
    for (i = 0; i < s_stub_count; i++) {
        if (strcmp(s_stubs[i].func_name, func_name) == 0) {
            s_stubs[i].str_default = (char*)val;
            return;
        }
    }
    if (s_stub_count < MOCK_FUNC_MAX) {
        stub_entry_t *st = &s_stubs[s_stub_count++];
        memset(st, 0, sizeof(stub_entry_t));
        strncpy(st->func_name, func_name, MOCK_NAME_MAX - 1);
        st->str_default = (char*)val;
    }
}

int stub_call_int(const char *func_name) {
    int i;
    for (i = 0; i < s_stub_count; i++) {
        if (strcmp(s_stubs[i].func_name, func_name) == 0) {
            stub_entry_t *st = &s_stubs[i];
            if (st->has_stub && st->stub.int_func) return st->stub.int_func();
            return st->int_default;
        }
    }
    return 0;
}

char* stub_call_string(const char *func_name) {
    int i;
    for (i = 0; i < s_stub_count; i++) {
        if (strcmp(s_stubs[i].func_name, func_name) == 0) {
            stub_entry_t *st = &s_stubs[i];
            if (st->has_stub && st->stub.str_func) return st->stub.str_func();
            return s_stubs[i].str_default;
        }
    }
    return NULL;
}

void* stub_call_ptr(const char *func_name) {
    int i;
    for (i = 0; i < s_stub_count; i++) {
        if (strcmp(s_stubs[i].func_name, func_name) == 0) {
            stub_entry_t *st = &s_stubs[i];
            if (st->has_stub && st->stub.ptr_func) return st->stub.ptr_func();
            return s_stubs[i].ptr_default;
        }
    }
    return NULL;
}

void spy_record(const char *func_name, int argc, ...) {
    if (!g_mock_ctx) return;
    if (s_mock_ctx.spy_count >= MOCK_CALLS_MAX) {
        fprintf(stderr, "WARNING: too many spy calls (max %d)\n", MOCK_CALLS_MAX);
        return;
    }
    spy_call_t *spy = &s_mock_ctx.spy_calls[s_mock_ctx.spy_count++];
    memset(spy, 0, sizeof(spy_call_t));
    strncpy(spy->func_name, func_name, MOCK_NAME_MAX - 1);
    spy->arg_count = argc;

    va_list args;
    va_start(args, argc);
    int i;
    for (i = 0; i < argc && i < MOCK_ARGS_MAX; i++) {
        spy->args[i].type = va_arg(args, mock_arg_type_t);
        switch (spy->args[i].type) {
            case MOCK_ARG_INT:    spy->args[i].value.int_val = va_arg(args, int); break;
            case MOCK_ARG_LONG:   spy->args[i].value.long_val = va_arg(args, long); break;
            case MOCK_ARG_FLOAT:  spy->args[i].value.float_val = (float)va_arg(args, double); break;
            case MOCK_ARG_DOUBLE: spy->args[i].value.double_val = va_arg(args, double); break;
            case MOCK_ARG_STRING: spy->args[i].value.str_val = va_arg(args, char*); break;
            case MOCK_ARG_PTR:    spy->args[i].value.ptr_val = va_arg(args, void*); break;
            case MOCK_ARG_BOOL:   spy->args[i].value.bool_val = (bool)va_arg(args, int); break;
        }
    }
    va_end(args);
}

int spy_get_count(const char *func_name) {
    int count = 0;
    int i;
    for (i = 0; i < s_mock_ctx.spy_count; i++) {
        if (strcmp(s_mock_ctx.spy_calls[i].func_name, func_name) == 0) count++;
    }
    return count;
}

spy_call_t* spy_get_call(const char *func_name, int index) {
    int count = 0;
    int i;
    for (i = 0; i < s_mock_ctx.spy_count; i++) {
        if (strcmp(s_mock_ctx.spy_calls[i].func_name, func_name) == 0) {
            if (count == index) return &s_mock_ctx.spy_calls[i];
            count++;
        }
    }
    return NULL;
}

static fake_store_t s_fake_store[MOCK_FUNC_MAX];
static int s_fake_count = 0;

void fake_put_int(const char *key, int val) {
    int i;
    for (i = 0; i < s_fake_count; i++) {
        if (strcmp(s_fake_store[i].key, key) == 0) {
            s_fake_store[i].int_val = val;
            s_fake_store[i].has_value = true;
            return;
        }
    }
    if (s_fake_count < MOCK_FUNC_MAX) {
        strncpy(s_fake_store[s_fake_count].key, key, MOCK_NAME_MAX - 1);
        s_fake_store[s_fake_count].int_val = val;
        s_fake_store[s_fake_count].has_value = true;
        s_fake_count++;
    }
}

int fake_get_int(const char *key) {
    int i;
    for (i = 0; i < s_fake_count; i++) {
        if (strcmp(s_fake_store[i].key, key) == 0 && s_fake_store[i].has_value) {
            return s_fake_store[i].int_val;
        }
    }
    return 0;
}

void fake_put_string(const char *key, const char *val) {
    int i;
    for (i = 0; i < s_fake_count; i++) {
        if (strcmp(s_fake_store[i].key, key) == 0) {
            s_fake_store[i].str_val = (char*)val;
            s_fake_store[i].has_value = true;
            return;
        }
    }
    if (s_fake_count < MOCK_FUNC_MAX) {
        strncpy(s_fake_store[s_fake_count].key, key, MOCK_NAME_MAX - 1);
        s_fake_store[s_fake_count].str_val = (char*)val;
        s_fake_store[s_fake_count].has_value = true;
        s_fake_count++;
    }
}

char* fake_get_string(const char *key) {
    int i;
    for (i = 0; i < s_fake_count; i++) {
        if (strcmp(s_fake_store[i].key, key) == 0 && s_fake_store[i].has_value) {
            return s_fake_store[i].str_val;
        }
    }
    return NULL;
}

void fake_clear(void) {
    memset(s_fake_store, 0, sizeof(s_fake_store));
    s_fake_count = 0;
}
