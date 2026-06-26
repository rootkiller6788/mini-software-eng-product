#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "adr_system.h"

void adr_log_init(AdrLog *log, const char *project_name)
{
    if (!log) return;
    memset(log, 0, sizeof(*log));
    if (project_name) {
        strncpy(log->project_name, project_name, ADR_MAX_TITLE_LENGTH - 1);
        log->project_name[ADR_MAX_TITLE_LENGTH - 1] = '\0';
    }
    log->next_id = 1;
    log->count = 0;
    log->created_at = time(NULL);
}

AdrLog *adr_log_create(const char *project_name)
{
    AdrLog *log = (AdrLog *)malloc(sizeof(AdrLog));
    if (!log) return NULL;
    adr_log_init(log, project_name);
    return log;
}

void adr_log_destroy(AdrLog *log)
{
    if (log) free(log);
}

unsigned int adr_log_add_record(AdrLog *log,
                                 const char *title,
                                 const char *context,
                                 const char *decision,
                                 const char *consequences,
                                 const char *stakeholder)
{
    if (!log || log->count >= ADR_LOG_MAX_ENTRIES || !title) return 0;

    AdrRecord *rec = &log->entries[log->count];
    memset(rec, 0, sizeof(*rec));

    rec->id = log->next_id++;
    rec->status = ADR_STATUS_PROPOSED;
    rec->created_at = time(NULL);
    rec->updated_at = rec->created_at;
    rec->is_active = true;
    rec->supersedes_count = 0;
    rec->tag_count = 0;
    rec->superseded_by_id = 0;

    strncpy(rec->title, title ? title : "", ADR_MAX_TITLE_LENGTH - 1);
    rec->title[ADR_MAX_TITLE_LENGTH - 1] = '\0';
    strncpy(rec->context, context ? context : "", ADR_MAX_CONTEXT_LENGTH - 1);
    rec->context[ADR_MAX_CONTEXT_LENGTH - 1] = '\0';
    strncpy(rec->decision, decision ? decision : "", ADR_MAX_DECISION_LENGTH - 1);
    rec->decision[ADR_MAX_DECISION_LENGTH - 1] = '\0';
    strncpy(rec->consequences, consequences ? consequences : "",
            ADR_MAX_CONSEQUENCES_LENGTH - 1);
    rec->consequences[ADR_MAX_CONSEQUENCES_LENGTH - 1] = '\0';
    strncpy(rec->stakeholder, stakeholder ? stakeholder : "",
            ADR_MAX_TITLE_LENGTH - 1);
    rec->stakeholder[ADR_MAX_TITLE_LENGTH - 1] = '\0';

    log->count++;
    return rec->id;
}

bool adr_log_accept(AdrLog *log, unsigned int id)
{
    AdrRecord *rec = adr_log_find(log, id);
    if (!rec) return false;
    if (rec->status != ADR_STATUS_PROPOSED) return false;
    rec->status = ADR_STATUS_ACCEPTED;
    rec->updated_at = time(NULL);
    return true;
}

bool adr_log_deprecate(AdrLog *log, unsigned int id)
{
    AdrRecord *rec = adr_log_find(log, id);
    if (!rec) return false;
    if (rec->status == ADR_STATUS_SUPERSEDED) return false;
    rec->status = ADR_STATUS_DEPRECATED;
    rec->is_active = false;
    rec->updated_at = time(NULL);
    return true;
}

bool adr_log_supersede(AdrLog *log, unsigned int old_id, unsigned int new_id)
{
    AdrRecord *old_rec = adr_log_find(log, old_id);
    AdrRecord *new_rec = adr_log_find(log, new_id);
    if (!old_rec || !new_rec) return false;
    if (old_rec->supersedes_count >= ADR_MAX_SUPERSEDES) return false;
    old_rec->status = ADR_STATUS_SUPERSEDED;
    old_rec->superseded_by_id = new_id;
    old_rec->is_active = false;
    old_rec->updated_at = time(NULL);
    new_rec->supersedes_ids[new_rec->supersedes_count++] = old_id;
    return true;
}

AdrRecord *adr_log_find(AdrLog *log, unsigned int id)
{
    if (!log) return NULL;
    for (size_t i = 0; i < log->count; i++) {
        if (log->entries[i].id == id) return &log->entries[i];
    }
    return NULL;
}

AdrRecord *adr_log_find_by_title(AdrLog *log, const char *title)
{
    if (!log || !title) return NULL;
    for (size_t i = 0; i < log->count; i++) {
        if (strcmp(log->entries[i].title, title) == 0) {
            return &log->entries[i];
        }
    }
    return NULL;
}

AdrRecord *adr_log_find_by_status(AdrLog *log, AdrStatus status,
                                   size_t *count)
{
    if (!log) return NULL;
    size_t found = 0;
    for (size_t i = 0; i < log->count; i++) {
        if (log->entries[i].status == status) found++;
    }
    if (count) *count = found;
    if (found == 0) return NULL;
    return log->entries;
}

bool adr_record_add_tag(AdrRecord *record, const char *tag)
{
    if (!record || !tag) return false;
    if (record->tag_count >= ADR_MAX_TAGS) return false;
    strncpy(record->tags[record->tag_count].name, tag, ADR_MAX_TAG_LENGTH - 1);
    record->tags[record->tag_count].name[ADR_MAX_TAG_LENGTH - 1] = '\0';
    record->tag_count++;
    return true;
}

bool adr_record_remove_tag(AdrRecord *record, const char *tag)
{
    if (!record || !tag) return false;
    for (size_t i = 0; i < record->tag_count; i++) {
        if (strcmp(record->tags[i].name, tag) == 0) {
            for (size_t j = i; j < record->tag_count - 1; j++) {
                record->tags[j] = record->tags[j + 1];
            }
            record->tag_count--;
            return true;
        }
    }
    return false;
}

bool adr_record_has_tag(const AdrRecord *record, const char *tag)
{
    if (!record || !tag) return false;
    for (size_t i = 0; i < record->tag_count; i++) {
        if (strcmp(record->tags[i].name, tag) == 0) return true;
    }
    return false;
}

const char *adr_status_to_string(AdrStatus status)
{
    switch (status) {
    case ADR_STATUS_PROPOSED:   return "Proposed";
    case ADR_STATUS_ACCEPTED:   return "Accepted";
    case ADR_STATUS_DEPRECATED: return "Deprecated";
    case ADR_STATUS_SUPERSEDED: return "Superseded";
    default:                    return "Unknown";
    }
}

AdrStatus adr_status_from_string(const char *str)
{
    if (!str) return ADR_STATUS_PROPOSED;
    if (strcmp(str, "Accepted") == 0)   return ADR_STATUS_ACCEPTED;
    if (strcmp(str, "Deprecated") == 0) return ADR_STATUS_DEPRECATED;
    if (strcmp(str, "Superseded") == 0) return ADR_STATUS_SUPERSEDED;
    return ADR_STATUS_PROPOSED;
}

void adr_log_print(const AdrLog *log)
{
    if (!log) return;
    printf("=== ADR Log: %s ===\n", log->project_name);
    printf("Total records: %zu | Created: %s", log->count, ctime(&log->created_at));
    printf("--------------------------------------------------\n");
    for (size_t i = 0; i < log->count; i++) {
        printf("[%04u] %s -- %s\n",
               log->entries[i].id,
               log->entries[i].title,
               adr_status_to_string(log->entries[i].status));
    }
    printf("--------------------------------------------------\n");
}

void adr_log_print_entry(const AdrRecord *record)
{
    if (!record) return;
    printf("--------------------------------------------------\n");
    printf("ADR #%04u: %s\n", record->id, record->title);
    printf("Status: %s | Stakeholder: %s\n",
           adr_status_to_string(record->status), record->stakeholder);
    printf("Created: %s", ctime(&record->created_at));
    printf("Context:\n%s\n\n", record->context);
    printf("Decision:\n%s\n\n", record->decision);
    printf("Consequences:\n%s\n\n", record->consequences);
    if (record->superseded_by_id) {
        printf("Superseded by: ADR #%04u\n", record->superseded_by_id);
    }
    if (record->supersedes_count > 0) {
        printf("Supersedes: ");
        for (size_t i = 0; i < record->supersedes_count; i++) {
            printf("#%04u ", record->supersedes_ids[i]);
        }
        printf("\n");
    }
    printf("--------------------------------------------------\n");
}

size_t adr_log_count_by_status(const AdrLog *log, AdrStatus status)
{
    if (!log) return 0;
    size_t count = 0;
    for (size_t i = 0; i < log->count; i++) {
        if (log->entries[i].status == status) count++;
    }
    return count;
}

size_t adr_log_count_total(const AdrLog *log)
{
    return log ? log->count : 0;
}

bool adr_log_get_supersede_chain(const AdrLog *log, unsigned int id,
                                  AdrSupersedeChain *chain)
{
    if (!log || !chain) return false;
    chain->length = 0;
    AdrRecord *rec = adr_log_find(log, id);
    if (!rec) return false;
    chain->chain[chain->length++] = rec->id;
    while (rec->superseded_by_id && chain->length < ADR_LOG_MAX_ENTRIES) {
        rec = adr_log_find(log, rec->superseded_by_id);
        if (!rec) break;
        chain->chain[chain->length++] = rec->id;
    }
    return chain->length > 0;
}

bool adr_log_validate(const AdrLog *log)
{
    if (!log) return false;
    if (log->count > ADR_LOG_MAX_ENTRIES) return false;
    for (size_t i = 0; i < log->count; i++) {
        if (log->entries[i].id == 0) return false;
        if (log->entries[i].title[0] == '\0') return false;
    }
    return true;
}

bool adr_log_export_markdown(const AdrLog *log, const char *filename)
{
    if (!log || !filename) return false;
    FILE *fp = fopen(filename, "w");
    if (!fp) return false;
    fprintf(fp, "# Architecture Decision Log: %s\n\n", log->project_name);
    fprintf(fp, "| ID | Title | Status | Stakeholder |\n");
    fprintf(fp, "|----|-------|--------|------------|\n");
    for (size_t i = 0; i < log->count; i++) {
        fprintf(fp, "| %04u | %s | %s | %s |\n",
                log->entries[i].id,
                log->entries[i].title,
                adr_status_to_string(log->entries[i].status),
                log->entries[i].stakeholder);
    }
    fprintf(fp, "\n");
    for (size_t i = 0; i < log->count; i++) {
        fprintf(fp, "## ADR #%04u: %s\n\n", log->entries[i].id,
                log->entries[i].title);
        fprintf(fp, "**Status:** %s\n\n", adr_status_to_string(log->entries[i].status));
        fprintf(fp, "### Context\n\n%s\n\n", log->entries[i].context);
        fprintf(fp, "### Decision\n\n%s\n\n", log->entries[i].decision);
        fprintf(fp, "### Consequences\n\n%s\n\n", log->entries[i].consequences);
    }
    fclose(fp);
    return true;
}

bool adr_log_import_markdown(AdrLog *log, const char *filename)
{
    (void)log;
    (void)filename;
    return false;
}
