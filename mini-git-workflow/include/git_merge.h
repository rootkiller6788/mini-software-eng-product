#ifndef GIT_MERGE_H
#define GIT_MERGE_H

#include <stdint.h>
#include <stdbool.h>

/* ================================================================
 * L1-L5: Merge & Rebase — Three-Way Merge Algorithm
 *
 * L4: Three-Way Merge Theorem:
 *   Given base B, ours O, theirs T:
 *   - If O == T for line L → keep L (both agree)
 *   - If O != B and T == B → take O (only ours changed)
 *   - If T != B and O == B → take T (only theirs changed)
 *   - If O != B and T != B and O != T → CONFLICT
 *
 * L5: Myers Diff Algorithm (longest common subsequence)
 *     Hunt-Szymanski algorithm for computing LCS efficiently
 *
 * Courses: CMU 15-410 OS, Stanford CS 144
 * ================================================================ */

#define MERGE_MAX_LINES        256
#define MERGE_MAX_CONFLICTS    32
#define MERGE_LINE_MAX_LEN     256
#define MERGE_FILE_MAX_LEN     128
#define DIFF_MAX_HUNKS         32
#define REBASE_MAX_STEPS       32

/* L1: Merge strategy types */
typedef enum {
    MERGE_STRATEGY_FAST_FORWARD  = 0,  /* linear, no merge commit needed */
    MERGE_STRATEGY_THREE_WAY     = 1,  /* standard 3-way merge */
    MERGE_STRATEGY_RECURSIVE     = 2,  /* criss-cross merge */
    MERGE_STRATEGY_OCTOPUS       = 3,  /* merge >2 branches */
    MERGE_STRATEGY_OURS          = 4,  /* keep ours entirely */
    MERGE_STRATEGY_SUBTREE       = 5   /* subtree-aware merge */
} MergeStrategy;

/* L1: Conflict marker type */
typedef enum {
    CONFLICT_NONE           = 0,
    CONFLICT_CONTENT        = 1,  /* same lines modified differently */
    CONFLICT_ADD_ADD        = 2,  /* both added same file */
    CONFLICT_DELETE_MODIFY  = 3,  /* one deleted, other modified */
    CONFLICT_RENAME         = 4,  /* renamed differently */
    CONFLICT_DIRECTORY_FILE = 5   /* dir vs file conflict */
} ConflictType;

/* L1: A single conflict */
typedef struct {
    int         id;
    ConflictType type;
    char        file_path[MERGE_FILE_MAX_LEN];
    int         base_start;    /* line in base */
    int         base_count;
    int         ours_start;    /* line in ours */
    int         ours_count;
    int         theirs_start;  /* line in theirs */
    int         theirs_count;
    char        ours_content[MERGE_MAX_LINES][MERGE_LINE_MAX_LEN];
    char        theirs_content[MERGE_MAX_LINES][MERGE_LINE_MAX_LEN];
    bool        resolved;
    char        resolution[MERGE_MAX_LINES][MERGE_LINE_MAX_LEN];
    int         resolution_count;
} MergeConflict;

/* L1: Three-way merge input (base, ours, theirs) */
typedef struct {
    char        file_path[MERGE_FILE_MAX_LEN];
    char        base_lines[MERGE_MAX_LINES][MERGE_LINE_MAX_LEN];
    int         base_count;
    char        ours_lines[MERGE_MAX_LINES][MERGE_LINE_MAX_LEN];
    int         ours_count;
    char        theirs_lines[MERGE_MAX_LINES][MERGE_LINE_MAX_LEN];
    int         theirs_count;
} MergeInput;

/* L1: Merge result */
typedef struct {
    bool        success;
    MergeStrategy strategy_used;
    char        merged_lines[MERGE_MAX_LINES][MERGE_LINE_MAX_LEN];
    int         merged_count;
    MergeConflict conflicts[MERGE_MAX_CONFLICTS];
    int         conflict_count;
    char        merge_commit_message[512];
} MergeResult;

/* L1: Diff hunk */
typedef struct {
    int         old_start;
    int         old_count;
    int         new_start;
    int         new_count;
    int         context_lines;
    bool        is_insertion;
    bool        is_deletion;
} DiffHunk;

/* L1: Rebase plan (list of commits to replay) */
typedef struct {
    int         step_ids[REBASE_MAX_STEPS];
    int         step_count;
    int         onto_commit_id;
    int         from_commit_id;
    bool        interactive;     /* pick/reword/squash/fixup per commit */
} RebasePlan;

/* L1: Cherry-pick options */
typedef struct {
    bool        allow_empty;
    bool        preserve_author;
    bool        sign_off;
    int         mainline;        /* for merge commits, parent number */
} CherryPickOptions;

/* === API Declarations === */

/* Three-Way Merge (L5: algorithm) */
MergeResult merge_three_way(const MergeInput *input);
bool        merge_is_fast_forward_possible(const char **base_lines, int base_count,
                                           const char **ours_lines, int ours_count,
                                           const char **theirs_lines, int theirs_count);
MergeResult merge_octopus(int num_merge_inputs, const MergeInput inputs[]);
MergeResult merge_recursive(const MergeInput *input, bool *criss_cross);

/* Conflict Handling */
bool        merge_has_conflicts(const MergeResult *result);
int         merge_count_conflicts(const MergeResult *result);
bool        merge_resolve_conflict(MergeResult *result, int conflict_id,
                                   const char **resolution, int resolution_count);
bool        merge_mark_all_resolved(MergeResult *result);
void        merge_print_conflict(const MergeConflict *conflict);
void        merge_print_result(const MergeResult *result);

/* Diff Algorithm (L5: Myers Diff / Hunt-Szymanski) */
int         diff_compute(const char **old_lines, int old_count,
                         const char **new_lines, int new_count,
                         DiffHunk *hunks, int max_hunks);
int         diff_lcs_length(const char **old_lines, int old_count,
                            const char **new_lines, int new_count);
double      diff_similarity(const char **old_lines, int old_count,
                            const char **new_lines, int new_count);
void        diff_print_unified(const char **old_lines, int old_count,
                               const char **new_lines, int new_count,
                               const char *old_label, const char *new_label);

/* Rebase (L5: algorithm) */
RebasePlan  rebase_plan_create(int from_commit, int onto_commit,
                               int *commits, int commit_count);
bool        rebase_execute(RebasePlan *plan);
bool        rebase_abort(RebasePlan *plan);
bool        rebase_skip(RebasePlan *plan, int step_index);
bool        rebase_continue(RebasePlan *plan);
void        rebase_print_plan(const RebasePlan *plan);

/* Cherry-Pick (L5: algorithm) */
bool        cherry_pick(int commit_id, int onto_branch,
                        const CherryPickOptions *options);

/* Merge strategy selection */
MergeStrategy merge_select_strategy(int commit_a, int commit_b, bool has_criss_cross);
const char*   merge_strategy_name(MergeStrategy strategy);
const char*   conflict_type_name(ConflictType type);

/* Auto-merge heuristics */
bool        merge_auto_resolve_whitespace(MergeResult *result);
bool        merge_auto_resolve_same_change(MergeResult *result);

#endif
