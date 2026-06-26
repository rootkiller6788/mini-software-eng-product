#include "git_dag.h"
#include "git_branch.h"
#include "git_merge.h"
#include "git_conflict.h"
#include "git_ci.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main(void) {
    srand(42);
    printf("===== Git Workflow Demo =====\n\n");

    /* Git DAG */
    printf("--- Git DAG ---\n");
    GitDag dag; git_dag_init(&dag);
    int c0 = git_dag_commit(&dag, -1, "Initial commit");
    int c1 = git_dag_commit(&dag, c0, "Add README");
    int c2 = git_dag_commit(&dag, c1, "Add core module");
    git_dag_branch(&dag, "main");
    git_dag_checkout(&dag, "main");
    printf("main branch tip: %s\n", dag.commits[dag.branches[0].tip_commit].hash);
    git_dag_branch(&dag, "feature/login");
    git_dag_checkout(&dag, "feature/login");
    int c3 = git_dag_commit(&dag, c2, "Add login page");
    int c4 = git_dag_commit(&dag, c3, "Add login tests");
    git_dag_checkout(&dag, "main");
    int c5 = git_dag_commit(&dag, c2, "Fix typo in README");
    int base = git_dag_common_ancestor(&dag, c4, c5);
    printf("Common ancestor of feature/login and main: %s\n",
           base >= 0 ? dag.commits[base].hash : "none");
    git_dag_print(&dag);
    git_dag_print_log(&dag);
    printf("\n");

    /* Branch models */
    printf("--- Branch Models ---\n");
    BranchModel bm;
    branch_model_init(&bm, BSTRAT_GITFLOW);
    branch_model_print(&bm);
    ReleaseTag rt;
    branch_model_release(&bm, &rt, 1, 0, 0);
    printf("Release: %s (tag: %s)\n", rt.version, rt.tag);
    printf("\n");

    branch_model_init(&bm, BSTRAT_GITHUB_FLOW);
    branch_model_print(&bm);
    printf("Valid: %s\n", branch_model_validate(&bm) ? "YES" : "NO");
    printf("\n");

    /* 3-way merge */
    printf("--- 3-Way Merge ---\n");
    MergeResult mr;
    memset(&mr, 0, sizeof(mr));
    char merged[512];
    bool ok = git_merge_three_way(&mr, "Hello", "Hello World", "Hello", merged, sizeof(merged));
    printf("Merge (ours changed): %s -> %s\n", ok ? "OK" : "CONFLICT", merged);

    mr.conflicts = 0;
    ok = git_merge_three_way(&mr, "Hello", "Hello", "Hello World", merged, sizeof(merged));
    printf("Merge (theirs changed): %s -> %s\n", ok ? "OK" : "CONFLICT", merged);

    mr.conflicts = 0;
    ok = git_merge_three_way(&mr, "Hello", "Hello Alice", "Hello Bob", merged, sizeof(merged));
    printf("Merge (both changed): %s -> %s\n", ok ? "OK" : "CONFLICT", merged);

    git_merge_branches(&mr, 1, 0, c2);
    merge_print_result(&mr);
    printf("\n");

    /* Conflict resolution */
    printf("--- Conflict Resolution ---\n");
    Conflict conflict;
    conflict_detect("Alice's version", "Bob's version", "Original", &conflict);
    printf("Conflict type: %d (0=none,1=content)\n", conflict.type);

    char resolved[512];
    conflict_resolve_ours(&conflict, resolved, sizeof(resolved));
    printf("Resolved (ours): %s\n", resolved);

    conflict_resolve_theirs(&conflict, resolved, sizeof(resolved));
    printf("Resolved (theirs): %s\n", resolved);

    conflict_resolve_union(&conflict, resolved, sizeof(resolved));
    printf("Resolved (union): %s\n", resolved);

    ConflictSet cs = {{conflict}, 1, false};
    conflict_analyze(&cs);
    conflict_print(&cs);
    printf("\n");

    /* CI Pipeline */
    printf("--- CI Pipeline ---\n");
    CiPipeline pipe;
    ci_pipeline_init(&pipe);
    ci_add_job(&pipe, "build", "make all");
    ci_add_job(&pipe, "unit-test", "make test");
    ci_add_job(&pipe, "lint", "make lint");
    ci_run_all(&pipe);
    ci_print_pipeline(&pipe);
    ci_print_status(&pipe);

    BranchProtection bp = {true, true, true, false};
    printf("Branch protection check (pass, review, linear): %s\n",
           ci_check_branch_protection(&bp, true, true, true, false) ? "PASS" : "FAIL");
    printf("Branch protection check (no review): %s\n",
           ci_check_branch_protection(&bp, true, false, true, false) ? "PASS" : "FAIL");

    ci_pre_commit_check("src/main.c", &bp);
    printf("Pre-commit check: OK\n");

    return 0;
}
