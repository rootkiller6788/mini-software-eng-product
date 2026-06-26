# mini-eng-quality — 工程质量 (C 语言实现)

## Overview

A lightweight C99 library for software engineering quality management, simulating core features found in enterprise tools like SonarQube, Gerrit, and clang-format.

## Modules

| Module | Header | Description |
|--------|--------|-------------|
| Code Review | `code_review.h` | Diff parsing, comment→resolve flow, reviewer assignment, checklists, approval rules, stats |
| Static Analysis | `static_analysis.h` | Bug/vulnerability/smell detection, rules engine, quality gate, technical debt ratio |
| Complexity Metrics | `complexity_metrics.h` | Cyclomatic & cognitive complexity, LOC, comment ratio, duplication, maintainability, hotspots |
| Tech Debt Sprint | `tech_debt_sprint.h` | Debt backlog, reduction sprints (20%), interest tracking, refactoring ROI, fitness functions |
| Lint & Formatter | `lint_formatter.h` | Naming/style/security rules, auto-fix, clang-format model, pre-commit hook, CI integration |

## Build

```
make        # build library and all targets
make demo   # build demo programs
make test   # run example programs
make clean  # remove build artifacts
```

## Usage

```c
#include "code_review.h"
#include "static_analysis.h"
#include "complexity_metrics.h"
#include "tech_debt_sprint.h"
#include "lint_formatter.h"
```

## License

MIT
