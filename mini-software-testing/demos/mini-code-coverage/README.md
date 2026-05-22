# mini-code-coverage — Code Coverage Tracker

## Goal

Track statement coverage, branch coverage, and MC/DC to measure test effectiveness and identify untested code paths.

## Steps

1. Initialize coverage data and add source files
2. Mark lines with `coverage_add_line()`, flagging whether each line has a branch
3. Simulate test execution by calling `coverage_hit()` for each executed line
4. For branch-bearing lines, call `coverage_branch_hit()` with true/false branch taken
5. Calculate percentages: line, branch, and MC/DC
6. Check if coverage meets thresholds (e.g., 80% line, 70% branch)
7. Print per-file and summary coverage reports

## Key APIs

- `coverage_add_file()` — Register a source file for tracking
- `coverage_add_line()` — Register a line, optionally with branch
- `coverage_hit()` — Mark line as executed (increment counter)
- `coverage_branch_hit()` — Mark which branch direction was taken
- `coverage_calculate()` — Recompute all percentages
- `coverage_line_pct()` / `coverage_branch_pct()` / `coverage_mcdc_pct()` — Get metrics
- `coverage_meets_threshold()` — Check against quality gate

## Extensions

- Add function coverage tracking
- Implement coverage diff (what changed since last run)
- Generate HTML coverage report with line coloring
- Track condition coverage for MC/DC (each condition independently affects outcome)
