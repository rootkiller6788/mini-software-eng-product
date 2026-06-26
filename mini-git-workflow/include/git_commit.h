#ifndef GIT_COMMIT_H
#define GIT_COMMIT_H

#include <stdint.h>
#include <stdbool.h>

/* ================================================================
 * L1-L4: Conventional Commits Specification v1.0.0
 *
 * L4 Standard: Conventional Commits (conventionalcommits.org)
 *   Format: <type>[optional scope]: <description>
 *           [optional body]
 *           [optional footer(s)]
 *
 * Types: feat, fix, docs, style, refactor, perf, test, build, ci, chore, revert
 *
 * L4 Standard: Semantic Versioning 2.0.0 (semver.org)
 *   MAJOR.MINOR.PATCH
 *   - MAJOR: incompatible API changes (BREAKING CHANGE footer)
 *   - MINOR: new backwards-compatible functionality (feat:)
 *   - PATCH: backwards-compatible bug fixes (fix:)
 *
 * Courses: Cambridge Part II SE, Stanford CS 144
 * ================================================================ */

#define COMMIT_TYPE_MAX_LEN       16
#define COMMIT_SCOPE_MAX_LEN      32
#define COMMIT_DESC_MAX_LEN       256
#define COMMIT_BODY_MAX_LEN       512
#define COMMIT_FOOTER_MAX         8
#define COMMIT_FOOTER_KEY_LEN     32
#define COMMIT_FOOTER_VAL_LEN     128
#define COMMIT_SHA_MAX_LEN        40
#define CHANGELOG_MAX_ENTRIES     64

/* L1: Conventional commit type enumeration */
typedef enum {
    COMMIT_TYPE_FEAT      = 0,
    COMMIT_TYPE_FIX       = 1,
    COMMIT_TYPE_DOCS      = 2,
    COMMIT_TYPE_STYLE     = 3,
    COMMIT_TYPE_REFACTOR  = 4,
    COMMIT_TYPE_PERF      = 5,
    COMMIT_TYPE_TEST      = 6,
    COMMIT_TYPE_BUILD     = 7,
    COMMIT_TYPE_CI        = 8,
    COMMIT_TYPE_CHORE     = 9,
    COMMIT_TYPE_REVERT    = 10,
    COMMIT_TYPE_UNKNOWN   = 11
} CommitType;

/* L1: Semantic version bump */
typedef enum {
    SEMVER_NONE   = 0,
    SEMVER_PATCH  = 1,  /* fix: */
    SEMVER_MINOR  = 2,  /* feat: */
    SEMVER_MAJOR  = 3   /* BREAKING CHANGE */
} SemverBump;

/* L1: Semantic version struct */
typedef struct {
    int         major;
    int         minor;
    int         patch;
    char        pre_release[32];
    char        build_meta[32];
} Semver;

/* L1: Commit footer (e.g., "BREAKING CHANGE: ...", "Closes #123") */
typedef struct {
    char        key[COMMIT_FOOTER_KEY_LEN];
    char        value[COMMIT_FOOTER_VAL_LEN];
    bool        is_breaking;
} CommitFooter;

/* L1: Conventional commit parsed representation */
typedef struct {
    CommitType  type;
    char        scope[COMMIT_SCOPE_MAX_LEN];
    bool        has_scope;
    char        description[COMMIT_DESC_MAX_LEN];
    char        body[COMMIT_BODY_MAX_LEN];
    bool        has_body;
    CommitFooter footers[COMMIT_FOOTER_MAX];
    int         footer_count;
    bool        is_breaking_change;
    SemverBump  version_bump;
    char        raw_message[COMMIT_DESC_MAX_LEN + COMMIT_BODY_MAX_LEN];
    bool        valid;           /* passes conventional commit validation */
} ConventionalCommit;

/* L1: Changelog entry */
typedef struct {
    Semver      version;
    CommitType  type;
    char        description[COMMIT_DESC_MAX_LEN];
    char        scope[COMMIT_SCOPE_MAX_LEN];
    bool        is_breaking;
} ChangelogEntry;

/* L1: Changelog (cumulative for a version) */
typedef struct {
    Semver          version;
    ChangelogEntry  entries[CHANGELOG_MAX_ENTRIES];
    int             entry_count;
    time_t          release_date;
} Changelog;

/* === API Declarations === */

/* Conventional Commit Parsing (L4: spec compliance) */
ConventionalCommit commit_parse(const char *message);
bool               commit_validate(const ConventionalCommit *cc);
bool               commit_is_conventional(const char *message);
SemverBump         commit_determine_bump(const ConventionalCommit *cc);

/* Commit Type Utilities */
CommitType   commit_type_from_string(const char *type_str);
const char*  commit_type_to_string(CommitType type);
bool         commit_type_causes_version_bump(CommitType type);
const char*  commit_type_description(CommitType type);

/* Semantic Versioning (L4: semver 2.0.0) */
Semver       semver_parse(const char *version_str);
int          semver_compare(const Semver *a, const Semver *b);
Semver       semver_bump(const Semver *version, SemverBump bump_type);
bool         semver_is_valid(const Semver *v);
void         semver_format(const Semver *v, char *buf, int buf_len);
Semver       semver_next(const Semver *current, SemverBump bump);

/* Changelog Generation */
Changelog    changelog_generate(const ConventionalCommit *commits, int commit_count,
                                const Semver *current_version, SemverBump bump);
void         changelog_print(const Changelog *log);
int          changelog_count_breaking(const Changelog *log);
int          changelog_count_feats(const Changelog *log);
int          changelog_count_fixes(const Changelog *log);

/* Message Construction */
int          commit_format_message(const ConventionalCommit *cc, char *buf, int buf_len);
ConventionalCommit commit_create(CommitType type, const char *scope,
                                 const char *description, const char *body);

/* Footer Utilities */
bool         commit_add_footer(ConventionalCommit *cc, const char *key, const char *value);
bool         commit_has_footer(const ConventionalCommit *cc, const char *key);
const char*  commit_get_footer_value(const ConventionalCommit *cc, const char *key);
bool         commit_detect_breaking(const char *message);

/* Batch Operations */
int          commit_filter_by_type(const ConventionalCommit *commits, int count,
                                   CommitType type, ConventionalCommit *result, int max_results);
int          commit_filter_breaking(const ConventionalCommit *commits, int count,
                                    ConventionalCommit *result, int max_results);
bool         commit_batch_validate(const ConventionalCommit *commits, int count,
                                   int *first_invalid_index);

/* Print/Debug */
void         commit_print(const ConventionalCommit *cc);
void         commit_print_compact(const ConventionalCommit *cc);
void         semver_print(const Semver *v);

#endif
