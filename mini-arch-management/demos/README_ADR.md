# ADR System Demo — Architecture Decision Records

## Introduction

Architecture Decision Records (ADRs) document the "why" behind architectural choices. This demo walks through the full lifecycle of an ADR using `mini-arch-management`.

## Quick Start

```bash
cd examples
make
./bin/examples/example_adr
```

## Creating an ADR Log

An `AdrLog` is a container for all ADRs in a project. Initialise it with a project name:

```c
#include "adr_system.h"

int main(void) {
    AdrLog log;
    adr_log_init(&log, "e-commerce-platform");
    /* ... */
}
```

## Adding Records

Each ADR has five required fields: title, context, decision, consequences, and stakeholder. The `adr_log_add_record` function returns a unique numeric ID:

```c
unsigned int adr_id = adr_log_add_record(&log,
    "Use PostgreSQL for primary data store",
    "The platform requires a relational database with ACID compliance, "
    "strong consistency guarantees, and rich indexing. Options: "
    "PostgreSQL, MySQL, MongoDB.",
    "Adopt PostgreSQL 15 as the primary data store. Use JSONB columns "
    "for semi-structured product catalogue data, and standard relational "
    "schema for orders and payments.",
    "Positive: Excellent SQL compliance, JSONB flexibility, robust "
    "replication. Negative: Operational complexity for multi-region "
    "deployments requires additional tooling (Patroni).",
    "Architecture Team");
```

All ADRs start with status `ADR_STATUS_PROPOSED`.

## Status Lifecycle

ADRs follow a strict state machine:

```
            ┌──────────┐
            │ Proposed │
            └────┬─────┘
                 │ accept()
            ┌────▼──────┐
            │ Accepted  │
            └─┬────┬────┘
              │    │ supersede()
    deprecate()│    └──────────┐
              │               │
         ┌────▼─────┐   ┌─────▼──────┐
         │Deprecated│   │ Superseded │
         └──────────┘   └────────────┘
```

```c
adr_log_accept(&log, adr_id);       /* Move to Accepted */
adr_log_deprecate(&log, adr_id);    /* Move to Deprecated */
adr_log_supersede(&log, old_id, new_id); /* Mark old as Superseded */
```

### Rules

- Only `Proposed` ADRs can be accepted
- An ADR cannot be deprecated if already superseded
- Superseding links both the old (superseded) and new (supersedes) ADRs

## Supersede Chains

When a decision is replaced, the supersede chain preserves the full history:

```c
AdrSupersedeChain chain;
if (adr_log_get_supersede_chain(&log, 1, &chain)) {
    printf("Chain: ");
    for (size_t i = 0; i < chain.length; i++) {
        printf("ADR #%04u -> ", chain.chain[i]);
    }
}
```

Output:
```
Chain: ADR #0001 -> ADR #0004 -> ADR #0007
```

This allows auditing the evolution of architectural decisions over time.

## Tagging

Organise ADRs with tags for filtering and categorisation:

```c
adr_record_add_tag(adr_log_find(&log, adr_id), "database");
adr_record_add_tag(adr_log_find(&log, adr_id), "storage-strategy");
adr_record_add_tag(adr_log_find(&log, adr_id), "performance");

bool is_db = adr_record_has_tag(adr_log_find(&log, adr_id), "database");
/* true */

adr_record_remove_tag(adr_log_find(&log, adr_id), "performance");
```

## Querying

Find ADRs by ID, title, or status:

```c
AdrRecord *rec = adr_log_find(&log, 3);              /* By ID */
AdrRecord *rec = adr_log_find_by_title(&log, "Title"); /* By title */

size_t count;
AdrRecord *accepted = adr_log_find_by_status(&log,
    ADR_STATUS_ACCEPTED, &count);
/* Iterate accepted[0] through accepted[count-1] */

size_t total = adr_log_count_total(&log);
size_t deprecated = adr_log_count_by_status(&log, ADR_STATUS_DEPRECATED);
```

## Export to Markdown

Generate a Markdown document suitable for version control:

```c
adr_log_export_markdown(&log, "docs/adr/decisions.md");
```

This creates a document with a summary table and full detail for each ADR:

```markdown
# Architecture Decision Log: e-commerce-platform

| ID | Title | Status | Stakeholder |
|----|-------|--------|-------------|
| 0001 | Use PostgreSQL for primary data store | Accepted | Architecture Team |
| 0002 | Implement CQRS for order processing | Proposed | Backend Team |

## ADR #0001: Use PostgreSQL for primary data store

**Status:** Accepted

### Context
...

### Decision
...

### Consequences
...
```

## Validation

Verify the integrity of an ADR log:

```c
if (adr_log_validate(&log)) {
    printf("ADR log is valid\n");
} else {
    fprintf(stderr, "ADR log validation failed\n");
}
```

## Best Practices

1. **Write ADRs at decision time**, not retrospectively — the context decays quickly
2. **Be specific in consequences** — include both positive and negative effects
3. **Link superseded ADRs** — never delete old decisions; the history is valuable
4. **Use consistent numbering** — sequential IDs provide a natural timeline
5. **Tag consistently** — agree on a tag taxonomy (e.g., `performance`, `security`, `cost`, `operational`)
6. **Include the stakeholder** — identifies who made the decision
7. **Version control ADRs** — treat them like code; review changes in pull requests
8. **Keep context brief but complete** — enough for someone joining 2 years later to understand
9. **One decision per ADR** — avoid bundling multiple decisions in a single record
10. **Reference related ADRs and architecture elements** — cross-link via tags or description text

## Template

When creating ADRs, follow this structure:

```markdown
# ADR #NNNN: [Title]

**Status:** Proposed | Accepted | Deprecated | Superseded

**Date:** YYYY-MM-DD

**Stakeholder:** [Name/Team]

## Context

[Describe the forces at play: technical constraints, business requirements,
 team capabilities, timelines, existing architecture.]

## Decision

[State the decision in active voice: "We will..."]

## Consequences

### Positive
- [List benefits]

### Negative
- [List trade-offs, costs, risks]

### Neutral
- [List effects that are neither clearly positive nor negative]
```

## Further Reading

- [Documenting Architecture Decisions](https://cognitect.com/blog/2011/11/15/documenting-architecture-decisions) — Michael Nygard (2011)
- [ADR GitHub Organization](https://adr.github.io/) — Community ADR resources and tooling
- [ThoughtWorks Technology Radar: Lightweight Architecture Decision Records](https://www.thoughtworks.com/radar/techniques/lightweight-architecture-decision-records)
