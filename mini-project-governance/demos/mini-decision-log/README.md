# mini-decision-log — Architecture Decision Log (ADR)

## Goal

Implement an Architecture Decision Record (ADR) system to log key technical decisions with options, pros/cons, chosen solution, and consequences.

## Steps

1. Initialize decision log
2. For each decision, add context describing the problem
3. Add 2-5 options, each with description, pros, and cons
4. Make a decision: select chosen option, who decided, and date
5. Track pending (undecided) decisions
6. Later, record actual consequences of the decision
7. Support later deprecation or supersession of prior decisions

## Key APIs

- `decision_add()` — Create decision with context
- `decision_add_option()` — Add option with pros/cons
- `decision_make()` — Select option, record decider and date
- `decision_pending_count()` — Count undecided items
- `decision_print()` — Full ADR output for one decision

## Extensions

- Add decision categories (architecture, process, tooling, etc.)
- Link related decisions (supersedes, depends on)
- Export to standard ADR markdown format
