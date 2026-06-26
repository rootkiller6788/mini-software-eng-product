# Knowledge Graph — mini-git-workflow

## L1: Core Definitions

| Entry | Type | Location | Description |
|-------|------|----------|-------------|
| GitObjType | enum | git_dag.h:30-35 | Git object types (blob, tree, commit, tag) |
| GitCommit | struct | git_dag.h:38-50 | DAG node with hash, parents, metadata |
| GitRef | struct | git_dag.h:53-59 | Branch/tag reference pointing to commit |
| GitDAG | struct | git_dag.h:62-72 | Commit graph with adjacency matrices |
| BranchStrategy | enum | git_dag.h:75-82 | Branching strategy enumeration |
| BranchType | enum | git_branch.h:21-29 | Branch purpose classification |
| BranchState | enum | git_branch.h:32-39 | Branch lifecycle states |
| GitBranch | struct | git_branch.h:42-56 | Full branch representation |
| BranchModelConfig | struct | git_branch.h:59-68 | Strategy configuration parameters |
| BranchProtectionRule | struct | git_branch.h:71-80 | Branch protection rules |
| BranchManager | struct | git_branch.h:83-90 | Branch collection manager |
| MergeStrategy | enum | git_merge.h:27-34 | Merge algorithm types |
| ConflictType | enum | git_merge.h:37-43 | Conflict classification |
| MergeConflict | struct | git_merge.h:46-58 | Single merge conflict |
| MergeInput | struct | git_merge.h:61-69 | Three-way merge inputs |
| MergeResult | struct | git_merge.h:72-79 | Merge output with conflicts |
| DiffHunk | struct | git_merge.h:82-89 | Diff change region |
| RebasePlan | struct | git_merge.h:92-96 | Commit replay plan |
| ConventionalCommit | struct | git_commit.h | Parsed conventional commit |
| Semver | struct | git_commit.h:62-67 | Semantic version |
| Changelog | struct | git_commit.h:76-81 | Cumulative changelog |
| PRState | enum | git_pr.h:20-29 | PR lifecycle states |
| PullRequest | struct | git_pr.h:50-69 | Full PR representation |
| PRManager | struct | git_pr.h:72-77 | PR collection manager |
| CIStatus | enum | git_ci.h:22-30 | Pipeline/job status |
| CIJob/CIPipeline | struct | git_ci.h | Job and pipeline definitions |
| CIManager | struct | git_ci.h | Pipeline manager |

## L2: Core Concepts

| Concept | Implementation | File |
|---------|---------------|------|
| DAG Commit Model | git_dag_add_commit, adjacency matrices | git_dag.c |
| Branch Lifecycle | branch_create/merge/close/delete | git_branch.c |
| PR State Machine | pr_create/submit/review/merge | git_pr.c |
| CI Pipeline Stages | ci_add_stage/job, status transitions | git_ci.c |
| Merge Strategies | merge_select_strategy | git_merge.c |
| Semantic Versioning | semver_parse/compare/bump | git_commit.c |

## L3: Engineering Structures

| Structure | Implementation | File |
|-----------|---------------|------|
| DAG Adjacency Matrix | adj_down[n][n], adj_up[n][n] | git_dag.c |
| Branch Manager with Rules | BranchProtectionRule[] | git_branch.c |
| PR Review Policy | PRReviewPolicy enforcement | git_pr.c |
| Pipeline Stage DAG | Stage ordering + job dependencies | git_ci.c |

## L4: Standards & Theorems

| Standard/Theorem | Verification | Location |
|-----------------|-------------|----------|
| Conventional Commits v1.0.0 | commit_parse, commit_validate | git_commit.c:40-184 |
| Semantic Versioning 2.0.0 | semver_parse, semver_compare | git_commit.c:235-290 |
| Three-Way Merge Theorem | merge_three_way | git_merge.c:29-180 |
| Four-Eyes Principle | pr_is_mergeable, approval_count | git_pr.c |
| Branch Naming Conventions | branch_validate_name | git_branch.c |

## L5: Algorithms & Methods

| Algorithm | Complexity | Implementation | File |
|-----------|-----------|---------------|------|
| BFS Reachability | O(V+E) | git_dag_reachable | git_dag.c:148-173 |
| Merge-Base (Paint-Down) | O(V+E) | git_dag_find_merge_base | git_dag.c:178-219 |
| Kahn Topological Sort | O(V+E) | git_dag_topological_order | git_dag.c:268-292 |
| Shortest Path in DAG | O(V+E) | git_dag_shortest_path | git_dag.c |
| Myers Diff LCS | O(N*M) | diff_lcs_length | git_merge.c:290-318 |
| Diff Hunks (Hunt-Szymanski) | O(N*M) | diff_compute | git_merge.c:380-438 |
| Three-Way Line Merge | O(N) | merge_three_way | git_merge.c:29-180 |
| Glob Pattern Matching | O(N*M) | branch_match_pattern | git_branch.c |
| Commit Message Parsing | O(N) | commit_parse | git_commit.c |
| Critical Path Analysis | O(V+E) | ci_get_critical_path_duration | git_ci.c |

## L6: Canonical Problems

| Problem | Solution | Example |
|---------|----------|---------|
| Git Flow Branching | branch_configure_strategy(STRATEGY_GIT_FLOW) | example_git_flow.c |
| Trunk-Based Development | branch_configure_strategy(STRATEGY_TRUNK_BASED) | example_trunk_based.c |
| Code Review Pipeline | pr_create + review workflow | tests/test_git.c |
| CI Build-Test-Deploy | ci_generate_default_pipeline | tests/test_git.c |

## L7: Applications

| Application | Implementation | Location |
|-------------|---------------|----------|
| Auto-resolve whitespace conflicts | merge_auto_resolve_whitespace | git_merge.c |
| Auto-resolve identical changes | merge_auto_resolve_same_change | git_merge.c |
| Deployment gate checks | ci_deploy_gate_check | git_ci.c |
| Rollback detection | ci_rollback_check | git_ci.c |
| Changelog generation | changelog_generate | git_commit.c |

## L8: Advanced Topics

| Topic | Implementation | Status |
|-------|---------------|--------|
| Parallel job scheduling | ci_schedule_parallel_jobs | Partial |
| Critical path optimization | ci_get_critical_path_duration | Partial |
| Interactive rebase | rebase_plan_create / rebase_execute | Partial |
| Octopus merge | merge_octopus | Partial |

## L9: Industry Frontiers

| Topic | Status |
|-------|--------|
| Stacked Diffs (Graphite/spr) | Documented |
| Merge Queue with conflict prediction | Documented |
| Monorepo DAG optimization (GVFS, Sapling) | Documented |
| AI-assisted code review | Documented |

