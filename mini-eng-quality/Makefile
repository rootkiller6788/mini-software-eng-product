CC       = gcc
CFLAGS   = -std=c99 -Wall -Wextra -pedantic -O2 -g
INCLUDES = -Iinclude
SRC_DIR  = src
DEMO_DIR = demo
EXAM_DIR = examples
BUILD    = build

LIBSRC   = $(wildcard $(SRC_DIR)/*.c)
LIBOBJ   = $(patsubst $(SRC_DIR)/%.c,$(BUILD)/%.o,$(LIBSRC))
DEMOSRC  = $(wildcard $(DEMO_DIR)/*.c)
DEMOBIN  = $(patsubst $(DEMO_DIR)/%.c,$(BUILD)/%,$(DEMOSRC))
EXAMSRC  = $(wildcard $(EXAM_DIR)/*.c)
EXAMBIN  = $(patsubst $(EXAM_DIR)/%.c,$(BUILD)/%,$(EXAMSRC))

.PHONY: all clean demo test

all: $(BUILD) demo test

$(BUILD):
	mkdir -p $(BUILD)

$(BUILD)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(BUILD)/%: $(DEMO_DIR)/%.c $(LIBOBJ)
	$(CC) $(CFLAGS) $(INCLUDES) $< $(LIBOBJ) -o $@

$(BUILD)/%: $(EXAM_DIR)/%.c $(LIBOBJ)
	$(CC) $(CFLAGS) $(INCLUDES) $< $(LIBOBJ) -o $@

demo: $(DEMOBIN)

test: $(EXAMBIN)
	@for exe in $(EXAMBIN); do echo "Running $$exe..."; $$exe; done

clean:
	rm -rf $(BUILD)
