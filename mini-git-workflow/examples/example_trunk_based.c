#include <stdio.h>
#include "git_dag.h"
#include "git_branch.h"
#include "git_pr.h"
#include "git_ci.h"

/* L6: Canonical Problem — Trunk-Based Development Demo
 *
 * Implements Paul Hammant's Trunk-Based Development model:
 *   Single main branch + short-lived feature branches (< 3 days)
 *   Squash merge + automated CI verification
 *
 * Key principles:
 *   - All developers commit to main at least once per day
 *   - Feature branches live < 3 days
 *   - Square merge (single commit on main)
 *   - CI must be green before merge
 *   - Pair programming reduces need for code review
 */

int main() {
    printf("=== Trunk-Based Development Demo ===\n\n");

    /* Setup */
    GitDAG dag;
    git_dag_init(&dag);
    BranchManager bm;
    branch_manager_init(&bm);
    branch_configure_strategy(&bm, STRATEGY_TRUNK_BASED);

    printf("Strategy: %s\n", git_dag_strategy_name(bm.config.strategy));
    printf("Squash merge: %s\n", bm.config.squash_merge ? "yes" : "no");
    printf("Rebase before merge: %s\n", bm.config.rebase_merge ? "yes" : "no");
    printf("Max branch age: %d days\n", bm.config.max_branch_age_days);
    printf("Delete after merge: %s\n\n", bm.config.delete_after_merge ? "yes" : "no");

    /* Step 1: Initialize trunk (main) */
    int root = git_dag_add_root_commit(&dag, "trunk00", "team", "chore: initialize trunk");
    int main_br = branch_create(&bm, "main", BRANCH_MAIN, root, "admin");
    printf("[1] Trunk initialized at commit %d\n\n", root);

    /* Step 2: Developer A creates short-lived feature branch */
    int feat_a = branch_create(&bm, "feature/add-cache", BRANCH_FEATURE, root, "alice");
    printf("[2] Alice created feature branch: %s\n", bm.branches[feat_a].name);

    /* Simulate a few commits on the feature branch */
    int c1 = git_dag_add_commit(&dag, "a1", &root, 1, "alice", "feat: add cache interface");
    int c2 = git_dag_add_commit(&dag, "a2", &c1, 1, "alice", "feat: implement LRU cache");
    bm.branches[feat_a].tip_commit_id = c2;
    printf("    Added 2 commits to feature branch (tip: %d)\n", c2);

    /* Step 3: Set up PR and CI */
    PRManager pm;
    PRReviewPolicy policy = {1, true, true, true, false, false};
    pr_manager_init(&pm, &policy);

    int pr = pr_create(&pm, "Add LRU Cache", "Performance: LRU caching layer",
                       100, feat_a, main_br);
    pr_submit_for_review(&pm, pr);
    pr_assign_reviewer(&pm, pr, 200);
    printf("[3] PR #%d created for cache feature\n", pr);

    /* Step 4: CI Pipeline runs */
    CIManager cm;
    ci_manager_init(&cm);
    ci_generate_default_pipeline(&cm, pr, feat_a, "a2_commit_sha");
    printf("[4] CI pipeline generated for PR #%d\n", pr);

    /* Simulate CI pass */
    pr_set_ci_status(&pm, pr, true);
    printf("    CI check: %s\n", pr_ci_check(&pm, pr) ? "PASS" : "FAIL");

    /* Step 5: Review and approval */
    pr_submit_review(&pm, pr, 200, REVIEW_APPROVED);
    printf("[5] Review approved by reviewer 200\n");

    /* Step 6: Check merge readiness */
    bool mergeable = pr_is_mergeable(&pm, pr);
    printf("[6] PR mergeable: %s\n", mergeable ? "YES" : "NO");

    if (mergeable) {
        /* Squash merge: single commit on main */
        int squash = git_dag_add_commit(&dag, "squash01", &root, 1,
                                         "alice", "feat: LRU cache layer (squashed)");
        bm.branches[main_br].tip_commit_id = squash;
        branch_merge(&bm, feat_a, main_br);
        pr_merge(&pm, pr);
        printf("    Squash-merged into main at commit %d\n", squash);
    }

    /* Step 7: Developer B starts new work from latest trunk */
    int feat_b = branch_create(&bm, "feature/add-logging", BRANCH_FEATURE,
                                bm.branches[main_br].tip_commit_id, "bob");
    printf("[7] Bob created feature branch from latest main: %s\n",
           bm.branches[feat_b].name);

    /* Summary */
    printf("\n=== Trunk-Based Development Summary ===\n");
    printf("Main branch commits: %d\n", bm.branches[main_br].tip_commit_id + 1);
    printf("Feature branches merged: %d\n", 1);
    printf("CI pipelines run: %d\n", cm.pipeline_count);
    branch_print_all(&bm);

    return 0;
}
