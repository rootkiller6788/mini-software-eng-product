#include "git_dag.h"
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>

static void make_hash(char *buf) { for (int i=0;i<7;i++) buf[i]="0123456789abcdef"[rand()%16]; buf[7]='\0'; }

void git_dag_init(GitDag *dag) { memset(dag,0,sizeof(*dag)); dag->head_branch=-1; dag->head_commit=-1; }

int git_dag_commit(GitDag *dag, int parent, const char *msg) {
    if (dag->commit_count >= DAG_MAX_COMMITS) return -1;
    GitCommit *c = &dag->commits[dag->commit_count]; make_hash(c->hash); c->timestamp=(uint32_t)time(NULL); c->parent_count=0;
    if (parent >= 0) c->parent[c->parent_count++]=parent;
    strncpy(c->message, msg, 63); c->message[63]='\0';
    int ci = dag->commit_count++;
    if (dag->head_branch >= 0) dag->branches[dag->head_branch].tip_commit = ci;
    return ci;
}

int git_dag_merge_commit(GitDag *dag, int p1, int p2, const char *msg) {
    if (dag->commit_count >= DAG_MAX_COMMITS) return -1;
    GitCommit *c = &dag->commits[dag->commit_count]; make_hash(c->hash); c->timestamp=(uint32_t)time(NULL); c->parent_count=2; c->parent[0]=p1; c->parent[1]=p2;
    strncpy(c->message, msg, 63); c->message[63]='\0';
    int ci = dag->commit_count++;
    if (dag->head_branch >= 0) dag->branches[dag->head_branch].tip_commit = ci;
    return ci;
}

int git_dag_branch(GitDag *dag, const char *name) {
    if (dag->branch_count >= DAG_MAX_BRANCHES) return -1;
    GitBranch *b = &dag->branches[dag->branch_count]; strncpy(b->name, name, 31); b->name[31]='\0'; b->is_remote=false;
    b->tip_commit = dag->commit_count > 0 ? dag->commit_count - 1 : -1;
    return dag->branch_count++;
}

bool git_dag_checkout(GitDag *dag, const char *bn) { for (int i=0; i<dag->branch_count; i++) if (strcmp(dag->branches[i].name,bn)==0) { dag->head_branch=i; dag->head_commit=-1; return true; } return false; }

bool git_dag_detached_head(GitDag *dag, int ci) { dag->head_branch=-1; dag->head_commit=ci; return true; }

int git_dag_common_ancestor(GitDag *dag, int c1, int c2) {
    int anc1[64], anc2[64], a1=0, a2=0;
    while (c1 >= 0 && a1 < 64) { anc1[a1++]=c1; c1=dag->commits[c1].parent_count>0?dag->commits[c1].parent[0]:-1; }
    while (c2 >= 0 && a2 < 64) { anc2[a2++]=c2; c2=dag->commits[c2].parent_count>0?dag->commits[c2].parent[0]:-1; }
    for (int i=0; i<a1; i++) for (int j=0; j<a2; j++) if (anc1[i]==anc2[j]) return anc1[i];
    return -1;
}

int git_dag_log(GitDag *dag, int from, int max, int *results) {
    int c=from, n=0; while (c>=0 && n<max) { results[n++]=c; c=dag->commits[c].parent_count>0?dag->commits[c].parent[0]:-1; }
    return n;
}

void git_dag_print(GitDag *dag) {
    printf("=== Git DAG: %d commits, %d branches ===\n", dag->commit_count, dag->branch_count);
    for (int i=0; i<dag->branch_count; i++) printf("  %s%s -> %s\n", dag->branches[i].name, dag->head_branch==i?" (HEAD)":"", dag->branches[i].tip_commit>=0?dag->commits[dag->branches[i].tip_commit].hash:"-");
}

void git_dag_print_log(GitDag *dag) {
    for (int i=0; i<dag->commit_count; i++) { GitCommit *c=&dag->commits[i]; printf("  %s %s", c->hash, c->message); if (c->parent_count>1) printf(" (merge)"); printf("\n"); }
}
