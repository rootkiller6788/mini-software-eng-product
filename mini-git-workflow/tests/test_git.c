#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "git_dag.h"
#include "git_branch.h"
#include "git_merge.h"
#include "git_commit.h"
#include "git_pr.h"
#include "git_ci.h"

/* ================================================================
 * Comprehensive Test Suite for mini-git-workflow
 * Covers: DAG operations, branching, merging, conventional commits,
 *         PR review, CI pipeline — all L1-L6
 * ================================================================ */

static int tests_passed = 0;
static int tests_failed = 0;

#define TEST(name) do { printf("  TEST: %s ... ", name); } while(0)
#define PASS() do { printf("PASS\n"); tests_passed++; } while(0)
#define FAIL(msg) do { printf("FAIL: %s\n", msg); tests_failed++; } while(0)
#define ASSERT_EQ(a, b) do { if ((a) != (b)) { FAIL("assertion failed"); return; } } while(0)
#define ASSERT_TRUE(a) do { if (!(a)) { FAIL("expected true"); return; } } while(0)
#define ASSERT_FALSE(a) do { if (a) { FAIL("expected false"); return; } } while(0)
#define ASSERT_STR_EQ(a, b) do { if (strcmp((a),(b)) != 0) { FAIL("string mismatch"); return; } } while(0)

/* --- Git DAG Tests (L1-L5) --- */

void test_dag_init() {
    TEST("dag_init");
    GitDAG dag;
    git_dag_init(&dag);
    ASSERT_EQ(dag.commit_count, 0);
    ASSERT_EQ(dag.ref_count, 0);
    ASSERT_EQ(dag.head_ref_id, -1);
    PASS();
}

void test_dag_add_root_commit() {
    TEST("dag_add_root_commit");
    GitDAG dag;
    git_dag_init(&dag);
    int id = git_dag_add_root_commit(&dag, "abc123", "Alice", "Initial commit");
    ASSERT_TRUE(id >= 0);
    ASSERT_EQ(dag.commit_count, 1);
    ASSERT_EQ(dag.commits[id].parent_count, 0);
    ASSERT_TRUE(!dag.commits[id].is_merge);
    ASSERT_EQ(dag.commits[id].generation, 0);
    PASS();
}

void test_dag_add_linear_commits() {
    TEST("dag_add_linear_commits");
    GitDAG dag;
    git_dag_init(&dag);
    int r = git_dag_add_root_commit(&dag, "r00", "A", "root");
    int c1 = git_dag_add_commit(&dag, "c01", &r, 1, "A", "commit 1");
    int c2 = git_dag_add_commit(&dag, "c02", &c1, 1, "A", "commit 2");
    ASSERT_EQ(dag.commit_count, 3);
    ASSERT_EQ(dag.commits[c2].generation, 2);
    ASSERT_TRUE(git_dag_is_linear(&dag));
    PASS();
}

void test_dag_merge_commit() {
    TEST("dag_merge_commit");
    GitDAG dag;
    git_dag_init(&dag);
    int r = git_dag_add_root_commit(&dag, "r00", "A", "root");
    int c1 = git_dag_add_commit(&dag, "c01", &r, 1, "A", "feat 1");
    int c2 = git_dag_add_commit(&dag, "c02", &r, 1, "B", "feat 2");
    int merge = git_dag_add_merge_commit(&dag, "m01", c1, c2, "A", "merge");
    ASSERT_EQ(dag.commit_count, 4);
    ASSERT_TRUE(dag.commits[merge].is_merge);
    ASSERT_EQ(dag.commits[merge].parent_count, 2);
    ASSERT_FALSE(git_dag_is_linear(&dag));
    PASS();
}

void test_dag_reachable() {
    TEST("dag_reachable");
    GitDAG dag;
    git_dag_init(&dag);
    int r = git_dag_add_root_commit(&dag, "r00", "A", "root");
    int c1 = git_dag_add_commit(&dag, "c01", &r, 1, "A", "c1");
    int c2 = git_dag_add_commit(&dag, "c02", &c1, 1, "A", "c2");
    ASSERT_TRUE(git_dag_reachable(&dag, c2, r));
    ASSERT_TRUE(git_dag_is_ancestor(&dag, r, c2));
    ASSERT_FALSE(git_dag_reachable(&dag, r, c2));
    PASS();
}

void test_dag_merge_base() {
    TEST("dag_merge_base");
    GitDAG dag;
    git_dag_init(&dag);
    int r = git_dag_add_root_commit(&dag, "r00", "A", "root");
    int c1 = git_dag_add_commit(&dag, "c01", &r, 1, "A", "branch-a");
    int c2 = git_dag_add_commit(&dag, "c02", &r, 1, "B", "branch-b");
    int base = git_dag_find_merge_base(&dag, c1, c2);
    ASSERT_EQ(base, r);
    PASS();
}

void test_dag_topological_order() {
    TEST("dag_topological_order");
    GitDAG dag;
    git_dag_init(&dag);
    int r = git_dag_add_root_commit(&dag, "r00", "A", "root");
    int c1 = git_dag_add_commit(&dag, "c01", &r, 1, "A", "c1");
    int c2 = git_dag_add_commit(&dag, "c02", &c1, 1, "A", "c2");
    int order[GIT_DAG_MAX_COMMITS];
    int n = git_dag_topological_order(&dag, order);
    ASSERT_EQ(n, 3);
    ASSERT_EQ(order[0], r);
    ASSERT_EQ(order[1], c1);
    ASSERT_EQ(order[2], c2);
    PASS();
}

void test_dag_branches_and_refs() {
    TEST("dag_branches_and_refs");
    GitDAG dag;
    git_dag_init(&dag);
    int r = git_dag_add_root_commit(&dag, "r00", "A", "root");
    int main_id = git_dag_create_branch(&dag, "main", r);
    ASSERT_TRUE(main_id >= 0);
    ASSERT_TRUE(git_dag_checkout(&dag, main_id));
    ASSERT_EQ(git_dag_get_head_commit(&dag), r);
    int tag_id = git_dag_create_tag(&dag, "v1.0", r);
    ASSERT_TRUE(tag_id >= 0);
    ASSERT_EQ(git_dag_find_ref(&dag, "main"), main_id);
    ASSERT_EQ(git_dag_find_ref(&dag, "v1.0"), tag_id);
    ASSERT_EQ(git_dag_branch_count(&dag), 1);
    PASS();
}

void test_dag_divergence() {
    TEST("dag_divergence");
    GitDAG dag;
    git_dag_init(&dag);
    int r = git_dag_add_root_commit(&dag, "r00", "A", "root");
    int c1 = git_dag_add_commit(&dag, "c01", &r, 1, "A", "a1");
    int c2 = git_dag_add_commit(&dag, "c02", &r, 1, "B", "b1");
    ASSERT_TRUE(git_dag_detect_conflict(&dag, c1, c2));
    /* After merge, no conflict remains */
    int m = git_dag_add_merge_commit(&dag, "m01", c1, c2, "A", "merge");
    ASSERT_FALSE(git_dag_detect_conflict(&dag, c1, m));
    PASS();
}

/* --- Branch Management Tests (L2-L6) --- */

void test_branch_create() {
    TEST("branch_create");
    BranchManager bm;
    branch_manager_init(&bm);
    int id = branch_create(&bm, "feature/login", BRANCH_FEATURE, 0, "Alice");
    ASSERT_TRUE(id >= 0);
    ASSERT_EQ(bm.branch_count, 1);
    ASSERT_EQ(bm.branches[id].type, BRANCH_FEATURE);
    ASSERT_EQ(bm.branches[id].state, BRANCH_STATE_ACTIVE);
    PASS();
}

void test_branch_name_validation() {
    TEST("branch_name_validation");
    BranchManager bm;
    branch_manager_init(&bm);
    ASSERT_TRUE(branch_validate_name(&bm, "feature/login"));
    ASSERT_TRUE(branch_validate_name(&bm, "fix/bug-123"));
    ASSERT_TRUE(branch_validate_name(&bm, "main"));
    ASSERT_TRUE(branch_validate_name(&bm, "develop"));
    ASSERT_FALSE(branch_validate_name(&bm, "bad name"));
    ASSERT_FALSE(branch_validate_name(&bm, ""));
    PASS();
}

void test_branch_lifecycle() {
    TEST("branch_lifecycle");
    BranchManager bm;
    branch_manager_init(&bm);
    int feat = branch_create(&bm, "feature/test", BRANCH_FEATURE, 0, "Alice");
    int main = branch_create(&bm, "main", BRANCH_MAIN, 0, "System");
    ASSERT_TRUE(branch_merge(&bm, feat, main));
    ASSERT_EQ(bm.branches[feat].state, BRANCH_STATE_DELETED);
    ASSERT_TRUE(branch_is_mergeable(&bm, feat, main) == false);
    PASS();
}

void test_branch_strategy_config() {
    TEST("branch_strategy_config");
    BranchManager bm;
    branch_manager_init(&bm);
    branch_configure_strategy(&bm, STRATEGY_GIT_FLOW);
    ASSERT_EQ(bm.config.strategy, STRATEGY_GIT_FLOW);
    ASSERT_TRUE(bm.config.use_release_branches);
    ASSERT_TRUE(bm.config.use_hotfix_branches);

    branch_configure_strategy(&bm, STRATEGY_TRUNK_BASED);
    ASSERT_EQ(bm.config.strategy, STRATEGY_TRUNK_BASED);
    ASSERT_TRUE(bm.config.squash_merge);
    ASSERT_TRUE(bm.config.delete_after_merge);
    ASSERT_EQ(bm.config.max_branch_age_days, 3);
    PASS();
}

void test_branch_protection_rules() {
    TEST("branch_protection_rules");
    BranchManager bm;
    branch_manager_init(&bm);
    int rule_id = branch_add_protection_rule(&bm, "main");
    ASSERT_TRUE(rule_id >= 0);
    ASSERT_TRUE(bm.rules[rule_id].require_pr);
    ASSERT_TRUE(bm.rules[rule_id].block_force_push);
    PASS();
}

void test_branch_pattern_match() {
    TEST("branch_pattern_match");
    ASSERT_TRUE(branch_match_pattern("feature/login", "feature/*"));
    ASSERT_TRUE(branch_match_pattern("main", "*"));
    ASSERT_TRUE(branch_match_pattern("hotfix/v1.2.3", "hotfix/*"));
    ASSERT_FALSE(branch_match_pattern("feature/login", "fix/*"));
    ASSERT_TRUE(branch_match_pattern("release/v2.0", "release/v?.?"));
    PASS();
}

/* --- Merge & Rebase Tests (L5) --- */

void test_three_way_merge_no_conflict() {
    TEST("three_way_merge_no_conflict");
    MergeInput input;
    memset(&input, 0, sizeof(input));
    strcpy(input.file_path, "test.c");
    strcpy(input.base_lines[0], "line1");
    strcpy(input.base_lines[1], "line2");
    input.base_count = 2;
    strcpy(input.ours_lines[0], "line1");
    strcpy(input.ours_lines[1], "line2-modified");
    input.ours_count = 2;
    strcpy(input.theirs_lines[0], "line1");
    strcpy(input.theirs_lines[1], "line2");
    input.theirs_count = 2;

    MergeResult result = merge_three_way(&input);
    ASSERT_TRUE(result.success);
    ASSERT_EQ(result.conflict_count, 0);
    ASSERT_STR_EQ(result.merged_lines[0], "line1");
    PASS();
}

void test_three_way_merge_conflict() {
    TEST("three_way_merge_conflict");
    MergeInput input;
    memset(&input, 0, sizeof(input));
    strcpy(input.file_path, "test.c");
    strcpy(input.base_lines[0], "original");
    input.base_count = 1;
    strcpy(input.ours_lines[0], "our-change");
    input.ours_count = 1;
    strcpy(input.theirs_lines[0], "their-change");
    input.theirs_count = 1;

    MergeResult result = merge_three_way(&input);
    ASSERT_FALSE(result.success);
    ASSERT_TRUE(result.conflict_count > 0);
    PASS();
}

void test_diff_lcs() {
    TEST("diff_lcs");
    const char *a[] = {"a", "b", "c", "d"};
    const char *b[] = {"a", "c", "d", "e"};
    int lcs = diff_lcs_length(a, 4, b, 4);
    ASSERT_EQ(lcs, 3);  /* "a", "c", "d" */
    PASS();
}

void test_diff_similarity() {
    TEST("diff_similarity");
    const char *a[] = {"hello", "world"};
    const char *b[] = {"hello", "world"};
    double sim = diff_similarity(a, 2, b, 2);
    ASSERT_TRUE(sim > 0.99);
    PASS();
}

/* --- Conventional Commits Tests (L4) --- */

void test_commit_parse_feat() {
    TEST("commit_parse_feat");
    ConventionalCommit cc = commit_parse("feat(auth): add login support");
    ASSERT_TRUE(cc.valid);
    ASSERT_EQ(cc.type, COMMIT_TYPE_FEAT);
    ASSERT_TRUE(cc.has_scope);
    ASSERT_STR_EQ(cc.scope, "auth");
    ASSERT_STR_EQ(cc.description, "add login support");
    ASSERT_EQ(cc.version_bump, SEMVER_MINOR);
    ASSERT_FALSE(cc.is_breaking_change);
    PASS();
}

void test_commit_parse_fix() {
    TEST("commit_parse_fix");
    ConventionalCommit cc = commit_parse("fix: resolve null pointer crash");
    ASSERT_TRUE(cc.valid);
    ASSERT_EQ(cc.type, COMMIT_TYPE_FIX);
    ASSERT_EQ(cc.version_bump, SEMVER_PATCH);
    ASSERT_FALSE(cc.has_scope);
    PASS();
}

void test_commit_parse_breaking() {
    TEST("commit_parse_breaking");
    ConventionalCommit cc = commit_parse(
        "feat!: drop support for v1 API\n\n"
        "BREAKING CHANGE: removed deprecated endpoints");
    ASSERT_TRUE(cc.valid);
    ASSERT_TRUE(cc.is_breaking_change);
    ASSERT_EQ(cc.version_bump, SEMVER_MAJOR);
    PASS();
}

void test_commit_parse_invalid() {
    TEST("commit_parse_invalid");
    ConventionalCommit cc = commit_parse("random message without format");
    ASSERT_FALSE(cc.valid);
    PASS();
}

void test_commit_format() {
    TEST("commit_format");
    ConventionalCommit cc = commit_create(COMMIT_TYPE_FEAT, "api", "add endpoint", NULL);
    char buf[256];
    int len = commit_format_message(&cc, buf, sizeof(buf));
    ASSERT_TRUE(len > 0);
    ASSERT_STR_EQ(buf, "feat(api): add endpoint");
    PASS();
}

void test_commit_footer_breaking() {
    TEST("commit_footer_breaking");
    ConventionalCommit cc = commit_create(COMMIT_TYPE_FIX, NULL, "fix bug", NULL);
    commit_add_footer(&cc, "BREAKING CHANGE", "API signature changed");
    ASSERT_TRUE(cc.is_breaking_change);
    ASSERT_EQ(cc.version_bump, SEMVER_MAJOR);
    ASSERT_TRUE(commit_has_footer(&cc, "BREAKING CHANGE"));
    ASSERT_STR_EQ(commit_get_footer_value(&cc, "BREAKING CHANGE"),
                  "API signature changed");
    PASS();
}

void test_commit_detect_breaking_in_message() {
    TEST("commit_detect_breaking_in_message");
    ASSERT_TRUE(commit_detect_breaking("feat: add x\n\nBREAKING CHANGE: removed y"));
    ASSERT_FALSE(commit_detect_breaking("fix: minor bug"));
    PASS();
}

/* --- Semantic Version Tests (L4) --- */

void test_semver_parse() {
    TEST("semver_parse");
    Semver v = semver_parse("1.2.3");
    ASSERT_EQ(v.major, 1);
    ASSERT_EQ(v.minor, 2);
    ASSERT_EQ(v.patch, 3);
    ASSERT_TRUE(semver_is_valid(&v));
    PASS();
}

void test_semver_parse_prerelease() {
    TEST("semver_parse_prerelease");
    Semver v = semver_parse("1.0.0-alpha.1+build.123");
    ASSERT_EQ(v.major, 1);
    ASSERT_STR_EQ(v.pre_release, "alpha.1");
    ASSERT_STR_EQ(v.build_meta, "build.123");
    PASS();
}

void test_semver_compare() {
    TEST("semver_compare");
    Semver v1 = {1, 2, 3};
    Semver v2 = {1, 3, 0};
    Semver v3 = {2, 0, 0};
    ASSERT_TRUE(semver_compare(&v1, &v2) < 0);
    ASSERT_TRUE(semver_compare(&v2, &v3) < 0);
    ASSERT_TRUE(semver_compare(&v2, &v1) > 0);
    ASSERT_EQ(semver_compare(&v1, &v1), 0);
    PASS();
}

void test_semver_bump() {
    TEST("semver_bump");
    Semver v = {1, 2, 3};
    Semver patch = semver_bump(&v, SEMVER_PATCH);
    ASSERT_EQ(patch.major, 1);
    ASSERT_EQ(patch.minor, 2);
    ASSERT_EQ(patch.patch, 4);
    Semver minor = semver_bump(&v, SEMVER_MINOR);
    ASSERT_EQ(minor.minor, 3);
    ASSERT_EQ(minor.patch, 0);
    Semver major = semver_bump(&v, SEMVER_MAJOR);
    ASSERT_EQ(major.major, 2);
    ASSERT_EQ(major.minor, 0);
    ASSERT_EQ(major.patch, 0);
    PASS();
}

/* --- PR Review Tests (L2-L6) --- */

void test_pr_create_and_submit() {
    TEST("pr_create_and_submit");
    PRManager pm;
    pr_manager_init(&pm, NULL);
    int pr_id = pr_create(&pm, "Add login feature", "Implements OAuth2 login",
                          100, 1, 0);
    ASSERT_TRUE(pr_id > 0);
    ASSERT_TRUE(pr_submit_for_review(&pm, pr_id));
    ASSERT_FALSE(pr_is_approved(&pm, pr_id));
    PASS();
}

void test_pr_review_flow() {
    TEST("pr_review_flow");
    PRManager pm;
    pr_manager_init(&pm, NULL);
    int pr_id = pr_create(&pm, "Test PR", "desc", 100, 1, 0);
    pr_submit_for_review(&pm, pr_id);
    pr_assign_reviewer(&pm, pr_id, 200);
    ASSERT_TRUE(pr_submit_review(&pm, pr_id, 200, REVIEW_APPROVED));
    ASSERT_TRUE(pr_is_approved(&pm, pr_id));
    PASS();
}

void test_pr_changes_requested() {
    TEST("pr_changes_requested");
    PRManager pm;
    pr_manager_init(&pm, NULL);
    int pr_id = pr_create(&pm, "Bug fix", "fixes issue", 100, 1, 0);
    pr_submit_for_review(&pm, pr_id);
    pr_assign_reviewer(&pm, pr_id, 200);
    ASSERT_TRUE(pr_submit_review(&pm, pr_id, 200, REVIEW_CHANGES));
    ASSERT_TRUE(pr_needs_changes(&pm, pr_id));
    ASSERT_FALSE(pr_is_mergeable(&pm, pr_id));
    PASS();
}

void test_pr_comments() {
    TEST("pr_comments");
    PRManager pm;
    pr_manager_init(&pm, NULL);
    int pr_id = pr_create(&pm, "Test", "desc", 100, 1, 0);
    pr_submit_for_review(&pm, pr_id);
    int cmt_id = pr_add_comment(&pm, pr_id, 200, "src/main.c", 42,
                                 "Consider using const here", COMMENT_SUGGESTION);
    ASSERT_TRUE(cmt_id >= 0);
    ASSERT_TRUE(pr_resolve_comment(&pm, pr_id, cmt_id));
    PASS();
}

void test_pr_labels() {
    TEST("pr_labels");
    PRManager pm;
    pr_manager_init(&pm, NULL);
    int pr_id = pr_create(&pm, "Feature X", "desc", 100, 1, 0);
    ASSERT_TRUE(pr_add_label(&pm, pr_id, "bug"));
    ASSERT_TRUE(pr_add_label(&pm, pr_id, "priority-high"));
    ASSERT_TRUE(pr_has_label(&pm, pr_id, "bug"));
    ASSERT_FALSE(pr_has_label(&pm, pr_id, "feature"));
    ASSERT_TRUE(pr_remove_label(&pm, pr_id, "bug"));
    ASSERT_FALSE(pr_has_label(&pm, pr_id, "bug"));
    PASS();
}

void test_pr_policy_enforcement() {
    TEST("pr_policy_enforcement");
    PRManager pm;
    PRReviewPolicy policy = {2, true, true, true, false, true};
    pr_manager_init(&pm, &policy);
    int pr_id = pr_create(&pm, "Compliance PR", "needs 2 approvals", 100, 1, 0);
    pr_submit_for_review(&pm, pr_id);
    pr_assign_reviewer(&pm, pr_id, 200);
    pr_submit_review(&pm, pr_id, 200, REVIEW_APPROVED);
    ASSERT_FALSE(pr_is_mergeable(&pm, pr_id)); /* needs 2 approvals, only 1 */
    pr_assign_reviewer(&pm, pr_id, 300);
    pr_submit_review(&pm, pr_id, 300, REVIEW_APPROVED);
    ASSERT_FALSE(pr_is_mergeable(&pm, pr_id)); /* CI not passed */
    pr_set_ci_status(&pm, pr_id, true);
    ASSERT_TRUE(pr_is_mergeable(&pm, pr_id));
    PASS();
}

void test_pr_find_functions() {
    TEST("pr_find_functions");
    PRManager pm;
    pr_manager_init(&pm, NULL);
    int pr1 = pr_create(&pm, "PR1", "", 100, 1, 0);
    pr_create(&pm, "PR2", "", 100, 2, 0);
    pr_submit_for_review(&pm, pr1);
    int open[10];
    int count = pr_find_open(&pm, &open[0], 10);
    ASSERT_TRUE(count >= 1);
    int by_author[10];
    count = pr_find_by_author(&pm, 100, by_author, 10);
    ASSERT_EQ(count, 2);
    PASS();
}

/* --- CI Pipeline Tests (L2-L7) --- */

void test_ci_create_pipeline() {
    TEST("ci_create_pipeline");
    CIManager cm;
    ci_manager_init(&cm);
    int pid = ci_create_pipeline(&cm, 1, 0, "abc123def456");
    ASSERT_TRUE(pid > 0);
    ASSERT_EQ(ci_get_pipeline_status(&cm, pid), CI_STATUS_PENDING);
    PASS();
}

void test_ci_pipeline_lifecycle() {
    TEST("ci_pipeline_lifecycle");
    CIManager cm;
    ci_manager_init(&cm);
    int pid = ci_create_pipeline(&cm, 0, 0, "sha");
    ASSERT_TRUE(ci_trigger_pipeline(&cm, pid));
    ASSERT_EQ(ci_get_pipeline_status(&cm, pid), CI_STATUS_RUNNING);
    ASSERT_TRUE(ci_cancel_pipeline(&cm, pid));
    ASSERT_EQ(ci_get_pipeline_status(&cm, pid), CI_STATUS_CANCELED);
    PASS();
}

void test_ci_add_stage_and_job() {
    TEST("ci_add_stage_and_job");
    CIManager cm;
    ci_manager_init(&cm);
    int pid = ci_create_pipeline(&cm, 0, 0, "sha");
    int stage_id = ci_add_stage(&cm, pid, "build", CI_STAGE_BUILD, 0, true);
    ASSERT_TRUE(stage_id >= 0);
    int job_id = ci_add_job(&cm, pid, stage_id, "compile", false, 2);
    ASSERT_TRUE(job_id > 0);
    PASS();
}

void test_ci_job_status_transitions() {
    TEST("ci_job_status_transitions");
    CIManager cm;
    ci_manager_init(&cm);
    int pid = ci_create_pipeline(&cm, 0, 0, "sha");
    int sid = ci_add_stage(&cm, pid, "test", CI_STAGE_TEST, 0, true);
    int jid = ci_add_job(&cm, pid, sid, "unit-test", false, 1);
    ASSERT_TRUE(ci_set_job_status(&cm, pid, jid, CI_STATUS_RUNNING, -1));
    ASSERT_EQ(ci_get_job_status(&cm, pid, jid), CI_STATUS_RUNNING);
    ASSERT_TRUE(ci_set_job_status(&cm, pid, jid, CI_STATUS_SUCCESS, 0));
    ASSERT_EQ(ci_get_job_status(&cm, pid, jid), CI_STATUS_SUCCESS);
    ASSERT_TRUE(ci_is_pipeline_green(&cm, pid));
    PASS();
}

void test_ci_job_logging() {
    TEST("ci_job_logging");
    CIManager cm;
    ci_manager_init(&cm);
    int pid = ci_create_pipeline(&cm, 0, 0, "sha");
    int sid = ci_add_stage(&cm, pid, "build", CI_STAGE_BUILD, 0, true);
    int jid = ci_add_job(&cm, pid, sid, "compile", false, 1);
    ASSERT_TRUE(ci_add_job_log(&cm, pid, jid, "[INFO] Starting compilation"));
    ASSERT_TRUE(ci_add_job_log(&cm, pid, jid, "[INFO] Compilation complete"));
    PASS();
}

void test_ci_generate_default_pipeline() {
    TEST("ci_generate_default_pipeline");
    CIManager cm;
    ci_manager_init(&cm);
    ASSERT_TRUE(ci_generate_default_pipeline(&cm, 42, 7, "abcdef"));
    ASSERT_TRUE(cm.pipeline_count >= 1);
    PASS();
}

void test_ci_deploy_gate() {
    TEST("ci_deploy_gate");
    CIManager cm;
    ci_manager_init(&cm);
    int pid = ci_create_pipeline(&cm, 0, 0, "sha");
    int sid = ci_add_stage(&cm, pid, "deploy", CI_STAGE_DEPLOY, 0, true);
    int jid = ci_add_job(&cm, pid, sid, "deploy-prod", false, 0);
    ci_set_job_status(&cm, pid, jid, CI_STATUS_RUNNING, -1);
    ASSERT_FALSE(ci_deploy_gate_check(&cm, pid));
    ci_set_job_status(&cm, pid, jid, CI_STATUS_SUCCESS, 0);
    ASSERT_TRUE(ci_deploy_gate_check(&cm, pid));
    PASS();
}

void test_ci_rollback() {
    TEST("ci_rollback");
    CIManager cm;
    ci_manager_init(&cm);
    int pid1 = ci_create_pipeline(&cm, 0, 0, "sha1");
    int sid1 = ci_add_stage(&cm, pid1, "deploy", CI_STAGE_DEPLOY, 0, true);
    int jid1 = ci_add_job(&cm, pid1, sid1, "deploy", false, 0);
    ci_set_job_status(&cm, pid1, jid1, CI_STATUS_RUNNING, -1);
    ci_set_job_status(&cm, pid1, jid1, CI_STATUS_SUCCESS, 0);
    int pid2 = ci_create_pipeline(&cm, 0, 0, "sha2");
    int prev;
    ASSERT_TRUE(ci_rollback_check(&cm, pid2, &prev));
    ASSERT_EQ(prev, pid1);
    PASS();
}

void test_ci_count_jobs() {
    TEST("ci_count_jobs");
    CIManager cm;
    ci_manager_init(&cm);
    int pid = ci_create_pipeline(&cm, 0, 0, "sha");
    int sid = ci_add_stage(&cm, pid, "test", CI_STAGE_TEST, 0, true);
    int j1 = ci_add_job(&cm, pid, sid, "test1", false, 1);
    int j2 = ci_add_job(&cm, pid, sid, "test2", false, 1);
    ci_set_job_status(&cm, pid, j1, CI_STATUS_RUNNING, -1);
    ci_set_job_status(&cm, pid, j1, CI_STATUS_SUCCESS, 0);
    ci_set_job_status(&cm, pid, j2, CI_STATUS_RUNNING, -1);
    ci_set_job_status(&cm, pid, j2, CI_STATUS_FAILED, 1);
    ASSERT_EQ(ci_count_passed_jobs(&cm, pid), 1);
    ASSERT_EQ(ci_count_failed_jobs(&cm, pid), 1);
    PASS();
}

/* --- Main --- */

int main() {
    printf("=== mini-git-workflow Test Suite ===\n\n");

    printf("[DAG Tests]\n");
    test_dag_init();
    test_dag_add_root_commit();
    test_dag_add_linear_commits();
    test_dag_merge_commit();
    test_dag_reachable();
    test_dag_merge_base();
    test_dag_topological_order();
    test_dag_branches_and_refs();
    test_dag_divergence();

    printf("\n[Branch Tests]\n");
    test_branch_create();
    test_branch_name_validation();
    test_branch_lifecycle();
    test_branch_strategy_config();
    test_branch_protection_rules();
    test_branch_pattern_match();

    printf("\n[Merge Tests]\n");
    test_three_way_merge_no_conflict();
    test_three_way_merge_conflict();
    test_diff_lcs();
    test_diff_similarity();

    printf("\n[Conventional Commits Tests]\n");
    test_commit_parse_feat();
    test_commit_parse_fix();
    test_commit_parse_breaking();
    test_commit_parse_invalid();
    test_commit_format();
    test_commit_footer_breaking();
    test_commit_detect_breaking_in_message();

    printf("\n[Semver Tests]\n");
    test_semver_parse();
    test_semver_parse_prerelease();
    test_semver_compare();
    test_semver_bump();

    printf("\n[PR Tests]\n");
    test_pr_create_and_submit();
    test_pr_review_flow();
    test_pr_changes_requested();
    test_pr_comments();
    test_pr_labels();
    test_pr_policy_enforcement();
    test_pr_find_functions();

    printf("\n[CI Tests]\n");
    test_ci_create_pipeline();
    test_ci_pipeline_lifecycle();
    test_ci_add_stage_and_job();
    test_ci_job_status_transitions();
    test_ci_job_logging();
    test_ci_generate_default_pipeline();
    test_ci_deploy_gate();
    test_ci_rollback();
    test_ci_count_jobs();

    printf("\n=== Results: %d passed, %d failed ===\n",
           tests_passed, tests_failed);
    return tests_failed > 0 ? 1 : 0;
}