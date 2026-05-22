CC       = gcc
CFLAGS   = -std=c99 -Wall -Wextra -pedantic -g
LDFLAGS  =

SRCS     = git_internals.c branching_models.c merge_rebase.c pr_review.c conventional_commits.c
OBJS     = $(SRCS:.c=.o)

EXAMPLES = example_basic_git_workflow example_conventional_commits example_rebase_flow
DEMOS    = demo_git_flow demo_trunk_based
DOCS     = doc_git_internals doc_branching_strategies

EXE      = $(EXAMPLES) $(DEMOS) $(DOCS)

.PHONY: all clean examples demos docs

all: examples demos docs

examples: $(EXAMPLES)
demos: $(DEMOS)
docs: $(DOCS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

example_basic_git_workflow: example_basic_git_workflow.c $(OBJS)
	$(CC) $(CFLAGS) $< $(OBJS) -o $@ $(LDFLAGS)

example_conventional_commits: example_conventional_commits.c $(OBJS)
	$(CC) $(CFLAGS) $< $(OBJS) -o $@ $(LDFLAGS)

example_rebase_flow: example_rebase_flow.c $(OBJS)
	$(CC) $(CFLAGS) $< $(OBJS) -o $@ $(LDFLAGS)

demo_git_flow: demo_git_flow.c $(OBJS)
	$(CC) $(CFLAGS) $< $(OBJS) -o $@ $(LDFLAGS)

demo_trunk_based: demo_trunk_based.c $(OBJS)
	$(CC) $(CFLAGS) $< $(OBJS) -o $@ $(LDFLAGS)

doc_git_internals: doc_git_internals.c $(OBJS)
	$(CC) $(CFLAGS) $< $(OBJS) -o $@ $(LDFLAGS)

doc_branching_strategies: doc_branching_strategies.c $(OBJS)
	$(CC) $(CFLAGS) $< $(OBJS) -o $@ $(LDFLAGS)

clean:
	del /q *.o *.exe $(EXE) 2>nul || rm -f *.o $(OBJS) $(EXE)
