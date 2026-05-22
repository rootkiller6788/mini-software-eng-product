CC = gcc
CFLAGS = -std=c99 -Wall -Wextra -pedantic -O2
LDFLAGS = -lm

SRCS = agile_scrum.c kanban_board.c project_tracker.c okr_tracker.c retrospective.c
OBJS = $(SRCS:.c=.o)
EXAMPLES = example_agile_scrum example_kanban example_okr
DEMOS = demo_scrum demo_kanban_retro
TARGETS = $(EXAMPLES) $(DEMOS)

.PHONY: all clean run_examples run_demos

all: $(TARGETS)

demo_scrum: demo_scrum.c agile_scrum.o project_tracker.o okr_tracker.o kanban_board.o retrospective.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

demo_kanban_retro: demo_kanban_retro.c kanban_board.o retrospective.o project_tracker.o agile_scrum.o okr_tracker.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

example_agile_scrum: example_agile_scrum.c agile_scrum.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

example_kanban: example_kanban.c kanban_board.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

example_okr: example_okr.c okr_tracker.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c %.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	del /f /q *.o *.exe $(EXAMPLES:=.exe) $(DEMOS:=.exe) 2>nul || rm -f *.o $(EXAMPLES) $(DEMOS)

run_examples: $(EXAMPLES)
	./example_agile_scrum
	./example_kanban
	./example_okr

run_demos: $(DEMOS)
	./demo_scrum
	./demo_kanban_retro
