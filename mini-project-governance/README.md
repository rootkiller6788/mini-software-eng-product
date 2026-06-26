# Mini Project Governance

**From-scratch, zero-dependency C implementation** of Agile project governance systems — Scrum, Kanban, OKR tracking, Jira-like issue tracking, velocity forecasting, and retrospectives. Educational modeling of real-world project management toolchains.

## Module Status: COMPLETE ✅

- **L1-L6**: Complete
- **L7**: Complete (3 applications)
- **L8**: Partial (3/5 advanced topics)
- **L9**: Partial (documented, not implemented)
- **include/ + src/ lines**: 3,168 (>= 3,000 ✅)

## Nine-Level Knowledge Coverage

| Level | Name | Status | Items |
|-------|------|--------|-------|
| **L1** | Definitions | Complete | Scrum roles/artifacts/events, Kanban columns/cards, OKR objectives/KRs, Issue types/statuses, Velocity records, Retro formats/actions, Governance/risk/stakeholder structs |
| **L2** | Core Concepts | Complete | Sprint lifecycle, WIP-limited pull system, OKR grading, workflow state machine, rolling velocity, structured retrospectives |
| **L3** | Engineering Structures | Complete | Sprint state machine, Kanban board with WIP limits, OKR alignment tree, workflow transition engine, velocity profile |
| **L4** | Standards/Theorems | Complete | Little's Law, Cone of Uncertainty, Parkinson's Law, Brooks's Law, Earned Value Management (SPI/CPI/EAC), OKR confidence model |
| **L5** | Algorithms/Methods | Complete | WSJF prioritization, weighted KPI scoring, Monte Carlo sprint forecasting, burndown calculation, CFD, bottleneck detection, dot-voting, 5 Whys |
| **L6** | Canonical Problems | Complete | Sprint simulation, Kanban+OKR dashboard, Retrospective+Velocity analysis |
| **L7** | Applications | Complete (3) | Scrum sprint simulator, Kanban board with OKR tracking, Retrospective with velocity analysis |
| **L8** | Advanced Topics | Partial (3) | OKR alignment tree, Monte Carlo forecasting, 5 Whys root cause analysis |
| **L9** | Industry Frontiers | Partial | Documented: AI-assisted sprint planning, ML-based estimation |

## Core Definitions (L1)

### Scrum Framework
- `ScrumBoard` — Sprint management with backlog, team, velocity
- `Sprint` — Timeboxed iteration (planning, burndown, review)
- `ProductBacklogItem` — PBIs with story points, WSJF, dependencies
- `ScrumTeam` — Roles: Scrum Master, Product Owner, Developer

### Kanban System
- `KanbanBoard` — Configurable columns with WIP limits
- `KanbanCard` — Work items with Class of Service
- `CumulativeFlowDiagram` — Flow metrics for cycle time analysis

### OKR System
- `OkrDashboard` — Quarterly OKR planning
- `KeyResult` — Measurable outcomes (percentage, absolute, binary, currency, ratio)
- `OkrTree` — Hierarchical alignment with cascading scoring

### Issue Tracker
- `IssueTracker` — Jira-like tracking
- `WorkflowEngine` — Configurable state machine
- `Issue` — Comments, watchers, labels, links, time tracking

### Velocity & Forecasting
- `VelocityProfile` — Rolling statistics, trend analysis
- `Forecast` — Multi-method prediction engine

## Core Theorems (L4)

### Little's Law (Kanban)
WIP = Throughput * Cycle Time

### Cone of Uncertainty
Estimation range = 4.0 * e^(-2.0 * phase) + 0.8

### Earned Value Management
SV = EV - PV, CV = EV - AC, SPI = EV/PV, CPI = EV/AC, EAC = BAC/CPI

### Parkinson's Law
Expansion factor = actual / estimate

### Brooks's Law
Delay = original * (1 + ramp_up * new * (team + new) / team)

## Core Algorithms (L5)

| Algorithm | Function | Description |
|-----------|----------|-------------|
| WSJF Prioritization | `pbi_calc_wsjf()` | SAFe Weighted Shortest Job First |
| Rolling Average | `velocity_rolling_avg()` | 3-sprint rolling window |
| Monte Carlo Forecast | `forecast_monte_carlo()` | 10,000 simulation runs |
| Burndown Calculation | `burndown_calc_ideal()` | Linear ideal burndown |
| Cumulative Flow | `cfd_record()` | CFD with bottleneck detection |
| Dot Voting | `retro_vote()` | Democratic prioritization |
| 5 Whys Analysis | `five_whys_add()` | Root cause analysis |

## Classic Problems (L6)

1. **Sprint Planning Simulator** (`examples/example1_scrum_sprint.c`)
2. **Kanban + OKR Dashboard** (`examples/example2_kanban_okr.c`)
3. **Retrospective + Velocity Analysis** (`examples/example3_retro_velocity.c`)

## Building

```
make all    # Build library + examples
make test   # Run tests (62 tests pass)
make clean  # Clean build artifacts
```

## Project Structure

```
mini-project-governance/
  include/  7 headers (1,028 lines)
  src/      7 sources (2,140 lines)
  tests/    62 test cases (all pass)
  examples/ 3 end-to-end examples
  docs/     Knowledge documentation
```

## License

MIT