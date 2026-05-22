#include "conventional_commits.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

const char *commit_type_to_string(CommitType t) {
    switch (t) {
        case CC_FEAT: return "feat";
        case CC_FIX: return "fix";
        case CC_DOCS: return "docs";
        case CC_STYLE: return "style";
        case CC_REFACTOR: return "refactor";
        case CC_PERF: return "perf";
        case CC_TEST: return "test";
        case CC_CHORE: return "chore";
        case CC_CI: return "ci";
        case CC_BUILD: return "build";
        case CC_REVERT: return "revert";
        default: return "unknown";
    }
}

CommitType commit_type_from_string(const char *s) {
    if (!s) return CC_UNKNOWN;
    if (strcmp(s, "feat") == 0) return CC_FEAT;
    if (strcmp(s, "fix") == 0) return CC_FIX;
    if (strcmp(s, "docs") == 0) return CC_DOCS;
    if (strcmp(s, "style") == 0) return CC_STYLE;
    if (strcmp(s, "refactor") == 0) return CC_REFACTOR;
    if (strcmp(s, "perf") == 0) return CC_PERF;
    if (strcmp(s, "test") == 0) return CC_TEST;
    if (strcmp(s, "chore") == 0) return CC_CHORE;
    if (strcmp(s, "ci") == 0) return CC_CI;
    if (strcmp(s, "build") == 0) return CC_BUILD;
    if (strcmp(s, "revert") == 0) return CC_REVERT;
    return CC_UNKNOWN;
}

/* ── Parse ──────────────────────────────────────────────────────── */

static char *extract_first_line(const char *msg) {
    const char *nl = strchr(msg, '\n');
    if (nl) {
        size_t len = (size_t)(nl - msg);
        char *out = (char *)malloc(len + 1);
        memcpy(out, msg, len);
        out[len] = '\0';
        return out;
    }
    return strdup(msg);
}

ConventionalCommit cc_parse(const char *raw_message) {
    return cc_parse_strict(raw_message, NULL, NULL);
}

ConventionalCommit cc_parse_strict(const char *raw_message, LintError **errors, size_t *err_count) {
    ConventionalCommit cc;
    memset(&cc, 0, sizeof(cc));
    cc.type = CC_UNKNOWN;
    cc.scope[0] = '\0';

    if (!raw_message) {
        if (errors && err_count) {
            *errors = (LintError *)malloc(sizeof(LintError));
            *err_count = 1;
            (*errors)[0].code = LINT_ERR_TYPE_REQUIRED;
            (*errors)[0].detail = strdup("Empty commit message");
        }
        return cc;
    }

    if (strncmp(raw_message, "Merge ", 6) == 0) {
        cc.is_merge_commit = 1;
        cc.description = strdup(raw_message);
        return cc;
    }

    cc.is_breaking = cc_is_breaking(raw_message);

    const char *p = raw_message;
    while (*p && isspace((unsigned char)*p)) p++;
    if (!*p) return cc;

    char type_buf[32] = {0};
    int ti = 0;
    while (*p && *p != '(' && *p != ':' && *p != '!' && !isspace((unsigned char)*p)) {
        if (ti < 31) type_buf[ti++] = *p;
        p++;
    }
    type_buf[ti] = '\0';
    cc.type = commit_type_from_string(type_buf);

    if (*p == '(') {
        p++;
        int si = 0;
        while (*p && *p != ')' && si < CC_SCOPE_MAX - 1) { cc.scope[si++] = *p; p++; }
        cc.scope[si] = '\0';
        if (*p == ')') p++;
    }

    if (*p == '!') { cc.is_breaking = 1; p++; }

    if (*p == ':') p++;
    while (*p == ' ') p++;

    if (*p) {
        const char *nl = strchr(p, '\n');
        if (nl) {
            size_t len = (size_t)(nl - p);
            cc.description = (char *)malloc(len + 1);
            memcpy(cc.description, p, len);
            cc.description[len] = '\0';
        } else {
            cc.description = strdup(p);
        }
    }

    /* body */
    const char *body_start = strstr(raw_message, "\n\n");
    if (body_start) {
        cc.body = strdup(body_start + 2);
        cc.has_body = 1;
    } else {
        cc.body = NULL;
        cc.has_body = 0;
    }

    /* footers */
    if (cc.body) {
        const char *bp = cc.body;
        while (*bp) {
            while (*bp && *bp != '\n') bp++;
            if (*bp == '\n') bp++;
            while (*bp == '\n') bp++;
            const char *sep = strstr(bp, ": ");
            if (sep) {
                char *key = (char *)malloc((size_t)(sep - bp) + 1);
                memcpy(key, bp, (size_t)(sep - bp));
                key[(size_t)(sep - bp)] = '\0';
                sep += 2;
                const char *nl2 = strchr(sep, '\n');
                char *val;
                if (nl2) {
                    val = (char *)malloc((size_t)(nl2 - sep) + 1);
                    memcpy(val, sep, (size_t)(nl2 - sep));
                    val[(size_t)(nl2 - sep)] = '\0';
                    bp = nl2 + 1;
                } else {
                    val = strdup(sep);
                    bp = sep + strlen(sep);
                }
                cc.footer_keys = (char **)realloc(cc.footer_keys, (cc.footer_count + 1) * sizeof(char *));
                cc.footer_values = (char **)realloc(cc.footer_values, (cc.footer_count + 1) * sizeof(char *));
                cc.footer_keys[cc.footer_count] = key;
                cc.footer_values[cc.footer_count] = val;
                cc.footer_count++;
            } else {
                break;
            }
        }
    }

    if (errors) *errors = NULL;
    if (err_count) *err_count = 0;
    return cc;
}

void cc_free(ConventionalCommit *cc) {
    free(cc->description);
    free(cc->body);
    size_t i;
    for (i = 0; i < cc->footer_count; i++) { free(cc->footer_keys[i]); free(cc->footer_values[i]); }
    free(cc->footer_keys); free(cc->footer_values);
}

/* ── Lint ───────────────────────────────────────────────────────── */

static int is_empty_line(const char *s) {
    while (*s) { if (!isspace((unsigned char)*s)) return 0; s++; }
    return 1;
}

void cc_lint(const char *raw_message, const LintConfig *cfg,
             LintError **out_errors, size_t *out_count) {
    LintError *errs = NULL;
    size_t count = 0, cap = 0;
    if (!out_errors) out_errors = &errs;
    if (!out_count) out_count = &count;
    *out_errors = NULL;
    *out_count = 0;

    if (!raw_message || !*raw_message) {
        cap = 1; *out_errors = (LintError *)malloc(sizeof(LintError));
        (*out_errors)[0].code = LINT_ERR_TYPE_REQUIRED;
        (*out_errors)[0].detail = strdup("Empty commit message");
        *out_count = 1;
        return;
    }

    if (cfg && cfg->allow_merge_commits && strncmp(raw_message, "Merge ", 6) == 0) return;

    /* Subject line */
    const char *nl = strchr(raw_message, '\n');
    size_t subj_len = nl ? (size_t)(nl - raw_message) : strlen(raw_message);

    if (subj_len > (cfg ? cfg->subject_max_len : 72)) {
        if (count >= cap) { cap = cap ? cap * 2 : 4; *out_errors = (LintError *)realloc(*out_errors, cap * sizeof(LintError)); }
        (*out_errors)[count].code = LINT_ERR_SUBJECT_LONG;
        (*out_errors)[count].detail = strdup("Subject line exceeds max length");
        count++;
    }

    /* First char lowercase */
    if (subj_len > 0 && isupper((unsigned char)raw_message[0])) {
        if (count >= cap) { cap = cap ? cap * 2 : 4; *out_errors = (LintError *)realloc(*out_errors, cap * sizeof(LintError)); }
        (*out_errors)[count].code = LINT_ERR_SUBJECT_CASE;
        (*out_errors)[count].detail = strdup("Subject should start lowercase");
        count++;
    }

    /* No period at end */
    if (subj_len > 0 && raw_message[subj_len - 1] == '.') {
        if (count >= cap) { cap = cap ? cap * 2 : 4; *out_errors = (LintError *)realloc(*out_errors, cap * sizeof(LintError)); }
        (*out_errors)[count].code = LINT_ERR_SUBJECT_PERIOD;
        (*out_errors)[count].detail = strdup("Subject should not end with period");
        count++;
    }

    /* Blank line after subject */
    if (nl) {
        const char *after = nl + 1;
        if (*after != '\n') {
            if (count >= cap) { cap = cap ? cap * 2 : 4; *out_errors = (LintError *)realloc(*out_errors, cap * sizeof(LintError)); }
            (*out_errors)[count].code = LINT_ERR_BLANK_LINE;
            (*out_errors)[count].detail = strdup("Missing blank line after subject");
            count++;
        }
        /* Body line length */
        const char *bl = after;
        if (*bl == '\n') {
            bl++;
            int lnum = 1;
            while (*bl) {
                const char *bnl = strchr(bl, '\n');
                size_t blen = bnl ? (size_t)(bnl - bl) : strlen(bl);
                if (blen > (cfg ? cfg->body_max_len : 100)) {
                    if (count >= cap) { cap = cap ? cap * 2 : 4; *out_errors = (LintError *)realloc(*out_errors, cap * sizeof(LintError)); }
                    (*out_errors)[count].code = LINT_ERR_BODY_TOO_LONG;
                    char *s = (char *)malloc(64);
                    snprintf(s, 64, "Body line %d too long", lnum);
                    (*out_errors)[count].detail = s;
                    count++;
                }
                if (!bnl) break;
                bl = bnl + 1;
                lnum++;
            }
        }
    }

    *out_count = count;
}

int cc_lint_ok(LintErrorCode code) { return code == LINT_OK; }

void lint_errors_free(LintError *errors, size_t count) {
    size_t i;
    for (i = 0; i < count; i++) free(errors[i].detail);
    free(errors);
}

void lint_config_default(LintConfig *cfg) {
    memset(cfg, 0, sizeof(*cfg));
    cfg->subject_max_len = 72;
    cfg->body_max_len = 100;
    cfg->desc_min_len = 5;
    cfg->body_wrap = 0;
    cfg->allow_merge_commits = 1;
    cfg->require_scope = 0;
    cfg->allow_ci = 1;
    cfg->allow_custom_types = 0;
}

void lint_config_free(LintConfig *cfg) {
    free(cfg->allowed_types);
    free(cfg->scope_regex);
}

/* ── Version bump ───────────────────────────────────────────────── */

BumpLevel cc_determine_bump(const ConventionalCommit *commits, size_t count) {
    BumpLevel max = BUMP_NONE;
    size_t i;
    for (i = 0; i < count; i++) {
        if (commits[i].is_breaking) {
            if (max < BUMP_MAJOR) max = BUMP_MAJOR;
        } else if (commits[i].type == CC_FEAT) {
            if (max < BUMP_MINOR) max = BUMP_MINOR;
        } else if (commits[i].type == CC_FIX) {
            if (max < BUMP_PATCH) max = BUMP_PATCH;
        }
    }
    return max;
}

BumpLevel cc_bump_max(BumpLevel a, BumpLevel b) { return a > b ? a : b; }

/* ── SemVer ─────────────────────────────────────────────────────── */

SemVer semver_parse(const char *version_str) {
    SemVer sv;
    memset(&sv, 0, sizeof(sv));
    if (!version_str) return sv;
    if (version_str[0] == 'v' || version_str[0] == 'V') version_str++;
    sscanf(version_str, "%u.%u.%u", &sv.major, &sv.minor, &sv.patch);

    const char *dash = strchr(version_str, '-');
    const char *plus = strchr(version_str, '+');
    if (dash) {
        size_t len;
        if (plus && plus > dash) len = (size_t)(plus - dash - 1);
        else len = strlen(dash + 1);
        sv.pre_release = (char *)malloc(len + 1);
        memcpy(sv.pre_release, dash + 1, len);
        sv.pre_release[len] = '\0';
    }
    if (plus) {
        sv.build_meta = strdup(plus + 1);
    }
    return sv;
}

SemVer semver_bump(const SemVer *current, BumpLevel level) {
    SemVer sv = *current;
    free(sv.pre_release); sv.pre_release = NULL;
    free(sv.build_meta); sv.build_meta = NULL;
    switch (level) {
        case BUMP_MAJOR: sv.major++; sv.minor = 0; sv.patch = 0; break;
        case BUMP_MINOR: sv.minor++; sv.patch = 0; break;
        case BUMP_PATCH: sv.patch++; break;
        default: break;
    }
    return sv;
}

int semver_compare(const SemVer *a, const SemVer *b) {
    if (a->major != b->major) return (int)a->major - (int)b->major;
    if (a->minor != b->minor) return (int)a->minor - (int)b->minor;
    if (a->patch != b->patch) return (int)a->patch - (int)b->patch;
    return 0;
}

void semver_to_string(const SemVer *sv, char *buf, size_t buf_size) {
    if (sv->pre_release && sv->build_meta)
        snprintf(buf, buf_size, "%u.%u.%u-%s+%s", sv->major, sv->minor, sv->patch, sv->pre_release, sv->build_meta);
    else if (sv->pre_release)
        snprintf(buf, buf_size, "%u.%u.%u-%s", sv->major, sv->minor, sv->patch, sv->pre_release);
    else if (sv->build_meta)
        snprintf(buf, buf_size, "%u.%u.%u+%s", sv->major, sv->minor, sv->patch, sv->build_meta);
    else
        snprintf(buf, buf_size, "%u.%u.%u", sv->major, sv->minor, sv->patch);
}

void semver_free(SemVer *sv) {
    free(sv->pre_release); sv->pre_release = NULL;
    free(sv->build_meta); sv->build_meta = NULL;
}

/* ── Changelog ──────────────────────────────────────────────────── */

void changelog_init(Changelog *cl, const char *repo_url) {
    memset(cl, 0, sizeof(*cl));
    cl->repo_url = repo_url ? strdup(repo_url) : NULL;
    cl->keep_a_changelog = 1;
}

void changelog_add_entry(Changelog *cl, const SemVer *version,
                          const ConventionalCommit *commits, size_t count) {
    cl->entries = (ChangelogEntry *)realloc(cl->entries,
                   (cl->entry_count + 1) * sizeof(ChangelogEntry));
    ChangelogEntry *e = &cl->entries[cl->entry_count++];
    memset(e, 0, sizeof(*e));
    e->version = *version;
    e->released_at = time(NULL);

    size_t i;
    for (i = 0; i < count; i++) {
        if (commits[i].is_breaking) {
            e->breaking_lines = (char **)realloc(e->breaking_lines, (e->breaking_count + 1) * sizeof(char *));
            char buf[256];
            cc_generate_changelog_line(&commits[i], buf, sizeof(buf));
            e->breaking_lines[e->breaking_count++] = strdup(buf);
        } else if (commits[i].type == CC_FEAT) {
            e->feat_lines = (char **)realloc(e->feat_lines, (e->feat_count + 1) * sizeof(char *));
            char buf[256];
            cc_generate_changelog_line(&commits[i], buf, sizeof(buf));
            e->feat_lines[e->feat_count++] = strdup(buf);
        } else if (commits[i].type == CC_FIX) {
            e->fix_lines = (char **)realloc(e->fix_lines, (e->fix_count + 1) * sizeof(char *));
            char buf[256];
            cc_generate_changelog_line(&commits[i], buf, sizeof(buf));
            e->fix_lines[e->fix_count++] = strdup(buf);
        } else {
            e->other_lines = (char **)realloc(e->other_lines, (e->other_count + 1) * sizeof(char *));
            char buf[256];
            cc_generate_changelog_line(&commits[i], buf, sizeof(buf));
            e->other_lines[e->other_count++] = strdup(buf);
        }
    }
}

void changelog_render(const Changelog *cl, char **out) {
    size_t cap = 4096, len = 0;
    *out = (char *)malloc(cap);
    (*out)[0] = '\0';
    size_t i, j;
    for (i = 0; i < cl->entry_count; i++) {
        const ChangelogEntry *e = &cl->entries[i];
        char vbuf[64];
        semver_to_string(&e->version, vbuf, sizeof(vbuf));
        len += snprintf(*out + len, cap - len, "## %s\n\n", vbuf);

        if (e->breaking_count) {
            char *brk_label = "### BREAKING CHANGES\n\n";
            len += snprintf(*out + len, cap - len, "%s", brk_label);
            for (j = 0; j < e->breaking_count; j++)
                len += snprintf(*out + len, cap - len, "- %s\n", e->breaking_lines[j]);
            len += snprintf(*out + len, cap - len, "\n");
        }
        if (e->feat_count) {
            len += snprintf(*out + len, cap - len, "### Added\n\n");
            for (j = 0; j < e->feat_count; j++)
                len += snprintf(*out + len, cap - len, "- %s\n", e->feat_lines[j]);
            len += snprintf(*out + len, cap - len, "\n");
        }
        if (e->fix_count) {
            len += snprintf(*out + len, cap - len, "### Fixed\n\n");
            for (j = 0; j < e->fix_count; j++)
                len += snprintf(*out + len, cap - len, "- %s\n", e->fix_lines[j]);
            len += snprintf(*out + len, cap - len, "\n");
        }
    }
}

void changelog_render_markdown(const Changelog *cl, char **out) {
    changelog_render(cl, out);
}

void changelog_free(Changelog *cl) {
    size_t i, j;
    for (i = 0; i < cl->entry_count; i++) {
        ChangelogEntry *e = &cl->entries[i];
        for (j = 0; j < e->feat_count; j++) free(e->feat_lines[j]);
        for (j = 0; j < e->fix_count; j++) free(e->fix_lines[j]);
        for (j = 0; j < e->breaking_count; j++) free(e->breaking_lines[j]);
        for (j = 0; j < e->other_count; j++) free(e->other_lines[j]);
        free(e->feat_lines); free(e->fix_lines);
        free(e->breaking_lines); free(e->other_lines);
        semver_free(&e->version);
    }
    free(cl->entries);
    free(cl->repo_url);
}

/* ── Utility ────────────────────────────────────────────────────── */

int cc_is_breaking(const char *raw_message) {
    if (!raw_message) return 0;
    if (strstr(raw_message, "BREAKING CHANGE") || strstr(raw_message, "BREAKING-CHANGE"))
        return 1;
    return 0;
}

void cc_generate_changelog_line(const ConventionalCommit *cc, char *buf, size_t size) {
    if (cc->scope[0])
        snprintf(buf, size, "**%s(%s):** %s", commit_type_to_string(cc->type), cc->scope, cc->description ? cc->description : "");
    else
        snprintf(buf, size, "**%s:** %s", commit_type_to_string(cc->type), cc->description ? cc->description : "");
}
