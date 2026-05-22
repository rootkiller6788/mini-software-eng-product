# mini-project-charter — Project Charter Builder

## Goal

Create a project charter that defines objectives, constraints, scope, budget, and schedule — the foundational governance document for any project.

## Steps

1. Initialize a charter with project name, sponsor, and manager
2. Add 3-5 measurable objectives with numeric targets
3. Define constraints (budget, timeline, quality, scope, legal categories)
4. Explicitly list what is IN scope and OUT of scope
5. Set budget and timeline baselines
6. Track progress against objectives periodically
7. Check if on-budget and on-schedule

## Key APIs

- `charter_init()` — Set name, sponsor, manager
- `charter_add_objective()` — Add measurable objective with target
- `charter_update_objective()` — Record current progress
- `charter_progress_pct()` — Average across all objectives
- `charter_add_constraint()` — Document category-based constraints
- `charter_add_scope()` — Explicitly mark items in/out of scope

## Extensions

- Add milestone tracking with dates
- Implement earned value management (EV/PV/AC)
- Support charter amendment history
