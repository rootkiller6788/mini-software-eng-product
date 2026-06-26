#include "branching_models.h"
#include "merge_rebase.h"
#include "pr_review.h"
#include "conventional_commits.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * doc_branching_strategies.c — Educational comparison of branching models
 *
 * Three major Git branching strategies:
 *
 *   1. Git Flow            — branch-heavy, release-oriented
 *   2. GitHub Flow         — simple, feature-branch based
 *   3. Trunk-Based Dev     — minimal branching, continuous delivery
 *
 * Each is documented with its workflow, pros, cons, and when to use it.
 */

/* ── Git Flow detailed ──────────────────────────────────────────── */

static void doc_git_flow(void) {
    printf("═══════════════════════════════════════════════\n");
    printf("  GIT FLOW\n");
    printf("═══════════════════════════════════════════════\n\n");

    printf("Proposed by Vincent Driessen (2010).\n\n");

    printf("Branches:\n");
    printf("  master    — production-ready code only\n");
    printf("               Every merge to master = new release\n");
    printf("               Tags mark versions (v1.0.0, v1.1.0)\n\n");

    printf("  develop   — integration branch\n");
    printf("               All feature branches merge here first\n");
    printf("               Represents the 'next release' state\n\n");

    printf("  feature/* — branches off develop, merges back to develop\n");
    printf("               Example: feature/new-login, feature/api-v2\n");
    printf("               Lifetime: days to weeks\n\n");

    printf("  release/* — branches off develop, merges to master + develop\n");
    printf("               Example: release/2.0.0\n");
    printf("               Only bug fixes and version bumps allowed\n");
    printf("               Lifetime: days (release QA cycle)\n\n");

    printf("  hotfix/*  — branches off master, merges to master + develop\n");
    printf("               Example: hotfix/2.0.1\n");
    printf("               Critical production bug fixes\n");
    printf("               Lifetime: hours (urgent)\n\n");

    printf("Workflow diagram:\n");
    printf("  master   ●────●────────●────────────●\n");
    printf("           │    │        │            │\n");
    printf("  develop  ├────┼──●──●──┼──●──●──●───┼\n");
    printf("           │    │  │     │  │         │\n");
    printf("  feature/ │    │  ├─────┘  │         │\n");
    printf("  login    ●────┘  │        │         │\n");
    printf("  release/         ●────────┘         │\n");
    printf("  hotfix/                             ●──┘\n\n");

    printf("When to use:\n");
    printf("  + Multiple production versions to maintain\n");
    printf("  + Scheduled releases (monthly, quarterly)\n");
    printf("  + Large teams with formal QA process\n");
    printf("  + Products with versioned SDKs/APIs\n\n");

    printf("When NOT to use:\n");
    printf("  - Continuous deployment (too much overhead)\n");
    printf("  - Small teams (<5 people)\n");
    printf("  - Web apps with single production version\n\n");
}

/* ── GitHub Flow detailed ───────────────────────────────────────── */

static void doc_github_flow(void) {
    printf("═══════════════════════════════════════════════\n");
    printf("  GITHUB FLOW\n");
    printf("═══════════════════════════════════════════════\n\n");

    printf("Popularized by GitHub. Simple, PR-centric.\n\n");

    printf("Principles:\n");
    printf("  1. Anything in main is deployable\n");
    printf("  2. Create a branch for each feature/fix\n");
    printf("  3. Open a Pull Request as soon as code exists\n");
    printf("  4. Discuss, review, and iterate on the PR\n");
    printf("  5. Merge to main when ready (with CI passing)\n");
    printf("  6. Deploy immediately after merge\n\n");

    printf("Branches used:\n");
    printf("  main      — single source of truth, always deployable\n");
    printf("  feature/* — short-lived branches, one per feature\n");
    printf("  fix/*     — bug fix branches\n\n");

    printf("Pull Request lifecycle:\n");
    printf("  1. DRAFT      — work in progress, not reviewable\n");
    printf("  2. OPEN       — ready for review\n");
    printf("  3. REVIEW     — reviewers providing feedback\n");
    printf("  4. CHANGES REQUESTED — must be fixed before re-review\n");
    printf("  5. APPROVED   — all reviews passed, CI green\n");
    printf("  6. MERGED     — merged to main, branch deleted\n\n");

    printf("Merge strategies:\n");
    printf("  - Merge commit:    preserve full history (default)\n");
    printf("  - Squash & merge:  one clean commit per PR\n");
    printf("  - Rebase & merge:  linear history, no merge commits\n\n");

    printf("When to use:\n");
    printf("  + Continuous deployment teams\n");
    printf("  + Web services with single production environment\n");
    printf("  + Teams that rely heavily on code review\n");
    printf("  + Open-source projects (familiar PR model)\n\n");

    printf("When NOT to use:\n");
    printf("  - Need to maintain multiple release versions\n");
    printf("  - Highly regulated environments (formal releases)\n\n");
}

/* ── Trunk-Based Development detailed ───────────────────────────── */

static void doc_trunk_based(void) {
    printf("═══════════════════════════════════════════════\n");
    printf("  TRUNK-BASED DEVELOPMENT\n");
    printf("═══════════════════════════════════════════════\n\n");

    printf("Used by Google, Facebook, and high-performing DevOps teams.\n\n");

    printf("Core principle: Everyone commits to a single branch (trunk).\n");
    printf("If branches exist, they last hours, not days.\n\n");

    printf("Key practices:\n\n");
    printf("  1. Small, frequent commits to trunk\n");
    printf("     - Each commit should be 'green' (CI passes)\n");
    printf("     - Commits are reviewed via pair programming or\n");
    printf("       pre-commit review\n\n");

    printf("  2. Feature flags (feature toggles)\n");
    printf("     - Incomplete features are hidden behind flags\n");
    printf("     - Types: release toggle, experiment, ops, permission\n");
    printf("     - Stale flags must be cleaned up after release\n\n");

    printf("  3. Branch by abstraction\n");
    printf("     - Large refactors done incrementally on trunk\n");
    printf("     - Old and new code coexist behind abstraction layer\n\n");

    printf("  4. Continuous Integration\n");
    printf("     - Every commit triggers full CI pipeline\n");
    printf("     - Build must stay green at all times\n");
    printf("     - Broken builds are fixed immediately (stop-the-line)\n\n");

    printf("  5. Release from trunk\n");
    printf("     - Cut release branches only if necessary\n");
    printf("     - Ideally deploy directly from trunk\n");
    printf("     - Release tags identify deployed versions\n\n");

    printf("Short-lived branch pattern:\n");
    printf("  trunk   ●─●─●─●─●─●─●─●─●─●\n");
    printf("          │\           │\n");
    printf("  feat/A  └──┘         │  (max 1 day)\n");
    printf("  feat/B               └──┘  (max 8 hours)\n\n");

    printf("Feature flag lifecycle:\n");
    printf("  1. Create:    add_flag(\"new-ui\", release)\n");
    printf("  2. Develop:   if (flag_enabled(\"new-ui\")) { ... }\n");
    printf("  3. Rollout:   toggle(\"new-ui\", true) at 10%%\n");
    printf("  4. Ship:      toggle(\"new-ui\", true) at 100%%\n");
    printf("  5. Cleanup:   remove flag code, mark stale\n\n");

    printf("When to use:\n");
    printf("  + Continuous delivery / continuous deployment\n");
    printf("  + High-performing DevOps teams\n");
    printf("  + Microservice architectures\n");
    printf("  + Need for low merge conflict overhead\n\n");

    printf("When NOT to use:\n");
    printf("  - Teams without feature flag infrastructure\n");
    printf("  - Products with strict version control requirements\n");
    printf("  - Distributed teams with poor CI infrastructure\n\n");
}

/* ── Comparison table ───────────────────────────────────────────── */

static void doc_comparison(void) {
    printf("═══════════════════════════════════════════════\n");
    printf("  STRATEGY COMPARISON\n");
    printf("═══════════════════════════════════════════════\n\n");

    printf("  ┌─────────────────┬──────────┬──────────┬──────────┐\n");
    printf("  │                 │ Git Flow │ GH Flow  │ TBD      │\n");
    printf("  ├─────────────────┼──────────┼──────────┼──────────┤\n");
    printf("  │ Long-lived br.  │    yes   │   maybe  │    no    │\n");
    printf("  │ Feature branches│ required │ required │ optional │\n");
    printf("  │ Release branches│    yes   │    no    │    no    │\n");
    printf("  │ Merge conflicts │   high   │ moderate │   low    │\n");
    printf("  │ Review process  │  formal  │ PR-based │ pre/post │\n");
    printf("  │ CI/CD ready     │ moderate │   good   │ required │\n");
    printf("  │ Overhead        │   high   │   low    │  lowest  │\n");
    printf("  │ Release cadence │ scheduled│continuous│continuous│\n");
    printf("  │ Feature flags   │ optional │ optional │ required │\n");
    printf("  │ Best team size  │  10+     │   3+     │  any     │\n");
    printf("  └─────────────────┴──────────┴──────────┴──────────┘\n\n");
}

/* ── Branch protection walkthrough ──────────────────────────────── */

static void doc_branch_protection(void) {
    printf("═══════════════════════════════════════════════\n");
    printf("  BRANCH PROTECTION RULES\n");
    printf("═══════════════════════════════════════════════\n\n");

    printf("Common branch protection rules:\n\n");

    printf("  1. Require a pull request before merging\n");
    printf("     - No direct pushes to protected branches\n");
    printf("     - All changes go through PR workflow\n\n");

    printf("  2. Require approvals\n");
    printf("     - Minimum number of approving reviews\n");
    printf("     - Dismiss stale reviews on new commits\n\n");

    printf("  3. Require status checks to pass\n");
    printf("     - CI pipeline (tests, lint, build) must be green\n");
    printf("     - Can require specific checks (e.g., 'security-scan')\n\n");

    printf("  4. Require conversation resolution\n");
    printf("     - All review threads must be resolved\n");
    printf("     - Prevents merging with unresolved discussions\n\n");

    printf("  5. Require linear history\n");
    printf("     - No merge commits on the protected branch\n");
    printf("     - Enforces rebase-only workflow\n\n");

    printf("  6. Require signed commits\n");
    printf("     - GPG/SSH signature verification\n");
    printf("     - Ensures commit authenticity\n\n");

    printf("  7. Require CODEOWNERS review\n");
    printf("     - Specific files/directories require owner approval\n");
    printf("     - Example: src/auth/* requires @security-team\n\n");

    printf("  8. Restrict push access\n");
    printf("     - Only specific users/teams can push\n");
    printf("     - Even to merge PRs\n\n");

    printf("  9. Lock branch (GitHub)\n");
    printf("     - Read-only, cannot be updated at all\n");
    printf("     - Useful for archived branches\n\n");

    printf(" 10. Allow force pushes\n");
    printf("     - By default: OFF\n");
    printf("     - Can specify who is allowed to force-push\n");
    printf("     - Use with: git push --force-with-lease\n\n");
}

/* ── CODEOWNERS walkthrough ──────────────────────────────────────── */

static void doc_codeowners(void) {
    printf("═══════════════════════════════════════════════\n");
    printf("  CODEOWNERS\n");
    printf("═══════════════════════════════════════════════\n\n");

    printf("Location: .github/CODEOWNERS (root, docs/, or .github/)\n\n");

    printf("File format:\n");
    printf("  # Lines starting with # are comments\n");
    printf("  # Each line has a file pattern and owners\n\n");

    printf("  # Root files\n");
    printf("  *                    @global-owner1 @global-owner2\n\n");

    printf("  # Directory owners\n");
    printf("  /src/                @frontend-team\n");
    printf("  /src/auth/           @security-team\n");
    printf("  /docs/               @docs-team\n\n");

    printf("  # File-specific\n");
    printf("  /package.json        @devops-team\n");
    printf("  /Dockerfile          @devops-team\n");
    printf("  *.md                 @docs-team\n\n");

    printf("  # With minimum approvals\n");
    printf("  /src/payments/       @payments-lead @compliance(2)\n\n");

    printf("Order matters — the last matching pattern wins.\n\n");

    printf("When a PR touches these paths, the listed owners\n");
    printf("are automatically requested as reviewers.\n\n");
}

/* ── PR template walkthrough ────────────────────────────────────── */

static void doc_pr_templates(void) {
    printf("═══════════════════════════════════════════════\n");
    printf("  PULL REQUEST TEMPLATES\n");
    printf("═══════════════════════════════════════════════\n\n");

    printf("Location: .github/PULL_REQUEST_TEMPLATE/\n\n");

    printf("Common templates:\n\n");

    printf("  feature.md:\n");
    printf("    ## Description\n");
    printf("    ## Related Issue\n");
    printf("    ## Screenshots (if UI)\n");
    printf("    ## Checklist\n");
    printf("    - [ ] Tests added\n");
    printf("    - [ ] Documentation updated\n\n");

    printf("  bug_fix.md:\n");
    printf("    ## Bug Description\n");
    printf("    ## Steps to Reproduce\n");
    printf("    ## Expected Behavior\n");
    printf("    ## Actual Behavior\n");
    printf("    ## Fix Description\n");
    printf("    ## Regression Test\n\n");

    printf("  breaking.md:\n");
    printf("    ## Breaking Change\n");
    printf("    ## Migration Guide\n");
    printf("    ## Deprecated APIs\n");
    printf("    ## Rollout Plan\n\n");

    printf("Template querystring:\n");
    printf("  ?template=bug_fix.md\n");
    printf("  ?expand=1&template=feature.md\n\n");
}

/* ── Conventional Commits in branching context ──────────────────── */

static void doc_conventional_commits_in_context(void) {
    printf("═══════════════════════════════════════════════\n");
    printf("  CONVENTIONAL COMMITS + BRANCHING\n");
    printf("═══════════════════════════════════════════════\n\n");

    printf("Type  ─ Branches it commonly appears on:\n");
    printf("  feat     ─ feature/* branches\n");
    printf("  fix      ─ fix/*, hotfix/* branches\n");
    printf("  docs     ─ any branch\n");
    printf("  style    ─ develop, main (formatting)\n");
    printf("  refactor ─ refactor/* branches\n");
    printf("  perf     ─ perf/* branches, main\n");
    printf("  test     ─ feature/*, fix/* branches\n");
    printf("  chore    ─ release/* (version bumps), main\n");
    printf("  ci       ─ any branch (CI config changes)\n");
    printf("  build    ─ any branch (build system changes)\n\n");

    printf("Semantic versioning from commits:\n");
    printf("  fix               → PATCH bump (1.0.0 → 1.0.1)\n");
    printf("  feat              → MINOR bump (1.0.0 → 1.1.0)\n");
    printf("  feat + BREAKING   → MAJOR bump (1.0.0 → 2.0.0)\n\n");

    printf("Integration with CI:\n");
    printf("  1. Commit hook: lint message before committing\n");
    printf("  2. PR check: validate all commits follow convention\n");
    printf("  3. Merge check: block merge if any commit fails lint\n");
    printf("  4. Post-merge: auto-bump version, generate changelog\n");
    printf("  5. Release: tag with new version, publish release notes\n\n");
}

/* ── Main ───────────────────────────────────────────────────────── */

int main(void) {
    printf("BRANCHING STRATEGIES — C99 Educational Documentation\n");
    printf("=====================================================\n\n");

    doc_git_flow();
    doc_github_flow();
    doc_trunk_based();
    doc_comparison();
    doc_branch_protection();
    doc_codeowners();
    doc_pr_templates();
    doc_conventional_commits_in_context();

    printf("═══════════════════════════════════════════════\n");
    printf("  FURTHER READING\n");
    printf("═══════════════════════════════════════════════\n\n");
    printf("  - Git Flow:       https://nvie.com/posts/a-successful-git-branching-model/\n");
    printf("  - GitHub Flow:    https://docs.github.com/en/get-started/quickstart/github-flow\n");
    printf("  - Trunk-Based:    https://trunkbaseddevelopment.com/\n");
    printf("  - Conv. Commits:  https://www.conventionalcommits.org/\n");
    printf("  - SemVer:         https://semver.org/\n");
    printf("  - Keep a Changelog: https://keepachangelog.com/\n\n");

    return 0;
}
