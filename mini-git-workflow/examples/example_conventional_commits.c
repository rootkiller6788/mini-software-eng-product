#include <stdio.h>
#include <string.h>
#include "git_commit.h"

/* L4-L7: Conventional Commits + Semantic Versioning Demo
 *
 * Demonstrates:
 *   1. Parsing conventional commit messages
 *   2. Determining version bumps
 *   3. Generating changelogs
 *   4. Semantic versioning operations
 *   5. Batch validation of commit messages
 */

int main() {
    printf("=== Conventional Commits & Semver Demo ===\n\n");

    /* Part 1: Parse individual commits */
    printf("--- Part 1: Parsing Commit Messages ---\n");

    const char *messages[] = {
        "feat(auth): add OAuth2 login support",
        "fix(api): resolve null pointer in request handler",
        "docs: update API documentation for v2",
        "feat!: drop support for legacy v1 endpoints\n\n"
        "BREAKING CHANGE: All v1 endpoints have been removed",
        "refactor(database): extract connection pool to separate module",
        "perf(cache): improve LRU eviction algorithm",
        "test(auth): add integration tests for login flow",
        "chore(deps): update dependencies to latest versions",
        "fix: critical memory leak in connection pool",
        "feat(ui): add dark mode support"
    };
    int num_messages = sizeof(messages) / sizeof(messages[0]);

    ConventionalCommit parsed[16];
    for (int i = 0; i < num_messages && i < 16; i++) {
        parsed[i] = commit_parse(messages[i]);
        printf("[%d] ", i + 1);
        commit_print_compact(&parsed[i]);
    }

    /* Part 2: Version Bump Analysis */
    printf("\n--- Part 2: Version Bump Analysis ---\n");

    Semver current = {1, 5, 0};
    SemverBump max_bump = SEMVER_NONE;

    for (int i = 0; i < num_messages; i++) {
        if (parsed[i].valid && parsed[i].version_bump > max_bump) {
            max_bump = parsed[i].version_bump;
        }
    }

    const char *bump_name;
    switch (max_bump) {
    case SEMVER_MAJOR: bump_name = "MAJOR (breaking)"; break;
    case SEMVER_MINOR: bump_name = "MINOR (feature)"; break;
    case SEMVER_PATCH: bump_name = "PATCH (fix)"; break;
    default:           bump_name = "NONE"; break;
    }

    printf("Current version: %d.%d.%d\n", current.major, current.minor, current.patch);
    printf("Recommended bump: %s\n", bump_name);
    Semver next = semver_next(&current, max_bump);
    printf("Next version: %d.%d.%d\n", next.major, next.minor, next.patch);

    /* Part 3: Changelog Generation */
    printf("\n--- Part 3: Changelog Generation ---\n");

    Changelog log = changelog_generate(parsed, num_messages, &current, max_bump);
    printf("Breaking changes: %d\n", changelog_count_breaking(&log));
    printf("Features: %d\n", changelog_count_feats(&log));
    printf("Fixes: %d\n", changelog_count_fixes(&log));
    changelog_print(&log);

    /* Part 4: Semantic Version Operations */
    printf("\n--- Part 4: Semantic Versioning ---\n");

    Semver v1 = semver_parse("2.3.4");
    Semver v2 = semver_parse("2.4.0");
    Semver v3 = semver_parse("3.0.0-alpha");

    printf("v1: "); semver_print(&v1);
    printf("v2: "); semver_print(&v2);
    printf("v3: "); semver_print(&v3);

    printf("v1 < v2: %s\n", semver_compare(&v1, &v2) < 0 ? "true" : "false");
    printf("v2 < v3: %s\n", semver_compare(&v2, &v3) < 0 ? "true" : "false");
    printf("v1 == v1: %s\n", semver_compare(&v1, &v1) == 0 ? "true" : "false");

    /* Part 5: Batch validation */
    printf("\n--- Part 5: Batch Validation ---\n");
    int first_invalid;
    bool all_valid = commit_batch_validate(parsed, num_messages, &first_invalid);
    printf("All commits valid: %s\n", all_valid ? "YES" : "NO");
    if (!all_valid) {
        printf("First invalid at index: %d\n", first_invalid);
    }

    /* Part 6: Breaking change detection */
    printf("\n--- Part 6: Breaking Change Detection ---\n");
    ConventionalCommit breaking[16];
    int breaking_count = commit_filter_breaking(parsed, num_messages,
                                                 breaking, 16);
    printf("Breaking changes found: %d\n", breaking_count);
    for (int i = 0; i < breaking_count; i++) {
        printf("  - ");
        commit_print_compact(&breaking[i]);
    }

    /* Part 7: Format a message programmatically */
    printf("\n--- Part 7: Programmatic Message Construction ---\n");
    ConventionalCommit new_cc = commit_create(COMMIT_TYPE_FEAT, "api",
                                               "add GraphQL endpoint", NULL);
    commit_add_footer(&new_cc, "Closes", "#456");
    commit_add_footer(&new_cc, "Reviewed-by", "alice");
    char formatted[512];
    commit_format_message(&new_cc, formatted, sizeof(formatted));
    printf("Formatted:\n%s\n", formatted);

    /* Part 8: Footer lookup */
    printf("\n--- Part 8: Footer Operations ---\n");
    if (commit_has_footer(&new_cc, "Closes")) {
        printf("Closes: %s\n", commit_get_footer_value(&new_cc, "Closes"));
    }
    if (commit_has_footer(&new_cc, "Reviewed-by")) {
        printf("Reviewed-by: %s\n", commit_get_footer_value(&new_cc, "Reviewed-by"));
    }

    return 0;
}
