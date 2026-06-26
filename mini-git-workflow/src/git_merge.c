#include "git_merge.h"
#include <stdio.h>
#include <string.h>

bool git_merge_three_way(MergeResult *mr, const char *base, const char *ours, const char *theirs, char *out, int sz) {
    if (strcmp(ours, theirs) == 0) { snprintf(out, sz, "%s", ours); return true; }
    if (strcmp(ours, base) == 0) { snprintf(out, sz, "%s", theirs); return true; }
    if (strcmp(theirs, base) == 0) { snprintf(out, sz, "%s", ours); return true; }
    snprintf(out, sz, "<<<<<<< OURS\n%s\n=======\n%s\n>>>>>>> THEIRS\n", ours, theirs);
    mr->conflicts++;
    return false;
}

int git_merge_branches(MergeResult *mr, int src, int tgt, int base) {
    memset(mr,0,sizeof(*mr)); mr->source_branch_idx=src; mr->target_branch_idx=tgt; mr->merge_base=base; return 0;
}

bool git_rebase_branch(MergeResult *mr, int src, int onto) { mr->source_branch_idx=src; mr->target_branch_idx=onto; return true; }

bool git_cherry_pick(MergeResult *mr, int commit, int onto_branch) { mr->source_branch_idx=commit; mr->target_branch_idx=onto_branch; return true; }

void merge_print_result(MergeResult *mr) {
    printf("=== Merge: branch %d -> %d ===\n", mr->source_branch_idx, mr->target_branch_idx);
    printf("  Base: %d, Files: %d, Conflicts: %d\n", mr->merge_base, mr->file_count, mr->conflicts);
    printf("  Success: %s\n", mr->success?"YES":"NO - resolve conflicts");
}
