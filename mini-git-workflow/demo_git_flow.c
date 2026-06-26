#include "branching_models.h"
#include "git_internals.h"
#include "merge_rebase.h"
#include "pr_review.h"
#include "conventional_commits.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* ── Demo: Git Flow end-to-end with conventional commits ──────────── */

static int commit_count = 0;

static void print_separator(const char *title) {
    printf("\n══════ %s ══════\n\n", title);
}

static void simulate_commit(const char *branch, const char *type,
                             const char *scope, const char *desc) {
    printf("  [%d] %s: %s(%s): %s\n", ++commit_count, branch, type, scope, desc);
}

int main(void) {
    printf("==================== GIT FLOW DEMO ====================\n");
    printf("Full workflow: Git Flow + Conventional Commits + PRs\n");
    printf("=======================================================\n");

    /* ── Phase 1: Initialize project ────────────────────────────── */
    print_separator("Phase 1: Initialize Project");

    GitFlow gf;
    git_flow_init(&gf);
    printf("  Branch: %s (protected, production)\n", gf.master.name);
    printf("  Branch: %s (integration)\n", gf.develop.name);

    /* Initialize PR system */
    PullRequestFull pr_master[10];
    PullRequestFull pr_dev[10];
    int pr_count = 0;
    memset(pr_master, 0, sizeof(pr_master));
    memset(pr_dev, 0, sizeof(pr_dev));

    /* Initialize version */
    SemVer current_version = semver_parse("0.1.0");
    printf("  Version: ");
    char vbuf[64];
    semver_to_string(&current_version, vbuf, sizeof(vbuf));
    printf("%s\n", vbuf);

    /* ── Phase 2: Feature development ───────────────────────────── */
    print_separator("Phase 2: Feature Development");

    const char *feat_names[] = { "feature/user-auth", "feature/dashboard", "feature/api-v2" };
    const char *ticket_ids[] = { "DEV-101", "DEV-102", "DEV-103" };
    int fi;

    for (fi = 0; fi < 3; fi++) {
        git_flow_start_feature(&gf, feat_names[fi], ticket_ids[fi]);
        printf("  Created %s (ticket: %s)\n", feat_names[fi], ticket_ids[fi]);

        /* Simulate commits on this feature */
        simulate_commit(feat_names[fi], "feat", "core", "implement initial module");
        simulate_commit(feat_names[fi], "test", "unit", "add test coverage");
        simulate_commit(feat_names[fi], "docs", "api", "document the module");

        /* Finish feature: open PR to develop, review, merge */
        prfull_init(&pr_dev[pr_count],
                     ticket_ids[fi], "Implements the feature",
                     feat_names[fi], gf.develop.name, "dev-team");
        prfull_add_assignee(&pr_dev[pr_count], "reviewer1");
        prfull_add_label(&pr_dev[pr_count], "enhancement");

        /* Review */
        prfull_add_review(&pr_dev[pr_count], "reviewer1", REVIEW_APPROVED, "LGTM");
        printf("  PR for %s: reviewed and approved\n", feat_names[fi]);

        git_flow_finish_feature(&gf, feat_names[fi]);
        printf("  Feature %s merged into develop\n", feat_names[fi]);
        pr_count++;
    }

    /* ── Phase 3: Bug fix ───────────────────────────────────────── */
    print_separator("Phase 3: Bug Fix on Develop");

    simulate_commit(gf.develop.name, "fix", "auth", "handle null session token");
    simulate_commit(gf.develop.name, "fix", "dashboard", "fix chart rendering on mobile");

    /* ── Phase 4: Release preparation ───────────────────────────── */
    print_separator("Phase 4: Release 0.2.0");

    BumpLevel release_bump = BUMP_MINOR;
    SemVer release_version = semver_bump(&current_version, release_bump);
    semver_to_string(&release_version, vbuf, sizeof(vbuf));
    printf("  Bump: 0.1.0 -> %s (minor)\n", vbuf);

    git_flow_start_release(&gf, vbuf);
    printf("  Created release/%s\n", vbuf);

    simulate_commit("release/0.2.0", "chore", "release", "update changelog");
    simulate_commit("release/0.2.0", "style", "lint", "apply clang-format");
    simulate_commit("release/0.2.0", "docs", "release", "add release notes");

    /* PR from release to master */
    prfull_init(&pr_master[0],
                 "Release v0.2.0", "Production release",
                 "release/0.2.0", gf.master.name, "release-manager");
    prfull_add_assignee(&pr_master[0], "senior-reviewer");
    prfull_add_review(&pr_master[0], "senior-reviewer", REVIEW_APPROVED, "Ready for production");
    printf("  PR release/0.2.0 -> master: approved\n");

    git_flow_finish_release(&gf, vbuf);
    printf("  Release %s merged to master and back to develop\n", vbuf);

    current_version = release_version;

    /* ── Phase 5: Hotfix ────────────────────────────────────────── */
    print_separator("Phase 5: Hotfix 0.2.1");

    git_flow_start_hotfix(&gf, "0.2.1");
    printf("  Created hotfix/0.2.1 (branched from master)\n");

    simulate_commit("hotfix/0.2.1", "fix", "security", "patch XSS vulnerability");

    /* PR from hotfix to master */
    prfull_init(&pr_master[1],
                 "Hotfix v0.2.1 - XSS patch", "Critical security fix",
                 "hotfix/0.2.1", gf.master.name, "security-team");
    prfull_add_label(&pr_master[1], "security");
    prfull_add_label(&pr_master[1], "hotfix");
    prfull_add_review(&pr_master[1], "security-lead", REVIEW_APPROVED, "URGENT: ship immediately");
    printf("  PR hotfix/0.2.1 -> master: approved (urgent)\n");

    SemVer hotfix_version = semver_parse("0.2.1");
    git_flow_finish_hotfix(&gf, "0.2.1");
    printf("  Hotfix merged to master and develop\n");

    current_version = hotfix_version;

    /* ── Phase 6: Changelog from conventional commits ───────────── */
    print_separator("Phase 6: Generate Changelog");

    ConventionalCommit cc_samples[5];
    memset(cc_samples, 0, sizeof(cc_samples));

    cc_samples[0] = cc_parse("feat(core): implement initial module");
    cc_samples[1] = cc_parse("fix(auth): handle null session token");
    cc_samples[2] = cc_parse("feat(dashboard): add real-time analytics");
    cc_samples[3] = cc_parse("fix(security): patch XSS vulnerability\n\nBREAKING CHANGE: deprecated old auth API");
    cc_samples[4] = cc_parse("docs(api): update endpoint documentation");

    BumpLevel cc_bump = cc_determine_bump(cc_samples, 5);
    printf("  Conventional commit analysis:\n");
    printf("    Recommended bump: ");
    switch (cc_bump) {
        case BUMP_MAJOR: printf("MAJOR (breaking change detected)\n"); break;
        case BUMP_MINOR: printf("MINOR (new features)\n"); break;
        default: printf("PATCH\n"); break;
    }

    Changelog cl;
    changelog_init(&cl, "https://github.com/myorg/mini-git-workflow");
    changelog_add_entry(&cl, &current_version, cc_samples, 5);

    char *changelog_text = NULL;
    changelog_render_markdown(&cl, &changelog_text);
    printf("  Changelog for v0.2.1:\n%s", changelog_text);
    free(changelog_text);

    /* ── Phase 7: Branch protection & PR statistics ─────────────── */
    print_separator("Phase 7: Branch Protection & Statistics");

    printf("  Master branch protection:\n");
    printf("    - Protected:          %s\n", gf.master.is_protected ? "yes" : "no");
    printf("    - Require PR review:  %s\n", gf.master.require_review ? "yes" : "no");
    printf("    - Require CI:         %s\n", gf.master.require_ci ? "yes" : "no");
    printf("  Develop branch protection:\n");
    printf("    - Protected:          %s\n", gf.develop.is_protected ? "yes" : "no");

    printf("\n  Git Flow statistics:\n");
    printf("    Total features:        %zu\n", gf.feature_count);
    printf("    Total releases:        %zu\n", gf.release_count);
    printf("    Total hotfixes:        %zu\n", gf.hotfix_count);
    printf("    Total PRs created:     %d\n", pr_count);
    printf("    Total commits:         %d\n", commit_count);

    /* ── Phase 8: Verify PR merge readiness ─────────────────────── */
    print_separator("Phase 8: Merge Queue Simulation");

    MergeQueue mq;
    mergequeue_init(&mq, 3);

    printf("  Merge queue batch size: %d\n", mq.batch_size);
    for (fi = 0; fi < pr_count && fi < 3; fi++) {
        int enq = mergequeue_enqueue(&mq, &pr_dev[fi]);
        printf("  Enqueue %s: %s\n",
               pr_dev[fi].title,
               enq == 0 ? "ready" : "not ready");
    }

    int merged = mergequeue_process(&mq);
    printf("  Processed batch: %d merged\n", merged);
    printf("  Remaining in queue: %zu\n", mq.queue_size);

    /* ── Phase 9: Feature flags for future work ─────────────────── */
    print_separator("Phase 9: Feature Flags (Post-Release)");

    TrunkBasedDev tbd;
    tbd_init(&tbd);
    tbd_add_feature_flag(&tbd, "beta-search", "New search engine", FF_TYPE_RELEASE);
    tbd_add_feature_flag(&tbd, "voice-input", "Voice command support", FF_TYPE_EXPERIMENT);
    tbd_add_feature_flag(&tbd, "kill-legacy-api", "Emergency rollback switch", FF_TYPE_OPS);

    printf("  Registered %zu feature flags for next cycle:\n", tbd.flag_count);
    size_t ffi;
    for (ffi = 0; ffi < tbd.flag_count; ffi++) {
        printf("    %-20s [%-12s] enabled=%s\n",
               tbd.flags[ffi].key,
               tbd.flags[ffi].type == FF_TYPE_RELEASE ? "release" :
               tbd.flags[ffi].type == FF_TYPE_EXPERIMENT ? "experiment" :
               tbd.flags[ffi].type == FF_TYPE_OPS ? "ops" : "permission",
               tbd.flags[ffi].enabled ? "yes" : "no");
    }

    /* ── Phase 10: CODEOWNERS ───────────────────────────────────── */
    print_separator("Phase 10: CODEOWNERS");

    CodeOwners co;
    codeowners_load(".", &co);
    printf("  CODEOWNERS entries loaded: %zu\n", co.entry_count);

    CodeOwnerEntry *required = NULL;
    size_t required_count = 0;
    if (codeowners_find_required(&co, "src/core/auth.c", &required, &required_count) == 0) {
        printf("  Owners for 'src/core/auth.c':\n");
        size_t oi;
        for (oi = 0; oi < required_count; oi++)
            printf("    pattern=%s  owners=%s\n", required[oi].path_pattern, required[oi].owners);
    }
    free(required);

    /* ── Cleanup ────────────────────────────────────────────────── */
    int ci;
    for (ci = 0; ci < 5; ci++) cc_free(&cc_samples[ci]);
    for (ci = 0; ci < pr_count; ci++) prfull_free(&pr_dev[ci]);
    prfull_free(&pr_master[0]);
    prfull_free(&pr_master[1]);
    semver_free(&current_version);
    semver_free(&release_version);
    semver_free(&hotfix_version);
    changelog_free(&cl);
    git_flow_free(&gf);
    tbd_free(&tbd);
    mergequeue_free(&mq);
    codeowners_free(&co);

    printf("\n==================== DEMO COMPLETE ====================\n");
    return 0;
}
