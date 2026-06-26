#include <stdio.h>
#include "git_dag.h"
#include "git_branch.h"

/* L6: Canonical Problem — Git Flow Branching Model Demo
 *
 * Implements Vincent Driessen's Git Flow model:
 *   main ← develop ← feature/xxx
 *                   ← release/x.y.z
 *                   ← hotfix/x.y.z
 *
 * This demonstrates the full lifecycle:
 * 1. Create main + develop branches
 * 2. Create feature branch from develop
 * 3. Merge feature back to develop
 * 4. Create release branch from develop
 * 5. Merge release to main + develop
 * 6. Create hotfix from main
 * 7. Merge hotfix to main + develop
 */

int main() {
    printf("=== Git Flow Branching Model Demo ===\n\n");

    /* Initialize DAG and Branch Manager */
    GitDAG dag;
    git_dag_init(&dag);
    BranchManager bm;
    branch_manager_init(&bm);
    branch_configure_strategy(&bm, STRATEGY_GIT_FLOW);

    printf("Strategy: %s\n", git_dag_strategy_name(bm.config.strategy));
    printf("Main branch: %s\n", bm.config.main_branch);
    printf("Develop branch: %s\n", bm.config.develop_branch);
    printf("Release branches: %s\n", bm.config.use_release_branches ? "yes" : "no");
    printf("Hotfix branches: %s\n\n", bm.config.use_hotfix_branches ? "yes" : "no");

    /* Step 1: Create initial commit and setup main + develop */
    int root = git_dag_add_root_commit(&dag, "root01", "team", "chore: initial project setup");
    int main_br = branch_create(&bm, "main", BRANCH_MAIN, root, "admin");
    int develop = branch_create(&bm, "develop", BRANCH_DEVELOP, root, "admin");

    printf("[1] Created main and develop branches from root commit %d\n\n", root);

    /* Step 2: Feature branch workflow */
    int c1 = git_dag_add_commit(&dag, "dev01", &root, 1, "alice", "feat: add user model");
    bm.branches[develop].tip_commit_id = c1;

    int feature = branch_create(&bm, "feature/oauth-login", BRANCH_FEATURE, c1, "alice");
    printf("[2] Created feature branch: %s (id=%d)\n",
           bm.branches[feature].name, feature);

    int c2 = git_dag_add_commit(&dag, "feat01", &c1, 1, "alice", "feat: implement OAuth login");
    int c3 = git_dag_add_commit(&dag, "feat02", &c2, 1, "alice", "feat: add token refresh");
    bm.branches[feature].tip_commit_id = c3;
    printf("    Added 2 commits to feature branch\n");

    /* Merge feature back to develop */
    branch_merge(&bm, feature, develop);
    printf("[3] Merged feature branch into develop\n\n");

    /* Step 3: Release workflow */
    int release = branch_create(&bm, "release/v1.0.0", BRANCH_RELEASE, c3, "pm");
    printf("[4] Created release branch: %s (id=%d)\n",
           bm.branches[release].name, release);

    int c4 = git_dag_add_commit(&dag, "rel01", &c3, 1, "pm", "chore: bump version to 1.0.0");
    bm.branches[release].tip_commit_id = c4;

    /* Merge release to main and develop */
    main_br = branch_find_by_name(&bm, "main");
    int dev_br = branch_find_by_name(&bm, "develop");
    branch_merge(&bm, release, main_br);
    branch_merge(&bm, release, dev_br);
    printf("[5] Merged release v1.0.0 into main and develop\n\n");

    /* Step 4: Hotfix workflow */
    int hotfix = branch_create(&bm, "hotfix/v1.0.1", BRANCH_HOTFIX, c4, "ops");
    printf("[6] Created hotfix branch: %s (id=%d)\n",
           bm.branches[hotfix].name, hotfix);

    int c5 = git_dag_add_commit(&dag, "hot01", &c4, 1, "ops", "fix: critical security patch");
    bm.branches[hotfix].tip_commit_id = c5;

    branch_merge(&bm, hotfix, main_br);
    branch_merge(&bm, hotfix, dev_br);
    printf("[7] Merged hotfix into main and develop\n\n");

    /* Summary */
    printf("=== Git Flow Summary ===\n");
    branch_print_all(&bm);
    printf("\n=== DAG State ===\n");
    git_dag_print(&dag);

    return 0;
}
