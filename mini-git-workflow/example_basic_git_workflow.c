#include "git_internals.h"
#include "branching_models.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Example: basic Git workflow — init objects, create branches, merge */

int main(void) {
    printf("=== Basic Git Workflow Example (C99) ===\n\n");

    /* ── 1. Create a Git object (blob) ──────────────────────────── */
    const char *content = "Hello, Git! This is file content.";
    GitOid blob_oid;
    git_hash_object((const unsigned char *)content, strlen(content),
                     GIT_OBJ_BLOB, &blob_oid);

    printf("1. Blob hash: ");
    int i;
    for (i = 0; i < GIT_SHA1_RAWSZ; i++) printf("%02x", blob_oid.id[i]);
    printf("\n");

    /* ── 2. Write blob to object database ───────────────────────── */
    GitOdb odb;
    memset(&odb, 0, sizeof(odb));
    odb.objects_dir = ".git/objects";

    GitOid written_oid;
    git_odb_write(&odb, GIT_OBJ_BLOB,
                  (const unsigned char *)content, strlen(content),
                  &written_oid);
    printf("2. Wrote blob to odb\n");

    /* ── 3. Check existence ─────────────────────────────────────── */
    if (git_odb_exists(&odb, &written_oid))
        printf("3. Blob exists in odb: yes\n");

    /* ── 4. Create a tree entry ─────────────────────────────────── */
    GitTree tree;
    memset(&tree, 0, sizeof(tree));
    tree.entries = (GitTreeEntry *)malloc(sizeof(GitTreeEntry));
    tree.entry_count = 1;
    tree.entries[0].mode = GIT_FILEMODE_NORMAL;
    tree.entries[0].type = GIT_OBJ_BLOB;
    memcpy(tree.entries[0].oid.id, blob_oid.id, GIT_SHA1_RAWSZ);
    tree.entries[0].name = strdup("hello.txt");
    printf("4. Tree with 1 entry (hello.txt)\n");

    /* ── 5. Reference: HEAD ─────────────────────────────────────── */
    GitHead head;
    head.type = GIT_REF_SYMBOLIC;
    head.target = strdup("refs/heads/main");
    printf("5. HEAD -> %s\n", head.target);

    /* ── 6. Branch: main ────────────────────────────────────────── */
    GitBranch main_br;
    main_br.name = strdup("main");
    memcpy(main_br.tip_oid.id, blob_oid.id, GIT_SHA1_RAWSZ);
    main_br.upstream = strdup("refs/remotes/origin/main");
    printf("6. Branch '%s' tip at ", main_br.name);
    for (i = 0; i < 5; i++) printf("%02x", main_br.tip_oid.id[i]);
    printf("..., upstream: %s\n", main_br.upstream);

    /* ── 7. Feature branch ──────────────────────────────────────── */
    GitFlow gf;
    git_flow_init(&gf);

    git_flow_start_feature(&gf, "feature/add-login", "JIRA-101");
    printf("\n7. Git Flow — started feature/add-login\n");

    git_flow_start_release(&gf, "1.2.0");
    printf("   Started release/1.2.0\n");

    git_flow_start_hotfix(&gf, "1.1.1");
    printf("   Started hotfix/1.1.1\n");

    printf("   Active features: %zu\n", gf.feature_count);
    printf("   Active releases: %zu\n", gf.release_count);
    printf("   Active hotfixes: %zu\n", gf.hotfix_count);

    /* ── 8. GitHub Flow ─────────────────────────────────────────── */
    GitHubFlow ghf;
    github_flow_init(&ghf);

    github_flow_create_feature(&ghf, "improve-search");
    printf("\n8. GitHub Flow — feature branch: improve-search\n");

    github_flow_open_pr(&ghf, "Improve search performance",
                         "improve-search", "main");
    printf("   Opened PR #1\n");

    github_flow_review_pr(&ghf, 0, 1);
    printf("   Reviewed & approved PR\n");

    github_flow_merge_pr(&ghf, 0, PR_MERGE_SQUASH);
    printf("   Merged PR with squash\n");

    /* ── 9. Trunk-Based Development ─────────────────────────────── */
    TrunkBasedDev tbd;
    tbd_init(&tbd);

    tbd_add_feature_flag(&tbd, "new-dashboard", "Dashboard v2", FF_TYPE_RELEASE);
    tbd_add_feature_flag(&tbd, "dark-mode", "Dark theme", FF_TYPE_EXPERIMENT);
    printf("\n9. TBD — feature flags: 2\n");
    printf("   Branch lifetime max: %d hours\n", tbd.max_branch_hours);
    printf("   Max commits/branch: %zu\n", tbd.max_commits_per_branch);

    tbd_toggle_flag(&tbd, "new-dashboard", 1);
    printf("   Toggled 'new-dashboard' ON\n");

    /* ── Cleanup ────────────────────────────────────────────────── */
    free(tree.entries[0].name);
    free(tree.entries);
    free(head.target);
    free(main_br.name);
    free(main_br.upstream);
    git_flow_free(&gf);
    github_flow_free(&ghf);
    tbd_free(&tbd);
    git_tree_free(&tree);

    printf("\n=== Done ===\n");
    return 0;
}
