CC      = gcc
CFLAGS  = -std=c99 -Wall -Wextra -pedantic -Iinclude
AR      = ar
ARFLAGS = rcs

SRCDIR  = src
INCDIR  = include
EXDIR   = examples
DEMODIR = demo
BUILDD  = build

SRCS    = $(wildcard $(SRCDIR)/*.c)
OBJS    = $(SRCS:$(SRCDIR)/%.c=$(BUILDD)/%.o)
LIB     = $(BUILDD)/libmini-product-eng.a

EXAMPLES_SRC = $(wildcard $(EXDIR)/*.c)
EXAMPLES_BIN = $(EXAMPLES_SRC:$(EXDIR)/%.c=$(BUILDD)/%)

DEMOS_SRC    = $(wildcard $(DEMODIR)/*.c)
DEMOS_BIN    = $(DEMOS_SRC:$(DEMODIR)/%.c=$(BUILDD)/%)

.PHONY: all lib examples demos clean

all: lib examples demos

lib: $(LIB)

$(BUILDD):
	@if not exist "$(BUILDD)" mkdir "$(BUILDD)"

$(BUILDD)/%.o: $(SRCDIR)/%.c | $(BUILDD)
	$(CC) $(CFLAGS) -c $< -o $@

$(LIB): $(OBJS)
	$(AR) $(ARFLAGS) $@ $^

$(BUILDD)/%: $(EXDIR)/%.c $(LIB) | $(BUILDD)
	$(CC) $(CFLAGS) $< -L$(BUILDD) -lmini-product-eng -o $@

$(BUILDD)/%: $(DEMODIR)/%.c $(LIB) | $(BUILDD)
	$(CC) $(CFLAGS) $< -L$(BUILDD) -lmini-product-eng -o $@

examples: $(EXAMPLES_BIN)

demos: $(DEMOS_BIN)

clean:
	@if exist "$(BUILDD)" rmdir /s /q "$(BUILDD)"
