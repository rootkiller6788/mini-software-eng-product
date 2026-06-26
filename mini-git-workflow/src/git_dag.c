#include "git_dag.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>

/* ================================================================
 * L2-L5: Git DAG Engine Implementation
 *
 * Implements:
 *   L2: DAG core concepts — commit parentage, reachability, merge-base
 *   L3: Engineering structure — adjacency matrix, topological ordering
 *   L5: Algorithms — BFS/DFS traversal, merge-base finding,
 *       Kahn topological sort, shortest path in DAG
 * ================================================================ */

void git_dag_init(GitDAG *dag) {
    if (!dag) return;
    memset(dag, 0, sizeof(*dag));
    dag->head_ref_id = -1;
}

int git_dag_add_commit(GitDAG *dag, const char *hash, int parents[],
                       int parent_count, const char *author,
                       const char *message) {
    if (!dag || !hash || !author || !message) return -1;
    if (dag->commit_count >= GIT_DAG_MAX_COMMITS) return -1;
    if (parent_count > GIT_DAG_MAX_PARENTS) return -1;

    int id = dag->commit_count;
    GitCommit *c = &dag->commits[id];
    memset(c, 0, sizeof(*c));
    c->id = id;
    strncpy(c->hash, hash, GIT_HASH_HEX_LEN);
    c->hash[GIT_HASH_HEX_LEN] = 0;
    strncpy(c->author, author, 63);
    c->author[63] = 0;
    strncpy(c->committer, author, 63);
    c->committer[63] = 0;
    strncpy(c->message, message, GIT_MSG_MAX_LEN - 1);
    c->message[GIT_MSG_MAX_LEN - 1] = 0;
    c->timestamp = time(NULL);
    c->parent_count = parent_count;

    int max_gen = 0;
    for (int i = 0; i < parent_count; i++) {
        if (parents[i] < 0 || parents[i] >= dag->commit_count) return -1;
        c->parent_ids[i] = parents[i];
        dag->adj_down[parents[i]][id] = 1;
        dag->adj_up[id][parents[i]] = 1;
        if (dag->commits[parents[i]].generation + 1 > max_gen)
            max_gen = dag->commits[parents[i]].generation + 1;
    }
    c->generation = max_gen;
    c->is_merge = (parent_count > 1);
    dag->commit_count++;
    return id;
}

int git_dag_add_root_commit(GitDAG *dag, const char *hash,
                            const char *author, const char *message) {
    return git_dag_add_commit(dag, hash, NULL, 0, author, message);
}

int git_dag_add_merge_commit(GitDAG *dag, const char *hash,
                              int parent1, int parent2,
                              const char *author, const char *message) {
    int parents[2] = {parent1, parent2};
    return git_dag_add_commit(dag, hash, parents, 2, author, message);
}

bool git_dag_remove_commit(GitDAG *dag, int commit_id) {
    if (!dag || commit_id < 0 || commit_id >= dag->commit_count) return false;
    for (int i = 0; i < dag->commit_count; i++) {
        if (i != commit_id && dag->adj_down[commit_id][i]) return false;
    }
    for (int i = 0; i < dag->commit_count; i++) {
        dag->adj_down[commit_id][i] = 0;
        dag->adj_down[i][commit_id] = 0;
        dag->adj_up[commit_id][i] = 0;
        dag->adj_up[i][commit_id] = 0;
    }
    dag->commits[commit_id].id = -1;
    return true;
}

int git_dag_create_branch(GitDAG *dag, const char *name, int target_commit) {
    if (!dag || !name || dag->ref_count >= GIT_MAX_REFS) return -1;
    if (target_commit < 0 || target_commit >= dag->commit_count) return -1;
    int id = dag->ref_count;
    GitRef *r = &dag->refs[id];
    memset(r, 0, sizeof(*r));
    r->id = id;
    strncpy(r->name, name, 127);
    r->name[127] = 0;
    r->target_commit_id = target_commit;
    r->is_branch = true;
    r->is_head = false;
    dag->ref_count++;
    return id;
}

int git_dag_create_tag(GitDAG *dag, const char *name, int target_commit) {
    if (!dag || !name || dag->ref_count >= GIT_MAX_REFS) return -1;
    if (target_commit < 0 || target_commit >= dag->commit_count) return -1;
    int id = dag->ref_count;
    GitRef *r = &dag->refs[id];
    memset(r, 0, sizeof(*r));
    r->id = id;
    strncpy(r->name, name, 127);
    r->name[127] = 0;
    r->target_commit_id = target_commit;
    r->is_branch = false;
    r->is_head = false;
    dag->ref_count++;
    return id;
}

bool git_dag_delete_ref(GitDAG *dag, int ref_id) {
    if (!dag || ref_id < 0 || ref_id >= dag->ref_count) return false;
    if (dag->head_ref_id == ref_id) dag->head_ref_id = -1;
    dag->refs[ref_id].id = -1;
    return true;
}

bool git_dag_checkout(GitDAG *dag, int ref_id) {
    if (!dag || ref_id < 0 || ref_id >= dag->ref_count) return false;
    if (dag->head_ref_id >= 0 && dag->head_ref_id < dag->ref_count)
        dag->refs[dag->head_ref_id].is_head = false;
    dag->head_ref_id = ref_id;
    dag->refs[ref_id].is_head = true;
    return true;
}

int git_dag_find_ref(GitDAG *dag, const char *name) {
    if (!dag || !name) return -1;
    for (int i = 0; i < dag->ref_count; i++) {
        if (dag->refs[i].id >= 0 && strcmp(dag->refs[i].name, name) == 0)
            return i;
    }
    return -1;
}

int git_dag_get_head_commit(GitDAG *dag) {
    if (!dag || dag->head_ref_id < 0 || dag->head_ref_id >= dag->ref_count)
        return -1;
    return dag->refs[dag->head_ref_id].target_commit_id;
}

bool git_dag_reachable(GitDAG *dag, int from_commit, int to_commit) {
    if (!dag || from_commit < 0 || to_commit < 0) return false;
    if (from_commit >= dag->commit_count || to_commit >= dag->commit_count)
        return false;
    if (from_commit == to_commit) return true;

    bool visited[GIT_DAG_MAX_COMMITS] = {false};
    int queue[GIT_DAG_MAX_COMMITS];
    int front = 0, back = 0;

    queue[back++] = from_commit;
    visited[from_commit] = true;

    while (front < back) {
        int current = queue[front++];
        if (current == to_commit) return true;
        for (int p = 0; p < dag->commits[current].parent_count; p++) {
            int parent = dag->commits[current].parent_ids[p];
            if (!visited[parent]) {
                visited[parent] = true;
                queue[back++] = parent;
            }
        }
    }
    return false;
}

int git_dag_find_merge_base(GitDAG *dag, int commit_a, int commit_b) {
    if (!dag || commit_a < 0 || commit_b < 0) return -1;
    if (commit_a >= dag->commit_count || commit_b >= dag->commit_count)
        return -1;
    if (commit_a == commit_b) return commit_a;

    int ancestor_of_a[GIT_DAG_MAX_COMMITS] = {0};
    int queue[GIT_DAG_MAX_COMMITS];
    int front = 0, back = 0;

    queue[back++] = commit_a;
    ancestor_of_a[commit_a] = 1;

    while (front < back) {
        int current = queue[front++];
        for (int p = 0; p < dag->commits[current].parent_count; p++) {
            int parent = dag->commits[current].parent_ids[p];
            if (!ancestor_of_a[parent]) {
                ancestor_of_a[parent] = 1;
                queue[back++] = parent;
            }
        }
    }

    int best_base = -1;
    int best_gen = -1;
    front = 0; back = 0;
    bool visited[GIT_DAG_MAX_COMMITS] = {false};
    queue[back++] = commit_b;
    visited[commit_b] = true;

    while (front < back) {
        int current = queue[front++];
        if (ancestor_of_a[current]) {
            if (dag->commits[current].generation > best_gen) {
                best_gen = dag->commits[current].generation;
                best_base = current;
            }
        }
        for (int p = 0; p < dag->commits[current].parent_count; p++) {
            int parent = dag->commits[current].parent_ids[p];
            if (!visited[parent]) {
                visited[parent] = true;
                queue[back++] = parent;
            }
        }
    }
    return best_base;
}

int git_dag_generation(GitDAG *dag, int commit_id) {
    if (!dag || commit_id < 0 || commit_id >= dag->commit_count) return -1;
    return dag->commits[commit_id].generation;
}

int git_dag_ancestors(GitDAG *dag, int commit_id, int *result, int max_results) {
    if (!dag || !result || commit_id < 0 || commit_id >= dag->commit_count)
        return 0;
    bool visited[GIT_DAG_MAX_COMMITS] = {false};
    int queue[GIT_DAG_MAX_COMMITS];
    int front = 0, back = 0, count = 0;
    queue[back++] = commit_id;
    visited[commit_id] = true;
    while (front < back && count < max_results) {
        int current = queue[front++];
        if (current != commit_id) {
            result[count++] = current;
        }
        for (int p = 0; p < dag->commits[current].parent_count; p++) {
            int parent = dag->commits[current].parent_ids[p];
            if (!visited[parent]) {
                visited[parent] = true;
                queue[back++] = parent;
            }
        }
    }
    return count;
}

int git_dag_descendants(GitDAG *dag, int commit_id, int *result, int max_results) {
    if (!dag || !result || commit_id < 0 || commit_id >= dag->commit_count)
        return 0;
    bool visited[GIT_DAG_MAX_COMMITS] = {false};
    int queue[GIT_DAG_MAX_COMMITS];
    int front = 0, back = 0, count = 0;
    queue[back++] = commit_id;
    visited[commit_id] = true;
    while (front < back && count < max_results) {
        int current = queue[front++];
        if (current != commit_id) {
            result[count++] = current;
        }
        for (int child = 0; child < dag->commit_count; child++) {
            if (dag->adj_down[current][child] && !visited[child]) {
                visited[child] = true;
                queue[back++] = child;
            }
        }
    }
    return count;
}

int git_dag_common_ancestors(GitDAG *dag, int a, int b, int *result, int max_results) {
    if (!dag || !result) return 0;
    int ancestors_a[GIT_DAG_MAX_COMMITS];
    int ancestors_b[GIT_DAG_MAX_COMMITS];
    int na = git_dag_ancestors(dag, a, ancestors_a, GIT_DAG_MAX_COMMITS);
    int nb = git_dag_ancestors(dag, b, ancestors_b, GIT_DAG_MAX_COMMITS);
    int count = 0;
    for (int i = 0; i < na && count < max_results; i++) {
        for (int j = 0; j < nb; j++) {
            if (ancestors_a[i] == ancestors_b[j]) {
                result[count++] = ancestors_a[i];
                break;
            }
        }
    }
    return count;
}

int git_dag_shortest_path(GitDAG *dag, int from, int to, int *path, int max_path) {
    if (!dag || !path || from < 0 || to < 0) return -1;
    if (from >= dag->commit_count || to >= dag->commit_count) return -1;
    if (from == to) { path[0] = from; return 1; }
    bool visited[GIT_DAG_MAX_COMMITS] = {false};
    int parent[GIT_DAG_MAX_COMMITS];
    int queue[GIT_DAG_MAX_COMMITS];
    int front = 0, back = 0;
    for (int i = 0; i < GIT_DAG_MAX_COMMITS; i++) parent[i] = -1;
    queue[back++] = from;
    visited[from] = true;
    while (front < back) {
        int current = queue[front++];
        if (current == to) {
            int len = 0, cur = to;
            int temp_path[GIT_DAG_MAX_COMMITS];
            while (cur != -1 && len < GIT_DAG_MAX_COMMITS) {
                temp_path[len++] = cur;
                cur = parent[cur];
            }
            int write_idx = 0;
            for (int i = len - 1; i >= 0 && write_idx < max_path; i--) {
                path[write_idx++] = temp_path[i];
            }
            return write_idx;
        }
        for (int p = 0; p < dag->commits[current].parent_count; p++) {
            int next = dag->commits[current].parent_ids[p];
            if (!visited[next]) {
                visited[next] = true;
                parent[next] = current;
                queue[back++] = next;
            }
        }
        for (int child = 0; child < dag->commit_count; child++) {
            if (dag->adj_down[current][child] && !visited[child]) {
                visited[child] = true;
                parent[child] = current;
                queue[back++] = child;
            }
        }
    }
    return -1;
}

bool git_dag_is_ancestor(GitDAG *dag, int ancestor, int descendant) {
    return git_dag_reachable(dag, descendant, ancestor);
}

int git_dag_topological_order(GitDAG *dag, int *order) {
    if (!dag || !order) return 0;
    int in_degree[GIT_DAG_MAX_COMMITS] = {0};
    for (int i = 0; i < dag->commit_count; i++) {
        for (int j = 0; j < dag->commit_count; j++) {
            if (dag->adj_down[j][i]) in_degree[i]++;
        }
    }
    int queue[GIT_DAG_MAX_COMMITS];
    int front = 0, back = 0, idx = 0;
    for (int i = 0; i < dag->commit_count; i++) {
        if (in_degree[i] == 0) queue[back++] = i;
    }
    while (front < back) {
        int u = queue[front++];
        order[idx++] = u;
        for (int v = 0; v < dag->commit_count; v++) {
            if (dag->adj_down[u][v]) {
                if (--in_degree[v] == 0) queue[back++] = v;
            }
        }
    }
    return idx;
}

int git_dag_count_reachable(GitDAG *dag, int from_commit) {
    if (!dag || from_commit < 0 || from_commit >= dag->commit_count) return 0;
    bool visited[GIT_DAG_MAX_COMMITS] = {false};
    int queue[GIT_DAG_MAX_COMMITS];
    int front = 0, back = 0, count = 0;
    queue[back++] = from_commit;
    visited[from_commit] = true;
    while (front < back) {
        int current = queue[front++];
        count++;
        for (int p = 0; p < dag->commits[current].parent_count; p++) {
            int parent = dag->commits[current].parent_ids[p];
            if (!visited[parent]) {
                visited[parent] = true;
                queue[back++] = parent;
            }
        }
    }
    return count;
}

int git_dag_max_depth(GitDAG *dag) {
    if (!dag || dag->commit_count == 0) return 0;
    int max_gen = 0;
    for (int i = 0; i < dag->commit_count; i++) {
        if (dag->commits[i].generation > max_gen)
            max_gen = dag->commits[i].generation;
    }
    return max_gen;
}

int git_dag_branch_count(GitDAG *dag) {
    if (!dag) return 0;
    int count = 0;
    for (int i = 0; i < dag->ref_count; i++) {
        if (dag->refs[i].id >= 0 && dag->refs[i].is_branch) count++;
    }
    return count;
}

int git_dag_divergence(GitDAG *dag, int commit_a, int commit_b) {
    if (!dag) return -1;
    int base = git_dag_find_merge_base(dag, commit_a, commit_b);
    if (base < 0) return -1;
    int gen_a = dag->commits[commit_a].generation;
    int gen_b = dag->commits[commit_b].generation;
    int gen_base = dag->commits[base].generation;
    return (gen_a - gen_base) + (gen_b - gen_base);
}

bool git_dag_is_linear(GitDAG *dag) {
    if (!dag || dag->commit_count <= 1) return true;
    for (int i = 0; i < dag->commit_count; i++) {
        if (dag->commits[i].is_merge) return false;
    }
    return true;
}

bool git_dag_detect_conflict(GitDAG *dag, int commit_a, int commit_b) {
    if (!dag) return false;
    if (git_dag_is_ancestor(dag, commit_a, commit_b)) return false;
    if (git_dag_is_ancestor(dag, commit_b, commit_a)) return false;
    return true;
}

void git_dag_print(GitDAG *dag) {
    if (!dag) return;
    printf("=== Git DAG ===\n");
    printf("Commits: %d, Refs: %d, HEAD: %d\n",
           dag->commit_count, dag->ref_count, dag->head_ref_id);
    printf("Linear: %s, Max Depth: %d\n",
           git_dag_is_linear(dag) ? "yes" : "no", git_dag_max_depth(dag));
    for (int i = 0; i < dag->commit_count; i++) {
        GitCommit *c = &dag->commits[i];
        if (c->id < 0) continue;
        printf("  [%d] %s gen=%d parents=[", c->id, c->hash, c->generation);
        for (int p = 0; p < c->parent_count; p++) {
            printf("%d%s", c->parent_ids[p], p < c->parent_count - 1 ? "," : "");
        }
        printf("] msg=`%s` author=%s\n", c->message, c->author);
    }
}

void git_dag_print_reachability(GitDAG *dag, int commit_id) {
    if (!dag || commit_id < 0 || commit_id >= dag->commit_count) return;
    printf("Reachability from commit %d (%s):\n",
           commit_id, dag->commits[commit_id].hash);
    int ancestors[GIT_DAG_MAX_COMMITS];
    int n = git_dag_ancestors(dag, commit_id, ancestors, GIT_DAG_MAX_COMMITS);
    printf("  Ancestors (%d): ", n);
    for (int i = 0; i < n; i++) printf("%d ", ancestors[i]);
    printf("\n");
    int descendants[GIT_DAG_MAX_COMMITS];
    int m = git_dag_descendants(dag, commit_id, descendants, GIT_DAG_MAX_COMMITS);
    printf("  Descendants (%d): ", m);
    for (int i = 0; i < m; i++) printf("%d ", descendants[i]);
    printf("\n");
}

void git_dag_print_refs(GitDAG *dag) {
    if (!dag) return;
    printf("=== Refs ===\n");
    for (int i = 0; i < dag->ref_count; i++) {
        GitRef *r = &dag->refs[i];
        if (r->id < 0) continue;
        printf("  [%d] %s -> commit %d (%s)%s\n",
               r->id, r->name, r->target_commit_id,
               r->is_branch ? "branch" : "tag",
               r->is_head ? " [HEAD]" : "");
    }
}

int git_dag_create_branch_strategy(GitDAG *dag, BranchStrategy strategy) {
    if (!dag) return -1;
    int root = git_dag_add_root_commit(dag,
        "a1b2c3d4e5f6a7b8c9d0e1f2a3b4c5d6e7f8a9b0",
        "developer", "Initial commit");
    if (root < 0) return -1;
    int main_ref = git_dag_create_branch(dag, "main", root);
    if (main_ref < 0) return -1;
    int second = git_dag_add_commit(dag,
        "b2c3d4e5f6a7b8c9d0e1f2a3b4c5d6e7f8a9b0c1",
        &root, 1, "developer", "chore: setup project structure");
    if (main_ref >= 0) dag->refs[main_ref].target_commit_id = second;
    switch (strategy) {
    case STRATEGY_GIT_FLOW:
        git_dag_create_branch(dag, "develop", second);
        break;
    case STRATEGY_TRUNK_BASED:
        break;
    case STRATEGY_GITHUB_FLOW:
        break;
    default:
        break;
    }
    git_dag_checkout(dag, main_ref);
    return 0;
}

const char* git_dag_strategy_name(BranchStrategy strategy) {
    switch (strategy) {
    case STRATEGY_GIT_FLOW:    return "Git Flow";
    case STRATEGY_TRUNK_BASED: return "Trunk-Based Development";
    case STRATEGY_GITHUB_FLOW: return "GitHub Flow";
    case STRATEGY_GITLAB_FLOW: return "GitLab Flow";
    case STRATEGY_CUSTOM:      return "Custom";
    default:                   return "Unknown";
    }
}