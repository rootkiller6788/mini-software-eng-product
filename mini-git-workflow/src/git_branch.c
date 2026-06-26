#include "git_branch.h"
#include "git_dag.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

/* ================================================================
 * L2-L5: Branch Management Implementation
 *
 * L2: Branch lifecycle concepts - create, merge, close, delete
 * L3: Engineering structure - BranchManager with rules engine
 * L5: Branch divergence/ahead/behind computation
 * L6: Branch naming validation per Conventional Commits
 * ================================================================ */

void branch_manager_init(BranchManager *bm) {
    if (!bm) return;
    memset(bm, 0, sizeof(*bm));
    bm->config.strategy = STRATEGY_GITHUB_FLOW;
    strncpy(bm->config.main_branch, "main", BRANCH_NAME_MAX_LEN - 1);
    strncpy(bm->config.develop_branch, "develop", BRANCH_NAME_MAX_LEN - 1);
    bm->config.use_release_branches = false;
    bm->config.use_hotfix_branches = true;
    bm->config.squash_merge = false;
    bm->config.rebase_merge = true;
    bm->config.delete_after_merge = true;
    bm->config.max_branch_age_days = 30;
}

int branch_create(BranchManager *bm, const char *name, BranchType type,
                  int base_commit_id, const char *created_by) {
    if (!bm || !name || !created_by) return -1;
    if (bm->branch_count >= GIT_MAX_BRANCHES) return -1;
    if (!branch_validate_name(bm, name)) return -1;
    int id = bm->branch_count;
    GitBranch *b = &bm->branches[id];
    memset(b, 0, sizeof(*b));
    b->id = id;
    strncpy(b->name, name, BRANCH_NAME_MAX_LEN - 1);
    b->name[BRANCH_NAME_MAX_LEN - 1] = 0;
    b->type = type;
    b->state = BRANCH_STATE_ACTIVE;
    b->base_commit_id = base_commit_id;
    b->tip_commit_id = base_commit_id;
    b->parent_branch_id = -1;
    b->created_at = time(NULL);
    b->merged_at = 0;
    strncpy(b->created_by, created_by, 63);
    b->created_by[63] = 0;
    b->is_protected = false;
    b->requires_ci_pass = false;
    bm->branch_count++;
    return id;
}

bool branch_merge(BranchManager *bm, int branch_id, int into_branch_id) {
    if (!bm || branch_id < 0 || into_branch_id < 0) return false;
    if (branch_id >= bm->branch_count || into_branch_id >= bm->branch_count)
        return false;
    GitBranch *src = &bm->branches[branch_id];
    GitBranch *dst = &bm->branches[into_branch_id];
    if (src->state != BRANCH_STATE_ACTIVE) return false;
    if (dst->state != BRANCH_STATE_ACTIVE) return false;
    if (branch_id == into_branch_id) return false;
    src->state = BRANCH_STATE_MERGED;
    src->merged_at = time(NULL);
    src->tip_commit_id = dst->tip_commit_id;
    if (bm->config.delete_after_merge && branch_should_auto_delete(bm, src->type)) {
        src->state = BRANCH_STATE_DELETED;
    }
    return true;
}

bool branch_close(BranchManager *bm, int branch_id) {
    if (!bm || branch_id < 0 || branch_id >= bm->branch_count) return false;
    if (bm->branches[branch_id].state == BRANCH_STATE_DELETED) return false;
    bm->branches[branch_id].state = BRANCH_STATE_CLOSED;
    return true;
}

bool branch_delete(BranchManager *bm, int branch_id) {
    if (!bm || branch_id < 0 || branch_id >= bm->branch_count) return false;
    if (bm->branches[branch_id].is_protected) return false;
    bm->branches[branch_id].state = BRANCH_STATE_DELETED;
    return true;
}

bool branch_reopen(BranchManager *bm, int branch_id) {
    if (!bm || branch_id < 0 || branch_id >= bm->branch_count) return false;
    if (bm->branches[branch_id].state != BRANCH_STATE_CLOSED) return false;
    bm->branches[branch_id].state = BRANCH_STATE_ACTIVE;
    return true;
}

int branch_find_by_name(BranchManager *bm, const char *name) {
    if (!bm || !name) return -1;
    for (int i = 0; i < bm->branch_count; i++) {
        if (bm->branches[i].state != BRANCH_STATE_DELETED &&
            strcmp(bm->branches[i].name, name) == 0)
            return i;
    }
    return -1;
}

int branch_find_by_type(BranchManager *bm, BranchType type,
                        int *results, int max_results) {
    if (!bm || !results) return 0;
    int count = 0;
    for (int i = 0; i < bm->branch_count && count < max_results; i++) {
        if (bm->branches[i].state != BRANCH_STATE_DELETED &&
            bm->branches[i].type == type) {
            results[count++] = i;
        }
    }
    return count;
}

bool branch_is_mergeable(BranchManager *bm, int branch_id, int into_branch_id) {
    if (!bm) return false;
    if (branch_id < 0 || into_branch_id < 0) return false;
    if (branch_id >= bm->branch_count || into_branch_id >= bm->branch_count)
        return false;
    GitBranch *src = &bm->branches[branch_id];
    GitBranch *dst = &bm->branches[into_branch_id];
    if (src->state != BRANCH_STATE_ACTIVE) return false;
    if (dst->state != BRANCH_STATE_ACTIVE) return false;
    return (branch_id != into_branch_id);
}

bool branch_is_stale(BranchManager *bm, int branch_id) {
    if (!bm || branch_id < 0 || branch_id >= bm->branch_count) return false;
    if (bm->branches[branch_id].state != BRANCH_STATE_ACTIVE) return false;
    time_t now = time(NULL);
    double age_days = difftime(now, bm->branches[branch_id].created_at) / 86400.0;
    return age_days > bm->config.max_branch_age_days;
}

int branch_ahead_count(BranchManager *bm, int branch_id, int base_branch_id) {
    if (!bm) return -1;
    int tip_gen = bm->branches[branch_id].tip_commit_id;
    int base_gen = bm->branches[base_branch_id].tip_commit_id;
    return tip_gen > base_gen ? tip_gen - base_gen : 0;
}

int branch_behind_count(BranchManager *bm, int branch_id, int base_branch_id) {
    if (!bm) return -1;
    int tip_gen = bm->branches[branch_id].tip_commit_id;
    int base_gen = bm->branches[base_branch_id].tip_commit_id;
    return base_gen > tip_gen ? base_gen - tip_gen : 0;
}

int branch_divergence_count(BranchManager *bm, int branch_id, int base_branch_id) {
    if (!bm) return -1;
    int ahead = branch_ahead_count(bm, branch_id, base_branch_id);
    int behind = branch_behind_count(bm, branch_id, base_branch_id);
    if (ahead < 0 || behind < 0) return -1;
    return ahead + behind;
}

/* L4: Branch strategy configuration.
 * Each strategy encodes specific engineering practices:
 * - Git Flow: long-lived develop + release/hotfix branches
 * - Trunk-Based: short-lived feature branches, daily integration
 * - GitHub Flow: main + PR-based feature branches
 * - GitLab Flow: environment branches (staging, production) */
void branch_configure_strategy(BranchManager *bm, BranchStrategy strategy) {
    if (!bm) return;
    memset(&bm->config, 0, sizeof(bm->config));
    bm->config.strategy = strategy;
    switch (strategy) {
    case STRATEGY_GIT_FLOW:
        strncpy(bm->config.main_branch, "main", BRANCH_NAME_MAX_LEN - 1);
        strncpy(bm->config.develop_branch, "develop", BRANCH_NAME_MAX_LEN - 1);
        bm->config.use_release_branches = true;
        bm->config.use_hotfix_branches = true;
        bm->config.squash_merge = false;
        bm->config.rebase_merge = false;
        bm->config.delete_after_merge = false;
        bm->config.max_branch_age_days = 0;
        break;
    case STRATEGY_TRUNK_BASED:
        strncpy(bm->config.main_branch, "main", BRANCH_NAME_MAX_LEN - 1);
        strncpy(bm->config.develop_branch, "", BRANCH_NAME_MAX_LEN - 1);
        bm->config.use_release_branches = false;
        bm->config.use_hotfix_branches = true;
        bm->config.squash_merge = true;
        bm->config.rebase_merge = true;
        bm->config.delete_after_merge = true;
        bm->config.max_branch_age_days = 3;
        break;
    case STRATEGY_GITHUB_FLOW:
        strncpy(bm->config.main_branch, "main", BRANCH_NAME_MAX_LEN - 1);
        strncpy(bm->config.develop_branch, "", BRANCH_NAME_MAX_LEN - 1);
        bm->config.use_release_branches = false;
        bm->config.use_hotfix_branches = true;
        bm->config.squash_merge = true;
        bm->config.rebase_merge = true;
        bm->config.delete_after_merge = true;
        bm->config.max_branch_age_days = 14;
        break;
    case STRATEGY_GITLAB_FLOW:
        strncpy(bm->config.main_branch, "main", BRANCH_NAME_MAX_LEN - 1);
        strncpy(bm->config.develop_branch, "", BRANCH_NAME_MAX_LEN - 1);
        bm->config.use_release_branches = true;
        bm->config.use_hotfix_branches = true;
        bm->config.squash_merge = false;
        bm->config.rebase_merge = false;
        bm->config.delete_after_merge = false;
        bm->config.max_branch_age_days = 30;
        break;
    default: break;
    }
}

bool branch_validate_name(BranchManager *bm, const char *name) {
    if (!bm || !name) return false;
    int len = (int)strlen(name);
    if (len == 0 || len >= BRANCH_NAME_MAX_LEN) return false;
    for (int i = 0; i < len; i++) {
        char c = name[i];
        if (c == ' ' || c == '\\' || c == '*' || c == '?' ||
            c == '[' || c == ']' || c == '~' || c == '^' ||
            c == ':' || c < 32 || c > 126) return false;
    }
    if (name[0] == '/' || name[0] == '.' ||
        name[len-1] == '/' || name[len-1] == '.') return false;
    if (strstr(name, "..") || strstr(name, "//")) return false;
    const char *valid_prefixes[] = {
        "feature/", "feat/", "fix/", "hotfix/", "bugfix/",
        "release/", "chore/", "docs/", "refactor/", "test/",
        "perf/", "ci/", "build/", "revert/"
    };
    int num_prefixes = sizeof(valid_prefixes) / sizeof(valid_prefixes[0]);
    bool has_valid_prefix = false;
    for (int i = 0; i < num_prefixes; i++) {
        if (strncmp(name, valid_prefixes[i], strlen(valid_prefixes[i])) == 0) {
            has_valid_prefix = true; break;
        }
    }
    if (strcmp(name, "main") == 0 || strcmp(name, "master") == 0 ||
        strcmp(name, "develop") == 0 || strcmp(name, "dev") == 0)
        has_valid_prefix = true;
    return has_valid_prefix;
}

const char* branch_type_name(BranchType type) {
    switch (type) {
    case BRANCH_MAIN:      return "main";
    case BRANCH_DEVELOP:   return "develop";
    case BRANCH_FEATURE:   return "feature";
    case BRANCH_RELEASE:   return "release";
    case BRANCH_HOTFIX:    return "hotfix";
    case BRANCH_BUGFIX:    return "bugfix";
    case BRANCH_EPHEMERAL: return "ephemeral";
    case BRANCH_CUSTOM:    return "custom";
    default:               return "unknown";
    }
}

const char* branch_state_name(BranchState state) {
    switch (state) {
    case BRANCH_STATE_CREATED: return "created";
    case BRANCH_STATE_ACTIVE:  return "active";
    case BRANCH_STATE_MERGED:  return "merged";
    case BRANCH_STATE_CLOSED:  return "closed";
    case BRANCH_STATE_STALE:   return "stale";
    case BRANCH_STATE_DELETED: return "deleted";
    default:                   return "unknown";
    }
}

int branch_add_protection_rule(BranchManager *bm, const char *pattern) {
    if (!bm || !pattern || bm->rule_count >= BRANCH_MAX_RULES) return -1;
    int id = bm->rule_count;
    BranchProtectionRule *r = &bm->rules[id];
    memset(r, 0, sizeof(*r));
    r->id = id;
    strncpy(r->branch_pattern, pattern, BRANCH_NAME_MAX_LEN - 1);
    r->branch_pattern[BRANCH_NAME_MAX_LEN - 1] = 0;
    r->require_pr = true;
    r->min_approvals = 1;
    r->require_ci = true;
    r->require_signed_commits = false;
    r->block_force_push = true;
    r->require_linear_history = false;
    r->require_up_to_date = true;
    bm->rule_count++;
    return id;
}

bool branch_check_protection(BranchManager *bm, int branch_id) {
    if (!bm || branch_id < 0 || branch_id >= bm->branch_count) return false;
    GitBranch *b = &bm->branches[branch_id];
    for (int i = 0; i < bm->rule_count; i++) {
        if (branch_match_pattern(b->name, bm->rules[i].branch_pattern)) {
            b->is_protected = true;
            b->requires_ci_pass = bm->rules[i].require_ci;
            return true;
        }
    }
    return false;
}

/* L5: Glob-style pattern matching with * and ? wildcards.
 * Uses backtracking for * matching. O(n*m) worst case. */
bool branch_match_pattern(const char *branch_name, const char *pattern) {
    if (!branch_name || !pattern) return false;
    if (strcmp(pattern, "*") == 0) return true;
    const char *b = branch_name;
    const char *p = pattern;
    const char *star_pos = NULL;
    const char *match_pos = NULL;
    while (*b) {
        if (*p == '*') { star_pos = p++; match_pos = b; }
        else if (*p == *b || *p == '?') { p++; b++; }
        else if (star_pos) { p = star_pos + 1; b = ++match_pos; }
        else return false;
    }
    while (*p == '*') p++;
    return *p == 0;
}

int branch_add_file_change(BranchManager *bm, int branch_id,
                           const char *path, int added, int deleted) {
    if (!bm || !path || branch_id < 0 || branch_id >= bm->branch_count)
        return -1;
    GitBranch *b = &bm->branches[branch_id];
    if (b->change_count >= BRANCH_MAX_FILES) return -1;
    BranchFileChange *fc = &b->changes[b->change_count];
    fc->id = b->change_count;
    strncpy(fc->path, path, BRANCH_FILE_NAME_MAX - 1);
    fc->path[BRANCH_FILE_NAME_MAX - 1] = 0;
    fc->lines_added = added;
    fc->lines_deleted = deleted;
    fc->is_binary = false;
    fc->is_renamed = false;
    b->change_count++;
    return fc->id;
}

int branch_total_changes(BranchManager *bm, int branch_id) {
    if (!bm || branch_id < 0 || branch_id >= bm->branch_count) return -1;
    GitBranch *b = &bm->branches[branch_id];
    int total = 0;
    for (int i = 0; i < b->change_count; i++) {
        total += b->changes[i].lines_added + b->changes[i].lines_deleted;
    }
    return total;
}

bool branch_should_auto_delete(BranchManager *bm, BranchType type) {
    if (!bm) return false;
    if (!bm->config.delete_after_merge) return false;
    return (type == BRANCH_FEATURE || type == BRANCH_BUGFIX ||
            type == BRANCH_HOTFIX || type == BRANCH_EPHEMERAL);
}

int branch_list_active(BranchManager *bm, int *results, int max_results) {
    if (!bm || !results) return 0;
    int count = 0;
    for (int i = 0; i < bm->branch_count && count < max_results; i++) {
        if (bm->branches[i].state == BRANCH_STATE_ACTIVE)
            results[count++] = i;
    }
    return count;
}

int branch_list_stale(BranchManager *bm, int *results, int max_results) {
    if (!bm || !results) return 0;
    int count = 0;
    for (int i = 0; i < bm->branch_count && count < max_results; i++) {
        if (branch_is_stale(bm, i)) results[count++] = i;
    }
    return count;
}

void branch_print(BranchManager *bm, int branch_id) {
    if (!bm || branch_id < 0 || branch_id >= bm->branch_count) return;
    GitBranch *b = &bm->branches[branch_id];
    printf("Branch [%d] %s\n", b->id, b->name);
    printf("  Type: %s, State: %s\n",
           branch_type_name(b->type), branch_state_name(b->state));
    printf("  Base: %d, Tip: %d, Parent: %d\n",
           b->base_commit_id, b->tip_commit_id, b->parent_branch_id);
    printf("  Changes: %d files, %d total lines\n",
           b->change_count, branch_total_changes(bm, branch_id));
    printf("  Protected: %s, CI Required: %s\n",
           b->is_protected ? "yes" : "no",
           b->requires_ci_pass ? "yes" : "no");
}

void branch_print_all(BranchManager *bm) {
    if (!bm) return;
    printf("=== Branch Manager ===\n");
    printf("Strategy: %s, Branches: %d\n",
           git_dag_strategy_name(bm->config.strategy), bm->branch_count);
    for (int i = 0; i < bm->branch_count; i++) {
        if (bm->branches[i].state != BRANCH_STATE_DELETED) {
            branch_print(bm, i); printf("\n");
        }
    }
}

void branch_print_tree(BranchManager *bm) {
    if (!bm) return;
    printf("=== Branch Tree ===\n");
    for (int i = 0; i < bm->branch_count; i++) {
        GitBranch *b = &bm->branches[i];
        if (b->state == BRANCH_STATE_DELETED) continue;
        int indent = 0;
        int parent = b->parent_branch_id;
        while (parent >= 0 && indent < 10) {
            indent++;
            parent = (parent < bm->branch_count) ?
                     bm->branches[parent].parent_branch_id : -1;
        }
        for (int j = 0; j < indent; j++) printf("  ");
        printf("%s [%s] -> tip:%d\n",
               b->name, branch_state_name(b->state), b->tip_commit_id);
    }
}
