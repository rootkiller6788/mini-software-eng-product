#include "branching_models.h"
#include "git_internals.h"
#include "merge_rebase.h"
#include "pr_review.h"
#include "conventional_commits.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* ── Demo: Trunk-Based Development + GitHub Flow ─────────────────── */

static int event_counter = 0;

#define LOG(fmt, ...) printf("  [%3d] " fmt "\n", ++event_counter, ##__VA_ARGS__)

int main(void) {
    printf("==================== TRUNK-BASED DEV DEMO ====================\n");
    printf("Workflow: TBD + GitHub Flow + Feature Flags + CI/CD\n");
    printf("==============================================================\n\n");

    /* ── 1. Initialize TBD ──────────────────────────────────────── */
    printf("── 1. Trunk-Based Development Setup ──\n");

    TrunkBasedDev tbd;
    tbd_init(&tbd);
    LOG("Trunk: %s (protected, review required, CI required)", tbd.trunk.name);
    LOG("Policy: max branch lifetime = %d hours", tbd.max_branch_hours);
    LOG("Policy: max commits/branch = %zu", tbd.max_commits_per_branch);

    /* ── 2. GitHub Flow setup ───────────────────────────────────── */
    printf("\n── 2. GitHub Flow Environment ──\n");

    GitHubFlow ghf;
    github_flow_init(&ghf);
    LOG("Main branch: %s (protected)", ghf.main_branch.name);

    /* ── 3. Feature flags ───────────────────────────────────────── */
    printf("\n── 3. Register Feature Flags ──\n");

    tbd_add_feature_flag(&tbd, "new-checkout", "New checkout experience", FF_TYPE_RELEASE);
    tbd_add_feature_flag(&tbd, "beta-analytics", "Real-time analytics dashboard", FF_TYPE_EXPERIMENT);
    tbd_add_feature_flag(&tbd, "dark-mode-v2", "Enhanced dark theme", FF_TYPE_RELEASE);
    tbd_add_feature_flag(&tbd, "emergency-rollback", "Kill switch for payments", FF_TYPE_OPS);
    tbd_add_feature_flag(&tbd, "admin-access-beta", "Admin-only new panel", FF_TYPE_PERMISSION);

    size_t fi;
    for (fi = 0; fi < tbd.flag_count; fi++) {
        LOG("Flag: %-22s [%s]", tbd.flags[fi].key,
            tbd.flags[fi].type == FF_TYPE_RELEASE   ? "release"   :
            tbd.flags[fi].type == FF_TYPE_EXPERIMENT ? "experiment" :
            tbd.flags[fi].type == FF_TYPE_OPS       ? "ops"       : "permission");
    }

    /* ── 4. Short-lived feature branches ────────────────────────── */
    printf("\n── 4. Short-Lived Feature Branches ──\n");

    const char *branches[] = {
        "feat/checkout-ui",
        "feat/analytics-engine",
        "fix/login-timeout",
        "feat/dark-mode-theme",
        "refactor/auth-service"
    };
    int nb = 5;

    PullRequestFull prs[10];
    int pr_idx = 0;
    memset(prs, 0, sizeof(prs));

    int i;
    for (i = 0; i < nb; i++) {
        github_flow_create_feature(&ghf, branches[i]);
        LOG("Created branch: %s (lifetime: %zu features active)", branches[i], ghf.feature_count);

        /* Simulate 1-3 commits on short-lived branch */
        int ncommits = (i % 3) + 1;
        int c;
        for (c = 0; c < ncommits; c++) {
            LOG("  commit on %s: implement part %d/%d", branches[i], c + 1, ncommits);
        }

        /* Open PR */
        char pr_title[256];
        snprintf(pr_title, sizeof(pr_title), "[%s] Implementation", branches[i]);
        github_flow_open_pr(&ghf, pr_title, branches[i], ghf.main_branch.name);
        LOG("  PR opened: %s", pr_title);

        /* Full PR initialization for review system */
        prfull_init(&prs[pr_idx], pr_title, "Detailed implementation description",
                     branches[i], ghf.main_branch.name, "dev-alice");
        prfull_add_assignee(&prs[pr_idx], "dev-alice");
        prfull_add_assignee(&prs[pr_idx], "dev-bob");

        if (i % 2 == 1)
            prfull_add_label(&prs[pr_idx], "enhancement");
        else
            prfull_add_label(&prs[pr_idx], "bug");

        pr_idx++;
    }

    /* ── 5. CI checks ───────────────────────────────────────────── */
    printf("\n── 5. CI/CD Status Checks ──\n");

    CiCheck ci_checks[3] = {
        { NULL, NULL, CI_PASSED, 0, 0 },
        { NULL, NULL, CI_PASSED, 0, 0 },
        { NULL, NULL, CI_PASSED, 0, 0 }
    };
    ci_check_set(&ci_checks[0], "unit-tests", CI_PASSED);
    ci_check_set(&ci_checks[1], "lint", CI_PASSED);
    ci_check_set(&ci_checks[2], "integration-tests", CI_PASSED);

    LOG("CI pipeline: %s, %s, %s - all PASSED",
        ci_checks[0].name, ci_checks[1].name, ci_checks[2].name);

    /* ── 6. Code review ─────────────────────────────────────────── */
    printf("\n── 6. Pull Request Review ──\n");

    for (i = 0; i < pr_idx; i++) {
        prfull_add_review(&prs[i], "reviewer-alice", REVIEW_APPROVED,
                           "Code looks clean, tests pass");
        LOG("PR [%s] reviewed by reviewer-alice: APPROVED", prs[i].title);

        /* Add line comments */
        PullRequestReview *last_rv = &prs[i].reviews[prs[i].review_count - 1];
        review_add_comment(last_rv, "src/main.c", 42,
                            "Consider using a constant for this magic number",
                            "reviewer-alice");
        review_add_comment(last_rv, "src/auth.c", 128,
                            "Add null-check before dereferencing",
                            "reviewer-alice");
        LOG("  +2 inline comments added");
    }

    /* ── 7. Merge check ─────────────────────────────────────────── */
    printf("\n── 7. Merge Readiness Verification ──\n");

    for (i = 0; i < pr_idx; i++) {
        int ready = prfull_ready_to_merge(&prs[i]);
        int approvals = prfull_approval_count(&prs[i]);
        LOG("PR [%s]: ready=%s, approvals=%d, mergeable=%s",
            prs[i].title,
            ready ? "YES" : "NO",
            approvals,
            prs[i].mergeable ? "yes" : "no");
    }

    /* ── 8. Merge via GitHub Flow ───────────────────────────────── */
    printf("\n── 8. Merging PRs ──\n");

    for (i = 0; i < pr_idx; i++) {
        /* Simulate review + merge */
        github_flow_review_pr(&ghf, (size_t)i, 1);
        github_flow_merge_pr(&ghf, (size_t)i,
                              (i < 2) ? PR_MERGE_SQUASH : PR_MERGE_REBASE);
        const char *style = (i < 2) ? "squash" : "rebase";
        LOG("Merged PR [%s] with %s merge", prs[i].title, style);
    }

    /* ── 9. Feature flag toggle and rollout ─────────────────────── */
    printf("\n── 9. Feature Flag Rollout ──\n");

    tbd_toggle_flag(&tbd, "new-checkout", 1);
    LOG("Toggled 'new-checkout' = ON (100%% rollout)");

    tbd_toggle_flag(&tbd, "dark-mode-v2", 1);
    tbd.flags[3].rollout_pct = 25.0;
    LOG("Toggled 'dark-mode-v2' = ON (25%% gradual rollout)");

    tbd_toggle_flag(&tbd, "beta-analytics", 1);
    tbd.flags[1].rollout_pct = 10.0;
    LOG("Toggled 'beta-analytics' = ON (10%% beta rollout)");

    printf("\n  Current feature flag status:\n");
    for (fi = 0; fi < tbd.flag_count; fi++) {
        printf("    %-22s enabled=%-3s  rollout=%-6.1f%%\n",
               tbd.flags[fi].key,
               tbd.flags[fi].enabled ? "yes" : "no",
               tbd.flags[fi].rollout_pct);
    }

    /* ── 10. Conventional commits on trunk ──────────────────────── */
    printf("\n── 10. Conventional Commits (Trunk) ──\n");

    ConventionalCommit trunk_commits[6];
    memset(trunk_commits, 0, sizeof(trunk_commits));

    trunk_commits[0] = cc_parse("feat(checkout): add new checkout flow\n\nFast and responsive checkout experience.");
    trunk_commits[1] = cc_parse("fix(checkout): correct tax calculation for EU");
    trunk_commits[2] = cc_parse("feat(analytics)!: redesign data pipeline\n\nBREAKING CHANGE: old event format deprecated");
    trunk_commits[3] = cc_parse("perf(db): add connection pooling");
    trunk_commits[4] = cc_parse("style(frontend): apply new formatting rules");
    trunk_commits[5] = cc_parse("test(e2e): add checkout flow integration tests");

    LOG("Trunk commits analyzed: %d", 6);

    BumpLevel trunk_bump = cc_determine_bump(trunk_commits, 6);
    LOG("Bump level: %s", trunk_bump == BUMP_MAJOR ? "MAJOR" :
                           trunk_bump == BUMP_MINOR ? "MINOR" : "PATCH");

    /* ── 11. Semantic versioning ────────────────────────────────── */
    printf("\n── 11. Semantic Versioning ──\n");

    SemVer current = semver_parse("2.3.1");
    SemVer bumped = semver_bump(&current, trunk_bump);

    char v_buf[64];
    semver_to_string(&current, v_buf, sizeof(v_buf));
    LOG("Current version: %s", v_buf);

    semver_to_string(&bumped, v_buf, sizeof(v_buf));
    LOG("Bumped version:  %s", v_buf);

    /* ── 12. Lint validation ────────────────────────────────────── */
    printf("\n── 12. Commit Linting ──\n");

    LintConfig lint_cfg;
    lint_config_default(&lint_cfg);

    const char *bad_msg = "Fix bug";
    LintError *errors = NULL;
    size_t err_count = 0;
    cc_lint(bad_msg, &lint_cfg, &errors, &err_count);
    LOG("Lint '%s': %zu errors", bad_msg, err_count);
    size_t ei;
    for (ei = 0; ei < err_count; ei++)
        LOG("  -> %s", errors[ei].detail);

    lint_errors_free(errors, err_count);

    const char *good_msg = "feat(core): implement zero-downtime deploy";
    cc_lint(good_msg, &lint_cfg, &errors, &err_count);
    LOG("Lint '%s': %zu errors (PASS)", good_msg, err_count);
    lint_errors_free(errors, err_count);

    /* ── 13. Changelog generation ───────────────────────────────── */
    printf("\n── 13. Changelog ──\n");

    Changelog cl;
    changelog_init(&cl, "https://github.com/myorg/tbd-project");
    changelog_add_entry(&cl, &bumped, trunk_commits, 6);

    char *cl_text = NULL;
    changelog_render_markdown(&cl, &cl_text);
    printf("%s", cl_text);
    free(cl_text);

    /* ── 14. Stale flag cleanup ─────────────────────────────────── */
    printf("\n── 14. Stale Flag Cleanup ──\n");

    tbd.flags[0].is_stale = 1;  /* new-checkout is fully shipped */
    LOG("Marked 'new-checkout' as stale");

    LOG("Before cleanup: %zu flags", tbd.flag_count);
    tbd_stale_cleanup(&tbd);
    LOG("After cleanup:  %zu flags", tbd.flag_count);

    /* ── 15. Merge queue for continuous delivery ────────────────── */
    printf("\n── 15. Merge Queue (Continuous Delivery) ──\n");

    MergeQueue mq;
    mergequeue_init(&mq, 5);

    int enqueued = 0;
    for (i = 0; i < pr_idx; i++) {
        if (mergequeue_enqueue(&mq, &prs[i]) == 0) enqueued++;
    }
    LOG("Enqueued: %d PRs, Queue size: %zu", enqueued, mq.queue_size);

    int processed = mergequeue_process(&mq);
    LOG("Processed: %d PRs merged, Remaining: %zu", processed, mq.queue_size);

    /* ── 16. Statistics & Summary ───────────────────────────────── */
    printf("\n── 16. Workflow Statistics ──\n");

    printf("\n");
    printf("  ┌─────────────────────────────────────┐\n");
    printf("  │  Trunk-Based Development Summary    │\n");
    printf("  ├─────────────────────────────────────┤\n");
    printf("  │  Total feature branches:   %-8zu │\n", ghf.feature_count);
    printf("  │  Total PRs created:        %-8d │\n", pr_idx);
    printf("  │  Total PRs merged:         %-8d │\n", pr_idx);
    printf("  │  Feature flags active:     %-8zu │\n", tbd.flag_count);
    printf("  │  Version:                  %-8s │\n", v_buf);
    printf("  │  Branch lifespan:          %-8d hrs │\n", tbd.max_branch_hours);
    printf("  │  Max commits/branch:       %-8zu │\n", tbd.max_commits_per_branch);
    printf("  │  Events logged:            %-8d │\n", event_counter);
    printf("  └─────────────────────────────────────┘\n");

    /* ── Cleanup ────────────────────────────────────────────────── */
    for (i = 0; i < 6; i++) cc_free(&trunk_commits[i]);
    for (i = 0; i < pr_idx; i++) prfull_free(&prs[i]);
    for (i = 0; i < 3; i++) { free(ci_checks[i].name); free(ci_checks[i].url); }
    semver_free(&current);
    semver_free(&bumped);
    changelog_free(&cl);
    lint_config_free(&lint_cfg);
    github_flow_free(&ghf);
    tbd_free(&tbd);
    mergequeue_free(&mq);

    printf("\n==================== DEMO COMPLETE ====================\n");
    return 0;
}
