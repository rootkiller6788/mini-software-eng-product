# mini-git-dag — Git Commit DAG Simulator

## Goal

Build a program that simulates a Git repository's commit graph — creating commits, branches, merges, and traversing history — to understand Git's internal object model.

## Concepts

- **Git DAG**: Git stores commits as a Directed Acyclic Graph where each commit points to its parent(s)
- **Commit hash**: SHA-1-like content hash (simulated with random hex strings)
- **Branch**: A mutable pointer to the latest commit (tip)
- **HEAD**: Points to the current branch or a detached commit
- **Merge base**: The common ancestor of two branches, used for 3-way merge

## Steps

### 1. Create the DAG
```c
GitDag dag;
git_dag_init(&dag);
```

### 2. Build commit history
Create a chain of commits: root → A → B → C. Print `git_dag_print_log()`.

### 3. Branch and diverge
Create a `feature` branch from commit C. Make commits D and E on feature. Switch back to main and make F. Observe the fork.

### 4. Find common ancestor
Use `git_dag_common_ancestor()` to find where the branches diverged.

### 5. Create a merge commit
Use `git_dag_merge_commit()` to combine the two branches.

## Expected Output

```
=== Git DAG: 7 commits, 2 branches ===
  main (HEAD) -> a3f8c21
  feature -> d7e2b14
  a1b2c3d Initial commit
  e4f5a6b Add README
  ...
  Common ancestor: e4f5a6b
```

## Extensions

- Implement `git_dag_reachable_from()` to find all commits reachable from a branch
- Add `git_dag_rev_list()` for topological sort
- Simulate rebase by replaying commits onto a new base

## References

- Pro Git, Chapter 10: Git Internals
- [Git Objects](https://git-scm.com/book/en/v2/Git-Internals-Git-Objects)
