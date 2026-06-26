#ifndef GIT_MERGE_H
#define GIT_MERGE_H
#include <stdbool.h>

#define MERGE_MAX_FILES 64

typedef struct { char path[64]; bool conflict; char ours[256]; char theirs[256]; char base[256]; char merged[512]; } MergeFile;

typedef struct {
    int source_branch_idx; int target_branch_idx; int merge_base;
    MergeFile files[MERGE_MAX_FILES]; int file_count; int conflicts; bool success;
} MergeResult;

bool git_merge_three_way(MergeResult *mr, const char *base, const char *ours, const char *theirs, char *out, int out_size);
int  git_merge_branches(MergeResult *mr, int source, int target, int base);
bool git_rebase_branch(MergeResult *mr, int source, int onto);
bool git_cherry_pick(MergeResult *mr, int commit, int onto_branch);
void merge_print_result(MergeResult *mr);
#endif
