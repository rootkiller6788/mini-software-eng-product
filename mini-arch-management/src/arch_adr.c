#include "arch_adr.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

void adr_init(AdrLog *log) { memset(log, 0, sizeof(*log)); }

int adr_add(AdrLog *log, const char *title, const char *context, const char *decision, const char *consequences) {
    if (log->count >= ADR_MAX) return -1;
    ArchitectureDecision *a = &log->records[log->count];
    a->id = log->count; a->status = ADR_PROPOSED; a->timestamp = (uint32_t)time(NULL);
    a->superseded_by = -1; a->related_count = 0;
    strncpy(a->title, title, ADR_TITLE_LEN-1); a->title[ADR_TITLE_LEN-1] = '\0';
    strncpy(a->context, context, ADR_CONTEXT_LEN-1); a->context[ADR_CONTEXT_LEN-1] = '\0';
    strncpy(a->decision, decision, ADR_CONTEXT_LEN-1); a->decision[ADR_CONTEXT_LEN-1] = '\0';
    strncpy(a->consequences, consequences, ADR_CONTEXT_LEN-1); a->consequences[ADR_CONTEXT_LEN-1] = '\0';
    return log->count++;
}

bool adr_accept(AdrLog *log, int id) { if (id < 0 || id >= log->count) return false; log->records[id].status = ADR_ACCEPTED; return true; }

bool adr_deprecate(AdrLog *log, int id, int superseded_by) {
    if (id < 0 || id >= log->count) return false;
    log->records[id].status = ADR_DEPRECATED; log->records[id].superseded_by = superseded_by;
    return true;
}

int adr_find_by_status(AdrLog *log, AdrStatus status, int *results, int max_results) {
    int count = 0;
    for (int i = 0; i < log->count && count < max_results; i++) if (log->records[i].status == status) results[count++] = i;
    return count;
}

void adr_link(AdrLog *log, int from_id, int to_id) {
    if (from_id >= log->count || to_id >= log->count) return;
    if (log->records[from_id].related_count < 8) log->records[from_id].related_adrs[log->records[from_id].related_count++] = to_id;
}

void adr_print_all(AdrLog *log) {
    printf("=== Architecture Decision Records (%d) ===\n", log->count);
    for (int i = 0; i < log->count; i++) printf("  ADR-%03d [%s] %s\n", log->records[i].id, log->records[i].status == ADR_ACCEPTED ? "OK" : "PEND", log->records[i].title);
}

void adr_print_one(AdrLog *log, int id) {
    if (id >= log->count) return;
    ArchitectureDecision *a = &log->records[id];
    printf("=== ADR-%03d: %s ===\n", a->id, a->title);
    printf("  Status: %d, Context: %s\n  Decision: %s\n  Consequences: %s\n", a->status, a->context, a->decision, a->consequences);
}
