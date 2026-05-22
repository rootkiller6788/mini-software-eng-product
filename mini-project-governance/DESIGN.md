# Design Document

## Architecture Overview

```
mini-project-governance/
├── agile_scrum.h / .c     — Sprint lifecycle, backlog, velocity, charts
├── kanban_board.h / .c    — Board columns, WIP, CFD, flow metrics
├── project_tracker.h / .c — Issue tracking, labels, workflow, JQL
├── okr_tracker.h / .c     — OKR hierarchy, grading, CFR
└── retrospective.h / .c   — Retro formats, health, morale, psych safety
```

Each module is self-contained with no cross-module dependencies at the core level. Demos combine modules.

## Design Principles

### 1. C99 Compliance
- ISO C99 with `-std=c99 -Wall -Wextra -pedantic`
- No VLAs in struct definitions (fixed-size arrays)
- POSIX `time()` from `<time.h>` for timestamps
- `<math.h>` for `fabs`, `ceil` in analytics functions

### 2. Fixed-Capacity Arrays
All data structures use pre-allocated fixed-size arrays instead of dynamic allocation per element:
- **Rationale**: Predictable memory, no fragmentation, cache-friendly, zero allocation overhead
- **Trade-off**: Upper bound on items (defined by `#define` constants, adjustable)
- **Examples**: `PT_MAX_ISSUES=4096`, `KANBAN_MAX_CARDS=2048`, `SCRUM_MAX_BACKLOG_ITEMS=1024`

### 3. Return Value Convention
- Functions return `0` on success, `-1` on error
- Creation/ID functions return the new entity ID (>0) on success, `-1` on error
- Pointer-returning functions return `NULL` on error

### 4. String Safety
All string copies use `strncpy` with explicit null termination:
```c
strncpy(dest, src, MAX - 1);
dest[MAX - 1] = '\0';
```
No format-string vulnerabilities — all `printf` calls use format strings.

## Module Design Details

### Agile Scrum (`agile_scrum.h`)

**Data Model**:
```
ScrumContext
├── ScrumSprint[64]          — Up to 64 sprints
│   ├── backlog_item_ids[]   — References to backlog items
│   ├── story_points         — Planned vs completed
│   └── velocity             — Per-sprint velocity
├── ScrumProductBacklog      — Single product backlog
│   └── ScrumBacklogItem[1024] — Up to 1024 items
├── ScrumStandup[512]        — Standup log
└── team_velocity_avg        — Running team velocity average
```

**Sprint Lifecycle**:
```
PLANNING → ACTIVE → COMPLETED (or CANCELLED)
```

**Velocity Calculation**:
- Per-sprint: `completed_points / planned_points`
- Team average: sum of completed across all completed sprints / sprint count
- Only considers sprints with `status == COMPLETED` and `points_planned > 0`

**Burndown Chart**: Ideal linear burn rate `total_points / duration_days`. For simplicity, actual tracking is done via sprint review accepted items, not daily log correlation.

### Kanban Board (`kanban_board.h`)

**Data Model**:
```
KanbanBoard
├── KanbanColumn[16]         — Configurable columns with WIP limits
│   ├── type                 — Enum: BACKLOG, TODO, IN_PROGRESS, REVIEW, DONE + 3 custom
│   └── wip_limit            — NOT NULL constraint analog
├── KanbanCard[2048]         — Cards flowing through columns
│   ├── created_at/started_at/completed_at — For cycle/lead time
│   └── blocked + block_reason — Impediment tracking
└── KanbanCFDPoint[365]      — Cumulative Flow Diagram snapshots
```

**Flow Metrics**:
- **Cycle Time** = `completed_at - started_at` (active work time)
- **Lead Time** = `completed_at - created_at` (total time from creation)
- **Throughput** = `completed_cards / period_days * 7` (weekly rate)
- **Flow Efficiency** = `throughput / (WIP * 7 / cycle_time)` (Little's Law derivation)

**Bottleneck Detection**:
- Threshold: column utilization ≥ 80% of WIP limit
- `congestion_score = cards_in_column / wip_limit`
- All columns with score ≥ 0.8 are flagged

### Project Tracker (`project_tracker.h`)

**Data Model**:
```
ProjectTracker
├── ProjectIssue[4096]       — All issues in a flat array
│   ├── type                 — Story, Task, Bug, Epic, Subtask
│   ├── status               — 10-state workflow
│   ├── labels[8]            — Up to 8 labels per issue
│   ├── components[4]        — Up to 4 components per issue
│   └── blocked/block_reason — Impediment tracking
├── ProjectLabel[64]         — Global label pool (color: 0xRRGGBB)
├── ProjectComponent[32]     — Global component pool
├── ProjectWorkflow          — Customizable state machine
│   ├── states[16]           — Ordered list of states
│   └── transitions[16][16]  — Adjacency matrix
└── SprintMetrics[64]        — Per-sprint metrics cache
```

**JQL Filtering** (simplified):
- Supported fields: `type`, `status`, `priority`, `assignee`, `sprint`, `title`, `blocked`
- Supported operators: `=`, `!=`, `>`, `<`, `>=`, `<=`, `IN`, `CONTAINS`, `!CONTAINS`, `IS EMPTY`, `IS NOT EMPTY`
- All conditions AND-ed together (no OR support in this version)
- Field values are integer-coded enums compared as integers

**Workflow Customization**:
- Default: Open → In Progress → Done
- Transitions stored as adjacency matrix `transitions[from][to] = 1`
- Validation: callers should check transition validity before calling `pt_issue_update`

### OKR Tracker (`okr_tracker.h`)

**Data Model**:
```
OKRContext
├── OKRObjective[256]        — Dynamically allocated objectives
│   ├── level                — Company → Department → Team → Individual
│   ├── parent_objective_id  — Alignment upward
│   ├── key_result_ids[8]    — Up to 8 KRs per objective
│   └── overall_grade        — Cached grade (0.0-1.0)
├── OKRKeyResult[1024]       — Dynamically allocated key results
│   ├── start_value / target_value / current_value
│   ├── unit                 — e.g., "users", "%", "USD", "ms"
│   └── weight               — 0.0-1.0 contribution weight within objective
├── OKRCheckin[208]          — Weekly check-in log (52 weeks × 4 objectives)
└── CFREntry[512]            — Conversations, Feedback, Recognition log
```

**Grading Logic**:
```
For each KR in objective:
    progress = (current_value - start_value) / (target_value - start_value)
    progress = clamp(progress, 0.0, 1.0)
    weighted_sum += progress * weight
    total_weight += weight

grade = weighted_sum / total_weight
```
Grade breakdown:
- **overall** = achievement score
- **achievement** = same as overall (KR-weighted completion)
- **progress** = same as achievement
- **learning** = fixed 0.5 (placeholder for qualitative assessment)

**Risk Detection**:
- Objectives with `overall_grade < 0.3` are flagged as at-risk

**Confidence Trend**:
- Average of all check-in `confidence` values for a given objective

### Retrospective (`retrospective.h`)

**Data Model**:
```
RetroContext
├── RetrospectiveSession[16] — Up to 16 retro sessions
│   ├── RetroCard[256]       — Cards with category + text + author
│   └── ActionItem[128]      — Trackable action items
├── ActionItem[]             — Pending actions pool
├── RetroHealthCheck         — Health check questionnaires
├── TeamMoraleSurvey         — Morale survey
└── PsychSafetyAssessment    — Psychological safety assessment
```

**Supported Retro Formats**:
1. **Start/Stop/Continue** — Categories: Start, Stop, Continue
2. **Mad/Sad/Glad** — Categories: Mad, Sad, Glad
3. **4Ls** — Categories: Liked, Learned, Lacked, Longed For
4. **Sailboat** and **KALM** — Enums defined, implementation TBD

**Action Item Tracking**:
- Status: Open → In Progress → Done (or Won't Do)
- `retro_action_item_find_pending` searches across all sessions for incomplete items

**Health Check Scoring**:
- Online mean calculation: `new_avg = (old_avg * count + new_score) / (count + 1)`
- Overall health: average of all answered questions

**Morale Index**:
- Per question: `agree_count / total_responses`
- Overall: average of all question scores

**Psychological Safety Index**:
- Raw average of statement scores (1.0-5.0) normalized to 0.0-1.0
- Safe zone: statements with avg_score ≥ 4.0

## Memory Management

| Context | Allocation | Free Function |
|---------|-----------|---------------|
| `ScrumContext` | `calloc(1, sizeof(...))` | `free(ctx)` |
| `KanbanBoard` | `calloc(1, sizeof(...))` | `free(board)` |
| `ProjectTracker` | `calloc(1, sizeof(...))` | `free(tracker)` |
| `OKRContext` | `calloc` + dynamic arrays for objectives/KRs | `free` all internal arrays + context |
| `RetroHealthCheck` | `calloc(1, sizeof(...))` | Caller must `free()` |
| `TeamMoraleSurvey` | `calloc(1, sizeof(...))` | Caller must `free()` |
| `PsychSafetyAssessment` | `calloc(1, sizeof(...))` | Caller must `free()` |

## Future Enhancements

1. **JSON serialization** — Import/export all contexts as JSON
2. **Persistent storage** — SQLite or file-based persistence layer
3. **WebSocket events** — Real-time board updates
4. **Advanced JQL** — OR conditions, nested queries, ORDER BY
5. **Gantt chart data** — Dependency graph for epic/task planning
6. **Multi-team OKR** — Cross-team alignment visualization
7. **Sentiment analysis** — Retro card NLP for trend detection
8. **Web dashboard** — HTML/CSS dashboard compiled from data structures
