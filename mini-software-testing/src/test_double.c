#include "test_double.h"
#include <stdio.h>
#include <string.h>

void double_registry_init(DoubleRegistry *dr) { memset(dr, 0, sizeof(*dr)); }

int double_add_stub(DoubleRegistry *dr, const char *name, int return_value) {
    if (dr->double_count >= MAX_DOUBLES) return -1;
    TestDouble *td = &dr->doubles[dr->double_count];
    memset(td, 0, sizeof(*td));
    strncpy(td->name, name, 63); td->name[63] = '\0';
    td->type = DTYPE_STUB;
    td->stub_return_value = return_value; td->stub_return_set = true;
    return dr->double_count++;
}

int double_add_mock(DoubleRegistry *dr, const char *name) {
    if (dr->double_count >= MAX_DOUBLES) return -1;
    TestDouble *td = &dr->doubles[dr->double_count];
    memset(td, 0, sizeof(*td));
    strncpy(td->name, name, 63); td->name[63] = '\0';
    td->type = DTYPE_MOCK;
    return dr->double_count++;
}

int double_mock_expect(DoubleRegistry *dr, int idx, const char *expected_args, int return_value) {
    if (idx < 0 || idx >= dr->double_count) return -1;
    TestDouble *td = &dr->doubles[idx];
    if (td->expected_count >= 16) return -1;
    strncpy(td->expected_calls[td->expected_count], expected_args, 127);
    td->mock_return_values[td->expected_count] = return_value;
    return td->expected_count++;
}

int double_add_fake(DoubleRegistry *dr, const char *name, int initial_state) {
    if (dr->double_count >= MAX_DOUBLES) return -1;
    TestDouble *td = &dr->doubles[dr->double_count];
    memset(td, 0, sizeof(*td));
    strncpy(td->name, name, 63); td->name[63] = '\0';
    td->type = DTYPE_FAKE; td->fake_state = initial_state;
    return dr->double_count++;
}

int double_add_spy(DoubleRegistry *dr, const char *name, int original_return) {
    if (dr->double_count >= MAX_DOUBLES) return -1;
    TestDouble *td = &dr->doubles[dr->double_count];
    memset(td, 0, sizeof(*td));
    strncpy(td->name, name, 63); td->name[63] = '\0';
    td->type = DTYPE_SPY; td->spy_original_return = original_return;
    return dr->double_count++;
}

void double_spy_record(DoubleRegistry *dr, int idx, const char *func_name, const char *args) {
    if (idx < 0 || idx >= dr->double_count) return;
    TestDouble *td = &dr->doubles[idx];
    if (td->spy_call_count >= MAX_CALL_RECORDS) return;
    CallRecord *cr = &td->spy_records[td->spy_call_count];
    strncpy(cr->function_name, func_name, 63);
    strncpy(cr->args, args, 255);
    cr->called = true;
    td->spy_call_count++;
}

int double_spy_call_count(DoubleRegistry *dr, int idx) {
    if (idx < 0 || idx >= dr->double_count) return 0;
    return dr->doubles[idx].spy_call_count;
}

bool double_mock_verify(DoubleRegistry *dr, int idx) {
    if (idx < 0 || idx >= dr->double_count) return false;
    TestDouble *td = &dr->doubles[idx];
    return td->actual_calls >= td->expected_count;
}

bool double_mock_verify_call(DoubleRegistry *dr, int idx, int call_idx) {
    if (idx < 0 || idx >= dr->double_count) return false;
    TestDouble *td = &dr->doubles[idx];
    if (call_idx < 0 || call_idx >= td->expected_count) return false;
    return call_idx < td->actual_calls;
}

void double_print(DoubleRegistry *dr, int idx) {
    if (idx < 0 || idx >= dr->double_count) return;
    TestDouble *td = &dr->doubles[idx];
    const char *ts[] = {"STUB","MOCK","FAKE","SPY"};
    printf("  %s (%s) calls=%d", td->name, ts[td->type], td->actual_calls);
    if (td->type == DTYPE_STUB) printf(" return=%d", td->stub_return_value);
    printf("\n");
}

void double_print_all(DoubleRegistry *dr) {
    printf("=== Test Doubles (%d) ===\n", dr->double_count);
    for (int i = 0; i < dr->double_count; i++) double_print(dr, i);
}
