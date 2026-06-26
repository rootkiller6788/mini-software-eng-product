#ifndef GIT_DAG_H
#define GIT_DAG_H
#include <stdint.h>
#include <stdbool.h>

#define DAG_MAX_COMMITS 128
#define DAG_MAX_BRANCHES 16

typedef struct { char hash[8]; char message[64]; int parent[2]; int parent_count; uint32_t timestamp; int author_id; } GitCommit;

typedef struct { char name[32]; int tip_commit; bool is_remote; } GitBranch;

typedef struct { GitCommit commits[DAG_MAX_COMMITS]; int commit_count; GitBranch branches[DAG_MAX_BRANCHES]; int branch_count; int head_branch; /* index into branches */ int head_commit; /* detached HEAD if >= 0 */ } GitDag;

void git_dag_init(GitDag *dag);
int  git_dag_commit(GitDag *dag, int parent, const char *msg);
int  git_dag_merge_commit(GitDag *dag, int parent1, int parent2, const char *msg);
int  git_dag_branch(GitDag *dag, const char *name);
bool git_dag_checkout(GitDag *dag, const char *branch_name);
bool git_dag_detached_head(GitDag *dag, int commit_idx);
int  git_dag_common_ancestor(GitDag *dag, int c1, int c2);
int  git_dag_log(GitDag *dag, int from_commit, int max_count, int *results);
void git_dag_print(GitDag *dag);
void git_dag_print_log(GitDag *dag);
#endif
