#ifndef GIT_DAG_H
#define GIT_DAG_H

#include <stdint.h>
#include <stdbool.h>
#include <time.h>

/* ================================================================
 * L1: Core Definitions — Git DAG data structures
 *
 * Based on: Git internals (Scott Chacon, Pro Git §10)
 *           MIT 6.004 Computation Structures — graph representation
 *           CMU 15-410 OS — snapshot/version model
 *
 * A Git repository is a DAG (Directed Acyclic Graph) where:
 *   - Nodes = commits (snapshots of the working tree)
 *   - Edges = parent → child (each commit points to its parents)
 *   - Roots = initial commits (no parents)
 *   - Leaves = branch tips (no children, or latest in branch)
 *
 * Invariant: No cycles (guaranteed by hash chain)
 * ================================================================ */

#define GIT_DAG_MAX_COMMITS  256
#define GIT_DAG_MAX_PARENTS  8
#define GIT_HASH_HEX_LEN     40
#define GIT_HASH_RAW_LEN     20
#define GIT_MAX_BRANCHES     32
#define GIT_MAX_REFS         64
#define GIT_MAX_TAGS         64
#define GIT_MSG_MAX_LEN      256

typedef enum {
    GIT_OBJ_BLOB   = 0,
    GIT_OBJ_TREE   = 1,
    GIT_OBJ_COMMIT = 2,
    GIT_OBJ_TAG    = 3
} GitObjType;

typedef struct {
    int         id;
    char        hash[GIT_HASH_HEX_LEN + 1];
    char        tree_hash[GIT_HASH_HEX_LEN + 1];
    int         parent_ids[GIT_DAG_MAX_PARENTS];
    int         parent_count;
    char        author[64];
    char        committer[64];
    time_t      timestamp;
    char        message[GIT_MSG_MAX_LEN];
    int         generation;
    bool        is_merge;
} GitCommit;

typedef struct {
    int         id;
    char        name[128];
    int         target_commit_id;
    bool        is_branch;
    bool        is_head;
} GitRef;

typedef struct {
    GitCommit   commits[GIT_DAG_MAX_COMMITS];
    int         commit_count;
    GitRef      refs[GIT_MAX_REFS];
    int         ref_count;
    int         head_ref_id;
    int         adj_down[GIT_DAG_MAX_COMMITS][GIT_DAG_MAX_COMMITS];
    int         adj_up[GIT_DAG_MAX_COMMITS][GIT_DAG_MAX_COMMITS];
} GitDAG;

typedef enum {
    STRATEGY_GIT_FLOW       = 0,
    STRATEGY_TRUNK_BASED    = 1,
    STRATEGY_GITHUB_FLOW    = 2,
    STRATEGY_GITLAB_FLOW    = 3,
    STRATEGY_CUSTOM         = 4
} BranchStrategy;

void        git_dag_init(GitDAG *dag);
int         git_dag_add_commit(GitDAG *dag, const char *hash, int parents[],
                               int parent_count, const char *author,
                               const char *message);
int         git_dag_add_root_commit(GitDAG *dag, const char *hash,
                                    const char *author, const char *message);
int         git_dag_add_merge_commit(GitDAG *dag, const char *hash,
                                      int parent1, int parent2,
                                      const char *author, const char *message);
bool        git_dag_remove_commit(GitDAG *dag, int commit_id);
int         git_dag_create_branch(GitDAG *dag, const char *name, int target_commit);
int         git_dag_create_tag(GitDAG *dag, const char *name, int target_commit);
bool        git_dag_delete_ref(GitDAG *dag, int ref_id);
bool        git_dag_checkout(GitDAG *dag, int ref_id);
int         git_dag_find_ref(GitDAG *dag, const char *name);
int         git_dag_get_head_commit(GitDAG *dag);
bool        git_dag_reachable(GitDAG *dag, int from_commit, int to_commit);
int         git_dag_find_merge_base(GitDAG *dag, int commit_a, int commit_b);
int         git_dag_generation(GitDAG *dag, int commit_id);
int         git_dag_ancestors(GitDAG *dag, int commit_id, int *result, int max_results);
int         git_dag_descendants(GitDAG *dag, int commit_id, int *result, int max_results);
int         git_dag_common_ancestors(GitDAG *dag, int a, int b, int *result, int max_results);
int         git_dag_shortest_path(GitDAG *dag, int from, int to, int *path, int max_path);
bool        git_dag_is_ancestor(GitDAG *dag, int ancestor, int descendant);
int         git_dag_topological_order(GitDAG *dag, int *order);
int         git_dag_count_reachable(GitDAG *dag, int from_commit);
int         git_dag_max_depth(GitDAG *dag);
int         git_dag_branch_count(GitDAG *dag);
int         git_dag_divergence(GitDAG *dag, int commit_a, int commit_b);
bool        git_dag_is_linear(GitDAG *dag);
bool        git_dag_detect_conflict(GitDAG *dag, int commit_a, int commit_b);
void        git_dag_print(GitDAG *dag);
void        git_dag_print_reachability(GitDAG *dag, int commit_id);
void        git_dag_print_refs(GitDAG *dag);
int         git_dag_create_branch_strategy(GitDAG *dag, BranchStrategy strategy);
const char* git_dag_strategy_name(BranchStrategy strategy);

#endif
