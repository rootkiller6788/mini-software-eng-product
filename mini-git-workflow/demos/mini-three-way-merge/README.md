# mini-three-way-merge — Three-Way Merge Algorithm

## Goal

Implement and understand the 3-way merge algorithm used by Git — comparing base, ours, and theirs to produce a merged result or detect conflicts.

## Concepts

- **3-Way Merge**: Compares three versions: base (common ancestor), ours (current branch), theirs (incoming branch)
- **Trivial merge**: If ours == theirs → copy either; if only one side changed → use the changed version
- **Content conflict**: Both sides changed the same lines differently → insert conflict markers
- **Conflict markers**: `<<<<<<< OURS`, `=======`, `>>>>>>> THEIRS`

## Steps

### 1. Test trivial cases
```c
// ours == theirs → auto-resolve
git_merge_three_way(&mr, "A", "B", "B", out, sz);  // both same

// only ours changed
git_merge_three_way(&mr, "A", "B", "A", out, sz);  // use B

// only theirs changed
git_merge_three_way(&mr, "A", "A", "C", out, sz);  // use C
```

### 2. Test conflict case
```c
// both changed from base → conflict
git_merge_three_way(&mr, "Hello", "Hello Alice", "Hello Bob", out, sz);
// Result: <<<<<<< OURS\nHello Alice\n=======\nHello Bob\n>>>>>>> THEIRS
```

### 3. Simulate file-level merge
Create multiple MergeFile entries, set `ours`/`theirs`/`base` for each, run merge on each file, and report `mr.conflicts`.

### 4. Implement rebase simulation
Use `git_rebase_branch()` to replay commits from one branch onto another tip.

### 5. Implement cherry-pick simulation
Use `git_cherry_pick()` to apply a single commit to another branch.

## Extensions

- Implement recursive merge strategy (criss-cross merge)
- Add rename detection (`CONF_RENAME` type)
- Implement patience/diff3-style merge

## References

- Pro Git, Chapter 7.8: Advanced Merging
- [Three-way merge](https://en.wikipedia.org/wiki/Merge_(version_control)#Three-way_merge)
