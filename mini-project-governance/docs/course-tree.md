# Course Dependency Tree — Project Governance

## Prerequisites
This module depends on knowledge from:

```
Mathematics (Module 0)
  └── Statistics (mean, median, std dev, linear regression)

Data Structures
  └── Arrays, linked structures (OKR tree)
  └── Queues (Kanban FIFO flow)
  └── State machines (Sprint, Workflow)

Algorithms
  └── Sorting (qsort-based prioritization)
  └── Random sampling (Monte Carlo simulation)
  └── Linear regression (velocity trend)

Software Engineering Fundamentals
  └── Project lifecycle concepts
  └── Team organization patterns
```

## Internal Dependencies

```
gov_governance.h
  ├── gov_scrum.h      (ScrumBoard, Sprint, PBI)
  ├── gov_kanban.h     (KanbanBoard, CFD)
  ├── gov_okr.h        (OkrDashboard, OkrTree)
  ├── gov_tracker.h    (IssueTracker, WorkflowEngine)
  ├── gov_velocity.h   (VelocityProfile, Forecast)
  └── gov_retro.h      (RetroLog, FiveWhys)
```

## Concepts Used By Other Modules

```
Project Governance → Software Testing (Module 18)
  Sprint tracking → Test sprint planning
  
Project Governance → Backend API (Module 8)
  OKR metrics → API performance KPIs
  
Project Governance → Data Engine (Module 7)
  Velocity data → Time-series storage patterns
```