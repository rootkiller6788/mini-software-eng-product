#ifndef ARCH_ADR_H
#define ARCH_ADR_H
#include <stdint.h>
#include <stdbool.h>

#define ADR_MAX 64
#define ADR_TITLE_LEN 128
#define ADR_CONTEXT_LEN 512

typedef enum { ADR_PROPOSED, ADR_ACCEPTED, ADR_DEPRECATED, ADR_SUPERSEDED } AdrStatus;

typedef struct {
    int id; char title[ADR_TITLE_LEN];
    char context[ADR_CONTEXT_LEN];
    char decision[ADR_CONTEXT_LEN];
    char consequences[ADR_CONTEXT_LEN];
    AdrStatus status;
    int superseded_by;
    uint32_t timestamp;
    int related_adrs[8]; int related_count;
} ArchitectureDecision;

typedef struct { ArchitectureDecision records[ADR_MAX]; int count; } AdrLog;

void adr_init(AdrLog *log);
int  adr_add(AdrLog *log, const char *title, const char *context, const char *decision, const char *consequences);
bool adr_accept(AdrLog *log, int id);
bool adr_deprecate(AdrLog *log, int id, int superseded_by);
int  adr_find_by_status(AdrLog *log, AdrStatus status, int *results, int max_results);
void adr_link(AdrLog *log, int from_id, int to_id);
void adr_print_all(AdrLog *log);
void adr_print_one(AdrLog *log, int id);
#endif
