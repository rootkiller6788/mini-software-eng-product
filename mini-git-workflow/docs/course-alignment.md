# Course Alignment — Git Workflow & CI/CD

## Academic Courses

| Course | Institution | Topics Covered |
|--------|------------|----------------|
| 6.031 — Software Construction | MIT | Version control, code review, CI/CD |
| 15-214 — Principles of Software Construction | CMU | Git workflow, branching strategies, testing |
| CS 169 — Software Engineering | UC Berkeley | Agile, CI/CD, Git workflows |
| CSE 403 — Software Engineering | UW | Version control, build systems, testing |
| CS 242 — Programming Studio | Stanford | Software tools, version control workflows |

## Industry References

- **Git Internals** — Pro Git (Scott Chacon): DAG-based object model, commit graphs
- **Branching Models** — GitFlow (Driessen 2010), GitHub Flow (GitHub 2011), Trunk-Based Development
- **CI/CD** — Fowler (2006): Continuous Integration principles
- **Merge Strategies** — Three-way merge algorithm, recursive merge
- **Branch Protection** — GitHub/GitLab protected branch rules

## Module Coverage

This module implements:
1. **Git DAG** — Commit graph with parent tracking, common ancestor, log traversal
2. **Branch Models** — GitFlow, GitHub Flow, Trunk-Based, GitLab Flow strategies
3. **3-Way Merge** — Base/ours/theirs merge with conflict markers
4. **Conflict Resolution** — Ours, theirs, and union merge strategies
5. **CI Pipeline** — Job execution simulation, branch protection checks
