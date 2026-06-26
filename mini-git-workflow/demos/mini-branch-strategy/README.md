# mini-branch-strategy — Branching Strategy Comparison

## Goal

Compare four branching strategies (GitFlow, GitHub Flow, Trunk-Based, GitLab Flow) by modeling their branch structures, release processes, and evaluating trade-offs.

## Concepts

- **GitFlow**: main + develop + feature/* + release/* + hotfix/* — strict branching for scheduled releases
- **GitHub Flow**: main + feature/* — simple, deployment-focused, every merge to main is deployable
- **Trunk-Based**: main + short-lived release/* — all devs commit to main, minimal branching
- **GitLab Flow**: main → pre-prod → prod — environment-based promotion

## Steps

### 1. Initialize each model
Call `branch_model_init()` for each strategy and `branch_model_print()` to see branches.

### 2. Compare branch counts
Count branches per strategy — GitFlow has the most (5), Trunk-Based the fewest (2).

### 3. Release process
Simulate releasing v1.0.0 in GitFlow: merge release/1.0 → main + develop. Tag with `branch_model_release()`.

### 4. Hotfix process
Simulate a hotfix in GitFlow: branch from main, fix, merge to both main and develop.

### 5. Evaluate strengths
- GitFlow: best for scheduled releases with multiple supported versions
- GitHub Flow: best for continuous deployment SaaS
- Trunk-Based: best for small teams with CI/CD rigor
- GitLab Flow: best for staged deployment environments

## Extensions

- Implement `branch_model_merge()` to simulate branch merging within a model
- Add commit count tracking per branch
- Visualize branch lifecycles with ASCII art

## References

- [A successful Git branching model](https://nvie.com/posts/a-successful-git-branching-model/) — Vincent Driessen
- [GitHub Flow](https://docs.github.com/en/get-started/using-github/github-flow)
- [Trunk Based Development](https://trunkbaseddevelopment.com/)
