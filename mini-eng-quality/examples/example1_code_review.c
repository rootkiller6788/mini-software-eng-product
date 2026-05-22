#include "code_review.h"
#include <stdio.h>
#include <string.h>

int main(void) {
    cr_review_t review;
    memset(&review, 0, sizeof(review));

    const char *diff =
        "diff --git a/src/main.c b/src/main.c\n"
        "--- a/src/main.c\n"
        "+++ b/src/main.c\n"
        "@@ -1,5 +1,7 @@\n"
        " int main(void) {\n"
        "+    int x = 0;\n"
        "     printf(\"hello\");\n"
        "+    return x;\n"
        " }\n";

    printf("Parsing diff...\n");
    int files = cr_parse_diff(&review, diff);
    printf("Parsed %d file(s)\n", files);

    for (int i = 0; i < review.file_count; i++) {
        printf("  File: %s -> %s (%d additions, %d deletions)\n",
               review.files[i].path_old, review.files[i].path_new,
               review.files[i].added_lines, review.files[i].deleted_lines);
        for (int j = 0; j < review.files[i].hunk_count; j++)
            printf("    Hunk: %s\n", review.files[i].hunks[j].header);
    }

    cr_add_comment(&review, 3, "Consider initializing x to a meaningful value",
                   CR_SEVERITY_MINOR, "alice");
    cr_add_comment(&review, 5, "Return value should be EXIT_SUCCESS",
                   CR_SEVERITY_MAJOR, "bob");
    cr_reply_to_comment(&review, 0, "Agreed, will update.", "reviewer");
    cr_resolve_comment(&review, 0, "reviewer");

    cr_init_checklist(&review.checklist);
    cr_add_checklist_item(&review.checklist, "Code compiles without warnings");
    cr_add_checklist_item(&review.checklist, "Unit tests pass");
    cr_add_checklist_item(&review.checklist, "No memory leaks");
    cr_check_item(&review.checklist, 0);
    cr_check_item(&review.checklist, 1);
    cr_check_item(&review.checklist, 2);

    cr_set_approval_rules(&review.rules, 1, 1);
    cr_assign_reviewer(&review, "senior-dev");

    cr_ownership_t owners[2];
    memset(owners, 0, sizeof(owners));
    strcpy(owners[0].path_pattern, "src/");
    strcpy(owners[0].owners[0], "alice");
    owners[0].owner_count = 1;
    strcpy(owners[1].path_pattern, "include/");
    strcpy(owners[1].owners[0], "bob");
    owners[1].owner_count = 1;
    cr_suggest_reviewers(&review, owners, 2);

    cr_compute_stats(&review);
    cr_print_stats(&review.stats);

    printf("\nAll checks passed: %s\n",
           cr_all_checked(&review.checklist) ? "YES" : "NO");
    printf("Approval criteria met: %s\n",
           cr_check_approval(&review) ? "YES" : "NO");

    return 0;
}
