#include "git_commit.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>

/* ================================================================
 * L4: Conventional Commits v1.0.0 Specification Implementation
 *
 * Standard: https://www.conventionalcommits.org/en/v1.0.0/
 * The Conventional Commits specification is a lightweight convention
 * on top of commit messages, providing an easy set of rules for
 * creating an explicit commit history.
 *
 * Format:
 *   <type>[optional scope]: <description>
 *   [optional body]
 *   [optional footer(s)]
 *
 * Types: feat, fix, docs, style, refactor, perf, test, build, ci, chore, revert
 *
 * L4: Semantic Versioning 2.0.0 (https://semver.org/)
 *   Given a version number MAJOR.MINOR.PATCH:
 *   - MAJOR: incompatible API changes
 *   - MINOR: added backwards-compatible functionality
 *   - PATCH: backwards-compatible bug fixes
 * ================================================================ */

/* L2: Parse a commit message into a ConventionalCommit struct.
 * This is a complete parser for the Conventional Commits spec.
 * Time: O(N) where N = message length. */
ConventionalCommit commit_parse(const char *message) {
    ConventionalCommit cc;
    memset(&cc, 0, sizeof(cc));
    cc.valid = false;
    cc.version_bump = SEMVER_NONE;

    if (!message || strlen(message) == 0) return cc;

    strncpy(cc.raw_message, message, sizeof(cc.raw_message) - 1);

    /* Parse type: extract characters before '(' or ':' */
    char type_str[COMMIT_TYPE_MAX_LEN] = {0};
    int ti = 0;
    const char *p = message;

    /* Skip leading whitespace */
    while (*p == ' ' || *p == '\n') p++;

    /* Extract type — stop at : ( ! scope or space */
    while (*p && *p != '(' && *p != ':' && *p != ' ' &&
           *p != '!' && *p != '\n' && ti < COMMIT_TYPE_MAX_LEN - 1) {
        type_str[ti++] = *p++;
    }
    type_str[ti] = 0;

    /* Check for breaking change marker: type! or type(scope)!: */
    if (*p == '!') {
        cc.is_breaking_change = true;
        p++; /* skip ! */
    }

    if (ti == 0) return cc;
    cc.type = commit_type_from_string(type_str);
    if (cc.type == COMMIT_TYPE_UNKNOWN) return cc;

    /* Parse optional scope: (...) */
    if (*p == '(') {
        p++; /* skip '(' */
        int si = 0;
        while (*p && *p != ')' && si < COMMIT_SCOPE_MAX_LEN - 1) {
            cc.scope[si++] = *p++;
        }
        cc.scope[si] = 0;
        if (*p == ')') { p++; cc.has_scope = true; }
        else { cc.has_scope = false; memset(cc.scope, 0, sizeof(cc.scope)); }
    }

    /* Expect ': ' separator */
    if (*p == ':') p++;
    while (*p == ' ') p++;

    /* Parse description (until newline or end) */
    int di = 0;
    while (*p && *p != '\n' && di < COMMIT_DESC_MAX_LEN - 1) {
        cc.description[di++] = *p++;
    }
    cc.description[di] = 0;
    if (*p == '\n') p++;

    if (di == 0) return cc; /* empty description invalid */

    /* Parse optional body (separated by blank line) */
    if (*p == '\n') {
        p++; /* skip blank line */
        int bi = 0;
        while (*p && bi < COMMIT_BODY_MAX_LEN - 1) {
            /* Stop at footer pattern: "KEY: VALUE" or "KEY #VALUE" */
            if (bi > 0 && cc.body[bi-1] == '\n') {
                const char *look = p;
                /* Check for footer pattern: word-token: or word-token # */
                while (*look && *look != ':' && *look != '#' &&
                       *look != '\n' && *look != ' ') look++;
                if (*look == ':' || *look == '#') break;
            }
            cc.body[bi++] = *p++;
        }
        cc.body[bi] = 0;
        cc.has_body = (bi > 0);
    }

    /* Parse footers */
    while (*p) {
        /* Skip whitespace between footers */
        while (*p == '\n') p++;
        if (!*p) break;

        if (cc.footer_count >= COMMIT_FOOTER_MAX) break;

        CommitFooter *footer = &cc.footers[cc.footer_count];

        /* Parse footer key */
        const char *key_start = p;
        while (*p && *p != ':' && *p != '#' && *p != '\n' && *p != ' ') {
            p++;
        }

        if (*p == ' ' && *(p+1) == '#') {
            /* Handle "KEY #value" format */
            int key_len = (int)(p - key_start);
            if (key_len >= COMMIT_FOOTER_KEY_LEN) key_len = COMMIT_FOOTER_KEY_LEN - 1;
            memcpy(footer->key, key_start, key_len);
            footer->key[key_len] = 0;
            p += 2; /* skip space and # */
        } else if (*p == ':' || *p == '#') {
            /* Handle "KEY: value" or "KEY #value" format */
            int key_len = (int)(p - key_start);
            if (key_len >= COMMIT_FOOTER_KEY_LEN) key_len = COMMIT_FOOTER_KEY_LEN - 1;
            memcpy(footer->key, key_start, key_len);
            footer->key[key_len] = 0;
            p++; /* skip : or # */
            if (*p == ' ') p++; /* skip optional space */
        } else {
            break; /* Not a footer */
        }

        /* Parse footer value (until newline or end) */
        int vi = 0;
        while (*p && *p != '\n' && vi < COMMIT_FOOTER_VAL_LEN - 1) {
            footer->value[vi++] = *p++;
        }
        footer->value[vi] = 0;

        /* Check if breaking change */
        if (strcmp(footer->key, "BREAKING CHANGE") == 0 ||
            strcmp(footer->key, "BREAKING-CHANGE") == 0) {
            footer->is_breaking = true;
            cc.is_breaking_change = true;
        }

        cc.footer_count++;
    }

    /* Also detect breaking change from scope ending with ! */
    if (cc.has_scope && cc.scope[strlen(cc.scope)-1] == '!') {
        cc.is_breaking_change = true;
        cc.scope[strlen(cc.scope)-1] = 0; /* strip ! from scope */
    }

    /* Determine semantic version bump */
    cc.version_bump = commit_determine_bump(&cc);
    cc.valid = true;
    return cc;
}

/* L4: Validate that a conventional commit meets all spec requirements. */
bool commit_validate(const ConventionalCommit *cc) {
    if (!cc) return false;
    if (!cc->valid) return false;
    if (cc->type == COMMIT_TYPE_UNKNOWN) return false;
    if (strlen(cc->description) == 0) return false;
    /* Type must be lowercase */
    const char *valid_types[] = {"feat","fix","docs","style","refactor",
         "perf","test","build","ci","chore","revert",NULL};
    for (const char **t = valid_types; *t; t++) {
        if (commit_type_from_string(*t) == cc->type) return true;
    }
    return false;
}

bool commit_is_conventional(const char *message) {
    if (!message) return false;
    ConventionalCommit cc = commit_parse(message);
    return cc.valid;
}

/* L4: Determine semantic version bump from conventional commit type
 * and breaking change status.
 *   feat: MINOR
 *   fix: PATCH
 *   BREAKING CHANGE (any): MAJOR
 *   others: NONE */
SemverBump commit_determine_bump(const ConventionalCommit *cc) {
    if (!cc) return SEMVER_NONE;
    if (cc->is_breaking_change) return SEMVER_MAJOR;
    switch (cc->type) {
    case COMMIT_TYPE_FEAT: return SEMVER_MINOR;
    case COMMIT_TYPE_FIX:  return SEMVER_PATCH;
    default:               return SEMVER_NONE;
    }
}

/* L4: Parse type string to enum. */
CommitType commit_type_from_string(const char *type_str) {
    if (!type_str) return COMMIT_TYPE_UNKNOWN;
    if (strcmp(type_str, "feat") == 0)     return COMMIT_TYPE_FEAT;
    if (strcmp(type_str, "fix") == 0)      return COMMIT_TYPE_FIX;
    if (strcmp(type_str, "docs") == 0)     return COMMIT_TYPE_DOCS;
    if (strcmp(type_str, "style") == 0)    return COMMIT_TYPE_STYLE;
    if (strcmp(type_str, "refactor") == 0) return COMMIT_TYPE_REFACTOR;
    if (strcmp(type_str, "perf") == 0)     return COMMIT_TYPE_PERF;
    if (strcmp(type_str, "test") == 0)     return COMMIT_TYPE_TEST;
    if (strcmp(type_str, "build") == 0)    return COMMIT_TYPE_BUILD;
    if (strcmp(type_str, "ci") == 0)       return COMMIT_TYPE_CI;
    if (strcmp(type_str, "chore") == 0)    return COMMIT_TYPE_CHORE;
    if (strcmp(type_str, "revert") == 0)   return COMMIT_TYPE_REVERT;
    return COMMIT_TYPE_UNKNOWN;
}

const char* commit_type_to_string(CommitType type) {
    switch (type) {
    case COMMIT_TYPE_FEAT:     return "feat";
    case COMMIT_TYPE_FIX:      return "fix";
    case COMMIT_TYPE_DOCS:     return "docs";
    case COMMIT_TYPE_STYLE:    return "style";
    case COMMIT_TYPE_REFACTOR: return "refactor";
    case COMMIT_TYPE_PERF:     return "perf";
    case COMMIT_TYPE_TEST:     return "test";
    case COMMIT_TYPE_BUILD:    return "build";
    case COMMIT_TYPE_CI:       return "ci";
    case COMMIT_TYPE_CHORE:    return "chore";
    case COMMIT_TYPE_REVERT:   return "revert";
    default:                   return "unknown";
    }
}

bool commit_type_causes_version_bump(CommitType type) {
    return (type == COMMIT_TYPE_FEAT || type == COMMIT_TYPE_FIX);
}

const char* commit_type_description(CommitType type) {
    switch (type) {
    case COMMIT_TYPE_FEAT:     return "A new feature";
    case COMMIT_TYPE_FIX:      return "A bug fix";
    case COMMIT_TYPE_DOCS:     return "Documentation only changes";
    case COMMIT_TYPE_STYLE:    return "Changes that do not affect meaning (whitespace, formatting, etc)";
    case COMMIT_TYPE_REFACTOR: return "A code change that neither fixes a bug nor adds a feature";
    case COMMIT_TYPE_PERF:     return "A code change that improves performance";
    case COMMIT_TYPE_TEST:     return "Adding missing tests or correcting existing tests";
    case COMMIT_TYPE_BUILD:    return "Changes that affect the build system or external dependencies";
    case COMMIT_TYPE_CI:       return "Changes to CI configuration files and scripts";
    case COMMIT_TYPE_CHORE:    return "Other changes that do not modify src or test files";
    case COMMIT_TYPE_REVERT:   return "Reverts a previous commit";
    default:                   return "Unknown commit type";
    }
}

/* ================================================================
 * L4: Semantic Versioning 2.0.0
 *
 * Parses: MAJOR.MINOR.PATCH[-pre-release][+build-metadata]
 * Examples: 1.2.3, 1.0.0-alpha, 1.0.0-alpha.1, 1.0.0+build.1
 * ================================================================ */

Semver semver_parse(const char *version_str) {
    Semver v = {0, 0, 0, "", ""};
    if (!version_str) return v;

    const char *p = version_str;

    /* Parse major */
    v.major = (int)strtol(p, (char**)&p, 10);
    if (*p != '.') return v;
    p++; /* skip '.' */

    /* Parse minor */
    v.minor = (int)strtol(p, (char**)&p, 10);
    if (*p != '.') return v;
    p++; /* skip '.' */

    /* Parse patch */
    v.patch = (int)strtol(p, (char**)&p, 10);

    /* Parse optional pre-release */
    if (*p == '-') {
        p++;
        int i = 0;
        while (*p && *p != '+' && i < 31) {
            v.pre_release[i++] = *p++;
        }
        v.pre_release[i] = 0;
    }

    /* Parse optional build metadata */
    if (*p == '+') {
        p++;
        int i = 0;
        while (*p && i < 31) {
            v.build_meta[i++] = *p++;
        }
        v.build_meta[i] = 0;
    }

    return v;
}

/* L4: Compare two semantic versions.
 * Returns: <0 if a < b, 0 if a == b, >0 if a > b.
 * Precedence: MAJOR > MINOR > PATCH > pre-release presence */
int semver_compare(const Semver *a, const Semver *b) {
    if (!a || !b) return 0;
    if (a->major != b->major) return a->major - b->major;
    if (a->minor != b->minor) return a->minor - b->minor;
    if (a->patch != b->patch) return a->patch - b->patch;

    /* Pre-release versions have lower precedence */
    int a_has_pre = strlen(a->pre_release) > 0 ? 1 : 0;
    int b_has_pre = strlen(b->pre_release) > 0 ? 1 : 0;
    if (a_has_pre != b_has_pre) return b_has_pre - a_has_pre;
    if (a_has_pre) return strcmp(a->pre_release, b->pre_release);
    return 0;
}

Semver semver_bump(const Semver *version, SemverBump bump_type) {
    Semver v;
    if (!version) { memset(&v, 0, sizeof(v)); return v; }
    v = *version;
    memset(v.pre_release, 0, sizeof(v.pre_release));
    memset(v.build_meta, 0, sizeof(v.build_meta));

    switch (bump_type) {
    case SEMVER_MAJOR:
        v.major++;
        v.minor = 0;
        v.patch = 0;
        break;
    case SEMVER_MINOR:
        v.minor++;
        v.patch = 0;
        break;
    case SEMVER_PATCH:
        v.patch++;
        break;
    default:
        break;
    }
    return v;
}

bool semver_is_valid(const Semver *v) {
    return v && (v->major > 0 || v->minor > 0 || v->patch > 0 ||
                 (v->major == 0 && v->minor == 0 && v->patch >= 0));
}

void semver_format(const Semver *v, char *buf, int buf_len) {
    if (!v || !buf || buf_len <= 0) return;
    int written = snprintf(buf, buf_len, "%d.%d.%d", v->major, v->minor, v->patch);
    if (strlen(v->pre_release) > 0 && written < buf_len) {
        written += snprintf(buf + written, buf_len - written, "-%s", v->pre_release);
    }
    if (strlen(v->build_meta) > 0 && written < buf_len) {
        snprintf(buf + written, buf_len - written, "+%s", v->build_meta);
    }
}

Semver semver_next(const Semver *current, SemverBump bump) {
    return semver_bump(current, bump);
}

/* L6: Changelog generation from conventional commits.
 * Groups changes by type, identifies breaking changes.
 * Following the Keep a Changelog format (keepachangelog.com). */
Changelog changelog_generate(const ConventionalCommit *commits, int commit_count,
                              const Semver *current_version, SemverBump bump) {
    Changelog log;
    memset(&log, 0, sizeof(log));
    log.version = semver_next(current_version, bump);
    log.release_date = time(NULL);

    for (int i = 0; i < commit_count && log.entry_count < CHANGELOG_MAX_ENTRIES; i++) {
        if (!commits[i].valid) continue;
        if (commits[i].version_bump == SEMVER_NONE &&
            !commits[i].is_breaking_change) continue;

        ChangelogEntry *e = &log.entries[log.entry_count];
        e->version = log.version;
        e->type = commits[i].type;
        strncpy(e->description, commits[i].description, COMMIT_DESC_MAX_LEN - 1);
        e->description[COMMIT_DESC_MAX_LEN - 1] = 0;
        if (commits[i].has_scope) {
            strncpy(e->scope, commits[i].scope, COMMIT_SCOPE_MAX_LEN - 1);
        }
        e->is_breaking = commits[i].is_breaking_change;
        log.entry_count++;
    }
    return log;
}

void changelog_print(const Changelog *log) {
    if (!log) return;
    char ver_str[64];
    semver_format(&log->version, ver_str, sizeof(ver_str));
    printf("=== Changelog %s ===\n", ver_str);

    /* Print breaking changes first */
    for (int i = 0; i < log->entry_count; i++) {
        if (log->entries[i].is_breaking) {
            printf("  **BREAKING** %s: %s\n",
                   commit_type_to_string(log->entries[i].type),
                   log->entries[i].description);
        }
    }

    /* Features */
    for (int i = 0; i < log->entry_count; i++) {
        if (!log->entries[i].is_breaking &&
            log->entries[i].type == COMMIT_TYPE_FEAT) {
            printf("  feat");
            if (strlen(log->entries[i].scope) > 0)
                printf("(%s)", log->entries[i].scope);
            printf(": %s\n", log->entries[i].description);
        }
    }

    /* Fixes */
    for (int i = 0; i < log->entry_count; i++) {
        if (!log->entries[i].is_breaking &&
            log->entries[i].type == COMMIT_TYPE_FIX) {
            printf("  fix");
            if (strlen(log->entries[i].scope) > 0)
                printf("(%s)", log->entries[i].scope);
            printf(": %s\n", log->entries[i].description);
        }
    }
}

int changelog_count_breaking(const Changelog *log) {
    if (!log) return 0;
    int count = 0;
    for (int i = 0; i < log->entry_count; i++) {
        if (log->entries[i].is_breaking) count++;
    }
    return count;
}

int changelog_count_feats(const Changelog *log) {
    if (!log) return 0;
    int count = 0;
    for (int i = 0; i < log->entry_count; i++) {
        if (log->entries[i].type == COMMIT_TYPE_FEAT) count++;
    }
    return count;
}

int changelog_count_fixes(const Changelog *log) {
    if (!log) return 0;
    int count = 0;
    for (int i = 0; i < log->entry_count; i++) {
        if (log->entries[i].type == COMMIT_TYPE_FIX) count++;
    }
    return count;
}

/* L4: Format a conventional commit message from parsed struct. */
int commit_format_message(const ConventionalCommit *cc, char *buf, int buf_len) {
    if (!cc || !buf || buf_len <= 0) return 0;
    int pos = 0;

    pos += snprintf(buf + pos, buf_len - pos, "%s", commit_type_to_string(cc->type));
    if (cc->has_scope) {
        pos += snprintf(buf + pos, buf_len - pos, "(%s)", cc->scope);
    }
    if (cc->is_breaking_change) {
        pos += snprintf(buf + pos, buf_len - pos, "!");
    }
    pos += snprintf(buf + pos, buf_len - pos, ": %s", cc->description);

    if (cc->has_body) {
        pos += snprintf(buf + pos, buf_len - pos, "\n\n%s", cc->body);
    }
    for (int i = 0; i < cc->footer_count; i++) {
        pos += snprintf(buf + pos, buf_len - pos, "\n%s: %s",
                       cc->footers[i].key, cc->footers[i].value);
    }
    return pos;
}

ConventionalCommit commit_create(CommitType type, const char *scope,
                                  const char *description, const char *body) {
    ConventionalCommit cc;
    memset(&cc, 0, sizeof(cc));
    cc.type = type;
    if (scope) {
        strncpy(cc.scope, scope, COMMIT_SCOPE_MAX_LEN - 1);
        cc.has_scope = (strlen(scope) > 0);
    }
    if (description)
        strncpy(cc.description, description, COMMIT_DESC_MAX_LEN - 1);
    if (body) {
        strncpy(cc.body, body, COMMIT_BODY_MAX_LEN - 1);
        cc.has_body = (strlen(body) > 0);
    }
    cc.valid = true;
    cc.version_bump = commit_determine_bump(&cc);
    return cc;
}

bool commit_add_footer(ConventionalCommit *cc, const char *key, const char *value) {
    if (!cc || !key || !value || cc->footer_count >= COMMIT_FOOTER_MAX)
        return false;
    CommitFooter *f = &cc->footers[cc->footer_count];
    strncpy(f->key, key, COMMIT_FOOTER_KEY_LEN - 1);
    strncpy(f->value, value, COMMIT_FOOTER_VAL_LEN - 1);
    if (strcmp(key, "BREAKING CHANGE") == 0 || strcmp(key, "BREAKING-CHANGE") == 0) {
        f->is_breaking = true;
        cc->is_breaking_change = true;
        cc->version_bump = SEMVER_MAJOR;
    }
    cc->footer_count++;
    return true;
}

bool commit_has_footer(const ConventionalCommit *cc, const char *key) {
    if (!cc || !key) return false;
    for (int i = 0; i < cc->footer_count; i++) {
        if (strcmp(cc->footers[i].key, key) == 0) return true;
    }
    return false;
}

const char* commit_get_footer_value(const ConventionalCommit *cc, const char *key) {
    if (!cc || !key) return NULL;
    for (int i = 0; i < cc->footer_count; i++) {
        if (strcmp(cc->footers[i].key, key) == 0)
            return cc->footers[i].value;
    }
    return NULL;
}

/* L5: Detect if a raw message contains breaking change indicators.
 * Looks for "BREAKING CHANGE", "BREAKING-CHANGE", or "!" after type. */
bool commit_detect_breaking(const char *message) {
    if (!message) return false;
    if (strstr(message, "BREAKING CHANGE") || strstr(message, "BREAKING-CHANGE"))
        return true;
    /* Check for "type!:" or "type(scope)!:" pattern */
    const char *p = message;
    while (*p && *p != ':' && *p != '\n') {
        if (*p == '!' && (*(p+1) == ':' || (*(p+1) == '\n')))
            return true;
        p++;
    }
    return false;
}

int commit_filter_by_type(const ConventionalCommit *commits, int count,
                          CommitType type, ConventionalCommit *result,
                          int max_results) {
    if (!commits || !result) return 0;
    int found = 0;
    for (int i = 0; i < count && found < max_results; i++) {
        if (commits[i].valid && commits[i].type == type) {
            result[found++] = commits[i];
        }
    }
    return found;
}

int commit_filter_breaking(const ConventionalCommit *commits, int count,
                           ConventionalCommit *result, int max_results) {
    if (!commits || !result) return 0;
    int found = 0;
    for (int i = 0; i < count && found < max_results; i++) {
        if (commits[i].valid && commits[i].is_breaking_change) {
            result[found++] = commits[i];
        }
    }
    return found;
}

bool commit_batch_validate(const ConventionalCommit *commits, int count,
                           int *first_invalid_index) {
    if (!commits) return false;
    for (int i = 0; i < count; i++) {
        if (!commit_validate(&commits[i])) {
            if (first_invalid_index) *first_invalid_index = i;
            return false;
        }
    }
    return true;
}

void commit_print(const ConventionalCommit *cc) {
    if (!cc) return;
    printf("=== Conventional Commit ===\n");
    printf("  Valid: %s, Type: %s, Bump: ",
           cc->valid ? "yes" : "no",
           commit_type_to_string(cc->type));
    switch (cc->version_bump) {
    case SEMVER_MAJOR: printf("MAJOR\n"); break;
    case SEMVER_MINOR: printf("MINOR\n"); break;
    case SEMVER_PATCH: printf("PATCH\n"); break;
    default:           printf("NONE\n"); break;
    }
    if (cc->has_scope) printf("  Scope: %s\n", cc->scope);
    printf("  Description: %s\n", cc->description);
    if (cc->has_body) printf("  Body: %s\n", cc->body);
    if (cc->footer_count > 0) {
        printf("  Footers:\n");
        for (int i = 0; i < cc->footer_count; i++) {
            printf("    %s: %s%s\n",
                   cc->footers[i].key, cc->footers[i].value,
                   cc->footers[i].is_breaking ? " [BREAKING]" : "");
        }
    }
    if (cc->is_breaking_change) printf("  ** BREAKING CHANGE **\n");
}

void commit_print_compact(const ConventionalCommit *cc) {
    if (!cc) return;
    printf("%s", commit_type_to_string(cc->type));
    if (cc->has_scope) printf("(%s)", cc->scope);
    if (cc->is_breaking_change) printf("!");
    printf(": %s", cc->description);
    if (cc->is_breaking_change) printf(" [BREAKING]");
    printf("\n");
}

void semver_print(const Semver *v) {
    if (!v) return;
    char buf[64];
    semver_format(v, buf, sizeof(buf));
    printf("Version: %s\n", buf);
}
