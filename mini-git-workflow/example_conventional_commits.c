#include "conventional_commits.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Example: Conventional Commits — parse, lint, bump, changelog */

int main(void) {
    printf("=== Conventional Commits Example (C99) ===\n\n");

    /* ── 1. Parse commits ───────────────────────────────────────── */
    const char *msgs[] = {
        "feat(auth): add OAuth2 login support\n\n"
        "Implemented Google and GitHub OAuth providers.\n"
        "BREAKING CHANGE: removed legacy password auth endpoint",

        "fix(parser): handle null input in JSON parser\n\n"
        "Added null-check before parsing to prevent segfault.",

        "docs(readme): update installation instructions\n\n"
        "Added Windows and macOS sections.",

        "feat!: drop support for Node.js 12\n\n"
        "Minimum supported version is now Node.js 14.",

        "chore(deps): bump lodash from 4.17.20 to 4.17.21",

        "style(ui): fix indentation in header component",

        "perf(db): optimize query for user listing\n\n"
        "Added composite index on (org_id, created_at) for 10x speedup."
    };

    size_t msg_count = sizeof(msgs) / sizeof(msgs[0]);
    ConventionalCommit *parsed = (ConventionalCommit *)malloc(msg_count * sizeof(ConventionalCommit));

    printf("1. Parsed %zu commits:\n", msg_count);
    size_t i;
    for (i = 0; i < msg_count; i++) {
        parsed[i] = cc_parse(msgs[i]);
        printf("   [%zu] %s", i, commit_type_to_string(parsed[i].type));
        if (parsed[i].scope[0]) printf("(%s)", parsed[i].scope);
        if (parsed[i].is_breaking) printf(" [BREAKING]");
        printf(": %s\n", parsed[i].description);
    }

    /* ── 2. Check for breaking ──────────────────────────────────── */
    printf("\n2. Breaking change check:\n");
    for (i = 0; i < msg_count; i++) {
        if (cc_is_breaking(msgs[i]))
            printf("   Commit %zu is BREAKING\n", i);
    }

    /* ── 3. Version bump ────────────────────────────────────────── */
    SemVer current = semver_parse("1.5.2");
    printf("\n3. Current version: ");
    char vbuf[64];
    semver_to_string(&current, vbuf, sizeof(vbuf));
    printf("%s\n", vbuf);

    BumpLevel bump = cc_determine_bump(parsed, msg_count);
    printf("   Recommended bump: ");
    switch (bump) {
        case BUMP_MAJOR: printf("MAJOR (breaking)\n"); break;
        case BUMP_MINOR: printf("MINOR (feat)\n"); break;
        case BUMP_PATCH: printf("PATCH (fix)\n"); break;
        default: printf("NONE\n"); break;
    }

    SemVer next = semver_bump(&current, bump);
    semver_to_string(&next, vbuf, sizeof(vbuf));
    printf("   Next version: %s\n", vbuf);

    /* ── 4. Lint a bad message ──────────────────────────────────── */
    LintConfig cfg;
    lint_config_default(&cfg);
    cfg.require_scope = 1;

    const char *bad_msg = "Fix bug";
    LintError *errors = NULL;
    size_t err_count = 0;
    cc_lint(bad_msg, &cfg, &errors, &err_count);

    printf("\n4. Lint result for '%s': %zu error(s)\n", bad_msg, err_count);
    for (i = 0; i < err_count; i++) {
        printf("   Error %zu: %s\n", i + 1, errors[i].detail);
    }
    lint_errors_free(errors, err_count);

    /* ── 5. Lint a good message ─────────────────────────────────── */
    const char *good_msg = "feat(core): add caching layer\n\n"
                           "Implemented LRU cache for frequently accessed resources.\n"
                           "Improves response time by 40% under load.";
    cc_lint(good_msg, &cfg, &errors, &err_count);
    printf("5. Lint result for good message: %zu error(s)\n", err_count);
    lint_errors_free(errors, err_count);

    /* ── 6. Changelog generation ────────────────────────────────── */
    Changelog cl;
    changelog_init(&cl, "https://github.com/myorg/myrepo");

    changelog_add_entry(&cl, &next, parsed, msg_count);
    printf("\n6. Changelog:\n");
    char *rendered = NULL;
    changelog_render_markdown(&cl, &rendered);
    printf("%s", rendered);
    free(rendered);

    /* ── 7. SemVer comparison ───────────────────────────────────── */
    SemVer v1 = semver_parse("2.1.0-alpha+sha.1234abc");
    SemVer v2 = semver_parse("2.0.1");
    printf("7. Compare %s vs %s: %d\n", vbuf, "2.0.1", semver_compare(&v1, &v2));

    /* ── Cleanup ────────────────────────────────────────────────── */
    for (i = 0; i < msg_count; i++) cc_free(&parsed[i]);
    free(parsed);
    semver_free(&current);
    semver_free(&next);
    semver_free(&v1);
    semver_free(&v2);
    changelog_free(&cl);
    lint_config_free(&cfg);

    printf("\n=== Done ===\n");
    return 0;
}
