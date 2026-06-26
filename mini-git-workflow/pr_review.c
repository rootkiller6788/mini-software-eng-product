#include "pr_review.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* ── Code owners ────────────────────────────────────────────────── */

void codeowners_load(const char *repo_root, CodeOwners *out) {
    size_t len = strlen(repo_root) + 32;
    char *path = (char *)malloc(len);
    snprintf(path, len, "%s/.github/CODEOWNERS", repo_root);
    FILE *f = fopen(path, "rb");
    free(path);
    out->entries = NULL;
    out->entry_count = 0;
    if (!f) return;

    char line[2048];
    while (fgets(line, (int)sizeof(line), f)) {
        if (line[0] == '#' || line[0] == '\n' || line[0] == '\r') continue;
        char *trim = line;
        while (*trim == ' ') trim++;
        char *nl = strchr(trim, '\n');
        if (nl) *nl = '\0';
        char *sp = trim;
        while (*sp && *sp != ' ' && *sp != '\t') sp++;
        if (!*sp) continue;
        *sp++ = '\0';
        while (*sp == ' ' || *sp == '\t') sp++;

        out->entries = (CodeOwnerEntry *)realloc(out->entries,
                         (out->entry_count + 1) * sizeof(CodeOwnerEntry));
        CodeOwnerEntry *e = &out->entries[out->entry_count++];
        e->path_pattern = strdup(trim);
        e->owners = strdup(sp);
        e->min_approvals = 1;
    }
    fclose(f);
}

int codeowners_find_required(const CodeOwners *co, const char *path,
                              CodeOwnerEntry **out, size_t *out_count) {
    size_t cap = 0, cnt = 0;
    *out = NULL;
    size_t i;
    for (i = 0; i < co->entry_count; i++) {
        const char *pat = co->entries[i].path_pattern;
        size_t plen = strlen(pat);
        if (plen == 0) continue;
        int match = 0;
        if (plen == 1 && pat[0] == '*') match = 1;
        else if (plen >= 2 && strncmp(path, pat, plen - 1) == 0) match = 1;
        else if (strstr(path, pat)) match = 1;

        if (match) {
            if (cnt >= cap) { cap = cap ? cap * 2 : 4; *out = (CodeOwnerEntry *)realloc(*out, cap * sizeof(CodeOwnerEntry)); }
            (*out)[cnt++] = co->entries[i];
        }
    }
    *out_count = cnt;
    return cnt > 0 ? 0 : -1;
}

size_t codeowners_count_approvals(const CodeOwners *co,
                                   const PullRequestFull *pr) {
    size_t i, j, count = 0;
    for (i = 0; i < pr->required_owner_count; i++) {
        const char *needed = pr->required_owners[i].owners;
        for (j = 0; j < pr->review_count; j++) {
            if (pr->reviews[j].decision == REVIEW_APPROVED &&
                pr->reviews[j].reviewer && needed &&
                strstr(needed, pr->reviews[j].reviewer)) {
                count++;
                break;
            }
        }
    }
    (void)co;
    return count;
}

void codeowners_free(CodeOwners *co) {
    size_t i;
    for (i = 0; i < co->entry_count; i++) {
        free(co->entries[i].path_pattern);
        free(co->entries[i].owners);
    }
    free(co->entries);
}

/* ── PR creation ────────────────────────────────────────────────── */

void prfull_init(PullRequestFull *pr,
                  const char *title, const char *body,
                  const char *head, const char *base,
                  const char *author) {
    memset(pr, 0, sizeof(*pr));
    pr->title = title ? strdup(title) : strdup("Untitled");
    pr->body = body ? strdup(body) : NULL;
    pr->head_branch = head ? strdup(head) : NULL;
    pr->base_branch = base ? strdup(base) : NULL;
    pr->author = author ? strdup(author) : NULL;
    pr->state = PR_STATE_OPEN;
    pr->is_draft = 0;
    pr->mergeable = 1;
    pr->can_be_rebased = 1;
    pr->require_ci_pass = 1;
    pr->require_approval_count = 1;
    pr->require_codeowner_approval = 1;
    pr->require_conversation_resolution = 1;
    pr->merge_method = PR_MERGE_SQUASH;
    pr->created_at = time(NULL);
    pr->updated_at = pr->created_at;
    pr->stale_after_days = 30;
    pr->last_activity = pr->created_at;
}

void prfull_set_template(PullRequestFull *pr, const PrTemplate *tmpl) {
    pr->template_name = tmpl ? tmpl->name : NULL;
}

void prfull_add_assignee(PullRequestFull *pr, const char *username) {
    pr->assignees = (char **)realloc(pr->assignees,
                      (pr->assignee_count + 1) * sizeof(char *));
    pr->assignees[pr->assignee_count++] = strdup(username);
}

void prfull_add_label(PullRequestFull *pr, const char *label) {
    pr->labels = (char **)realloc(pr->labels,
                   (pr->label_count + 1) * sizeof(char *));
    pr->labels[pr->label_count++] = strdup(label);
}

void prfull_free(PullRequestFull *pr) {
    size_t i;
    free(pr->title); free(pr->body); free(pr->head_branch);
    free(pr->base_branch); free(pr->author); free(pr->template_name);
    for (i = 0; i < pr->assignee_count; i++) free(pr->assignees[i]);
    free(pr->assignees);
    for (i = 0; i < pr->label_count; i++) free(pr->labels[i]);
    free(pr->labels);
    for (i = 0; i < pr->check_count; i++) free(pr->checks[i].name);
    free(pr->checks);
    for (i = 0; i < pr->review_count; i++) {
        free(pr->reviews[i].reviewer);
        free(pr->reviews[i].summary);
        size_t j;
        for (j = 0; j < pr->reviews[i].comment_count; j++) {
            free(pr->reviews[i].comments[j].file_path);
            free(pr->reviews[i].comments[j].body);
            free(pr->reviews[i].comments[j].author);
        }
        free(pr->reviews[i].comments);
    }
    free(pr->reviews);
}

/* ── CI ─────────────────────────────────────────────────────────── */

void ci_check_set(CiCheck *c, const char *name, CiStatus status) {
    c->name = strdup(name);
    c->status = status;
    c->completed_at = time(NULL);
}

int prfull_ci_all_passed(const PullRequestFull *pr) {
    size_t i;
    for (i = 0; i < pr->check_count; i++)
        if (pr->checks[i].status != CI_PASSED && pr->checks[i].status != CI_SKIPPED)
            return 0;
    return 1;
}

/* ── Reviews ────────────────────────────────────────────────────── */

void prfull_add_review(PullRequestFull *pr,
                        const char *reviewer,
                        ReviewDecision decision,
                        const char *summary) {
    pr->reviews = (PullRequestReview *)realloc(pr->reviews,
                    (pr->review_count + 1) * sizeof(PullRequestReview));
    PullRequestReview *rv = &pr->reviews[pr->review_count++];
    memset(rv, 0, sizeof(*rv));
    rv->reviewer = strdup(reviewer);
    rv->decision = decision;
    rv->summary = summary ? strdup(summary) : NULL;
    rv->submitted_at = time(NULL);
    pr->last_activity = rv->submitted_at;
    pr->updated_at = rv->submitted_at;
}

void review_add_comment(PullRequestReview *rv,
                         const char *file, size_t line,
                         const char *body, const char *author) {
    rv->comments = (ReviewComment *)realloc(rv->comments,
                     (rv->comment_count + 1) * sizeof(ReviewComment));
    ReviewComment *rc = &rv->comments[rv->comment_count++];
    memset(rc, 0, sizeof(*rc));
    rc->file_path = strdup(file);
    rc->line_start = line;
    rc->line_end = 0;
    rc->body = strdup(body);
    rc->author = strdup(author);
    rc->created_at = time(NULL);
    rc->resolved = 0;
    rc->is_suggestion = 0;
}

int prfull_approval_count(const PullRequestFull *pr) {
    size_t i;
    int count = 0;
    for (i = 0; i < pr->review_count; i++)
        if (pr->reviews[i].decision == REVIEW_APPROVED)
            count++;
    return count;
}

int prfull_is_approved(const PullRequestFull *pr, int min_approvals) {
    return prfull_approval_count(pr) >= min_approvals;
}

int prfull_conversations_resolved(const PullRequestFull *pr) {
    size_t i, j;
    for (i = 0; i < pr->review_count; i++)
        for (j = 0; j < pr->reviews[i].comment_count; j++)
            if (!pr->reviews[i].comments[j].resolved)
                return 0;
    return 1;
}

/* ── Merge checks ───────────────────────────────────────────────── */

int prfull_ready_to_merge(const PullRequestFull *pr) {
    if (pr->state != PR_STATE_OPEN) return 0;
    if (pr->is_draft) return 0;
    if (!pr->mergeable) return 0;
    if (pr->require_ci_pass && !prfull_ci_all_passed(pr)) return 0;
    if (pr->require_approval_count > 0 &&
        prfull_approval_count(pr) < (int)pr->require_approval_count) return 0;
    if (pr->require_codeowner_approval && pr->required_owner_count > 0) {
        if ((int)codeowners_count_approvals(NULL, pr) == 0) return 0;
    }
    if (pr->require_conversation_resolution && !prfull_conversations_resolved(pr)) return 0;
    return 1;
}

/* ── PR template ────────────────────────────────────────────────── */

void prtemplate_load(const char *tmpl_path, PrTemplate *out) {
    memset(out, 0, sizeof(*out));
    FILE *f = fopen(tmpl_path, "rb");
    if (!f) return;
    fseek(f, 0, SEEK_END);
    long sz = ftell(f);
    fseek(f, 0, SEEK_SET);
    out->content = (char *)malloc((size_t)sz + 1);
    fread(out->content, 1, (size_t)sz, f);
    out->content[sz] = '\0';
    fclose(f);
}

void prtemplate_render(const PrTemplate *tmpl,
                        const char **keys, const char **values,
                        size_t pair_count, char **out) {
    if (!tmpl || !tmpl->content) { *out = NULL; return; }
    char *buf = strdup(tmpl->content);
    size_t i;
    for (i = 0; i < pair_count; i++) {
        char placeholder[128];
        snprintf(placeholder, sizeof(placeholder), "{{%s}}", keys[i]);
        char *pos = strstr(buf, placeholder);
        if (pos) {
            size_t off = (size_t)(pos - buf);
            size_t plen = strlen(placeholder);
            size_t vlen = values[i] ? strlen(values[i]) : 0;
            size_t newlen = strlen(buf) - plen + vlen + 1;
            char *rep = (char *)malloc(newlen);
            memcpy(rep, buf, off);
            if (values[i]) memcpy(rep + off, values[i], vlen);
            memcpy(rep + off + vlen, buf + off + plen, strlen(buf + off + plen) + 1);
            free(buf);
            buf = rep;
        }
    }
    *out = buf;
}

void prtemplate_free(PrTemplate *tmpl) {
    free(tmpl->content);
    free(tmpl->name);
}

/* ── Merge queue ────────────────────────────────────────────────── */

void mergequeue_init(MergeQueue *mq, int batch_size) {
    memset(mq, 0, sizeof(*mq));
    mq->batch_size = batch_size > 0 ? batch_size : 5;
}

int mergequeue_enqueue(MergeQueue *mq, PullRequestFull *pr) {
    if (!prfull_ready_to_merge(pr)) return -1;
    mq->queue = (PullRequestFull **)realloc(mq->queue,
                  (mq->queue_size + 1) * sizeof(PullRequestFull *));
    mq->queue[mq->queue_size++] = pr;
    return 0;
}

int mergequeue_process(MergeQueue *mq) {
    int processed = 0;
    size_t i, j = 0;
    for (i = 0; i < mq->queue_size && processed < mq->batch_size; i++) {
        PullRequestFull *pr = mq->queue[i];
        if (prfull_ready_to_merge(pr)) {
            pr->state = PR_STATE_MERGED;
            pr->merged_at = time(NULL);
            processed++;
            continue;
        }
        if (i != j) mq->queue[j] = mq->queue[i];
        j++;
    }
    for (; i < mq->queue_size; i++) {
        if (i != j) mq->queue[j] = mq->queue[i];
        j++;
    }
    mq->queue_size = j;
    return processed;
}

void mergequeue_free(MergeQueue *mq) {
    free(mq->queue);
    mq->queue = NULL;
    mq->queue_size = 0;
}
