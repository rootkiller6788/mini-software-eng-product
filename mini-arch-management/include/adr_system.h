#ifndef ADR_SYSTEM_H
#define ADR_SYSTEM_H

#include <stdbool.h>
#include <stddef.h>
#include <time.h>

#define ADR_MAX_TITLE_LENGTH       256
#define ADR_MAX_CONTEXT_LENGTH    2048
#define ADR_MAX_DECISION_LENGTH   2048
#define ADR_MAX_CONSEQUENCES_LENGTH 2048
#define ADR_LOG_MAX_ENTRIES        256
#define ADR_MAX_SUPERSEDES          8
#define ADR_MAX_TAGS               16
#define ADR_MAX_TAG_LENGTH         64

typedef enum AdrStatus {
    ADR_STATUS_PROPOSED,
    ADR_STATUS_ACCEPTED,
    ADR_STATUS_DEPRECATED,
    ADR_STATUS_SUPERSEDED
} AdrStatus;

typedef struct AdrTag {
    char name[ADR_MAX_TAG_LENGTH];
} AdrTag;

typedef struct AdrRecord {
    unsigned int id;
    char title[ADR_MAX_TITLE_LENGTH];
    AdrStatus status;
    char context[ADR_MAX_CONTEXT_LENGTH];
    char decision[ADR_MAX_DECISION_LENGTH];
    char consequences[ADR_MAX_CONSEQUENCES_LENGTH];
    time_t created_at;
    time_t updated_at;
    unsigned int superseded_by_id;
    unsigned int supersedes_ids[ADR_MAX_SUPERSEDES];
    size_t supersedes_count;
    AdrTag tags[ADR_MAX_TAGS];
    size_t tag_count;
    char stakeholder[ADR_MAX_TITLE_LENGTH];
    bool is_active;
} AdrRecord;

typedef struct AdrLog {
    AdrRecord entries[ADR_LOG_MAX_ENTRIES];
    size_t count;
    unsigned int next_id;
    char project_name[ADR_MAX_TITLE_LENGTH];
    time_t created_at;
    char repository_url[ADR_MAX_TITLE_LENGTH];
} AdrLog;

void adr_log_init(AdrLog *log, const char *project_name);
AdrLog *adr_log_create(const char *project_name);
void adr_log_destroy(AdrLog *log);

unsigned int adr_log_add_record(AdrLog *log,
                                const char *title,
                                const char *context,
                                const char *decision,
                                const char *consequences,
                                const char *stakeholder);

bool adr_log_accept(AdrLog *log, unsigned int id);
bool adr_log_deprecate(AdrLog *log, unsigned int id);
bool adr_log_supersede(AdrLog *log, unsigned int old_id, unsigned int new_id);

AdrRecord *adr_log_find(AdrLog *log, unsigned int id);
AdrRecord *adr_log_find_by_title(AdrLog *log, const char *title);
AdrRecord *adr_log_find_by_status(AdrLog *log, AdrStatus status, size_t *count);

bool adr_record_add_tag(AdrRecord *record, const char *tag);
bool adr_record_remove_tag(AdrRecord *record, const char *tag);
bool adr_record_has_tag(const AdrRecord *record, const char *tag);

const char *adr_status_to_string(AdrStatus status);
AdrStatus adr_status_from_string(const char *str);

void adr_log_print(const AdrLog *log);
void adr_log_print_entry(const AdrRecord *record);

bool adr_log_export_markdown(const AdrLog *log, const char *filename);
bool adr_log_import_markdown(AdrLog *log, const char *filename);

size_t adr_log_count_by_status(const AdrLog *log, AdrStatus status);
size_t adr_log_count_total(const AdrLog *log);

typedef struct AdrSupersedeChain {
    unsigned int chain[ADR_LOG_MAX_ENTRIES];
    size_t length;
} AdrSupersedeChain;

bool adr_log_get_supersede_chain(const AdrLog *log, unsigned int id,
                                  AdrSupersedeChain *chain);

bool adr_log_validate(const AdrLog *log);

#endif /* ADR_SYSTEM_H */
