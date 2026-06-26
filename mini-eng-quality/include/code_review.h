#ifndef CODE_REVIEW_H
#define CODE_REVIEW_H

#include <stddef.h>
#include <stdint.h>

#define CR_MAX_DIFF_FILES    256
#define CR_MAX_HUNKS         128
#define CR_MAX_COMMENTS      512
#define CR_MAX_CHECKLIST     32
#define CR_MAX_OWNERS        64
#define CR_MAX_REVIEWERS     16
#define CR_MAX_LINE_LEN      1024
#define CR_MAX_FILE_PATH     256

typedef enum {
    CR_HUNK_ADD,
    CR_HUNK_DEL,
    CR_HUNK_MOD
} cr_hunk_type_e;

typedef enum {
    CR_COMMENT_OPEN,
    CR_COMMENT_RESOLVED,
    CR_COMMENT_WONT_FIX,
    CR_COMMENT_DUPLICATE
} cr_comment_status_e;

typedef enum {
    CR_SEVERITY_NIT,
    CR_SEVERITY_MINOR,
    CR_SEVERITY_MAJOR,
    CR_SEVERITY_CRITICAL
} cr_severity_e;

typedef enum {
    CR_REVIEW_PENDING,
    CR_REVIEW_APPROVED,
    CR_REVIEW_CHANGES_REQUESTED,
    CR_REVIEW_COMMENTED
} cr_review_status_e;

typedef struct {
    int  start_old;
    int  count_old;
    int  start_new;
    int  count_new;
    cr_hunk_type_e type;
    char header[256];
    char *lines[CR_MAX_LINE_LEN];
    int  line_count;
} cr_hunk_t;

typedef struct {
    char path_old[CR_MAX_FILE_PATH];
    char path_new[CR_MAX_FILE_PATH];
    cr_hunk_t hunks[CR_MAX_HUNKS];
    int  hunk_count;
    int  added_lines;
    int  deleted_lines;
} cr_diff_file_t;

typedef struct {
    int  id;
    char file_path[CR_MAX_FILE_PATH];
    int  line_number;
    cr_severity_e severity;
    cr_comment_status_e status;
    char author[64];
    char body[1024];
    char resolved_by[64];
    int  thread_id;
    int  parent_id;
} cr_comment_t;

typedef struct {
    char path_pattern[CR_MAX_FILE_PATH];
    char owners[CR_MAX_OWNERS][64];
    int  owner_count;
} cr_ownership_t;

typedef struct {
    char *items[CR_MAX_CHECKLIST];
    int  checked[CR_MAX_CHECKLIST];
    int  count;
} cr_checklist_t;

typedef struct {
    int  min_approvals;
    int  require_owner_approval;
    int  auto_resolve_threads;
    int  stale_days;
    char required_reviewers[CR_MAX_REVIEWERS][64];
    int  required_count;
} cr_approval_rules_t;

typedef struct {
    int  total_reviews;
    int  approved;
    int  rejected;
    int  comments_total;
    int  comments_resolved;
    double avg_time_to_resolve_hours;
    double avg_comments_per_review;
} cr_stats_t;

typedef struct {
    cr_diff_file_t files[CR_MAX_DIFF_FILES];
    int  file_count;
    cr_comment_t  comments[CR_MAX_COMMENTS];
    int  comment_count;
    cr_checklist_t checklist;
    cr_approval_rules_t rules;
    cr_review_status_e status;
    cr_stats_t stats;
    char reviewers[CR_MAX_REVIEWERS][64];
    int  reviewer_count;
    char title[256];
    int  id;
} cr_review_t;

int   cr_parse_diff(cr_review_t *review, const char *diff_text);
int   cr_add_hunk(cr_diff_file_t *file, int start_old, int count_old,
                  int start_new, int count_new, cr_hunk_type_e type);
int   cr_add_comment(cr_review_t *review, int line, const char *body,
                     cr_severity_e severity, const char *author);
int   cr_resolve_comment(cr_review_t *review, int comment_id,
                         const char *resolver);
int   cr_reopen_comment(cr_review_t *review, int comment_id);
int   cr_reply_to_comment(cr_review_t *review, int parent_id,
                          const char *body, const char *author);
void  cr_load_ownership(cr_ownership_t *ownerships, int count,
                        const char *csv_path);
int   cr_suggest_reviewers(cr_review_t *review,
                           const cr_ownership_t *ownerships,
                           int ownership_count);
int   cr_init_checklist(cr_checklist_t *cl);
int   cr_add_checklist_item(cr_checklist_t *cl, const char *item);
int   cr_check_item(cr_checklist_t *cl, int index);
int   cr_all_checked(const cr_checklist_t *cl);
void  cr_set_approval_rules(cr_approval_rules_t *rules,
                            int min_approvals,
                            int require_owner);
int   cr_check_approval(const cr_review_t *review);
void  cr_compute_stats(cr_review_t *review);
void  cr_print_stats(const cr_stats_t *stats);
int   cr_is_approved(const cr_review_t *review);
void  cr_assign_reviewer(cr_review_t *review, const char *reviewer);

#endif
