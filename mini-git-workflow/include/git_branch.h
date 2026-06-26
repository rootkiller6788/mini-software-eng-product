#ifndef GIT_BRANCH_H
#define GIT_BRANCH_H

#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include "git_dag.h"

/* ================================================================
 * L1-L2: Branching Models — Core definitions and concepts
 *
 * Covers:
 *   Git Flow (Vincent Driessen, 2010)
 *   Trunk-Based Development (Paul Hammant, 2013)
 *   GitHub Flow (Scott Chacon, 2011)
 *
 * L4: Branch naming conventions (Conventional Commits v1.0.0)
 * ================================================================ */

#define BRANCH_NAME_MAX_LEN    128
#define BRANCH_MAX_FILES       64
#define BRANCH_FILE_NAME_MAX   128
#define BRANCH_MAX_RULES       32

/* L1: Branch type enumeration */
typedef enum {
    BRANCH_MAIN       = 0,  /* main/master — production-ready */
    BRANCH_DEVELOP    = 1,  /* develop — integration branch */
    BRANCH_FEATURE    = 2,  /* feature/xxx — new functionality */
    BRANCH_RELEASE    = 3,  /* release/x.y.z — release preparation */
    BRANCH_HOTFIX     = 4,  /* hotfix/x.y.z — urgent production fix */
    BRANCH_BUGFIX     = 5,  /* bugfix/xxx — non-urgent fix */
    BRANCH_EPHEMERAL  = 6,  /* short-lived (TBD) */
    BRANCH_CUSTOM     = 7
} BranchType;

/* L1: Branch lifecycle state */
typedef enum {
    BRANCH_STATE_CREATED   = 0,
    BRANCH_STATE_ACTIVE    = 1,
    BRANCH_STATE_MERGED    = 2,
    BRANCH_STATE_CLOSED    = 3,
    BRANCH_STATE_STALE     = 4,
    BRANCH_STATE_DELETED   = 5
} BranchState;

/* L1: File change information in a branch */
typedef struct {
    int     id;
    char    path[BRANCH_FILE_NAME_MAX];
    int     lines_added;
    int     lines_deleted;
    bool    is_binary;
    bool    is_renamed;
    char    old_path[BRANCH_FILE_NAME_MAX];
} BranchFileChange;

/* L1: Branch definition */
typedef struct {
    int             id;
    char            name[BRANCH_NAME_MAX_LEN];
    BranchType      type;
    BranchState     state;
    int             base_commit_id;      /* where branch diverged from parent */
    int             tip_commit_id;       /* latest commit on branch */
    int             parent_branch_id;    /* branch this was created from */
    time_t          created_at;
    time_t          merged_at;
    char            created_by[64];
    BranchFileChange changes[BRANCH_MAX_FILES];
    int             change_count;
    bool            is_protected;        /* requires PR review */
    bool            requires_ci_pass;   /* requires CI green */
} GitBranch;

/* L1: Branching model configuration */
typedef struct {
    BranchStrategy   strategy;            /* overall strategy enum */
    char            main_branch[BRANCH_NAME_MAX_LEN];    /* "main" or "master" */
    char            develop_branch[BRANCH_NAME_MAX_LEN]; /* "develop" */
    bool            use_release_branches;
    bool            use_hotfix_branches;
    bool            squash_merge;          /* GitHub-style squash */
    bool            rebase_merge;          /* rebase before merge */
    bool            delete_after_merge;    /* auto-delete merged branches */
    int             max_branch_age_days;   /* stale branch threshold */
} BranchModelConfig;

/* L1: Branch protection rule */
typedef struct {
    int             id;
    char            branch_pattern[BRANCH_NAME_MAX_LEN];
    bool            require_pr;
    int             min_approvals;
    bool            require_ci;
    bool            require_signed_commits;
    bool            block_force_push;
    bool            require_linear_history;
    bool            require_up_to_date;    /* must be up-to-date with base */
} BranchProtectionRule;

/* L3: Branch manager — engineering structure */
typedef struct {
    GitBranch       branches[GIT_MAX_BRANCHES];
    int             branch_count;
    BranchModelConfig config;
    BranchProtectionRule rules[BRANCH_MAX_RULES];
    int             rule_count;
    int             (*dag)(void);          /* callback to git_dag */
} BranchManager;

/* API Declarations */

/* Branch lifecycle */
void        branch_manager_init(BranchManager *bm);
int         branch_create(BranchManager *bm, const char *name, BranchType type,
                          int base_commit_id, const char *created_by);
bool        branch_merge(BranchManager *bm, int branch_id, int into_branch_id);
bool        branch_close(BranchManager *bm, int branch_id);
bool        branch_delete(BranchManager *bm, int branch_id);
bool        branch_reopen(BranchManager *bm, int branch_id);
int         branch_find_by_name(BranchManager *bm, const char *name);
int         branch_find_by_type(BranchManager *bm, BranchType type, int *results, int max_results);

/* Branch state queries */
bool        branch_is_mergeable(BranchManager *bm, int branch_id, int into_branch_id);
bool        branch_is_stale(BranchManager *bm, int branch_id);
int         branch_divergence_count(BranchManager *bm, int branch_id, int base_branch_id);
int         branch_ahead_count(BranchManager *bm, int branch_id, int base_branch_id);
int         branch_behind_count(BranchManager *bm, int branch_id, int base_branch_id);

/* Branch strategy */
void        branch_configure_strategy(BranchManager *bm, BranchStrategy strategy);
bool        branch_validate_name(BranchManager *bm, const char *name);
const char* branch_type_name(BranchType type);
const char* branch_state_name(BranchState state);

/* Protection rules */
int         branch_add_protection_rule(BranchManager *bm, const char *pattern);
bool        branch_check_protection(BranchManager *bm, int branch_id);
bool        branch_match_pattern(const char *branch_name, const char *pattern);

/* File tracking */
int         branch_add_file_change(BranchManager *bm, int branch_id,
                                   const char *path, int added, int deleted);
int         branch_total_changes(BranchManager *bm, int branch_id);

/* Model operations */
bool        branch_should_auto_delete(BranchManager *bm, BranchType type);
int         branch_list_active(BranchManager *bm, int *results, int max_results);
int         branch_list_stale(BranchManager *bm, int *results, int max_results);
void        branch_print(BranchManager *bm, int branch_id);
void        branch_print_all(BranchManager *bm);
void        branch_print_tree(BranchManager *bm);

#endif
