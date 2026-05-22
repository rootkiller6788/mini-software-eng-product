#include "merge_rebase.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

void merge_commit_free(MergeCommit *mc) {
    free(mc->parents);
    free(mc->message);
}

/* ── Find merge base ────────────────────────────────────────────── */

static int oid_equal(const MergeOid *a, const MergeOid *b) {
    return memcmp(a->id, b->id, MERGE_SHA1_RAWSZ) == 0;
}

MergeBase merge_find_base(const MergeCommit *ours,
                           const MergeCommit *theirs) {
    MergeBase mb;
    memset(&mb, 0, sizeof(mb));
    (void)ours; (void)theirs;
    /* Simplified: return ours as base if no history */
    return mb;
}

/* ── Diff/merge line-based ──────────────────────────────────────── */

typedef struct { char *line; size_t len; } Line;

static Line *split_lines(const char *text, size_t *out_count) {
    if (!text) { *out_count = 0; return NULL; }
    Line *lines = NULL;
    size_t count = 0, cap = 0;
    const char *p = text;
    while (*p) {
        const char *start = p;
        while (*p && *p != '\n') p++;
        size_t len = (size_t)(p - start);
        if (count >= cap) { cap = cap ? cap * 2 : 32; lines = (Line *)realloc(lines, cap * sizeof(Line)); }
        lines[count].line = (char *)malloc(len + 1);
        memcpy(lines[count].line, start, len);
        lines[count].line[len] = '\0';
        lines[count].len = len;
        count++;
        if (*p == '\n') p++;
    }
    *out_count = count;
    return lines;
}

static void free_lines(Line *lines, size_t count) {
    size_t i;
    for (i = 0; i < count; i++) free(lines[i].line);
    free(lines);
}

static char *join_lines(Line *lines, size_t count) {
    size_t i, total = 0;
    for (i = 0; i < count; i++) total += lines[i].len + 1;
    char *out = (char *)malloc(total + 1);
    char *d = out;
    for (i = 0; i < count; i++) {
        memcpy(d, lines[i].line, lines[i].len);
        d += lines[i].len;
        *d++ = '\n';
    }
    *d = '\0';
    return out;
}

/* ── Three-way merge ────────────────────────────────────────────── */

static int lines_equal(Line *a, Line *b) {
    return a->len == b->len && strcmp(a->line, b->line) == 0;
}

MergeResult merge_three_way(const MergeCommit *base,
                             const MergeCommit *ours,
                             const MergeCommit *theirs,
                             const char *ours_content,
                             const char *theirs_content) {
    MergeResult mr;
    memset(&mr, 0, sizeof(mr));
    (void)base; (void)ours; (void)theirs;

    if (!ours_content && !theirs_content) return mr;
    if (!ours_content) { mr.merged_content = strdup(theirs_content); return mr; }
    if (!theirs_content) { mr.merged_content = strdup(ours_content); return mr; }

    size_t oc, tc;
    Line *ol = split_lines(ours_content, &oc);
    Line *tl = split_lines(theirs_content, &tc);

    /* Simple: if identical, use ours */
    if (oc == tc) {
        int same = 1;
        size_t i;
        for (i = 0; i < oc && same; i++) if (!lines_equal(&ol[i], &tl[i])) same = 0;
        if (same) {
            mr.merged_content = join_lines(ol, oc);
            mr.conflict_count = 0;
            free_lines(ol, oc); free_lines(tl, tc);
            return mr;
        }
    }

    /* Otherwise produce conflict markers */
    size_t maxc = oc + tc + 4;
    Line *result = (Line *)malloc(maxc * sizeof(Line));
    size_t rc = 0;
    result[rc++] = (Line){ strdup("<<<<<<< OURS"), 14 };

    size_t i;
    for (i = 0; i < oc; i++) {
        result[rc].line = strdup(ol[i].line);
        result[rc].len = ol[i].len;
        rc++;
    }
    result[rc++] = (Line){ strdup("======="), 7 };
    for (i = 0; i < tc; i++) {
        result[rc].line = strdup(tl[i].line);
        result[rc].len = tl[i].len;
        rc++;
    }
    result[rc++] = (Line){ strdup(">>>>>>> THEIRS"), 14 };

    mr.merged_content = join_lines(result, rc);
    mr.conflict_count = 1;
    mr.conflicts = (MergeConflict *)malloc(sizeof(MergeConflict));
    mr.conflicts[0].ours_start = 1;
    mr.conflicts[0].ours_end = oc;
    mr.conflicts[0].theirs_start = oc + 2;
    mr.conflicts[0].theirs_end = oc + 2 + tc;
    mr.conflicts[0].resolved = 0;
    mr.conflicts[0].resolved_text = NULL;

    free_lines(ol, oc); free_lines(tl, tc);
    size_t j;
    for (j = 0; j < rc; j++) free(result[j].line);
    free(result);
    return mr;
}

int merge_is_fast_forward(const MergeCommit *ours,
                           const MergeCommit *theirs,
                           int *ours_is_ancestor,
                           int *theirs_is_ancestor) {
    if (ours_is_ancestor) *ours_is_ancestor = 1;
    if (theirs_is_ancestor) *theirs_is_ancestor = 0;
    (void)ours; (void)theirs;
    return 1;
}

MergeResult merge_octopus(const MergeCommit **branches, size_t count) {
    (void)branches; (void)count;
    MergeResult mr; memset(&mr, 0, sizeof(mr));
    return mr;
}

void merge_result_free(MergeResult *mr) {
    if (!mr) return;
    free(mr->merged_content);
    size_t i;
    for (i = 0; i < mr->conflict_count; i++) free(mr->conflicts[i].resolved_text);
    free(mr->conflicts);
}

/* ── Rebase ─────────────────────────────────────────────────────── */

void rebase_plan_build(RebasePlan *plan,
                       const MergeCommit *commits, size_t count,
                       const MergeOid *onto,
                       const MergeOid *upstream) {
    plan->commits = (MergeCommit *)malloc(count * sizeof(MergeCommit));
    plan->commit_count = count;
    memcpy(plan->commits, commits, count * sizeof(MergeCommit));
    if (onto) memcpy(&plan->onto_oid, onto, sizeof(MergeOid));
    if (upstream) memcpy(&plan->upstream_oid, upstream, sizeof(MergeOid));
    plan->preserve_merges = 0;
}

MergeCommit rebase_apply(const RebasePlan *plan,
                         int (*conflict_handler)(MergeConflict *, void *),
                         void *userdata) {
    MergeCommit result; memset(&result, 0, sizeof(result));
    if (plan->commit_count > 0) {
        result = plan->commits[plan->commit_count - 1];
        result.parent_count = 1;
        result.parents = (MergeOid *)malloc(sizeof(MergeOid));
        memcpy(result.parents, &plan->onto_oid, sizeof(MergeOid));
    }
    (void)conflict_handler; (void)userdata;
    return result;
}

void rebase_plan_free(RebasePlan *plan) {
    size_t i;
    for (i = 0; i < plan->commit_count; i++) {
        free(plan->commits[i].parents);
        free(plan->commits[i].message);
    }
    free(plan->commits);
}

/* ── Interactive rebase ─────────────────────────────────────────── */

void interactive_rebase_init(InteractiveRebase *ir,
                              const MergeCommit *commits,
                              size_t count,
                              const MergeOid *onto) {
    ir->steps = (RebaseStep *)malloc(count * sizeof(RebaseStep));
    ir->step_count = count;
    ir->current_step = 0;
    ir->in_progress = 0;
    memcpy(&ir->onto, onto, sizeof(MergeOid));
    size_t i;
    for (i = 0; i < count; i++) {
        ir->steps[i].commit = commits[i];
        ir->steps[i].command = REBASE_PICK;
        ir->steps[i].new_message = NULL;
        ir->steps[i].exec_cmd = NULL;
    }
}

void interactive_rebase_set_command(InteractiveRebase *ir,
                                     size_t idx,
                                     RebaseCommand cmd,
                                     const char *extra) {
    if (idx >= ir->step_count) return;
    ir->steps[idx].command = cmd;
    if (cmd == REBASE_REWORD || cmd == REBASE_SQUASH) {
        free(ir->steps[idx].new_message);
        ir->steps[idx].new_message = extra ? strdup(extra) : NULL;
    }
    if (cmd == REBASE_EXEC) {
        free(ir->steps[idx].exec_cmd);
        ir->steps[idx].exec_cmd = extra ? strdup(extra) : NULL;
    }
}

int interactive_rebase_execute(InteractiveRebase *ir) {
    ir->in_progress = 1;
    for (ir->current_step = 0; ir->current_step < ir->step_count; ir->current_step++) {
        RebaseStep *step = &ir->steps[ir->current_step];
        if (step->command == REBASE_DROP) continue;
        if (step->command == REBASE_PICK) continue;
        if (step->command == REBASE_BREAK) break;
    }
    ir->in_progress = 0;
    return 0;
}

void interactive_rebase_free(InteractiveRebase *ir) {
    size_t i;
    for (i = 0; i < ir->step_count; i++) {
        free(ir->steps[i].new_message);
        free(ir->steps[i].exec_cmd);
        free(ir->steps[i].commit.message);
        free(ir->steps[i].commit.parents);
    }
    free(ir->steps);
}

/* ── Cherry-pick ────────────────────────────────────────────────── */

int cherry_pick_apply(const CherryPick *cp,
                       const char *ours_content,
                       char **out) {
    (void)cp;
    *out = ours_content ? strdup(ours_content) : NULL;
    return 0;
}

/* ── Rerere ─────────────────────────────────────────────────────── */

void rerere_cache_init(RerereCache *rc, const char *git_dir) {
    rc->records = NULL;
    rc->record_count = 0;
    size_t len = strlen(git_dir) + 16;
    rc->rerere_dir = (char *)malloc(len);
    snprintf(rc->rerere_dir, len, "%s/rr-cache", git_dir);
}

int rerere_record(RerereCache *rc, const char *hash,
                  const char *patch, size_t len) {
    rc->records = (RerereRecord *)realloc(rc->records,
                    (rc->record_count + 1) * sizeof(RerereRecord));
    RerereRecord *rr = &rc->records[rc->record_count++];
    rr->conflict_hash = strdup(hash);
    rr->patch = (char *)malloc(len + 1);
    memcpy(rr->patch, patch, len);
    rr->patch[len] = '\0';
    rr->patch_size = len;
    rr->recorded_at = time(NULL);
    return 0;
}

int rerere_reuse(RerereCache *rc, const char *hash,
                 char **patch, size_t *len) {
    size_t i;
    for (i = 0; i < rc->record_count; i++) {
        if (strcmp(rc->records[i].conflict_hash, hash) == 0) {
            *patch = (char *)malloc(rc->records[i].patch_size + 1);
            memcpy(*patch, rc->records[i].patch, rc->records[i].patch_size + 1);
            *len = rc->records[i].patch_size;
            return 0;
        }
    }
    return -1;
}

void rerere_cache_free(RerereCache *rc) {
    size_t i;
    for (i = 0; i < rc->record_count; i++) {
        free(rc->records[i].conflict_hash);
        free(rc->records[i].patch);
    }
    free(rc->records);
    free(rc->rerere_dir);
}

/* ── Squash merge ───────────────────────────────────────────────── */

int squash_merge_apply(const SquashMerge *sm,
                        const MergeCommit *source_commits,
                        size_t count,
                        const char *target_content,
                        char **out) {
    (void)sm; (void)source_commits; (void)count;
    *out = target_content ? strdup(target_content) : NULL;
    return 0;
}
