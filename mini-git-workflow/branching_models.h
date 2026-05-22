#ifndef BRANCHING_MODELS_H
#define BRANCHING_MODELS_H

#include <stddef.h>
#include <time.h>

/* ── Common Branch Types ────────────────────────────────────────── */

typedef enum {
    BRANCH_FEATURE,
    BRANCH_RELEASE,
    BRANCH_HOTFIX,
    BRANCH_MAIN,
    BRANCH_DEVELOP,
    BRANCH_SUPPORT
} BranchKind;

typedef struct {
    char      *name;           /* short name */
    BranchKind kind;
    char      *base_ref;       /* branch created from */
    int         is_protected;  /* cannot force-push / delete */
    int         require_review;/* PR required to merge */
    int         require_ci;    /* CI must pass */
} Branch;

/* ── Feature Branch ─────────────────────────────────────────────── */
/*   Branch off main (or develop), merge back via PR.                */

typedef struct {
    Branch      branch;
    char       *ticket_id;     /* e.g. JIRA-1234 */
    char       *summary;
    time_t      created_at;
    time_t      last_activity;
    int         merged;        /* 0 = open, 1 = merged */
} FeatureBranch;

/* ── Git Flow model ──────────────────────────────────────────────── */
/*   master (production) ← release ← develop ← feature branches     */
/*   hotfix branches fork from master, merge into master & develop   */

typedef struct {
    Branch   master;           /* production-ready code */
    Branch   develop;          /* integration branch */

    FeatureBranch *features;   /* active feature branches */
    size_t         feature_count;

    FeatureBranch *releases;   /* release branches */
    size_t         release_count;

    FeatureBranch *hotfixes;   /* hotfix branches */
    size_t         hotfix_count;
} GitFlow;

/* ── GitHub Flow model ───────────────────────────────────────────── */
/*   main → feature branch → PR → review → merge                     */

typedef enum {
    PR_STATUS_DRAFT,
    PR_STATUS_OPEN,
    PR_STATUS_REVIEW,
    PR_STATUS_APPROVED,
    PR_STATUS_MERGED,
    PR_STATUS_CLOSED
} PullRequestStatus;

typedef enum {
    PR_MERGE_MERGE,
    PR_MERGE_SQUASH,
    PR_MERGE_REBASE
} PullRequestMergeStyle;

typedef struct {
    char               *title;
    char               *body;
    char               *head_ref;    /* source branch */
    char               *base_ref;    /* target branch */
    PullRequestStatus   status;
    PullRequestMergeStyle merge_style;

    char               *reviewer;    /* required reviewer(s) comma-separated */
    int                 review_count;
    int                 ci_passed;   /* all checks green */

    time_t              opened_at;
    time_t              merged_at;
} PullRequest;

typedef struct {
    Branch         main_branch;
    FeatureBranch *features;
    size_t         feature_count;

    PullRequest   *prs;
    size_t         pr_count;

    /* Branch protection */
    int            require_pr_review;   /* 1 = cannot push directly to main */
    int            require_status_checks;
    int            require_up_to_date;  /* must be up-to-date with base */
} GitHubFlow;

/* ── Trunk-Based Development ─────────────────────────────────────── */
/*   Developers commit to trunk (main) directly or via short-lived   */
/*   branches (hours, not days). Feature flags gate incomplete work.  */

typedef enum {
    FF_TYPE_RELEASE,    /* release toggled flag */
    FF_TYPE_EXPERIMENT, /* A/B experiment */
    FF_TYPE_OPS,        /* operational kill-switch */
    FF_TYPE_PERMISSION  /* permission-based access */
} FeatureFlagType;

typedef struct {
    char           *key;            /* e.g. "new-checkout-ui" */
    char           *description;
    FeatureFlagType type;
    int             enabled;
    double          rollout_pct;    /* 0.0 – 100.0 */

    /* stale flags should be removed */
    time_t          created_at;
    time_t          stale_at;       /* when declared stale */
    int             is_stale;
} FeatureFlag;

typedef struct {
    Branch         trunk;           /* "main" */
    FeatureFlag   *flags;
    size_t         flag_count;

    /* short-lived branch policy */
    int            max_branch_hours;  /* max lifetime before merge (e.g. 24) */
    size_t         max_commits_per_branch; /* keep merges small */

    /* release strategy */
    int            release_from_trunk;   /* 1 = tag trunk, 0 = release branch */
} TrunkBasedDev;

/* ── Branch Protection Rules ────────────────────────────────────── */

typedef struct {
    int require_pull_request;
    int required_approval_count;   /* minimum reviews */
    int dismiss_stale_reviews;     /* re-review after new commit */
    int require_ci_pass;

    /* enforce linear history (no merge commits) */
    int require_linear_history;

    /* restrict who can push */
    char **allowed_pushers;       /* usernames */
    size_t allowed_pusher_count;

    /* require signed commits */
    int require_signed_commits;
} BranchProtection;

/* ── API ────────────────────────────────────────────────────────── */

void  branch_init(Branch *b, const char *name, BranchKind kind,
                  const char *base_ref);
void  branch_free(Branch *b);

void  git_flow_init(GitFlow *gf);
void  git_flow_start_feature(GitFlow *gf, const char *name,
                              const char *ticket_id);
void  git_flow_finish_feature(GitFlow *gf, const char *name);
void  git_flow_start_release(GitFlow *gf, const char *version);
void  git_flow_finish_release(GitFlow *gf, const char *version);
void  git_flow_start_hotfix(GitFlow *gf, const char *version);
void  git_flow_finish_hotfix(GitFlow *gf, const char *version);
void  git_flow_free(GitFlow *gf);

void  github_flow_init(GitHubFlow *ghf);
void  github_flow_create_feature(GitHubFlow *ghf, const char *name);
void  github_flow_open_pr(GitHubFlow *ghf, const char *title,
                           const char *head, const char *base);
void  github_flow_review_pr(GitHubFlow *ghf, size_t pr_index,
                             int approve);
void  github_flow_merge_pr(GitHubFlow *ghf, size_t pr_index,
                            PullRequestMergeStyle style);
void  github_flow_free(GitHubFlow *ghf);

void  tbd_init(TrunkBasedDev *tbd);
void  tbd_add_feature_flag(TrunkBasedDev *tbd, const char *key,
                            const char *desc, FeatureFlagType type);
void  tbd_toggle_flag(TrunkBasedDev *tbd, const char *key, int enabled);
void  tbd_stale_cleanup(TrunkBasedDev *tbd);
void  tbd_free(TrunkBasedDev *tbd);

#endif /* BRANCHING_MODELS_H */
