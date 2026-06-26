#include "git_branch.h"
#include <stdio.h>
#include <string.h>

const char *branch_strategy_name(BranchStrategy s) { switch(s) { case BSTRAT_GITFLOW: return "GitFlow"; case BSTRAT_GITHUB_FLOW: return "GitHub Flow"; case BSTRAT_TRUNK_BASED: return "Trunk-Based"; case BSTRAT_GITLAB_FLOW: return "GitLab Flow"; default: return "?"; } }

void branch_model_init(BranchModel *bm, BranchStrategy s) {
    memset(bm,0,sizeof(*bm)); bm->strategy = s;
    switch(s) {
    case BSTRAT_GITFLOW: branch_model_add(bm,"main","",true,true); branch_model_add(bm,"develop","main",true,false); branch_model_add(bm,"feature/*","develop",false,false); branch_model_add(bm,"release/*","develop",false,true); branch_model_add(bm,"hotfix/*","main",false,false); break;
    case BSTRAT_GITHUB_FLOW: branch_model_add(bm,"main","",true,true); branch_model_add(bm,"feature/*","main",false,false); break;
    case BSTRAT_TRUNK_BASED: branch_model_add(bm,"main","",true,true); branch_model_add(bm,"release/*","main",false,false); break;
    case BSTRAT_GITLAB_FLOW: branch_model_add(bm,"main","",true,true); branch_model_add(bm,"pre-prod","main",true,false); branch_model_add(bm,"prod","pre-prod",true,true); break;
    }
}

int branch_model_add(BranchModel *bm, const char *name, const char *base, bool ll, bool prot) { if (bm->branch_count>=10) return -1; BranchDef *b=&bm->branches[bm->branch_count]; strncpy(b->name,name,31); strncpy(b->base_branch,base,31); b->is_long_lived=ll; b->protected_branch=prot; return bm->branch_count++; }

bool branch_model_validate(BranchModel *bm) { return bm->branch_count >= 1; }

void branch_model_release(BranchModel *bm, ReleaseTag *rt, int ma, int mi, int pa) { snprintf(rt->version,sizeof(rt->version),"%d.%d.%d",ma,mi,pa); rt->major=ma; rt->minor=mi; rt->patch=pa; rt->is_release=true; snprintf(rt->tag,sizeof(rt->tag),"v%s",rt->version); (void)bm; }

void branch_model_print(BranchModel *bm) {
    printf("=== %s ===\n", branch_strategy_name(bm->strategy));
    for (int i=0; i<bm->branch_count; i++) printf("  %s -> %s %s%s\n", bm->branches[i].name, bm->branches[i].base_branch, bm->branches[i].is_long_lived?"[long-lived]":"", bm->branches[i].protected_branch?" [protected]":"");
}
