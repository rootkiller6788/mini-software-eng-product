CC      = gcc
CFLAGS  = -std=c99 -Wall -Wextra -pedantic -Iinclude
AR      = ar
ARFLAGS = rcs

SRCDIR  = src
INCDIR  = include
EXDIR   = examples
DEMODIR = demos
TESTDIR = tests
BUILDD  = build

SRCS    = $(SRCDIR)/gov_scrum.c $(SRCDIR)/gov_kanban.c $(SRCDIR)/gov_okr.c $(SRCDIR)/gov_tracker.c $(SRCDIR)/gov_velocity.c $(SRCDIR)/gov_retro.c $(SRCDIR)/gov_governance.c
OBJS    = $(SRCS:$(SRCDIR)/%.c=$(BUILDD)/%.o)
LIB     = $(BUILDD)/libmini-project-governance.a

EXAMPLES_SRC = $(EXDIR)/example1_scrum_sprint.c $(EXDIR)/example2_kanban_okr.c $(EXDIR)/example3_retro_velocity.c
EXAMPLES_BIN = $(EXAMPLES_SRC:$(EXDIR)/%.c=$(BUILDD)/%)

TEST_SRC = $(TESTDIR)/test_governance.c
TEST_BIN = $(BUILDD)/test_governance.exe

.PHONY: all lib examples test clean rebuild

all: lib examples

lib: $(LIB)

$(BUILDD):
	@mkdir -p $(BUILDD)

$(BUILDD)/%.o: $(SRCDIR)/%.c | $(BUILDD)
	$(CC) $(CFLAGS) -c $< -o $@

$(LIB): $(OBJS)
	$(AR) $(ARFLAGS) $@ $^

$(BUILDD)/example1_scrum_sprint: $(EXDIR)/example1_scrum_sprint.c $(LIB) | $(BUILDD)
	$(CC) $(CFLAGS) $< -L$(BUILDD) -lmini-project-governance -lm -o $@

$(BUILDD)/example2_kanban_okr: $(EXDIR)/example2_kanban_okr.c $(LIB) | $(BUILDD)
	$(CC) $(CFLAGS) $< -L$(BUILDD) -lmini-project-governance -lm -o $@

$(BUILDD)/example3_retro_velocity: $(EXDIR)/example3_retro_velocity.c $(LIB) | $(BUILDD)
	$(CC) $(CFLAGS) $< -L$(BUILDD) -lmini-project-governance -lm -o $@

$(TEST_BIN): $(TEST_SRC) $(LIB) | $(BUILDD)
	$(CC) $(CFLAGS) $< -L$(BUILDD) -lmini-project-governance -lm -o $@

examples: $(EXAMPLES_BIN)

test: $(TEST_BIN)
	@echo Running tests...
	@$(TEST_BIN)
	@echo Tests complete.

clean:
	@rm -rf $(BUILDD)

rebuild: clean all
