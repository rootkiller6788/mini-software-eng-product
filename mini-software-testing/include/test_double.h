#ifndef TEST_DOUBLE_H
#define TEST_DOUBLE_H
#include <stdbool.h>

#define MAX_DOUBLES 8
#define MAX_CALL_RECORDS 32

typedef enum { DTYPE_STUB, DTYPE_MOCK, DTYPE_FAKE, DTYPE_SPY } DoubleType;

typedef struct {
    char function_name[64]; char args[256]; int return_value; bool called;
} CallRecord;

typedef struct {
    char name[64]; DoubleType type;
    /* stub: fixed return value */
    int stub_return_value; bool stub_return_set;
    /* mock: expected calls with return values */
    char expected_calls[16][128]; int expected_count;
    int mock_return_values[16]; int mock_return_count;
    /* fake: lightweight real implementation */
    int fake_state; /* arbitrary state for fake */
    /* spy: counts calls and captures args */
    CallRecord spy_records[MAX_CALL_RECORDS]; int spy_call_count;
    int spy_original_return; /* what the real function would return */
    /* shared */
    int actual_calls; bool verified;
} TestDouble;

typedef struct {
    TestDouble doubles[MAX_DOUBLES]; int double_count;
} DoubleRegistry;

void double_registry_init(DoubleRegistry *dr);
int  double_add_stub(DoubleRegistry *dr, const char *name, int return_value);
int  double_add_mock(DoubleRegistry *dr, const char *name);
int  double_mock_expect(DoubleRegistry *dr, int idx, const char *expected_args, int return_value);
int  double_add_fake(DoubleRegistry *dr, const char *name, int initial_state);
int  double_add_spy(DoubleRegistry *dr, const char *name, int original_return);
void double_spy_record(DoubleRegistry *dr, int idx, const char *func_name, const char *args);
int  double_spy_call_count(DoubleRegistry *dr, int idx);
bool double_mock_verify(DoubleRegistry *dr, int idx); /* all expected calls made */
bool double_mock_verify_call(DoubleRegistry *dr, int idx, int call_idx);
void double_print(DoubleRegistry *dr, int idx);
void double_print_all(DoubleRegistry *dr);
#endif
