#include "code_review.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

int cr_parse_diff(cr_review_t *review, const char *diff_text) {
    char line[CR_MAX_LINE_LEN];
    int li = 0, ci = 0, in_hunk = 0;
    int start_old = 0, count_old = 0, start_new = 0, count_new = 0;
    const char *p = diff_text;

    if (!review || !diff_text) return -1;
    review->file_count = 0;

    while (*p) {
        ci = 0;
        while (*p && *p != '\n' && *p != '\r' && ci < (int)sizeof(line) - 1)
            line[ci++] = *p++;
        line[ci] = '\0';
        if (*p == '\r') p++;
        if (*p == '\n') p++;

        if (strncmp(line, "diff --git", 10) == 0) {
            cr_diff_file_t *f = &review->files[review->file_count];
            memset(f, 0, sizeof(*f));
            char *a = strstr(line, " a/");
            char *b = strstr(line, " b/");
            if (a && b) {
                char *end_a = strchr(a + 3, ' ');
                if (end_a) *end_a = '\0';
                snprintf(f->path_old, CR_MAX_FILE_PATH, "%s", a + 3);
                if (end_a) *end_a = ' ';
                snprintf(f->path_new, CR_MAX_FILE_PATH, "%s", b + 3);
            }
            review->file_count++;
            in_hunk = 0;
        } else if (strncmp(line, "--- ", 4) == 0 || strncmp(line, "+++ ", 4) == 0) {
            continue;
        } else if (line[0] == '@' && line[1] == '@') {
            cr_diff_file_t *f = &review->files[review->file_count - 1];
            sscanf(line, "@@ -%d,%d +%d,%d @@",
                   &start_old, &count_old, &start_new, &count_new);
            cr_add_hunk(f, start_old, count_old, start_new, count_new,
                        CR_HUNK_MOD);
            in_hunk = 1;
        } else if (in_hunk && review->file_count > 0) {
            cr_diff_file_t *f = &review->files[review->file_count - 1];
            cr_hunk_t *h = &f->hunks[f->hunk_count - 1];
            if (h->line_count < CR_MAX_LINE_LEN) {
                if (line[0] == '+') { f->added_lines++; }
                else if (line[0] == '-') { f->deleted_lines++; }
            }
        }
        li++;
    }
    return review->file_count;
}

int cr_add_hunk(cr_diff_file_t *file, int start_old, int count_old,
                int start_new, int count_new, cr_hunk_type_e type) {
    if (!file || file->hunk_count >= CR_MAX_HUNKS) return -1;
    cr_hunk_t *h = &file->hunks[file->hunk_count];
    memset(h, 0, sizeof(*h));
    h->start_old = start_old;
    h->count_old = count_old;
    h->start_new = start_new;
    h->count_new = count_new;
    h->type = type;
    h->line_count = 0;
    snprintf(h->header, sizeof(h->header), "@@ -%d,%d +%d,%d @@",
             start_old, count_old, start_new, count_new);
    file->hunk_count++;
    return 0;
}

int cr_add_comment(cr_review_t *review, int line, const char *body,
                   cr_severity_e severity, const char *author) {
    if (!review || review->comment_count >= CR_MAX_COMMENTS) return -1;
    cr_comment_t *c = &review->comments[review->comment_count];
    memset(c, 0, sizeof(*c));
    c->id = review->comment_count;
    c->line_number = line;
    c->severity = severity;
    c->status = CR_COMMENT_OPEN;
    c->thread_id = c->id;
    c->parent_id = -1;
    snprintf(c->body, sizeof(c->body), "%s", body ? body : "");
    snprintf(c->author, sizeof(c->author), "%s", author ? author : "unknown");
    review->comment_count++;
    return c->id;
}

int cr_resolve_comment(cr_review_t *review, int comment_id,
                       const char *resolver) {
    if (!review || comment_id < 0 || comment_id >= review->comment_count)
        return -1;
    review->comments[comment_id].status = CR_COMMENT_RESOLVED;
    snprintf(review->comments[comment_id].resolved_by,
             sizeof(review->comments[0].resolved_by), "%s",
             resolver ? resolver : "");
    return 0;
}

int cr_reopen_comment(cr_review_t *review, int comment_id) {
    if (!review || comment_id < 0 || comment_id >= review->comment_count)
        return -1;
    review->comments[comment_id].status = CR_COMMENT_OPEN;
    return 0;
}

int cr_reply_to_comment(cr_review_t *review, int parent_id,
                        const char *body, const char *author) {
    if (!review || review->comment_count >= CR_MAX_COMMENTS) return -1;
    if (parent_id < 0 || parent_id >= review->comment_count) return -1;
    int id = cr_add_comment(review, review->comments[parent_id].line_number,
                            body, review->comments[parent_id].severity, author);
    if (id < 0) return -1;
    review->comments[id].thread_id = review->comments[parent_id].thread_id;
    review->comments[id].parent_id = parent_id;
    return id;
}

void cr_load_ownership(cr_ownership_t *ownerships, int count,
                       const char *csv_path) {
    (void)csv_path;
    (void)ownerships;
    (void)count;
}

int cr_suggest_reviewers(cr_review_t *review,
                         const cr_ownership_t *ownerships,
                         int ownership_count) {
    if (!review || !ownerships) return 0;
    int added = 0;
    for (int i = 0; i < review->file_count && added < CR_MAX_REVIEWERS; i++) {
        for (int j = 0; j < ownership_count && added < CR_MAX_REVIEWERS; j++) {
            const char *pat = ownerships[j].path_pattern;
            if (strstr(review->files[i].path_new, pat) ||
                strstr(review->files[i].path_old, pat)) {
                for (int k = 0; k < ownerships[j].owner_count &&
                     added < CR_MAX_REVIEWERS; k++) {
                    int exists = 0;
                    for (int r = 0; r < review->reviewer_count; r++)
                        if (strcmp(review->reviewers[r],
                                   ownerships[j].owners[k]) == 0) {
                            exists = 1;
                            break;
                        }
                    if (!exists) {
                        cr_assign_reviewer(review, ownerships[j].owners[k]);
                        added++;
                    }
                }
            }
        }
    }
    return added;
}

int cr_init_checklist(cr_checklist_t *cl) {
    if (!cl) return -1;
    cl->count = 0;
    return 0;
}

int cr_add_checklist_item(cr_checklist_t *cl, const char *item) {
    if (!cl || !item || cl->count >= CR_MAX_CHECKLIST) return -1;
    cl->items[cl->count] = strdup(item);
    cl->checked[cl->count] = 0;
    cl->count++;
    return 0;
}

int cr_check_item(cr_checklist_t *cl, int index) {
    if (!cl || index < 0 || index >= cl->count) return -1;
    cl->checked[index] = 1;
    return 0;
}

int cr_all_checked(const cr_checklist_t *cl) {
    if (!cl || cl->count == 0) return 1;
    for (int i = 0; i < cl->count; i++)
        if (!cl->checked[i]) return 0;
    return 1;
}

void cr_set_approval_rules(cr_approval_rules_t *rules,
                           int min_approvals, int require_owner) {
    if (!rules) return;
    memset(rules, 0, sizeof(*rules));
    rules->min_approvals = min_approvals;
    rules->require_owner_approval = require_owner;
    rules->auto_resolve_threads = 1;
    rules->stale_days = 14;
}

int cr_check_approval(const cr_review_t *review) {
    if (!review) return 0;
    if (review->reviewer_count < review->rules.min_approvals) return 0;
    return cr_all_checked(&review->checklist);
}

int cr_is_approved(const cr_review_t *review) {
    if (!review) return 0;
    return review->status == CR_REVIEW_APPROVED;
}

void cr_assign_reviewer(cr_review_t *review, const char *reviewer) {
    if (!review || !reviewer || review->reviewer_count >= CR_MAX_REVIEWERS)
        return;
    snprintf(review->reviewers[review->reviewer_count],
             sizeof(review->reviewers[0]), "%s", reviewer);
    review->reviewer_count++;
}

void cr_compute_stats(cr_review_t *review) {
    if (!review) return;
    cr_stats_t *s = &review->stats;
    memset(s, 0, sizeof(*s));
    s->total_reviews = 1;
    s->comments_total = review->comment_count;
    int resolved = 0;
    for (int i = 0; i < review->comment_count; i++)
        if (review->comments[i].status == CR_COMMENT_RESOLVED) resolved++;
    s->comments_resolved = resolved;
    s->avg_comments_per_review = (double)s->comments_total;
    s->avg_time_to_resolve_hours = 2.5;
}

void cr_print_stats(const cr_stats_t *stats) {
    if (!stats) return;
    printf("=== Review Statistics ===\n");
    printf("  Reviews:        %d\n", stats->total_reviews);
    printf("  Approved:       %d\n", stats->approved);
    printf("  Rejected:       %d\n", stats->rejected);
    printf("  Total Comments: %d\n", stats->comments_total);
    printf("  Resolved:       %d\n", stats->comments_resolved);
    printf("  Avg Time/Review: %.1fh\n", stats->avg_time_to_resolve_hours);
    printf("  Avg Comments:    %.1f\n", stats->avg_comments_per_review);
}
