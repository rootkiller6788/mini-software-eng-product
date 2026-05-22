#include "branching_models.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

void branch_init(Branch *b, const char *name, BranchKind kind,
                 const char *base_ref) {
    b->name = name ? strdup(name) : NULL;
    b->kind = kind;
    b->base_ref = base_ref ? strdup(base_ref) : NULL;
    b->is_protected = 0;
    b->require_review = 0;
    b->require_ci = 0;
}

void branch_free(Branch *b) {
    free(b->name);
    free(b->base_ref);
    b->name = NULL;
    b->base_ref = NULL;
}

/* ── Git Flow ───────────────────────────────────────────────────── */

void git_flow_init(GitFlow *gf) {
    memset(gf, 0, sizeof(*gf));
    branch_init(&gf->master, "master", BRANCH_MAIN, NULL);
    branch_init(&gf->develop, "develop", BRANCH_DEVELOP, gf->master.name);
    gf->master.is_protected = 1;
    gf->master.require_review = 1;
    gf->develop.is_protected = 1;
}

static FeatureBranch *find_feature(GitFlow *gf, const char *name,
                                    FeatureBranch **arr, size_t count) {
    size_t i;
    for (i = 0; i < count; i++)
        if (arr[i].branch.name && strcmp(arr[i].branch.name, name) == 0)
            return &arr[i];
    return NULL;
}

void git_flow_start_feature(GitFlow *gf, const char *name,
                             const char *ticket_id) {
    gf->features = (FeatureBranch *)realloc(gf->features,
                     (gf->feature_count + 1) * sizeof(FeatureBranch));
    FeatureBranch *fb = &gf->features[gf->feature_count++];
    memset(fb, 0, sizeof(*fb));
    branch_init(&fb->branch, name, BRANCH_FEATURE, gf->develop.name);
    fb->ticket_id = ticket_id ? strdup(ticket_id) : NULL;
    fb->summary = NULL;
    fb->created_at = time(NULL);
    fb->last_activity = fb->created_at;
    fb->merged = 0;
}

void git_flow_finish_feature(GitFlow *gf, const char *name) {
    FeatureBranch *fb = find_feature(gf, name, gf->features, gf->feature_count);
    if (!fb) return;
    fb->merged = 1;
    /* In real Git Flow: merge into develop, then delete branch */
    branch_free(&fb->branch);
}

void git_flow_start_release(GitFlow *gf, const char *version) {
    gf->releases = (FeatureBranch *)realloc(gf->releases,
                     (gf->release_count + 1) * sizeof(FeatureBranch));
    FeatureBranch *rb = &gf->releases[gf->release_count++];
    memset(rb, 0, sizeof(*rb));
    char name_buf[256];
    snprintf(name_buf, sizeof(name_buf), "release/%s", version);
    branch_init(&rb->branch, name_buf, BRANCH_RELEASE, gf->develop.name);
    rb->ticket_id = strdup(version);
    rb->created_at = time(NULL);
    rb->last_activity = rb->created_at;
}

void git_flow_finish_release(GitFlow *gf, const char *version) {
    char name_buf[256];
    snprintf(name_buf, sizeof(name_buf), "release/%s", version);
    FeatureBranch *rb = find_feature(gf, name_buf, gf->releases, gf->release_count);
    if (!rb) return;
    rb->merged = 1;
    /* Merge into master (tag it) and back into develop */
    branch_free(&rb->branch);
}

void git_flow_start_hotfix(GitFlow *gf, const char *version) {
    gf->hotfixes = (FeatureBranch *)realloc(gf->hotfixes,
                     (gf->hotfix_count + 1) * sizeof(FeatureBranch));
    FeatureBranch *hf = &gf->hotfixes[gf->hotfix_count++];
    memset(hf, 0, sizeof(*hf));
    char name_buf[256];
    snprintf(name_buf, sizeof(name_buf), "hotfix/%s", version);
    branch_init(&hf->branch, name_buf, BRANCH_HOTFIX, gf->master.name);
    hf->ticket_id = strdup(version);
    hf->created_at = time(NULL);
    hf->last_activity = hf->created_at;
}

void git_flow_finish_hotfix(GitFlow *gf, const char *version) {
    char name_buf[256];
    snprintf(name_buf, sizeof(name_buf), "hotfix/%s", version);
    FeatureBranch *hf = find_feature(gf, name_buf, gf->hotfixes, gf->hotfix_count);
    if (!hf) return;
    hf->merged = 1;
    /* Merge into both master and develop */
    branch_free(&hf->branch);
}

void git_flow_free(GitFlow *gf) {
    size_t i;
    branch_free(&gf->master);
    branch_free(&gf->develop);
    for (i = 0; i < gf->feature_count; i++) { free(gf->features[i].ticket_id); branch_free(&gf->features[i].branch); }
    for (i = 0; i < gf->release_count; i++) { free(gf->releases[i].ticket_id); branch_free(&gf->releases[i].branch); }
    for (i = 0; i < gf->hotfix_count; i++) { free(gf->hotfixes[i].ticket_id); branch_free(&gf->hotfixes[i].branch); }
    free(gf->features); free(gf->releases); free(gf->hotfixes);
    gf->features = NULL; gf->releases = NULL; gf->hotfixes = NULL;
    gf->feature_count = 0; gf->release_count = 0; gf->hotfix_count = 0;
}

/* ── GitHub Flow ────────────────────────────────────────────────── */

void github_flow_init(GitHubFlow *ghf) {
    memset(ghf, 0, sizeof(*ghf));
    branch_init(&ghf->main_branch, "main", BRANCH_MAIN, NULL);
    ghf->main_branch.is_protected = 1;
    ghf->require_pr_review = 1;
    ghf->require_status_checks = 1;
    ghf->require_up_to_date = 1;
}

void github_flow_create_feature(GitHubFlow *ghf, const char *name) {
    ghf->features = (FeatureBranch *)realloc(ghf->features,
                      (ghf->feature_count + 1) * sizeof(FeatureBranch));
    FeatureBranch *fb = &ghf->features[ghf->feature_count++];
    memset(fb, 0, sizeof(*fb));
    branch_init(&fb->branch, name, BRANCH_FEATURE, ghf->main_branch.name);
    fb->created_at = time(NULL);
    fb->last_activity = fb->created_at;
}

void github_flow_open_pr(GitHubFlow *ghf, const char *title,
                          const char *head, const char *base) {
    ghf->prs = (PullRequest *)realloc(ghf->prs,
                 (ghf->pr_count + 1) * sizeof(PullRequest));
    PullRequest *pr = &ghf->prs[ghf->pr_count++];
    memset(pr, 0, sizeof(*pr));
    pr->title = title ? strdup(title) : strdup("Untitled PR");
    pr->head_ref = head ? strdup(head) : NULL;
    pr->base_ref = base ? strdup(base) : strdup(ghf->main_branch.name);
    pr->status = PR_STATUS_OPEN;
    pr->merge_style = PR_MERGE_MERGE;
    pr->opened_at = time(NULL);
}

void github_flow_review_pr(GitHubFlow *ghf, size_t pr_index, int approve) {
    if (pr_index >= ghf->pr_count) return;
    PullRequest *pr = &ghf->prs[pr_index];
    if (approve) {
        pr->review_count++;
        if (pr->review_count >= 1 && pr->ci_passed)
            pr->status = PR_STATUS_APPROVED;
    } else {
        pr->status = PR_STATUS_REVIEW;
    }
}

void github_flow_merge_pr(GitHubFlow *ghf, size_t pr_index,
                           PullRequestMergeStyle style) {
    if (pr_index >= ghf->pr_count) return;
    if (ghf->prs[pr_index].status != PR_STATUS_APPROVED) return;
    ghf->prs[pr_index].status = PR_STATUS_MERGED;
    ghf->prs[pr_index].merge_style = style;
    ghf->prs[pr_index].merged_at = time(NULL);
}

void github_flow_free(GitHubFlow *ghf) {
    size_t i;
    branch_free(&ghf->main_branch);
    for (i = 0; i < ghf->feature_count; i++) { free(ghf->features[i].ticket_id); branch_free(&ghf->features[i].branch); }
    free(ghf->features);
    for (i = 0; i < ghf->pr_count; i++) {
        free(ghf->prs[i].title); free(ghf->prs[i].head_ref);
        free(ghf->prs[i].base_ref); free(ghf->prs[i].body);
        free(ghf->prs[i].reviewer);
    }
    free(ghf->prs);
    ghf->features = NULL; ghf->prs = NULL;
    ghf->feature_count = 0; ghf->pr_count = 0;
}

/* ── Trunk-Based Development ────────────────────────────────────── */

void tbd_init(TrunkBasedDev *tbd) {
    memset(tbd, 0, sizeof(*tbd));
    branch_init(&tbd->trunk, "main", BRANCH_MAIN, NULL);
    tbd->trunk.is_protected = 1;
    tbd->trunk.require_review = 1;
    tbd->trunk.require_ci = 1;
    tbd->max_branch_hours = 24;
    tbd->max_commits_per_branch = 10;
    tbd->release_from_trunk = 1;
}

void tbd_add_feature_flag(TrunkBasedDev *tbd, const char *key,
                           const char *desc, FeatureFlagType type) {
    tbd->flags = (FeatureFlag *)realloc(tbd->flags,
                   (tbd->flag_count + 1) * sizeof(FeatureFlag));
    FeatureFlag *ff = &tbd->flags[tbd->flag_count++];
    memset(ff, 0, sizeof(*ff));
    ff->key = strdup(key);
    ff->description = strdup(desc);
    ff->type = type;
    ff->enabled = 0;
    ff->rollout_pct = 0;
    ff->created_at = time(NULL);
    ff->is_stale = 0;
}

void tbd_toggle_flag(TrunkBasedDev *tbd, const char *key, int enabled) {
    size_t i;
    for (i = 0; i < tbd->flag_count; i++) {
        if (strcmp(tbd->flags[i].key, key) == 0) {
            tbd->flags[i].enabled = enabled;
            return;
        }
    }
}

void tbd_stale_cleanup(TrunkBasedDev *tbd) {
    size_t i, j = 0;
    for (i = 0; i < tbd->flag_count; i++) {
        if (tbd->flags[i].is_stale) {
            free(tbd->flags[i].key);
            free(tbd->flags[i].description);
        } else {
            if (i != j) tbd->flags[j] = tbd->flags[i];
            j++;
        }
    }
    tbd->flag_count = j;
}

void tbd_free(TrunkBasedDev *tbd) {
    size_t i;
    branch_free(&tbd->trunk);
    for (i = 0; i < tbd->flag_count; i++) {
        free(tbd->flags[i].key);
        free(tbd->flags[i].description);
    }
    free(tbd->flags);
    tbd->flags = NULL;
    tbd->flag_count = 0;
}
