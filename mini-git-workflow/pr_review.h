#ifndef PR_REVIEW_H
#define PR_REVIEW_H

#include <stddef.h>
#include <time.h>

/* ── OID stub ───────────────────────────────────────────────────── */

#define PR_OID_RAWSZ 20
typedef struct { unsigned char id[PR_OID_RAWSZ]; } PrOid;

/* ── Code-owner entry ───────────────────────────────────────────── */

typedef struct {
    char   *path_pattern;   /* glob: "src/**", "docs/*.md" */
    char   *owners;         /* comma-separated usernames/teams, e.g. "@team-a" */
    int     min_approvals;  /* minimum reviews from owners */
} CodeOwnerEntry;

typedef struct {
    CodeOwnerEntry *entries;
    size_t          entry_count;
} CodeOwners;

/* ── CI check ───────────────────────────────────────────────────── */

typedef enum {
    CI_PENDING,
    CI_PASSED,
    CI_FAILED,
    CI_SKIPPED,
    CI_CANCELLED
} CiStatus;

typedef struct {
    char    *name;            /* e.g. "unit-tests", "lint" */
    char    *url;             /* link to build log */
    CiStatus status;
    time_t  queued_at;
    time_t  completed_at;
} CiCheck;

/* ── Review comment ──────────────────────────────────────────────── */

typedef struct {
    char    *file_path;
    size_t   line_start;
    size_t   line_end;       /* 0 for single-line */
    char    *body;           /* markdown */
    char    *author;
    time_t   created_at;
    int      resolved;       /* 1 = thread resolved */
    int      is_suggestion;  /* can be committed from UI */
} ReviewComment;

/* ── Review decision ─────────────────────────────────────────────── */

typedef enum {
    REVIEW_COMMENTED,   /* general feedback (no explicit approve/deny) */
    REVIEW_APPROVED,    /* LGTM */
    REVIEW_REQUESTED_CHANGES /* must be fixed before merge */
} ReviewDecision;

typedef struct {
    char           *reviewer;
    ReviewDecision  decision;
    char           *summary;      /* overall review summary */
    ReviewComment  *comments;
    size_t          comment_count;
    time_t          submitted_at;
} PullRequestReview;

/* ── Pull Request ────────────────────────────────────────────────── */

typedef enum {
    PR_MERGE_MERGE_COMMIT,
    PR_MERGE_SQUASH,
    PR_MERGE_REBASE
} PrMergeMethod;

typedef enum {
    PR_STATE_OPEN,
    PR_STATE_MERGED,
    PR_STATE_CLOSED,
    PR_STATE_DRAFT
} PrState;

typedef struct {
    size_t   additions;
    size_t   deletions;
    size_t   changed_files;
} PrDiffStat;

typedef struct {
    char          *title;
    char          *body;
    char          *template_name;  /* e.g. "bug_fix.md" */

    char          *head_branch;
    char          *base_branch;
    PrOid          head_oid;
    PrOid          base_oid;

    char          *author;
    char          **assignees;
    size_t          assignee_count;

    char          **labels;
    size_t          label_count;

    PrState         state;
    int             is_draft;
    int             mergeable;     /* no conflicts */
    int             can_be_rebased; /* source branch is ahead */

    PrDiffStat      diff_stat;

    /* CI */
    CiCheck        *checks;
    size_t          check_count;

    /* Review */
    PullRequestReview *reviews;
    size_t             review_count;

    CodeOwnerEntry *required_owners; /* from CODEOWNERS */
    size_t          required_owner_count;

    PrMergeMethod   merge_method;

    /* Merge conditions */
    int   require_ci_pass;
    int   require_approval_count;
    int   require_codeowner_approval; /* CODEOWNERS enforced */
    int   require_conversation_resolution; /* all threads resolved */
    int   require_linear_history;      /* no merge commits in PR */
    int   allow_merge_commits;

    /* Stale bot */
    int   stale_after_days;
    time_t last_activity;

    time_t created_at;
    time_t updated_at;
    time_t merged_at;
    time_t closed_at;
} PullRequestFull;

/* ── PR Template ────────────────────────────────────────────────── */

typedef enum {
    TEMPLATE_FEATURE,
    TEMPLATE_BUG_FIX,
    TEMPLATE_BREAKING,
    TEMPLATE_HOTFIX,
    TEMPLATE_DOCS
} PrTemplateKind;

typedef struct {
    char          *name;       /* filename: feature.md, bug_fix.md */
    PrTemplateKind kind;
    char          *content;    /* template with placeholders */
} PrTemplate;

/* ── Merge queue ────────────────────────────────────────────────── */

typedef struct {
    PullRequestFull **queue;
    size_t            queue_size;

    int               batch_size;    /* max concurrent merges */
    int               speculative;   /* try merge queue branch */

    /* backoff on failure */
    int               fail_count;
    time_t            backoff_until;
} MergeQueue;

/* ── API ────────────────────────────────────────────────────────── */

/* Code owners */
void   codeowners_load(const char *repo_root, CodeOwners *out);
int    codeowners_find_required(const CodeOwners *co, const char *path,
                                CodeOwnerEntry **out, size_t *out_count);
size_t codeowners_count_approvals(const CodeOwners *co,
                                   const PullRequestFull *pr);
void   codeowners_free(CodeOwners *co);

/* PR creation */
void   prfull_init(PullRequestFull *pr,
                    const char *title, const char *body,
                    const char *head, const char *base,
                    const char *author);
void   prfull_set_template(PullRequestFull *pr, const PrTemplate *tmpl);
void   prfull_add_assignee(PullRequestFull *pr, const char *username);
void   prfull_add_label(PullRequestFull *pr, const char *label);
void   prfull_free(PullRequestFull *pr);

/* CI */
void   ci_check_set(CiCheck *c, const char *name, CiStatus status);
int    prfull_ci_all_passed(const PullRequestFull *pr);

/* Reviews */
void   prfull_add_review(PullRequestFull *pr,
                          const char *reviewer,
                          ReviewDecision decision,
                          const char *summary);
void   review_add_comment(PullRequestReview *rv,
                          const char *file, size_t line,
                          const char *body, const char *author);
int    prfull_approval_count(const PullRequestFull *pr);
int    prfull_is_approved(const PullRequestFull *pr,
                          int min_approvals);
int    prfull_conversations_resolved(const PullRequestFull *pr);

/* Merge checks */
int    prfull_ready_to_merge(const PullRequestFull *pr);

/* PR template */
void   prtemplate_load(const char *tmpl_path, PrTemplate *out);
void   prtemplate_render(const PrTemplate *tmpl,
                          const char **keys, const char **values,
                          size_t pair_count, char **out);
void   prtemplate_free(PrTemplate *tmpl);

/* Merge queue */
void   mergequeue_init(MergeQueue *mq, int batch_size);
int    mergequeue_enqueue(MergeQueue *mq, PullRequestFull *pr);
int    mergequeue_process(MergeQueue *mq);
void   mergequeue_free(MergeQueue *mq);

#endif /* PR_REVIEW_H */
