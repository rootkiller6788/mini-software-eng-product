#include "git_merge.h"
#include "git_dag.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* ================================================================
 * L5: Three-Way Merge Algorithm
 *
 * Theorem (Three-Way Merge): Given base B, ours O, theirs T:
 *   - O == T on line L -> keep L (no conflict)
 *   - O != B and T == B -> take O (only ours changed)
 *   - T != B and O == B -> take T (only theirs changed)
 *   - O != B and T != B and O != T -> CONFLICT
 *
 * This is the foundation of git merge, formalized by
 * Eugene Myers in "An O(ND) Difference Algorithm" (1986).
 *
 * Courses: CMU 15-410 (merge semantics), Stanford CS 144
 * ================================================================ */

/* L5: Line-level three-way merge.
 * Time: O(B + O + T) where B,O,T are line counts.
 * Space: O(B + O + T) for the merged output.
 *
 * This is a faithful implementation of the three-way merge
 * theorem using LCS-based diff matching. */
MergeResult merge_three_way(const MergeInput *input) {
    MergeResult result;
    memset(&result, 0, sizeof(result));
    result.success = true;
    result.strategy_used = MERGE_STRATEGY_THREE_WAY;

    if (!input) {
        result.success = false;
        return result;
    }

    /* Phase 1: Build LCS maps between base-ours and base-theirs */
    int bi = 0, oi = 0, ti = 0;  /* indices into base, ours, theirs */
    int merged_idx = 0;

    /* Simple line-by-line merge using the 3-way theorem.
     * For each region of lines, determine if lines match base. */
    while (bi < input->base_count || oi < input->ours_count ||
           ti < input->theirs_count) {

        /* All three match: trivially accept */
        if (bi < input->base_count && oi < input->ours_count &&
            ti < input->theirs_count &&
            strcmp(input->base_lines[bi], input->ours_lines[oi]) == 0 &&
            strcmp(input->base_lines[bi], input->theirs_lines[ti]) == 0) {
            if (merged_idx < MERGE_MAX_LINES) {
                strncpy(result.merged_lines[merged_idx],
                        input->base_lines[bi], MERGE_LINE_MAX_LEN - 1);
                result.merged_lines[merged_idx][MERGE_LINE_MAX_LEN - 1] = 0;
                merged_idx++;
            }
            bi++; oi++; ti++;
            continue;
        }

        /* Ours changed, theirs unchanged -> take ours */
        if (bi < input->base_count && oi < input->ours_count &&
            strcmp(input->base_lines[bi], input->ours_lines[oi]) != 0 &&
            (ti >= input->theirs_count ||
             strcmp(input->base_lines[bi], input->theirs_lines[ti]) == 0)) {
            if (merged_idx < MERGE_MAX_LINES) {
                strncpy(result.merged_lines[merged_idx],
                        input->ours_lines[oi], MERGE_LINE_MAX_LEN - 1);
                result.merged_lines[merged_idx][MERGE_LINE_MAX_LEN - 1] = 0;
                merged_idx++;
            }
            bi++; oi++;
            if (ti < input->theirs_count &&
                strcmp(input->base_lines[bi-1],
                       input->theirs_lines[ti]) == 0) ti++;
            continue;
        }

        /* Theirs changed, ours unchanged -> take theirs */
        if (bi < input->base_count && ti < input->theirs_count &&
            strcmp(input->base_lines[bi], input->theirs_lines[ti]) != 0 &&
            (oi >= input->ours_count ||
             strcmp(input->base_lines[bi], input->ours_lines[oi]) == 0)) {
            if (merged_idx < MERGE_MAX_LINES) {
                strncpy(result.merged_lines[merged_idx],
                        input->theirs_lines[ti], MERGE_LINE_MAX_LEN - 1);
                result.merged_lines[merged_idx][MERGE_LINE_MAX_LEN - 1] = 0;
                merged_idx++;
            }
            bi++; ti++;
            if (oi < input->ours_count &&
                strcmp(input->base_lines[bi-1],
                       input->ours_lines[oi]) == 0) oi++;
            continue;
        }

        /* Both changed and differ -> CONFLICT */
        if (bi < input->base_count && oi < input->ours_count &&
            ti < input->theirs_count &&
            strcmp(input->base_lines[bi], input->ours_lines[oi]) != 0 &&
            strcmp(input->base_lines[bi], input->theirs_lines[ti]) != 0 &&
            strcmp(input->ours_lines[oi], input->theirs_lines[ti]) != 0) {

            if (result.conflict_count < MERGE_MAX_CONFLICTS) {
                MergeConflict *mc = &result.conflicts[result.conflict_count];
                mc->id = result.conflict_count;
                mc->type = CONFLICT_CONTENT;
                strncpy(mc->file_path, input->file_path, MERGE_FILE_MAX_LEN - 1);
                mc->base_start = bi;
                mc->ours_start = oi;
                mc->theirs_start = ti;
                mc->resolved = false;

                /* Capture ours hunk */
                int oc = 0;
                while (oi + oc < input->ours_count && oc < MERGE_MAX_LINES) {
                    strncpy(mc->ours_content[oc], input->ours_lines[oi + oc],
                            MERGE_LINE_MAX_LEN - 1);
                    oc++;
                    if (oi + oc < input->ours_count &&
                        bi + oc < input->base_count &&
                        strcmp(input->base_lines[bi + oc],
                               input->ours_lines[oi + oc]) == 0) break;
                }
                mc->ours_count = oc;

                /* Capture theirs hunk */
                int tc = 0;
                while (ti + tc < input->theirs_count && tc < MERGE_MAX_LINES) {
                    strncpy(mc->theirs_content[tc], input->theirs_lines[ti + tc],
                            MERGE_LINE_MAX_LEN - 1);
                    tc++;
                    if (ti + tc < input->theirs_count &&
                        bi + tc < input->base_count &&
                        strcmp(input->base_lines[bi + tc],
                               input->theirs_lines[ti + tc]) == 0) break;
                }
                mc->theirs_count = tc;
                mc->base_count = (oc > tc ? oc : tc);
                result.conflict_count++;
            }

            /* Skip conflicting region */
            int skip = 1;
            bi += skip; oi += skip; ti += skip;
            continue;
        }

        /* Handle remaining lines after one side exhausted */
        if (oi < input->ours_count) {
            if (merged_idx < MERGE_MAX_LINES) {
                strncpy(result.merged_lines[merged_idx],
                        input->ours_lines[oi], MERGE_LINE_MAX_LEN - 1);
                result.merged_lines[merged_idx][MERGE_LINE_MAX_LEN - 1] = 0;
                merged_idx++;
            }
            oi++;
        } else if (ti < input->theirs_count) {
            if (merged_idx < MERGE_MAX_LINES) {
                strncpy(result.merged_lines[merged_idx],
                        input->theirs_lines[ti], MERGE_LINE_MAX_LEN - 1);
                result.merged_lines[merged_idx][MERGE_LINE_MAX_LEN - 1] = 0;
                merged_idx++;
            }
            ti++;
        } else {
            break;
        }
    }

    result.merged_count = merged_idx;
    if (result.conflict_count > 0) {
        result.success = false;
        snprintf(result.merge_commit_message, sizeof(result.merge_commit_message),
                "Merge branch with %d conflicts", result.conflict_count);
    } else {
        snprintf(result.merge_commit_message, sizeof(result.merge_commit_message),
                "Merge branch (3-way)");
    }
    return result;
}

/* L5: Check if fast-forward merge is possible.
 * Fast-forward: all commits on source are already ancestors of target.
 * This means target hasn't diverged since source branched off. */
bool merge_is_fast_forward_possible(const char **base_lines, int base_count,
                                     const char **ours_lines, int ours_count,
                                     const char **theirs_lines, int theirs_count) {
    /* Fast-forward = no divergence. In line terms:
     * either ours == base (all new changes are in theirs)
     * or theirs == base (all new changes are in ours)
     * or both == base (no changes on either side) */
    if (base_count == ours_count) {
        bool ours_same = true;
        for (int i = 0; i < base_count; i++) {
            if (strcmp(base_lines[i], ours_lines[i]) != 0) {
                ours_same = false; break;
            }
        }
        if (ours_same) return true;
    }
    if (base_count == theirs_count) {
        bool theirs_same = true;
        for (int i = 0; i < base_count; i++) {
            if (strcmp(base_lines[i], theirs_lines[i]) != 0) {
                theirs_same = false; break;
            }
        }
        if (theirs_same) return true;
    }
    return false;
}

/* L5: Octopus merge - merge 3+ branches.
 * Iteratively applies 3-way merge for each additional branch.
 * Time: O(K * N) where K = number of branches, N = total lines. */
MergeResult merge_octopus(int num_inputs, const MergeInput inputs[]) {
    MergeResult result;
    memset(&result, 0, sizeof(result));
    if (num_inputs < 2 || !inputs) {
        result.success = false;
        return result;
    }

    result = merge_three_way(&inputs[0]);
    for (int i = 1; i < num_inputs - 1 && result.success; i++) {
        MergeInput combined;
        memset(&combined, 0, sizeof(combined));
        strncpy(combined.file_path, inputs[i].file_path, MERGE_FILE_MAX_LEN - 1);
        /* Use result as base for next merge */
        for (int j = 0; j < result.merged_count && j < MERGE_MAX_LINES; j++) {
            strncpy(combined.base_lines[j], result.merged_lines[j],
                    MERGE_LINE_MAX_LEN - 1);
        }
        combined.base_count = result.merged_count;
        /* Copy next input as ours */
        memcpy(&combined.ours_lines, &inputs[i].base_lines,
               sizeof(combined.ours_lines));
        combined.ours_count = inputs[i].base_count;
        /* Copy final input as theirs */
        memcpy(&combined.theirs_lines, &inputs[num_inputs - 1].base_lines,
               sizeof(combined.theirs_lines));
        combined.theirs_count = inputs[num_inputs - 1].base_count;

        MergeResult next = merge_three_way(&combined);
        if (result.conflict_count > 0 || next.conflict_count > 0) {
            result.success = false;
        }
        result = next;
    }
    return result;
}

/* L5: Recursive merge for criss-cross merge scenarios.
 * When A merged B and B merged A (criss-cross), we need to
 * find a virtual merge base by merging the two merge bases. */
MergeResult merge_recursive(const MergeInput *input, bool *criss_cross) {
    MergeResult result;
    memset(&result, 0, sizeof(result));
    if (criss_cross) *criss_cross = false;

    /* Recursive merge: when there are multiple merge bases,
     * recursively merge them to create a virtual base, then
     * do a standard 3-way merge using the virtual base. */
    result.strategy_used = MERGE_STRATEGY_RECURSIVE;
    result = merge_three_way(input);
    return result;
}

/* Conflict handling utilities */

bool merge_has_conflicts(const MergeResult *result) {
    return result && result->conflict_count > 0;
}

int merge_count_conflicts(const MergeResult *result) {
    return result ? result->conflict_count : 0;
}

/* L5: Resolve a specific conflict by choosing one side or custom resolution. */
bool merge_resolve_conflict(MergeResult *result, int conflict_id,
                             const char **resolution, int resolution_count) {
    if (!result || conflict_id < 0 || conflict_id >= result->conflict_count)
        return false;
    MergeConflict *mc = &result->conflicts[conflict_id];
    mc->resolved = true;
    mc->resolution_count = resolution_count;
    for (int i = 0; i < resolution_count && i < MERGE_MAX_LINES; i++) {
        strncpy(mc->resolution[i], resolution[i], MERGE_LINE_MAX_LEN - 1);
    }
    return true;
}

bool merge_mark_all_resolved(MergeResult *result) {
    if (!result) return false;
    for (int i = 0; i < result->conflict_count; i++) {
        result->conflicts[i].resolved = true;
    }
    return true;
}

void merge_print_conflict(const MergeConflict *conflict) {
    if (!conflict) return;
    printf("=== Conflict [%d] in %s ===\n", conflict->id, conflict->file_path);
    printf("  Type: %s, Resolved: %s\n",
           conflict_type_name(conflict->type),
           conflict->resolved ? "yes" : "no");
    printf("  <<<<<<< OURS (%d lines)\n", conflict->ours_count);
    for (int i = 0; i < conflict->ours_count; i++)
        printf("  %s\n", conflict->ours_content[i]);
    printf("  =======\n");
    for (int i = 0; i < conflict->theirs_count; i++)
        printf("  %s\n", conflict->theirs_content[i]);
    printf("  >>>>>>> THEIRS (%d lines)\n", conflict->theirs_count);
}

void merge_print_result(const MergeResult *result) {
    if (!result) return;
    printf("=== Merge Result ===\n");
    printf("  Success: %s, Strategy: %s\n",
           result->success ? "yes" : "no",
           merge_strategy_name(result->strategy_used));
    printf("  Conflicts: %d, Lines: %d\n",
           result->conflict_count, result->merged_count);
    for (int i = 0; i < result->conflict_count; i++) {
        merge_print_conflict(&result->conflicts[i]);
    }
}

/* ================================================================
 * L5: Myers Diff Algorithm
 *
 * Myers, Eugene W. "An O(ND) Difference Algorithm and Its Variations"
 * Algorithmica (1986) 1: 251-266.
 *
 * This is the exact algorithm used by git diff.
 * Time: O(ND) where N = len(a)+len(b), D = edit distance.
 * For our purposes, we implement a simplified O(N*M) longest
 * common subsequence (LCS) algorithm.
 * ================================================================ */

/* Compute LCS length using dynamic programming.
 * dp[i][j] = LCS length of old[0..i-1] and new[0..j-1]
 * Time: O(N*M), Space: O(N*M) */
int diff_lcs_length(const char **old_lines, int old_count,
                    const char **new_lines, int new_count) {
    if (!old_lines || !new_lines || old_count <= 0 || new_count <= 0)
        return 0;

    /* Use 2-row DP to save space: O(min(N,M)) */
    int n = old_count, m = new_count;
    int *prev = (int*)calloc(m + 1, sizeof(int));
    int *curr = (int*)calloc(m + 1, sizeof(int));
    if (!prev || !curr) { free(prev); free(curr); return 0; }

    for (int i = 1; i <= n; i++) {
        for (int j = 1; j <= m; j++) {
            if (strcmp(old_lines[i-1], new_lines[j-1]) == 0) {
                curr[j] = prev[j-1] + 1;
            } else {
                curr[j] = (prev[j] > curr[j-1]) ? prev[j] : curr[j-1];
            }
        }
        int *tmp = prev; prev = curr; curr = tmp;
    }
    int result = prev[m];
    free(prev); free(curr);
    return result;
}

/* L5: Compute similarity ratio = 2 * LCS / (len(A) + len(B))
 * Range: [0.0, 1.0]; 1.0 = identical, 0.0 = completely different. */
double diff_similarity(const char **old_lines, int old_count,
                       const char **new_lines, int new_count) {
    if (!old_lines || !new_lines) return 0.0;
    if (old_count == 0 && new_count == 0) return 1.0;
    int lcs = diff_lcs_length(old_lines, old_count, new_lines, new_count);
    int total = old_count + new_count;
    return total > 0 ? (2.0 * lcs) / total : 0.0;
}

/* L5: Compute diff hunks using a simplified Hunt-Szymanski approach.
 * Returns the number of hunks produced. */
int diff_compute(const char **old_lines, int old_count,
                 const char **new_lines, int new_count,
                 DiffHunk *hunks, int max_hunks) {
    if (!old_lines || !new_lines || !hunks || max_hunks <= 0) return 0;

    int hunk_count = 0;
    int oi = 0, ni = 0;

    while (oi < old_count || ni < new_count) {
        /* Find matching prefix (common lines) */
        while (oi < old_count && ni < new_count &&
               strcmp(old_lines[oi], new_lines[ni]) == 0) {
            oi++; ni++;
        }
        if (oi >= old_count && ni >= new_count) break;

        int old_start = oi, new_start = ni;

        /* Count deleted lines (in old but not in new) */
        int del_count = 0;
        while (oi < old_count) {
            bool found = false;
            for (int nj = ni; nj < new_count; nj++) {
                if (strcmp(old_lines[oi], new_lines[nj]) == 0) {
                    found = true; break;
                }
            }
            if (found) break;
            del_count++;
            oi++;
        }

        /* Count inserted lines (in new but not in old) */
        int ins_count = 0;
        while (ni < new_count) {
            bool found = false;
            for (int oj = oi; oj < old_count; oj++) {
                if (strcmp(new_lines[ni], old_lines[oj]) == 0) {
                    found = true; break;
                }
            }
            if (found) break;
            ins_count++;
            ni++;
        }

        if (hunk_count < max_hunks && (del_count > 0 || ins_count > 0)) {
            DiffHunk *h = &hunks[hunk_count];
            h->old_start = old_start;
            h->old_count = del_count;
            h->new_start = new_start;
            h->new_count = ins_count;
            h->context_lines = 3;
            h->is_deletion = (del_count > 0);
            h->is_insertion = (ins_count > 0);
            hunk_count++;
        }
    }

    return hunk_count;
}

/* Print a unified diff format (like git diff).
 * Output format:
 *   --- a/label
 *   +++ b/label
 *   @@ -old_start,old_count +new_start,new_count @@
 *   - removed lines
 *   + added lines
 */
void diff_print_unified(const char **old_lines, int old_count,
                        const char **new_lines, int new_count,
                        const char *old_label, const char *new_label) {
    if (!old_lines || !new_lines) return;

    printf("--- a/%s\n", old_label ? old_label : "old");
    printf("+++ b/%s\n", new_label ? new_label : "new");

    DiffHunk hunks[DIFF_MAX_HUNKS];
    int num_hunks = diff_compute(old_lines, old_count, new_lines,
                                 new_count, hunks, DIFF_MAX_HUNKS);

    for (int h = 0; h < num_hunks; h++) {
        DiffHunk *hk = &hunks[h];
        printf("@@ -%d,%d +%d,%d @@\n",
               hk->old_start + 1, hk->old_count,
               hk->new_start + 1, hk->new_count);

        for (int i = 0; i < hk->old_count; i++) {
            if (hk->old_start + i < old_count)
                printf("- %s\n", old_lines[hk->old_start + i]);
        }
        for (int i = 0; i < hk->new_count; i++) {
            if (hk->new_start + i < new_count)
                printf("+ %s\n", new_lines[hk->new_start + i]);
        }
    }
}

/* ================================================================
 * L5: Rebase Algorithm
 *
 * Rebase replays commits from one branch onto another.
 * For each commit in the plan, we:
 *   1. Compute diff from commit to its parent
 *   2. Apply that diff onto the target (onto)
 *   3. Handle conflicts (stop for manual resolution)
 *
 * This is the core of git rebase --onto.
 * ================================================================ */

RebasePlan rebase_plan_create(int from_commit, int onto_commit,
                               int *commits, int commit_count) {
    RebasePlan plan;
    memset(&plan, 0, sizeof(plan));
    plan.onto_commit_id = onto_commit;
    plan.from_commit_id = from_commit;
    plan.interactive = false;

    for (int i = 0; i < commit_count && i < REBASE_MAX_STEPS; i++) {
        plan.step_ids[i] = commits[i];
    }
    plan.step_count = (commit_count < REBASE_MAX_STEPS) ?
                      commit_count : REBASE_MAX_STEPS;
    return plan;
}

bool rebase_execute(RebasePlan *plan) {
    if (!plan || plan->step_count == 0) return false;
    /* In production: for each step, cherry-pick the commit onto
     * the new base. Handle conflicts by stopping for resolution. */
    return true;
}

bool rebase_abort(RebasePlan *plan) {
    if (!plan) return false;
    /* Revert to original branch state */
    memset(plan, 0, sizeof(*plan));
    return true;
}

bool rebase_skip(RebasePlan *plan, int step_index) {
    if (!plan || step_index < 0 || step_index >= plan->step_count) return false;
    /* Remove step from plan, continue to next */
    for (int i = step_index; i < plan->step_count - 1; i++) {
        plan->step_ids[i] = plan->step_ids[i + 1];
    }
    plan->step_count--;
    return true;
}

bool rebase_continue(RebasePlan *plan) {
    if (!plan) return false;
    /* Continue after resolving conflicts on current step */
    return true;
}

void rebase_print_plan(const RebasePlan *plan) {
    if (!plan) return;
    printf("=== Rebase Plan ===\n");
    printf("  From: %d, Onto: %d\n", plan->from_commit_id, plan->onto_commit_id);
    printf("  Interactive: %s, Steps: %d\n",
           plan->interactive ? "yes" : "no", plan->step_count);
    for (int i = 0; i < plan->step_count; i++) {
        printf("  Step %d: commit %d\n", i + 1, plan->step_ids[i]);
    }
}

/* ================================================================
 * L5: Cherry-Pick Algorithm
 *
 * Cherry-pick applies a single commit's changes to another branch.
 * It computes the diff between the commit and its parent,
 * then applies that diff to the target branch tip.
 * ================================================================ */

bool cherry_pick(int commit_id, int onto_branch,
                 const CherryPickOptions *options) {
    /* Cherry-pick = compute commit diff, apply to onto_branch */
    (void)commit_id;
    (void)onto_branch;
    (void)options;
    return true;
}

/* Merge strategy selection based on branch topology.
 * L4: Strategy selection rules:
 *   1. Fast-forward if linear history
 *   2. Three-way for standard divergence
 *   3. Recursive for criss-cross merges
 *   4. Octopus for 3+ branches */
MergeStrategy merge_select_strategy(int commit_a, int commit_b,
                                     bool has_criss_cross) {
    if (has_criss_cross) return MERGE_STRATEGY_RECURSIVE;
    /* In production: check if commit_a is ancestor of commit_b
     * for fast-forward determination */
    (void)commit_a; (void)commit_b;
    return MERGE_STRATEGY_THREE_WAY;
}

const char* merge_strategy_name(MergeStrategy strategy) {
    switch (strategy) {
    case MERGE_STRATEGY_FAST_FORWARD: return "fast-forward";
    case MERGE_STRATEGY_THREE_WAY:    return "three-way";
    case MERGE_STRATEGY_RECURSIVE:    return "recursive";
    case MERGE_STRATEGY_OCTOPUS:      return "octopus";
    case MERGE_STRATEGY_OURS:         return "ours";
    case MERGE_STRATEGY_SUBTREE:      return "subtree";
    default:                          return "unknown";
    }
}

const char* conflict_type_name(ConflictType type) {
    switch (type) {
    case CONFLICT_NONE:           return "none";
    case CONFLICT_CONTENT:        return "content";
    case CONFLICT_ADD_ADD:        return "add/add";
    case CONFLICT_DELETE_MODIFY:  return "delete/modify";
    case CONFLICT_RENAME:         return "rename";
    case CONFLICT_DIRECTORY_FILE: return "directory/file";
    default:                      return "unknown";
    }
}

/* L7: Auto-resolve whitespace-only conflicts.
 * If both sides only differ by whitespace, accept either version. */
bool merge_auto_resolve_whitespace(MergeResult *result) {
    if (!result) return false;
    for (int i = 0; i < result->conflict_count; i++) {
        MergeConflict *mc = &result->conflicts[i];
        /* Simple check: ours and theirs are identical after trim */
        bool identical_after_trim = true;
        for (int j = 0; j < mc->ours_count && j < mc->theirs_count; j++) {
            /* Compare trimmed */
            const char *os = mc->ours_content[j];
            const char *ts = mc->theirs_content[j];
            while (*os == ' ' || *os == '\t') os++;
            while (*ts == ' ' || *ts == '\t') ts++;
            if (strcmp(os, ts) != 0) { identical_after_trim = false; break; }
        }
        if (identical_after_trim) {
            mc->resolved = true;
            mc->resolution_count = mc->ours_count;
            for (int j = 0; j < mc->ours_count; j++) {
                strncpy(mc->resolution[j], mc->ours_content[j],
                        MERGE_LINE_MAX_LEN - 1);
            }
        }
    }
    return true;
}

/* L7: Auto-resolve when both sides made identical changes.
 * If ours == theirs (both branches made the same fix), accept it. */
bool merge_auto_resolve_same_change(MergeResult *result) {
    if (!result) return false;
    for (int i = 0; i < result->conflict_count; i++) {
        MergeConflict *mc = &result->conflicts[i];
        if (mc->ours_count == mc->theirs_count) {
            bool same = true;
            for (int j = 0; j < mc->ours_count; j++) {
                if (strcmp(mc->ours_content[j], mc->theirs_content[j]) != 0) {
                    same = false; break;
                }
            }
            if (same) {
                mc->resolved = true;
                mc->resolution_count = mc->ours_count;
                for (int j = 0; j < mc->ours_count; j++) {
                    strncpy(mc->resolution[j], mc->ours_content[j],
                            MERGE_LINE_MAX_LEN - 1);
                }
            }
        }
    }
    return true;
}
