CC = gcc
CFLAGS = -std=c99 -Wall -Wextra -pedantic -O2 -Iinclude
LDFLAGS =

SRCDIR = src
INCDIR = include
EXAMPLESDIR = examples
DEMOSDIR = demos
BUILDDIR = build

SRCS = $(SRCDIR)/domain_model.c $(SRCDIR)/workflow_engine.c $(SRCDIR)/rule_engine.c $(SRCDIR)/state_machine.c $(SRCDIR)/approval_process.c
OBJS = $(SRCS:$(SRCDIR)/%.c=$(BUILDDIR)/%.o)
LIBS = $(BUILDDIR)/libbusiness.a

EXAMPLES = $(BUILDDIR)/example1_domain_model $(BUILDDIR)/example2_workflow $(BUILDDIR)/example3_rule_engine
DEMOS = $(BUILDDIR)/demo1_full_system $(BUILDDIR)/demo2_approval_flow

.PHONY: all clean examples demos run-examples run-demos

all: $(LIBS)

$(BUILDDIR):
	mkdir -p $(BUILDDIR)

$(BUILDDIR)/%.o: $(SRCDIR)/%.c $(BUILDDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(LIBS): $(OBJS)
	ar rcs $@ $^

examples: $(EXAMPLES)
demos: $(DEMOS)

$(BUILDDIR)/example1_domain_model: $(EXAMPLESDIR)/example1_domain_model.c $(LIBS)
	$(CC) $(CFLAGS) $< -L$(BUILDDIR) -lbusiness -o $@

$(BUILDDIR)/example2_workflow: $(EXAMPLESDIR)/example2_workflow.c $(LIBS)
	$(CC) $(CFLAGS) $< -L$(BUILDDIR) -lbusiness -o $@

$(BUILDDIR)/example3_rule_engine: $(EXAMPLESDIR)/example3_rule_engine.c $(LIBS)
	$(CC) $(CFLAGS) $< -L$(BUILDDIR) -lbusiness -o $@

$(BUILDDIR)/demo1_full_system: $(DEMOSDIR)/demo1_full_system.c $(LIBS)
	$(CC) $(CFLAGS) $< -L$(BUILDDIR) -lbusiness -o $@

$(BUILDDIR)/demo2_approval_flow: $(DEMOSDIR)/demo2_approval_flow.c $(LIBS)
	$(CC) $(CFLAGS) $< -L$(BUILDDIR) -lbusiness -o $@

run-examples: examples
	$(BUILDDIR)/example1_domain_model
	$(BUILDDIR)/example2_workflow
	$(BUILDDIR)/example3_rule_engine

run-demos: demos
	$(BUILDDIR)/demo1_full_system
	$(BUILDDIR)/demo2_approval_flow

clean:
	rm -rf $(BUILDDIR)
