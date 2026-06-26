#include "merge_rebase.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Example: rebase & interactive rebase workflow */

static MergeCommit make_commit(const char *id, const char *msg) {
    MergeCommit c;
    memset(&c, 0, sizeof(c));
    c.parents = (MergeOid *)malloc(sizeof(MergeOid));
    c.parent_count = 1;
    memset(c.parents[0].id, 0, MERGE_SHA1_RAWSZ);
    /* Simple: use first byte of id string as id */
    if (id) c.oid.id[0] = (unsigned char)id[0];
    c.message = msg ? strdup(msg) : strdup("commit");
    c.authored_at = time(NULL);
    return c;
}

static void print_oid(const MergeOid *oid) {
    int i;
    for (i = 0; i < 5; i++) printf("%02x", oid->id[i]);
}

int main(void) {
    printf("=== Rebase & Interactive Rebase Example (C99) ===\n\n");

    /* ── 1. Setup branches ──────────────────────────────────────── */
    MergeCommit ours = make_commit("A", "main: initial setup");
    MergeCommit theirs = make_commit("B", "feature: awesome widget");
    printf("1. Main branch tip: "); print_oid(&ours.oid); printf("\n");
    printf("   Feature branch tip: "); print_oid(&theirs.oid); printf("\n");

    /* ── 2. Fast-forward check ──────────────────────────────────── */
    int ours_anc, theirs_anc;
    merge_is_fast_forward(&ours, &theirs, &ours_anc, &theirs_anc);
    printf("2. Fast-forward check:\n");
    printf("   Ours is ancestor: %s\n", ours_anc ? "yes" : "no");
    printf("   Theirs is ancestor: %s\n", theirs_anc ? "yes" : "no");

    /* ── 3. Three-way merge ─────────────────────────────────────── */
    MergeBase mb = merge_find_base(&ours, &theirs);

    const char *ours_content = "line 1: original\n"
                               "line 2: modified by main\n"
                               "line 3: common\n";
    const char *theirs_content = "line 1: original\n"
                                  "line 2: modified by feature\n"
                                  "line 3: common\n";

    MergeResult mr = merge_three_way(&mb.base, &mb.ours, &mb.theirs,
                                      ours_content, theirs_content);
    printf("\n3. Three-way merge:\n");
    printf("   Conflicts: %zu\n", mr.conflict_count);
    if (mr.merged_content) {
        printf("   Result:\n%s", mr.merged_content);
    }

    /* ── 4. Build rebase plan ───────────────────────────────────── */
    MergeCommit history[3] = {
        make_commit("C", "feat: add search bar"),
        make_commit("D", "feat: add dark mode toggle"),
        make_commit("E", "fix: correct button alignment")
    };

    MergeOid onto_oid;
    memset(&onto_oid, 0, sizeof(onto_oid));
    RerereCache rr;
    rerere_cache_init(&rr, ".git");

    RebasePlan plan;
    rebase_plan_build(&plan, history, 3, &onto_oid, NULL);
    printf("\n4. Rebase plan: %zu commits to replay onto ", plan.commit_count);
    print_oid(&plan.onto_oid);
    printf("\n");
    size_t i;
    for (i = 0; i < plan.commit_count; i++)
        printf("   [%zu] %s\n", i + 1, plan.commits[i].message);

    /* ── 5. Interactive rebase ──────────────────────────────────── */
    InteractiveRebase ir;
    interactive_rebase_init(&ir, history, 3, &onto_oid);

    /* Change commands: pick → pick, pick → squash, pick → reword */
    interactive_rebase_set_command(&ir, 0, REBASE_PICK, NULL);
    interactive_rebase_set_command(&ir, 1, REBASE_SQUASH, "feat: improved theme support");
    interactive_rebase_set_command(&ir, 2, REBASE_REWORD, "fix: button alignment in header");

    printf("\n5. Interactive rebase plan:\n");
    for (i = 0; i < ir.step_count; i++) {
        const char *cmd;
        switch (ir.steps[i].command) {
            case REBASE_PICK: cmd = "pick"; break;
            case REBASE_REWORD: cmd = "reword"; break;
            case REBASE_SQUASH: cmd = "squash"; break;
            case REBASE_FIXUP: cmd = "fixup"; break;
            case REBASE_DROP: cmd = "drop"; break;
            case REBASE_BREAK: cmd = "break"; break;
            default: cmd = "?"; break;
        }
        printf("   %s %s", cmd, ir.steps[i].commit.message);
        if (ir.steps[i].new_message) printf(" [new: %s]", ir.steps[i].new_message);
        printf("\n");
    }

    /* ── 6. Cherry-pick ─────────────────────────────────────────── */
    printf("\n6. Cherry-pick example:\n");
    CherryPick cp;
    cp.source_commit = make_commit("F", "fix: critical hotfix from release branch");
    cp.allow_empty = 0;
    cp.record_origin = 1;
    char *cp_out = NULL;
    cherry_pick_apply(&cp, ours_content, &cp_out);
    printf("   Applied cherry-pick: %s\n", cp_out ? "success" : "no output");

    /* ── 7. Rerere: record a resolution ──────────────────────────── */
    printf("\n7. Rerere: reuse recorded resolution\n");
    const char *conflict_hash_str = "abc123def456abc123def456abc123def456abc123";
    const char *resolved_patch = "---\n+++\n@@ -1,3 +1,4 @@\n line 1\n+line 2\n line 3\n";
    rerere_record(&rr, conflict_hash_str, resolved_patch, strlen(resolved_patch));
    printf("   Recorded resolution for hash %s...\n", conflict_hash_str);

    char *reused = NULL;
    size_t reused_len = 0;
    if (rerere_reuse(&rr, conflict_hash_str, &reused, &reused_len) == 0) {
        printf("   Reused resolution (%zu bytes): ", reused_len);
        printf("%.40s...\n", reused);
    }

    /* ── 8. Squash merge ────────────────────────────────────────── */
    printf("\n8. Squash merge:\n");
    SquashMerge sm;
    memset(&sm.base_oid, 0, sizeof(sm.base_oid));
    memset(&sm.tip_oid, 0, sizeof(sm.tip_oid));
    sm.discard_messages = 1;
    sm.new_message = strdup("feat: consolidated search feature");

    char *squash_out = NULL;
    squash_merge_apply(&sm, history, 3, ours_content, &squash_out);

    if (squash_out) {
        printf("   Squashed content (%zu bytes)\n", strlen(squash_out));
    }

    /* ── Cleanup ────────────────────────────────────────────────── */
    merge_result_free(&mr);
    merge_commit_free(&ours);
    merge_commit_free(&theirs);
    size_t j;
    for (j = 0; j < 3; j++) merge_commit_free(&history[j]);
    rebase_plan_free(&plan);
    interactive_rebase_free(&ir);
    merge_commit_free(&cp.source_commit);
    free(cp_out);
    free(reused);
    free(sm.new_message);
    free(squash_out);
    rerere_cache_free(&rr);

    printf("\n=== Done ===\n");
    return 0;
}
