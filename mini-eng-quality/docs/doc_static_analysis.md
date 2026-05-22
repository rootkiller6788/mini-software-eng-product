# Static Analysis Module

## Overview

The static analysis module (`static_analysis.h`) implements a configurable rules engine for detecting bugs, vulnerabilities, and code smells with severity-level quality gates.

## Rules Engine

### Severity Levels

| Level | Description | Example |
|-------|-------------|---------|
| `SA_SEVERITY_INFO` | Informational | Best practice suggestion |
| `SA_SEVERITY_MINOR` | Minor issue | Naming convention |
| `SA_SEVERITY_MAJOR` | Major issue | High complexity function |
| `SA_SEVERITY_CRITICAL` | Critical issue | Potential security vulnerability |
| `SA_SEVERITY_BLOCKER` | Blocker | Must-fix before release |

### Categories

- **Bugs** — Runtime errors, null dereferences, leaks
- **Vulnerabilities** — Buffer overflows, format strings, injection
- **Code Smells** — Maintainability issues, complexity, naming
- **Performance** — Inefficient patterns, allocations
- **Security Hotspots** — Sensitive areas needing manual review

## Quality Gate

Quality gates define pass/fail criteria for automated pipelines:

```c
sa_quality_gate_t gate;
sa_quality_gate_default(&gate);
gate.blocker_threshold = 0.0;   // Zero tolerance
gate.critical_threshold = 5.0;  // Up to 5 criticals
```

Gate results:
- `SA_QUALITY_PASS` — All thresholds met
- `SA_QUALITY_WARN` — Warnings present, review recommended
- `SA_QUALITY_FAIL` — Thresholds exceeded, merge blocked

## Technical Debt Ratio

```
Technical Debt Ratio = (Remediation Hours / Development Cost) × 100%
```

Where:
- **Remediation Hours** = sum of all open issue fix times
- **Development Cost** = LOC × cost_per_loc (default 0.05 hours/LOC)

A ratio below 5% is considered healthy. Above 10% indicates significant debt.

## Issue Lifecycle

```
OPEN → CONFIRMED → FIXED
  ↓         ↓
FALSE_POSITIVE   WONT_FIX
```

## Report Generation

The `sa_generate_report()` function produces a comprehensive report including:
- Issue counts by severity and category
- Total remediation effort
- Technical debt ratio
- Quality gate evaluation result

## Module Dependencies

- Standard C99 library plus `<math.h>` for debt ratio calculations
- No external dependencies
