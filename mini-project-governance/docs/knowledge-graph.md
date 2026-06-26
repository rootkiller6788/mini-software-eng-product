# Knowledge Graph — Project Governance

## L1: Core Definitions (Complete)
| Entry | Location | C Type |
|-------|----------|--------|
| ScrumBoard | gov_scrum.h | struct |
| Sprint | gov_scrum.h | struct (state machine) |
| ProductBacklogItem | gov_scrum.h | struct |
| ScrumTeam/TeamMember | gov_scrum.h | struct |
| ScrumRole enum | gov_scrum.h | enum (SM, PO, Dev, Stakeholder) |
| SprintState enum | gov_scrum.h | enum (5 states) |
| PBIState enum | gov_scrum.h | enum (7 states) |
| KanbanBoard | gov_kanban.h | struct |
| KanbanColumn | gov_kanban.h | struct (with WIP limit) |
| KanbanCard | gov_kanban.h | struct |
| ClassOfService enum | gov_kanban.h | enum (4 classes) |
| CumulativeFlowDiagram | gov_kanban.h | struct |
| OkrDashboard | gov_okr.h | struct |
| Objective | gov_okr.h | struct |
| KeyResult | gov_okr.h | struct |
| KRType enum | gov_okr.h | enum (5 types) |
| OkrConfidence enum | gov_okr.h | enum (4 levels) |
| OkrTree/OkrNode | gov_okr.h | struct (alignment tree) |
| IssueTracker | gov_tracker.h | struct |
| Issue | gov_tracker.h | struct |
| IssueStatus enum | gov_tracker.h | enum (10 states) |
| IssuePriority enum | gov_tracker.h | enum (5 levels) |
| WorkflowEngine/Transition | gov_tracker.h | struct (state machine) |
| VelocityProfile | gov_velocity.h | struct |
| VelocityRecord | gov_velocity.h | struct |
| Forecast | gov_velocity.h | struct |
| RetroLog | gov_retro.h | struct |
| RetroSession | gov_retro.h | struct |
| ActionItem | gov_retro.h | struct |
| FiveWhys | gov_retro.h | struct |
| GovernanceProgram | gov_governance.h | struct (orchestration) |
| GovernanceRisk | gov_governance.h | struct |
| EarnedValue | gov_governance.h | struct (EVM) |
| GovernanceScorecard | gov_governance.h | struct |

## L2: Core Concepts (Complete)
- Sprint lifecycle: Planning → Active → Review → Retrospective → Closed
- WIP-limited pull system (Kanban)
- OKR grading scale (0.0-1.0, Google model)
- Workflow state machine with validated transitions
- Rolling velocity with trend analysis
- Structured retrospective formats (6 formats)
- Risk exposure = probability * impact
- Stakeholder RACI matrix (Responsible/Accountable/Consulted/Informed)

## L3: Engineering Structures (Complete)
- Sprint state machine: 5 states with validated transitions
- Kanban board: dynamic column WIP limits, pull-based movement
- OKR alignment tree: parent-child cascading with depth tracking
- Workflow engine: configurable transition rules (14 default transitions)
- Velocity profile: 3-sprint rolling window with std dev
- Cumulative Flow Diagram: time-series snapshots
- Action item tracker: open/closed with retry counts

## L4: Standards/Theorems (Complete)
### Little's Law
WIP = Throughput * Cycle Time
Verified in: kanban_verify_littles_law()

### Cone of Uncertainty
Range multiplier = 4.0 * e^(-2.0 * phase_pct) + 0.8
Source: Boehm (1981), McConnell (2006)

### Earned Value Management (ANSI/EIA-748)
SV = EV - PV, CV = EV - AC
SPI = EV/PV, CPI = EV/AC
EAC = BAC/CPI

### Parkinson's Law
Work expands to fill available time
expansion = actual / estimate

### Brooks's Law (Mythical Man-Month)
Adding manpower to late project makes it later
delay = original * (1 + 0.2 * new_members * ratio)

### OKR Confidence
pace_ratio = progress / elapsed_pct
>= 0.7: ON_TRACK, >= 0.4: AT_RISK, < 0.4: OFF_TRACK

## L5: Algorithms/Methods (Complete)
| Algorithm | Complexity | Location |
|-----------|------------|----------|
| WSJF prioritization | O(n) | pbi_calc_wsjf() |
| Priority sort | O(n log n) | pbi_sort_by_priority() |
| Rolling average | O(w) per update | velocity_rolling_avg() |
| Linear regression (trend) | O(n) | velocity_calc_trend() |
| Monte Carlo simulation | O(sims * sprints) | forecast_monte_carlo() |
| Burndown (ideal) | O(days) | burndown_calc_ideal() |
| Burndown projection | O(n) linear fit | burndown_projected_completion() |
| Bottleneck detection | O(cols) | kanban_bottleneck_column() |
| Dot voting (top N) | O(n^2) selection sort | retro_top_voted() |
| Top risks (by exposure) | O(n^2) selection sort | gov_top_risks() |
| 5 Whys | O(1) per why | five_whys_add() |
| OKR tree build | O(n) | okr_build_tree() |
| OKR confidence | O(1) | okr_assess_confidence() |
| Sprint simulation | O(sprints) | sprint_sim_run() |
| EVM calculation | O(1) | evm_update() |
| Scorecard aggregation | O(n) | gov_calc_scorecard() |

## L6: Canonical Problems (Complete)
1. Sprint Planning Simulator — example1_scrum_sprint.c
2. Kanban Board with OKR Dashboard — example2_kanban_okr.c
3. Retrospective + Velocity Analysis — example3_retro_velocity.c

## L7: Applications (Complete - 3 examples)
- Agile sprint simulator with backlog and burndown
- Kanban flow management with OKR alignment
- Team health monitoring and retrospective analysis

## L8: Advanced Topics (Partial - 3 of 5)
1. ✅ OKR alignment tree with cascading scoring
2. ✅ Monte Carlo sprint forecasting (10K simulations)
3. ✅ 5 Whys root cause analysis
4. ❌ SAFe PI Planning simulation (future)
5. ❌ Probabilistic forecasting with confidence bands (future)

## L9: Industry Frontiers (Partial - documented)
- AI-assisted sprint planning (ML story point estimation)
- NLP-based PRD-to-backlog decomposition
- Anomaly detection in sprint burndown patterns
- Not implemented: serves as conceptual roadmap