# mini-ci-pipeline — CI/CD Pipeline Simulator

## Goal

Build a simulated CI/CD pipeline — define jobs (build, test, lint, deploy), execute them, track status, and enforce branch protection rules.

## Concepts

- **CI Pipeline**: A sequence of automated jobs that run on every push/PR
- **Job States**: PENDING → RUNNING → PASSED / FAILED / SKIPPED
- **Branch Protection**: Rules that block merges unless CI passes, reviews exist, commits are signed, and history is linear
- **Pre-commit checks**: Hooks that run before a commit is created locally

## Steps

### 1. Define a pipeline
```c
CiPipeline pipe; ci_pipeline_init(&pipe);
ci_add_job(&pipe, "build", "gcc -std=c99 -Wall -O2 -Iinclude -c src/*.c");
ci_add_job(&pipe, "unit-test", "./run_tests.sh");
ci_add_job(&pipe, "lint", "cppcheck --enable=all src/");
ci_add_job(&pipe, "security-scan", "bandit -r src/");
```

### 2. Run pipeline
Call `ci_run_all(&pipe)` and observe that each job goes from PENDING → RUNNING → PASSED.

### 3. Handle failures
Modify `ci_run_job()` logic or add a failing job scenario. Check `p.all_passed` afterward.

### 4. Branch protection
```c
BranchProtection bp = {
    .require_ci_pass = true,
    .require_review = true,
    .require_linear_history = true,
    .require_signed_commits = true
};
bool can_merge = ci_check_branch_protection(&bp, ci_pass, reviewed, linear, signed);
```

### 5. Pre-commit hooks
Simulate `ci_pre_commit_check()` running before local commits — lint + unit tests.

## Extensions

- Add parallel job execution (jobs with no dependency run concurrently)
- Implement `CiStage` groups (build stage → test stage → deploy stage)
- Add retry logic for flaky jobs
- Simulate GitHub Actions / GitLab CI YAML parsing

## References

- [Continuous Integration](https://martinfowler.com/articles/continuousIntegration.html) — Martin Fowler
- [GitHub Branch Protection Rules](https://docs.github.com/en/repositories/configuring-branches-and-merges-in-your-repository/managing-protected-branches)
- Accelerate (Forsgren, Humble, Kim) — CI/CD metrics
