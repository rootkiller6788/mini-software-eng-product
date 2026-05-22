# Mini Software Eng Product

**From-scratch, zero-dependency C implementations** of software engineering practices, project management, architecture management, and product development. Each module models core software engineering practices at educational fidelity — from architecture decision records and code review to Git workflows, agile project management, and software testing frameworks.

## Modules

| Module | Topics | Key References |
|--------|--------|----------------|
| [mini-arch-management](mini-arch-management/) | Architecture decision records (ADR), C4 model, architecture reviews, technical debt tracking | C4 Model, ADR GitHub |
| [mini-business-system](mini-business-system/) | Domain modeling, workflow engine, rule engine, state machine, approval process | Camunda, Drools |
| [mini-eng-quality](mini-eng-quality/) | Code review, SonarQube sim, cyclomatic complexity, technical debt, refactoring | SonarQube, Clean Code |
| [mini-git-workflow](mini-git-workflow/) | Git internals (objects/refs), branching models, merge/rebase, PR review, conventional commits | Pro Git, Conventional Commits |
| [mini-product-eng](mini-product-eng/) | PRD→spec→design→implement→test→release cycle, roadmap, stakeholder mgmt | SVPG Product, Shape Up |
| [mini-project-governance](mini-project-governance/) | Agile (Scrum/Kanban), Jira-like tracking, velocity, OKR tracking, retrospectives | Scrum Guide, SAFe |
| [mini-software-testing](mini-software-testing/) | Unit test framework, mock/stub, TDD, BDD, e2e, contract testing, mutation testing | JUnit, Jest, Pact |

## Design Philosophy

- **Zero external dependencies** — pure C (C99/C11), only `libc` and `libm`
- **Self-contained modules** — each directory has its own `Makefile`, `include/`, `src/`, `examples/`, `demos/`, `tests/`
- **Engineering simulation in user-space** — educational models of software engineering toolchains, project management, and product development workflows
- **Theory-to-code mapping** — every module includes `docs/` with practice-standard-alignment notes
- **Practical demos** — Git internal object browser, workflow engine, code quality analyzer, test framework, project kanban board, and more

## Building

Each module is standalone. Navigate to a module directory and run:

```bash
cd mini-git-workflow
make all    # build everything
make test   # run tests
```

Requires **GCC** and **GNU Make**.

## Project Structure

```
mini-software-eng-product/
├── mini-arch-management/        # Architecture Management
├── mini-business-system/        # Business Systems
├── mini-eng-quality/            # Engineering Quality
├── mini-git-workflow/           # Git Workflow
├── mini-product-eng/            # Product Engineering
├── mini-project-governance/     # Project Governance
└── mini-software-testing/       # Software Testing
```

## License

MIT
