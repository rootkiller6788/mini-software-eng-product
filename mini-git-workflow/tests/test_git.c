#include "git_dag.h"
#include "git_branch.h"
#include "git_merge.h"
#include "git_conflict.h"
#include "git_ci.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

static int tests = 0, passed = 0;
#define TEST(name) do { tests++; printf("  TEST %s... ", name); } while(0)
#define OK() do { passed++; printf("OK\n"); } while(0)
#define FAIL(msg) do { printf("FAIL: %s\n", msg); } while(0)

int main(void) {
    srand(12345);
    printf("=== Git Workflow Tests ===\n\n");

    /* git_dag tests */
    printf("git_dag:\n");
    TEST("init"); { GitDag dag; git_dag_init(&dag);
      assert(dag.commit_count == 0); assert(dag.branch_count == 0); OK(); }
    TEST("commit"); { GitDag dag; git_dag_init(&dag);
      int c = git_dag_commit(&dag, -1, "test");
      assert(c == 0); assert(dag.commit_count == 1); OK(); }
    TEST("chain"); { GitDag dag; git_dag_init(&dag);
      int a = git_dag_commit(&dag, -1, "A");
      int b = git_dag_commit(&dag, a, "B");
      int c = git_dag_commit(&dag, b, "C");
      assert(a == 0); assert(b == 1); assert(c == 2); OK(); }
    TEST("merge commit"); { GitDag dag; git_dag_init(&dag);
      int a = git_dag_commit(&dag, -1, "A");
      int b = git_dag_commit(&dag, a, "B");
      int m = git_dag_merge_commit(&dag, a, b, "merge");
      assert(m == 2); assert(dag.commits[m].parent_count == 2);
      assert(dag.commits[m].parent[0] == a);
      assert(dag.commits[m].parent[1] == b); OK(); }
    TEST("branch"); { GitDag dag; git_dag_init(&dag);
      git_dag_commit(&dag, -1, "A");
      int bi = git_dag_branch(&dag, "main");
      assert(bi == 0); assert(dag.branch_count == 1); OK(); }
    TEST("checkout"); { GitDag dag; git_dag_init(&dag);
      git_dag_commit(&dag, -1, "A");
      git_dag_branch(&dag, "main");
      assert(git_dag_checkout(&dag, "main")); assert(dag.head_branch == 0);
      assert(!git_dag_checkout(&dag, "nonexistent")); OK(); }
    TEST("detached head"); { GitDag dag; git_dag_init(&dag);
      int c = git_dag_commit(&dag, -1, "A");
      assert(git_dag_detached_head(&dag, c)); assert(dag.head_branch == -1);
      assert(dag.head_commit == c); OK(); }
    TEST("common ancestor"); { GitDag dag; git_dag_init(&dag);
      int c0 = git_dag_commit(&dag, -1, "root");
      int c1 = git_dag_commit(&dag, c0, "left");
      int c2 = git_dag_commit(&dag, c0, "right");
      int anc = git_dag_common_ancestor(&dag, c1, c2);
      assert(anc == c0); OK(); }
    TEST("log"); { GitDag dag; git_dag_init(&dag);
      int a = git_dag_commit(&dag, -1, "A");
      int b = git_dag_commit(&dag, a, "B");
      int c = git_dag_commit(&dag, b, "C");
      int results[10];
      int n = git_dag_log(&dag, c, 10, results);
      assert(n == 3); assert(results[0] == c); assert(results[2] == a); OK(); }
    TEST("max commits"); { GitDag dag; git_dag_init(&dag);
      int last = -1;
      for (int i = 0; i < DAG_MAX_COMMITS; i++) last = git_dag_commit(&dag, last, "x");
      assert(last == DAG_MAX_COMMITS - 1);
      assert(git_dag_commit(&dag, last, "overflow") == -1); OK(); }

    /* git_branch tests */
    printf("git_branch:\n");
    TEST("strategy name"); {
      assert(strcmp(branch_strategy_name(BSTRAT_GITFLOW), "GitFlow") == 0);
      assert(strcmp(branch_strategy_name(BSTRAT_GITHUB_FLOW), "GitHub Flow") == 0);
      assert(strcmp(branch_strategy_name(BSTRAT_TRUNK_BASED), "Trunk-Based") == 0);
      assert(strcmp(branch_strategy_name(BSTRAT_GITLAB_FLOW), "GitLab Flow") == 0); OK(); }
    TEST("gitflow init"); { BranchModel bm;
      branch_model_init(&bm, BSTRAT_GITFLOW);
      assert(bm.branch_count == 5); assert(bm.strategy == BSTRAT_GITFLOW); OK(); }
    TEST("github flow init"); { BranchModel bm;
      branch_model_init(&bm, BSTRAT_GITHUB_FLOW);
      assert(bm.branch_count == 2); assert(branch_model_validate(&bm)); OK(); }
    TEST("trunk-based init"); { BranchModel bm;
      branch_model_init(&bm, BSTRAT_TRUNK_BASED);
      assert(bm.branch_count == 2); OK(); }
    TEST("release tag"); { BranchModel bm; ReleaseTag rt;
      branch_model_init(&bm, BSTRAT_GITFLOW);
      branch_model_release(&bm, &rt, 2, 3, 1);
      assert(rt.major == 2); assert(rt.minor == 3); assert(rt.patch == 1);
      assert(strcmp(rt.version, "2.3.1") == 0);
      assert(strcmp(rt.tag, "v2.3.1") == 0); assert(rt.is_release); OK(); }
    TEST("max branches"); { BranchModel bm;
      branch_model_init(&bm, BSTRAT_GITFLOW);
      for (int i = 0; i < 5; i++)
        branch_model_add(&bm, "extra", "main", false, false);
      assert(bm.branch_count == 10);
      assert(branch_model_add(&bm, "one-more", "main", false, false) == -1); OK(); }

    /* git_merge tests */
    printf("git_merge:\n");
    TEST("identical"); { MergeResult mr; memset(&mr,0,sizeof(mr)); char out[512];
      assert(git_merge_three_way(&mr, "X", "X", "X", out, sizeof(out)));
      assert(strcmp(out, "X") == 0); OK(); }
    TEST("ours modified"); { MergeResult mr; memset(&mr,0,sizeof(mr)); char out[512];
      assert(git_merge_three_way(&mr, "A", "B", "A", out, sizeof(out)));
      assert(strcmp(out, "B") == 0); OK(); }
    TEST("theirs modified"); { MergeResult mr; memset(&mr,0,sizeof(mr)); char out[512];
      assert(git_merge_three_way(&mr, "A", "A", "B", out, sizeof(out)));
      assert(strcmp(out, "B") == 0); OK(); }
    TEST("both modified"); { MergeResult mr; memset(&mr,0,sizeof(mr)); char out[512];
      assert(!git_merge_three_way(&mr, "A", "B", "C", out, sizeof(out)));
      assert(strstr(out, "<<<<<<<") != NULL); assert(mr.conflicts == 1); OK(); }
    TEST("merge branches"); { MergeResult mr;
      assert(git_merge_branches(&mr, 1, 0, 3) == 0);
      assert(mr.source_branch_idx == 1); assert(mr.merge_base == 3); OK(); }
    TEST("rebase"); { MergeResult mr; memset(&mr,0,sizeof(mr));
      assert(git_rebase_branch(&mr, 2, 0)); OK(); }
    TEST("cherry-pick"); { MergeResult mr; memset(&mr,0,sizeof(mr));
      assert(git_cherry_pick(&mr, 5, 0)); OK(); }

    /* git_conflict tests */
    printf("git_conflict:\n");
    TEST("no conflict"); { Conflict c;
      conflict_detect("same", "same", "base", &c);
      assert(c.type == CONF_NONE); OK(); }
    TEST("ours only changed"); { Conflict c;
      conflict_detect("base", "new", "base", &c);
      assert(strcmp(c.ours_content, "new") == 0); OK(); }
    TEST("theirs only changed"); { Conflict c;
      conflict_detect("new", "base", "base", &c);
      assert(strcmp(c.ours_content, "new") == 0); OK(); }
    TEST("both changed"); { Conflict c;
      conflict_detect("v1", "v2", "base", &c);
      assert(c.type == CONF_CONTENT);
      assert(strcmp(c.ours_content, "v1") == 0);
      assert(strcmp(c.theirs_content, "v2") == 0); OK(); }
    TEST("resolve ours"); { Conflict c; char buf[512];
      conflict_detect("v1", "v2", "base", &c);
      conflict_resolve_ours(&c, buf, sizeof(buf));
      assert(strcmp(buf, "v1") == 0); OK(); }
    TEST("resolve theirs"); { Conflict c; char buf[512];
      conflict_detect("v1", "v2", "base", &c);
      conflict_resolve_theirs(&c, buf, sizeof(buf));
      assert(strcmp(buf, "v2") == 0); OK(); }
    TEST("resolve union"); { Conflict c; char buf[512];
      conflict_detect("v1", "v2", "base", &c);
      conflict_resolve_union(&c, buf, sizeof(buf));
      assert(strstr(buf, "v1") != NULL); assert(strstr(buf, "v2") != NULL); OK(); }

    /* git_ci tests */
    printf("git_ci:\n");
    TEST("pipeline init"); { CiPipeline p;
      ci_pipeline_init(&p);
      assert(p.job_count == 0); assert(p.all_passed); OK(); }
    TEST("add job"); { CiPipeline p; ci_pipeline_init(&p);
      assert(ci_add_job(&p, "build", "make") == 0);
      assert(p.job_count == 1); OK(); }
    TEST("max jobs"); { CiPipeline p; ci_pipeline_init(&p);
      int last = -1;
      for (int i = 0; i < 16; i++) last = ci_add_job(&p, "j", "cmd");
      assert(last == 15);
      assert(ci_add_job(&p, "extra", "cmd") == -1); OK(); }
    TEST("run job"); { CiPipeline p; ci_pipeline_init(&p);
      ci_add_job(&p, "test", "make test");
      assert(ci_run_job(&p, 0));
      assert(p.jobs[0].status == CI_PASSED);
      assert(p.jobs[0].duration_ms == 100); OK(); }
    TEST("run all"); { CiPipeline p; ci_pipeline_init(&p);
      ci_add_job(&p, "a", "cmd"); ci_add_job(&p, "b", "cmd"); ci_add_job(&p, "c", "cmd");
      assert(ci_run_all(&p));
      assert(p.total_duration_ms == 300); OK(); }
    TEST("bad job id"); { CiPipeline p; ci_pipeline_init(&p);
      assert(!ci_run_job(&p, 0)); assert(!ci_run_job(&p, 99)); OK(); }
    TEST("branch protection - all pass"); { BranchProtection bp = {true,true,true,true};
      assert(ci_check_branch_protection(&bp, true, true, true, true)); OK(); }
    TEST("branch protection - ci fail"); { BranchProtection bp = {true,true,true,true};
      assert(!ci_check_branch_protection(&bp, false, true, true, true)); OK(); }
    TEST("branch protection - no review"); { BranchProtection bp = {false,true,false,false};
      assert(!ci_check_branch_protection(&bp, true, false, true, false)); OK(); }
    TEST("branch protection - not linear"); { BranchProtection bp = {false,false,true,false};
      assert(!ci_check_branch_protection(&bp, true, true, false, false)); OK(); }
    TEST("branch protection - not signed"); { BranchProtection bp = {false,false,false,true};
      assert(!ci_check_branch_protection(&bp, true, true, true, false)); OK(); }

    printf("\n=== Results: %d/%d passed ===\n", passed, tests);
    return passed == tests ? 0 : 1;
}
