# Design Document — mini-arch-management

## Design Philosophy

`mini-arch-management` is designed around six principles:

1. **Correctness over performance** — Produce correct architecture data first; optimise later
2. **Explicit over implicit** — Every transition, state, and relationship is visible in the API
3. **Composable modules** — Each module works independently; integration is additive
4. **Zero external dependencies** — Only C99 standard library
5. **Pragmatic completeness** — Cover 80% of real-world architecture management needs with 20% of the complexity
6. **Readable code as documentation** — Function names and struct fields should be self-documenting

## Naming Conventions

| Convention | Scope | Example |
|------------|-------|---------|
| `snake_case` | Functions, file names, local variables | `adr_log_add_record()`, `tech_debt.h` |
| `PascalCase` | Types, structs, enums, typedefs | `AdrLog`, `C4RelationshipType` |
| `UPPER_SNAKE_CASE` | Macros, constants, enum values | `ADR_MAX_TITLE_LENGTH`, `C4_LEVEL_CONTEXT` |
| `module_prefix_` | All public identifiers | `adr_log_`, `c4_model_`, `arch_review_`, `tech_debt_`, `quality_model_` |

### Module Prefixes

| Module | Prefix |
|--------|--------|
| ADR System | `adr_` |
| C4 Model | `c4_` |
| Architecture Review | `arch_` |
| Technical Debt | `tech_debt_` |
| Quality Attributes | `quality_model_`, `qa_` |

## Type Design

### Enums

All enums are `typedef`'d and use the module name as a prefix:

```c
typedef enum AdrStatus {
    ADR_STATUS_PROPOSED,
    ADR_STATUS_ACCEPTED,
    ADR_STATUS_DEPRECATED,
    ADR_STATUS_SUPERSEDED
} AdrStatus;
```

Enum values follow `MODULE_CATEGORY_VALUE` pattern.

### Structs

Structs are designed for direct member access (no opaque types). Fields are grouped logically:

1. **Identity** — `id`, `name`
2. **Classification** — `status`, `type`, `quadrant`
3. **Content** — `title`, `description`, `context`
4. **Relations** — `*_ids[]`, `*_count`
5. **Timestamps** — `created_at`, `updated_at`, `*_date`
6. **Collections** — `entries[]`, `items[]`, `count`

### Arrays vs Linked Lists

Fixed-size arrays are preferred over linked lists for:
- Cache-friendly iteration
- Predictable memory access patterns
- Bounds checking via `MAX_*` macros
- No pointer-chasing overhead

The trade-off is a hard limit on capacity, which is configurable at compile time.

## API Design Patterns

### Lifecycle Pattern

Modules with state-managed entities follow a consistent lifecycle:

```c
/* Construction */
void   *_init(Struct *s, ...);       /* In-place initialisation */
Struct *_create(...);                /* Heap allocation + initialisation */
void    _destroy(Struct *s);         /* Heap deallocation */

/* Mutation */
bool    _add_*(Struct *s, ...);      /* Returns success/failure */
bool    _remove_*(Struct *s, ...);   /* Returns success/failure */
bool    _transition(Struct *s, ...); /* State machine transition */

/* Query */
Type   *_find(Struct *s, id);        /* Returns pointer or NULL */
size_t  _count(const Struct *s);     /* Cardinality query */
double  _calculate(const Struct *s); /* Derived computation */

/* Output */
void    _print(const Struct *s);     /* stdout display */
bool    _export(const Struct *s, filename); /* File export */
bool    _validate(const Struct *s);  /* Integrity check */
```

### State Machine Pattern

Status transitions are validated before execution:

```c
bool adr_log_accept(AdrLog *log, unsigned int id) {
    AdrRecord *rec = adr_log_find(log, id);
    if (!rec) return false;
    if (rec->status != ADR_STATUS_PROPOSED) return false; /* Guard */
    rec->status = ADR_STATUS_ACCEPTED;                     /* Transition */
    rec->updated_at = time(NULL);                          /* Side effect */
    return true;
}
```

States form a directed acyclic graph. Invalid transitions return `false`.

### Builder Pattern

Entities with many optional fields use a create-then-configure pattern:

```c
unsigned int id = adr_log_add_record(&log, title, context, decision,
                                      consequences, stakeholder);
/* Now configure optional fields */
AdrRecord *rec = adr_log_find(&log, id);
adr_record_add_tag(rec, "security");
adr_record_add_tag(rec, "high-priority");
```

## Data Structure Design

### AdrLog

```
AdrLog
├── project_name
├── created_at
├── next_id: unsigned int (monotonic counter)
├── count: size_t
└── entries[256]: AdrRecord[]
    ├── AdrRecord[0]
    │   ├── id: 1
    │   ├── title: "Use C99"
    │   ├── status: ACCEPTED
    │   ├── supersedes_ids[8]: [0] (none)
    │   └── tags[16]: ["language", "portability"]
    ├── AdrRecord[1]
    │   ├── id: 2
    │   ├── status: SUPERSEDED
    │   └── superseded_by_id: 4
    └── ...
```

ID assignment: `next_id` is incremented on every `add_record` call. IDs are never reused.

Supersede tracking: `superseded_by_id` is a single back-pointer; `supersedes_ids[]` is a forward list. This forms a linked list that can be traversed in either direction.

### C4Model

```
C4Model
├── workspace_name
├── next_id
├── people[32]: C4Person[]
├── systems[32]: C4System[]
│   └── adr_links[16]: C4AdrLink[]
├── containers[64]: C4Container[]
│   ├── system_id ────► C4System.id
│   └── adr_links[16]
├── components[128]: C4Component[]
│   ├── container_id ─► C4Container.id
│   └── adr_links[16]
├── code_elements[256]: C4CodeElement[]
│   └── component_id ─► C4Component.id
└── relationships[512]: C4Relationship[]
    └── adr_links[16]
```

Parent-child relationships use foreign-key-style unsigned integers. Validation traverses these to verify referential integrity.

The four C4 levels are implicit in the data structure: `system -> container -> component -> code_element`.

### TechDebtRegister

```
TechDebtRegister
├── base_interest_rate: double
├── last_updated: time_t
├── items[256]: TechDebtItem[]
    ├── TechDebtItem
    │   ├── quadrant: TechDebtQuadrant
    │   ├── principal_hours: double
    │   ├── interest_rate_percent: double
    │   ├── annual_interest_hours: double (computed)
    │   ├── code_smells[8]: CodeSmell[]
    │   │   ├── type: CodeSmellType
    │   │   ├── location: CodeLocation
    │   │   │   ├── file_path
    │   │   │   ├── line_start
    │   │   │   └── line_end
    │   │   └── severity: double
    │   └── payback_plan[16]: PaybackStep[]
    │       ├── estimated_effort_hours: double
    │       ├── interest_reduction_percent: double
    │       └── is_completed: bool
```

Interest calculation uses simple interest: `accumulated = principal × rate × years`. Compound interest could be added as a configurable option.

### QualityModel

```
QualityModel
├── profiles[16]: QualityAttributeProfile[]
│   ├── type: QualityAttributeType
│   ├── target_score: double
│   ├── current_score: double (computed)
│   ├── scenarios[64]: QualityScenario[]
│   │   ├── stimulus, source, environment, artifact
│   │   ├── response, response_measure
│   │   └── is_met: bool
│   └── sla_items[32]: SlaItem[]
├── trade_offs[64]: TradeOffAnalysis[]
│   ├── attribute_a, attribute_b
│   ├── weight_a, weight_b
│   └── is_resolved: bool
├── slo_items[64]: SloItem[]
└── sli_items[64]: SliItem[]
```

The `current_score` of a profile is the percentage of tested scenarios that are met. The `overall_score` is the mean of all profile scores.

Trade-off analysis uses weighted scoring: `weighted_a = score_a × weight_a`. Resolution records the scores and rationale.

## File Format Design

### ADR Markdown Export

```
# Architecture Decision Log: {project_name}

| ID | Title | Status | Stakeholder |
|----|-------|--------|-------------|
| ... |

## ADR #NNNN: {title}
**Status:** {status}
### Context
{context}
### Decision
{decision}
### Consequences
{consequences}
```

### PlantUML Export

```
@startuml
title {workspace_name} - {level}
{level_specific_entities}
{relationships}
@enduml
```

### Tech Debt CSV Export

```
ID,Name,Quadrant,Status,PrincipalHours,InterestRate,AnnualInterest,AccumulatedInterest,Owner
1,Duplicated validation,"Inadvertent & Prudent",In Repayment,80.0,12.0,9.6,2.4,backend-team
```

### Architecture Review Report (Markdown)

```
# Architecture Review Report
## {title}
**Status:** {status}
## Stakeholders
| Name | Role | Decision Maker | Veto |
| ...
## Findings
| ID | Title | Risk | Resolved |
| ...
## Checklist
| Item | Category | Met |
| ...
```

## Future Considerations

1. **JSON export** — Machine-readable export format for integration with other tools
2. **YAML/TOML config** — Load architectural models from configuration files
3. **Diff engine** — Compare two models and generate change reports
4. **Graph analysis** — Dependency graph algorithms (cyclomatic complexity, coupling metrics)
5. **Plugin API** — Dynamic loading of custom validators and exporters
6. **Persistent storage** — SQLite or LMDB backend for large models
7. **Network protocol** — gRPC or REST API for distributed architecture management
8. **Visualisation** — ASCII art diagram rendering without PlantUML dependency
9. **Version compatibility matrix** — Track which component versions are compatible
10. **Cost modelling** — Extend debt interest model to infrastructure cost estimation
