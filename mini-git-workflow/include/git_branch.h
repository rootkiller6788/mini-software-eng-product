#ifndef GIT_BRANCH_H
#define GIT_BRANCH_H
#include <stdbool.h>

typedef enum { BSTRAT_GITFLOW, BSTRAT_GITHUB_FLOW, BSTRAT_TRUNK_BASED, BSTRAT_GITLAB_FLOW } BranchStrategy;

typedef struct { char name[32]; char base_branch[32]; bool is_long_lived; bool protected_branch; } BranchDef;

typedef struct { BranchStrategy strategy; BranchDef branches[10]; int branch_count; int environment_count; } BranchModel;

typedef struct { char version[16]; int major, minor, patch; bool is_release; char tag[32]; } ReleaseTag;

void branch_model_init(BranchModel *bm, BranchStrategy strategy);
int  branch_model_add(BranchModel *bm, const char *name, const char *base, bool long_lived, bool protected_);
bool branch_model_validate(BranchModel *bm); /* checks for required branches */
void branch_model_release(BranchModel *bm, ReleaseTag *rt, int major, int minor, int patch);
const char *branch_strategy_name(BranchStrategy s);
void branch_model_print(BranchModel *bm);
#endif
