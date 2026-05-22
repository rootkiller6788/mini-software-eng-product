CC       = gcc
CFLAGS   = -Wall -Wextra -std=c99 -pedantic -I.
LDFLAGS  =

SRC_DIR  = src
INC_DIR  = include
EX_DIR   = examples
DM_DIR   = demo
DOC_DIR  = docs

SRCS     = $(SRC_DIR)/unit_test_fw.c $(SRC_DIR)/mock_stub.c $(SRC_DIR)/tdd_bdd.c $(SRC_DIR)/e2e_contract.c $(SRC_DIR)/mutation_testing.c
OBJS     = $(SRCS:.c=.o)

EXAMPLE_SRCS = $(EX_DIR)/example_unit_test.c $(EX_DIR)/example_mock.c $(EX_DIR)/example_tdd.c
EXAMPLE_BINS = $(EX_DIR)/example_unit_test.exe $(EX_DIR)/example_mock.exe $(EX_DIR)/example_tdd.exe

DEMO_SRCS = $(DM_DIR)/demo_testing.c $(DM_DIR)/demo_e2e.c
DEMO_BINS = $(DM_DIR)/demo_testing.exe $(DM_DIR)/demo_e2e.exe

.PHONY: all clean examples demo

all: examples demo

examples: $(EXAMPLE_BINS)

demo: $(DEMO_BINS)

$(EX_DIR)/example_unit_test.exe: $(EX_DIR)/example_unit_test.c $(SRC_DIR)/unit_test_fw.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(EX_DIR)/example_mock.exe: $(EX_DIR)/example_mock.c $(SRC_DIR)/unit_test_fw.c $(SRC_DIR)/mock_stub.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(EX_DIR)/example_tdd.exe: $(EX_DIR)/example_tdd.c $(SRC_DIR)/tdd_bdd.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(DM_DIR)/demo_testing.exe: $(DM_DIR)/demo_testing.c $(SRCS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(DM_DIR)/demo_e2e.exe: $(DM_DIR)/demo_e2e.c $(SRC_DIR)/unit_test_fw.c $(SRC_DIR)/e2e_contract.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	del /Q "$(SRC_DIR)\*.o" 2>nul
	del /Q "$(EX_DIR)\*.exe" 2>nul
	del /Q "$(DM_DIR)\*.exe" 2>nul
	del /Q "$(EX_DIR)\*.o" 2>nul
	del /Q "$(DM_DIR)\*.o" 2>nul
