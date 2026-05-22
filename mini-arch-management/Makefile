CC = gcc
CFLAGS = -Wall -Wextra -O2 -I include
OUTDIR = bin
EXAMPLEDIR = $(OUTDIR)\examples
SRCDIR = src
EXAMPLESRCDIR = examples

SRCS = $(SRCDIR)\adr_system.c $(SRCDIR)\c4_model.c $(SRCDIR)\arch_review.c $(SRCDIR)\tech_debt.c $(SRCDIR)\quality_attributes.c
OBJS = $(OUTDIR)\adr_system.o $(OUTDIR)\c4_model.o $(OUTDIR)\arch_review.o $(OUTDIR)\tech_debt.o $(OUTDIR)\quality_attributes.o

EXAMPLE_SRCS = $(EXAMPLESRCDIR)\example_adr.c $(EXAMPLESRCDIR)\example_c4.c $(EXAMPLESRCDIR)\example_tech_debt.c
EXAMPLE_BINS = $(EXAMPLEDIR)\example_adr.exe $(EXAMPLEDIR)\example_c4.exe $(EXAMPLEDIR)\example_tech_debt.exe

all: $(OUTDIR) $(OBJS) $(EXAMPLEDIR) examples

$(OUTDIR):
	if not exist "$(OUTDIR)" mkdir "$(OUTDIR)"

$(EXAMPLEDIR):
	if not exist "$(EXAMPLEDIR)" mkdir "$(EXAMPLEDIR)"

$(OUTDIR)\adr_system.o: $(SRCDIR)\adr_system.c include\adr_system.h
	$(CC) $(CFLAGS) -c "$(SRCDIR)\adr_system.c" -o "$(OUTDIR)\adr_system.o"

$(OUTDIR)\c4_model.o: $(SRCDIR)\c4_model.c include\c4_model.h include\adr_system.h
	$(CC) $(CFLAGS) -c "$(SRCDIR)\c4_model.c" -o "$(OUTDIR)\c4_model.o"

$(OUTDIR)\arch_review.o: $(SRCDIR)\arch_review.c include\arch_review.h include\adr_system.h
	$(CC) $(CFLAGS) -c "$(SRCDIR)\arch_review.c" -o "$(OUTDIR)\arch_review.o"

$(OUTDIR)\tech_debt.o: $(SRCDIR)\tech_debt.c include\tech_debt.h
	$(CC) $(CFLAGS) -c "$(SRCDIR)\tech_debt.c" -o "$(OUTDIR)\tech_debt.o"

$(OUTDIR)\quality_attributes.o: $(SRCDIR)\quality_attributes.c include\quality_attributes.h
	$(CC) $(CFLAGS) -c "$(SRCDIR)\quality_attributes.c" -o "$(OUTDIR)\quality_attributes.o"

examples: $(EXAMPLE_BINS)

$(EXAMPLEDIR)\example_adr.exe: $(EXAMPLESRCDIR)\example_adr.c $(OUTDIR)\adr_system.o
	$(CC) $(CFLAGS) "$(EXAMPLESRCDIR)\example_adr.c" "$(OUTDIR)\adr_system.o" -o "$(EXAMPLEDIR)\example_adr.exe"

$(EXAMPLEDIR)\example_c4.exe: $(EXAMPLESRCDIR)\example_c4.c $(OUTDIR)\c4_model.o $(OUTDIR)\adr_system.o
	$(CC) $(CFLAGS) "$(EXAMPLESRCDIR)\example_c4.c" "$(OUTDIR)\c4_model.o" "$(OUTDIR)\adr_system.o" -o "$(EXAMPLEDIR)\example_c4.exe"

$(EXAMPLEDIR)\example_tech_debt.exe: $(EXAMPLESRCDIR)\example_tech_debt.c $(OBJS)
	$(CC) $(CFLAGS) "$(EXAMPLESRCDIR)\example_tech_debt.c" $(OBJS) -o "$(EXAMPLEDIR)\example_tech_debt.exe"

clean:
	if exist "$(OUTDIR)" rmdir /s /q "$(OUTDIR)"

rebuild: clean all

debug: CFLAGS = -Wall -Wextra -g -O0 -I include
debug: all

.PHONY: all examples clean rebuild debug
