# mini-conflict-resolution — Merge Conflict Resolution Strategies

## Goal

Implement and compare three conflict resolution strategies — ours, theirs, and union — to understand how merge tools resolve conflicting changes.

## Concepts

- **Conflict Detection**: Compare ours vs theirs vs base to identify change type
  - `CONF_NONE`: both identical
  - `CONF_CONTENT`: both modified same lines differently
  - `CONF_RENAME`: one side renamed, other modified
  - `CONF_DELETE_MODIFY`: one side deleted, other modified
  - `CONF_BINARY`: binary file conflict

- **Resolution Strategies**:
  - **Ours**: Keep our version, discard theirs
  - **Theirs**: Accept their version, discard ours
  - **Union**: Combine both versions (concatenate)

## Steps

### 1. Create conflict scenarios
Define three string versions (ours, theirs, base) for each conflict type.

### 2. Detect conflict type
Use `conflict_detect()` to classify each scenario.

### 3. Apply each resolution
For each conflict, try all three resolution strategies and compare output.

### 4. Build a conflict set
Create a `ConflictSet` with multiple file conflicts, then `conflict_analyze()` to determine `has_conflicts`.

### 5. Manual resolution simulation
Beyond automatic strategies, simulate a human picking specific hunks — accept ours for hunk 1, theirs for hunk 2.

## Expected Behavior

```
=== Conflicts: 3 ===
  src/main.c: CONTENT conflict
  src/config.h: CONTENT conflict
  README.md: CONTENT conflict
  Has conflicts: YES

Resolution:
  src/main.c (ours):  "Alice's code"
  src/main.c (theirs): "Bob's code"
  src/main.c (union): "Alice's code\nBob's code"
```

## Extensions

- Implement `conflict_resolve_interactive()` with hunk-level selection
- Add `conflict_resolve_external()` for external merge tool integration
- Simulate `git rerere` (reuse recorded resolution)

## References

- [git-merge documentation](https://git-scm.com/docs/git-merge)
- Advanced Git: Merge Strategies and Conflict Resolution
