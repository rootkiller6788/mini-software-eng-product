CC = gcc
CFLAGS = -Wall -Wextra -O2 -I include
LDFLAGS = -Wl,--stack,16777216
OUTDIR = bin
EXAMPLEDIR = $(OUTDIR)/examples
SRCDIR = src

SRCS = $(SRCDIR)/git_dag.c $(SRCDIR)/git_branch.c $(SRCDIR)/git_merge.c $(SRCDIR)/git_commit.c $(SRCDIR)/git_pr.c $(SRCDIR)/git_ci.c
OBJS = $(OUTDIR)/git_dag.o $(OUTDIR)/git_branch.o $(OUTDIR)/git_merge.o $(OUTDIR)/git_commit.o $(OUTDIR)/git_pr.o $(OUTDIR)/git_ci.o
TEST_SRC = tests/test_git.c
TEST_BIN = $(OUTDIR)/test_git.exe

all: $(OBJS) examples test

$(OUTDIR)/git_dag.o: $(SRCDIR)/git_dag.c include/git_dag.h
	@mkdir -p $(OUTDIR)
	$(CC) $(CFLAGS) -c "$(SRCDIR)/git_dag.c" -o "$(OUTDIR)/git_dag.o"

$(OUTDIR)/git_branch.o: $(SRCDIR)/git_branch.c include/git_branch.h include/git_dag.h
	@mkdir -p $(OUTDIR)
	$(CC) $(CFLAGS) -c "$(SRCDIR)/git_branch.c" -o "$(OUTDIR)/git_branch.o"

$(OUTDIR)/git_merge.o: $(SRCDIR)/git_merge.c include/git_merge.h include/git_dag.h
	@mkdir -p $(OUTDIR)
	$(CC) $(CFLAGS) -c "$(SRCDIR)/git_merge.c" -o "$(OUTDIR)/git_merge.o"

$(OUTDIR)/git_commit.o: $(SRCDIR)/git_commit.c include/git_commit.h
	@mkdir -p $(OUTDIR)
	$(CC) $(CFLAGS) -c "$(SRCDIR)/git_commit.c" -o "$(OUTDIR)/git_commit.o"

$(OUTDIR)/git_pr.o: $(SRCDIR)/git_pr.c include/git_pr.h
	@mkdir -p $(OUTDIR)
	$(CC) $(CFLAGS) -c "$(SRCDIR)/git_pr.c" -o "$(OUTDIR)/git_pr.o"

$(OUTDIR)/git_ci.o: $(SRCDIR)/git_ci.c include/git_ci.h include/git_commit.h
	@mkdir -p $(OUTDIR)
	$(CC) $(CFLAGS) -c "$(SRCDIR)/git_ci.c" -o "$(OUTDIR)/git_ci.o"

test: $(TEST_BIN)
	./$(TEST_BIN)

$(TEST_BIN): $(TEST_SRC) $(OBJS)
	@mkdir -p $(OUTDIR)
	$(CC) $(CFLAGS) $(LDFLAGS) "$(TEST_SRC)" $(OBJS) -o "$(TEST_BIN)"

examples: example_git_flow example_trunk_based example_conventional_commits

example_git_flow: $(OBJS)
	@mkdir -p $(EXAMPLEDIR)
	$(CC) $(CFLAGS) $(LDFLAGS) "examples/example_git_flow.c" $(OBJS) -o "$(EXAMPLEDIR)/example_git_flow.exe"

example_trunk_based: $(OBJS)
	@mkdir -p $(EXAMPLEDIR)
	$(CC) $(CFLAGS) $(LDFLAGS) "examples/example_trunk_based.c" $(OBJS) -o "$(EXAMPLEDIR)/example_trunk_based.exe"

example_conventional_commits: $(OBJS)
	@mkdir -p $(EXAMPLEDIR)
	$(CC) $(CFLAGS) $(LDFLAGS) "examples/example_conventional_commits.c" $(OBJS) -o "$(EXAMPLEDIR)/example_conventional_commits.exe"

clean:
	rm -rf "$(OUTDIR)"

rebuild: clean all

debug: CFLAGS = -Wall -Wextra -g -O0 -I include
debug: all

.PHONY: all test examples clean rebuild debug example_git_flow example_trunk_based example_conventional_commits
