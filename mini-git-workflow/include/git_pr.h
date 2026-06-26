#ifndef GIT_PR_H
#define GIT_PR_H

#include <stdint.h>
#include <stdbool.h>
#include <time.h>

/* ================================================================
 * L1-L6: Pull Request / Code Review Workflow
 *
 * Covers: GitHub/GitLab PR model, review states, approval gates
 *
 * L4: Four-Eyes Principle — at least one reviewer must approve
 * L6: Canonical Problem — PR-based code review pipeline
 *
 * Courses: Cambridge Part II SE, Georgia Tech CS 6300
 * ================================================================ */

#define PR_TITLE_MAX_LEN      128
#define PR_DESC_MAX_LEN       512
#define PR_MAX_REVIEWERS      8
#define PR_MAX_COMMENTS       32
#define PR_COMMENT_MAX_LEN    256
#define PR_MAX_LABELS         8
#define PR_LABEL_MAX_LEN      32
#define PR_FILE_MAX           128

/* L1: PR state machine */
typedef enum {
    PR_STATE_DRAFT     = 0,  /* work in progress, not ready for review */
    PR_STATE_OPEN      = 1,  /* ready for review */
    PR_STATE_REVIEW    = 2,  /* under active review */
    PR_STATE_APPROVED  = 3,  /* all reviewers approved */
    PR_STATE_CHANGES   = 4,  /* changes requested */
    PR_STATE_MERGED    = 5,
    PR_STATE_CLOSED    = 6,
    PR_STATE_REOPENED  = 7
} PRState;

/* L1: Review verdict */
typedef enum {
    REVIEW_PENDING     = 0,
    REVIEW_APPROVED    = 1,
    REVIEW_COMMENTED   = 2,  /* commented but no explicit approval */
    REVIEW_CHANGES     = 3,  /* changes requested */
    REVIEW_DISMISSED   = 4   /* review dismissed (stale) */
} ReviewVerdict;

/* L1: Comment severity */
typedef enum {
    COMMENT_INFO       = 0,
    COMMENT_SUGGESTION  = 1,
    COMMENT_NITPICK    = 2,
    COMMENT_IMPORTANT  = 3,
    COMMENT_BLOCKING   = 4
} CommentSeverity;

/* L1: Single review comment */
typedef struct {
    int             id;
    int             reviewer_id;
    char            file_path[PR_FILE_MAX];
    int             line_number;
    char            content[PR_COMMENT_MAX_LEN];
    CommentSeverity severity;
    bool            resolved;
    time_t          created_at;
    char            suggestion[PR_COMMENT_MAX_LEN];  /* code suggestion */
    bool            has_suggestion;
} ReviewComment;

/* L1: Review record */
typedef struct {
    int             id;
    int             reviewer_id;
    ReviewVerdict   verdict;
    ReviewComment   comments[PR_MAX_COMMENTS];
    int             comment_count;
    time_t          submitted_at;
    bool            stale;               /* needs re-review */
} Review;

/* L1: Pull Request */
typedef struct {
    int             id;
    char            title[PR_TITLE_MAX_LEN];
    char            description[PR_DESC_MAX_LEN];
    int             author_id;
    int             source_branch_id;
    int             target_branch_id;
    PRState         state;
    Review          reviews[PR_MAX_REVIEWERS];
    int             review_count;
    int             assigned_reviewers[PR_MAX_REVIEWERS];
    int             assigned_count;
    char            labels[PR_MAX_LABELS][PR_LABEL_MAX_LEN];
    int             label_count;
    int             commit_ids[64];
    int             commit_count;
    time_t          created_at;
    time_t          updated_at;
    time_t          merged_at;
    bool            ci_passed;
    bool            mergeable;           /* no conflicts */
    bool            draft;
    int             approval_count;
    int             blocking_review_count;
} PullRequest;

/* L1: PR review policy */
typedef struct {
    int             min_approvals;         /* minimum required approvals */
    bool            require_ci_pass;       /* CI must be green */
    bool            require_up_to_date;    /* must be up-to-date with target */
    bool            dismiss_stale_reviews; /* dismiss reviews on new commits */
    bool            allow_self_approve;    /* can author approve own PR */
    bool            require_resolve_all;   /* all threads resolved before merge */
} PRReviewPolicy;

/* L3: PR Manager */
typedef struct {
    PullRequest     prs[64];
    int             pr_count;
    int             next_pr_id;
    PRReviewPolicy  policy;
} PRManager;

/* === API Declarations === */

/* PR Lifecycle */
void        pr_manager_init(PRManager *pm, const PRReviewPolicy *policy);
int         pr_create(PRManager *pm, const char *title, const char *desc,
                      int author, int source_branch, int target_branch);
bool        pr_submit_for_review(PRManager *pm, int pr_id);
bool        pr_close(PRManager *pm, int pr_id);
bool        pr_reopen(PRManager *pm, int pr_id);
bool        pr_merge(PRManager *pm, int pr_id);

/* Review Operations */
int         pr_assign_reviewer(PRManager *pm, int pr_id, int reviewer_id);
bool        pr_submit_review(PRManager *pm, int pr_id, int reviewer_id,
                             ReviewVerdict verdict);
int         pr_add_comment(PRManager *pm, int pr_id, int reviewer_id,
                           const char *file_path, int line,
                           const char *content, CommentSeverity severity);
bool        pr_resolve_comment(PRManager *pm, int pr_id, int comment_id);
bool        pr_dismiss_review(PRManager *pm, int pr_id, int review_id);

/* State Checks */
bool        pr_is_mergeable(PRManager *pm, int pr_id);
bool        pr_is_approved(PRManager *pm, int pr_id);
bool        pr_needs_changes(PRManager *pm, int pr_id);
int         pr_pending_review_count(PRManager *pm, int pr_id);
bool        pr_all_comments_resolved(PRManager *pm, int pr_id);

/* Label Management */
bool        pr_add_label(PRManager *pm, int pr_id, const char *label);
bool        pr_remove_label(PRManager *pm, int pr_id, const char *label);
bool        pr_has_label(PRManager *pm, int pr_id, const char *label);

/* CI Integration */
void        pr_set_ci_status(PRManager *pm, int pr_id, bool passed);
bool        pr_ci_check(PRManager *pm, int pr_id);

/* Policy */
void        pr_configure_policy(PRManager *pm, const PRReviewPolicy *policy);
bool        pr_check_policy(PRManager *pm, int pr_id);

/* Search & Filter */
int         pr_find_open(PRManager *pm, int *results, int max_results);
int         pr_find_by_author(PRManager *pm, int author_id, int *results, int max_results);
int         pr_find_awaiting_review(PRManager *pm, int *results, int max_results);
int         pr_find_approved(PRManager *pm, int *results, int max_results);

/* Print & Debug */
void        pr_print(const PRManager *pm, int pr_id);
void        pr_print_summary(const PRManager *pm, int pr_id);
void        pr_print_all(const PRManager *pm);
const char* pr_state_name(PRState state);
const char* review_verdict_name(ReviewVerdict verdict);

#endif
