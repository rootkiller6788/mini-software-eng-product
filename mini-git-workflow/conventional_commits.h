#ifndef CONVENTIONAL_COMMITS_H
#define CONVENTIONAL_COMMITS_H

#include <stddef.h>
#include <time.h>

/* ── Conventional commit types ──────────────────────────────────── */

typedef enum {
    CC_FEAT,       /* feat     – new feature */
    CC_FIX,        /* fix      – bug fix */
    CC_DOCS,       /* docs     – documentation */
    CC_STYLE,      /* style    – formatting, missing semi colons, etc */
    CC_REFACTOR,   /* refactor – code change that neither fixes nor adds */
    CC_PERF,       /* perf     – performance improvement */
    CC_TEST,       /* test     – adding/updating tests */
    CC_CHORE,      /* chore    – grunt work, build config, etc */
    CC_CI,         /* ci       – CI/CD changes */
    CC_BUILD,      /* build    – build system / external deps */
    CC_REVERT,     /* revert   – revert a previous commit */
    CC_UNKNOWN
} CommitType;

const char *commit_type_to_string(CommitType t);
CommitType  commit_type_from_string(const char *s);

/* ── Scope ──────────────────────────────────────────────────────── */

#define CC_SCOPE_MAX 64

/* ── Parsed conventional commit ──────────────────────────────────── */

typedef struct {
    CommitType type;
    char       scope[CC_SCOPE_MAX];  /* empty string if no scope */
    char      *description;

    int        is_breaking;          /* ! after type/scope or BREAKING CHANGE footer */

    /* Footer tokens (key: value pairs) */
    char     **footer_keys;
    char     **footer_values;
    size_t     footer_count;

    /* Raw message for reference */
    char      *body;                 /* multi-line body after subject */
    int        has_body;

    /* Metadata */
    int        is_merge_commit;      /* skip linting */
    int        is_revert;            /* revert commits get relaxed rules */
} ConventionalCommit;

/* ── Semantic version ───────────────────────────────────────────── */

typedef struct {
    unsigned int major;
    unsigned int minor;
    unsigned int patch;

    /* Pre-release: alpha, beta, rc.1, etc. */
    char *pre_release;
    /* Build metadata: sha.1234abc, exp.sha.5114f85 */
    char *build_meta;
} SemVer;

/* ── Version bump rules ──────────────────────────────────────────── */
/*   fix → PATCH bump                                               */
/*   feat → MINOR bump, reset PATCH to 0                             */
/*   feat + BREAKING CHANGE → MAJOR bump, reset MINOR & PATCH        */

typedef enum {
    BUMP_NONE,
    BUMP_PATCH,
    BUMP_MINOR,
    BUMP_MAJOR,
    BUMP_PRERELEASE
} BumpLevel;

/* ── Changelog entry ────────────────────────────────────────────── */

typedef struct {
    SemVer       version;
    time_t       released_at;

    char       **feat_lines;     /* Added / Changed / Deprecated */
    size_t       feat_count;
    char       **fix_lines;      /* Fixed / Security */
    size_t       fix_count;
    char       **breaking_lines; /* BREAKING CHANGES */
    size_t       breaking_count;
    char       **other_lines;    /* docs, style, refactor, perf, test, chore, ci, build */
    size_t       other_count;
} ChangelogEntry;

typedef struct {
    ChangelogEntry *entries;      /* newest first */
    size_t          entry_count;
    char           *repo_url;     /* for compare links */
    int             keep_a_changelog; /* 1 = follow keepachangelog.com */
} Changelog;

/* ── Commit lint rule ───────────────────────────────────────────── */

typedef enum {
    LINT_ERR_TYPE_REQUIRED,       /* type must be present */
    LINT_ERR_TYPE_UNKNOWN,        /* type not in standard set */
    LINT_ERR_SCOPE_EMPTY,         /* scope is empty parenthesis () */
    LINT_ERR_SCOPE_TOO_LONG,      /* scope exceeds CC_SCOPE_MAX */
    LINT_ERR_DESC_MISSING,        /* no description after colon */
    LINT_ERR_DESC_TOO_SHORT,      /* description < min chars */
    LINT_ERR_DESC_TOO_LONG,       /* description > max chars (72 headers) */
    LINT_ERR_SUBJECT_LONG,        /* subject line > 72 chars (or user-defined) */
    LINT_ERR_SUBJECT_CASE,        /* first char should be lowercase */
    LINT_ERR_SUBJECT_PERIOD,      /* no trailing period */
    LINT_ERR_BLANK_LINE,          /* missing blank line after subject */
    LINT_ERR_BODY_TOO_LONG,       /* body line > 100 chars */
    LINT_OK = 0
} LintErrorCode;

typedef struct {
    LintErrorCode code;
    size_t        line;           /* 1-indexed line in raw message */
    size_t        column;
    char         *detail;         /* human-readable explanation */
} LintError;

/* ── Lint config ────────────────────────────────────────────────── */

typedef struct {
    size_t  subject_max_len;     /* default 72 */
    size_t  body_max_len;        /* default 100 */
    size_t  desc_min_len;        /* default 5 */
    size_t  body_wrap;           /* 0 = no wrap */
    int     allow_merge_commits; /* skip merge commits */
    int     require_scope;       /* enforce scope presence */
    int     allow_ci;            /* allow 'ci' type */
    int     allow_custom_types;  /* accept unknown types */
    CommitType *allowed_types;
    size_t      allowed_count;
    char       *scope_regex;     /* validate scope pattern, e.g. "^(core|ui|api)$" */
} LintConfig;

/* ── API ────────────────────────────────────────────────────────── */

/* Parse */
ConventionalCommit cc_parse(const char *raw_message);
ConventionalCommit cc_parse_strict(const char *raw_message, LintError **errors, size_t *err_count);
void               cc_free(ConventionalCommit *cc);

/* Lint */
void      cc_lint(const char *raw_message, const LintConfig *cfg,
                  LintError **out_errors, size_t *out_count);
int       cc_lint_ok(LintErrorCode code);
void      lint_errors_free(LintError *errors, size_t count);
void      lint_config_default(LintConfig *cfg);
void      lint_config_free(LintConfig *cfg);

/* Version bump */
BumpLevel cc_determine_bump(const ConventionalCommit *commits, size_t count);
BumpLevel cc_bump_max(BumpLevel a, BumpLevel b);

/* SemVer */
SemVer    semver_parse(const char *version_str);
SemVer    semver_bump(const SemVer *current, BumpLevel level);
int       semver_compare(const SemVer *a, const SemVer *b);
void      semver_to_string(const SemVer *sv, char *buf, size_t buf_size);
void      semver_free(SemVer *sv);

/* Changelog */
void      changelog_init(Changelog *cl, const char *repo_url);
void      changelog_add_entry(Changelog *cl, const SemVer *version,
                               const ConventionalCommit *commits, size_t count);
void      changelog_render(const Changelog *cl, char **out);
void      changelog_render_markdown(const Changelog *cl, char **out);
void      changelog_free(Changelog *cl);

/* Utility */
int       cc_is_breaking(const char *raw_message);
void      cc_generate_changelog_line(const ConventionalCommit *cc, char *buf, size_t size);

#endif /* CONVENTIONAL_COMMITS_H */
