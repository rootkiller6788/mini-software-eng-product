# mini-arch-management — 架构管理 (C 语言实现)

## Overview

`mini-arch-management` is a C99 library for software architecture management. It provides five core modules for documenting, reviewing, and governing the architecture of software-intensive systems.

## Modules

| Module | Header | Description |
|--------|--------|-------------|
| ADR System | `adr_system.h` | Architecture Decision Records with status lifecycle and supersede tracking |
| C4 Model | `c4_model.h` | Hierarchical architecture diagrams (Context→Container→Component→Code) with relationship management |
| Architecture Review | `arch_review.h` | Structured review process with checklist, stakeholders, findings, and decision log integration |
| Technical Debt | `tech_debt.h` | Debt register with interest quantification, code smell mapping, payback planning, and quadrant classification |
| Quality Attributes | `quality_attributes.h` | ATAM-style trade-off analysis, SLA/SLO/SLI mapping, and quality scenario testing |

## Building

```bash
make
```

This compiles all source files in `src/` into `bin/`, linking with the headers in `include/`.

### Build Options

| Target | Description |
|--------|-------------|
| `make` | Build all objects and examples |
| `make examples` | Build example programs into `bin/examples/` |
| `make clean` | Remove build artifacts |
| `make rebuild` | Clean and rebuild from scratch |
| `make debug` | Build with `-g -O0` for debugging |

### Compiler Flags

```
CC      = gcc
CFLAGS  = -Wall -Wextra -O2 -I include
OUTDIR  = bin
```

## Usage Examples

See the `examples/` directory for complete runnable programs:

- `example_adr.c` — Demonstrates ADR creation, status transitions, supersede chains, tag management, and Markdown export.
- `example_c4.c` — Builds a complete C4 model for an online banking platform across all four abstraction levels, with PlantUML export.
- `example_tech_debt.c` — Comprehensive example integrating all five modules: technical debt register, architecture review lifecycle, and quality attribute trade-off analysis.

```bash
make examples
./bin/examples/example_adr
./bin/examples/example_c4
./bin/examples/example_tech_debt
```

## API Documentation

### ADR System (`include/adr_system.h`)

Architecture Decision Records provide a lightweight but structured way to document architectural choices.

**Core Types:**
- `AdrStatus` — Proposed, Accepted, Deprecated, Superseded
- `AdrRecord` — Full ADR entry with context, decision, consequences
- `AdrLog` — Ordered collection of ADRs with project metadata

**Key Functions:**
```c
unsigned int adr_log_add_record(AdrLog *log, ...);
bool adr_log_accept(AdrLog *log, unsigned int id);
bool adr_log_supersede(AdrLog *log, unsigned int old_id, unsigned int new_id);
bool adr_log_export_markdown(const AdrLog *log, const char *filename);
```

### C4 Model (`include/c4_model.h`)

Implements the [C4 model](https://c4model.com/) for visualising software architecture at four levels:

1. **Context** — System + Users (people and external systems)
2. **Container** — Applications and data stores within a system
3. **Component** — Modules within a container
4. **Code** — Classes and interfaces within a component

**Key Functions:**
```c
unsigned int c4_model_add_person(C4Model *model, ...);
unsigned int c4_model_add_system(C4Model *model, ...);
unsigned int c4_model_add_relationship(C4Model *model, ...);
bool c4_model_export_plantuml(const C4Model *model, const char *filename, C4Level level);
```

### Architecture Review (`include/arch_review.h`)

Structured architecture review process with stakeholder management, checklist tracking, risk-based findings, and meeting documentation.

**Key Functions:**
```c
unsigned int arch_review_add_finding(ArchReview *review, ...);
bool arch_review_add_checklist_item(ArchReview *review, ...);
double arch_review_progress(const ArchReview *review);
bool arch_review_export_report(const ArchReview *review, const char *filename);
```

### Technical Debt (`include/tech_debt.h`)

Quantitative technical debt management based on the metaphor of financial debt:

- **Principal** — Estimated effort to fix the debt
- **Interest rate** — Additional cost incurred per period the debt remains
- **Quadrant** — Deliberate/Inadvertent × Prudent/Reckless (Martin Fowler's classification)
- **Code smells** — Links debt items to specific code locations and smell types

**Key Functions:**
```c
unsigned int tech_debt_register_add_item(TechDebtRegister *register_ptr, ...);
double tech_debt_calculate_accumulated_interest(const TechDebtItem *item, time_t current_date);
double tech_debt_register_total_principal(const TechDebtRegister *register_ptr);
bool tech_debt_item_is_deliberate(const TechDebtItem *item);
```

### Quality Attributes (`include/quality_attributes.h`)

ATAM (Architecture Tradeoff Analysis Method) — inspired quality attribute analysis:

- **Quality scenarios** — Stimulus/response scenarios per quality attribute
- **Trade-off analysis** — Pairwise comparison of competing quality attributes
- **SLA** — Service Level Agreements (business commitments)
- **SLO** — Service Level Objectives (internal targets)
- **SLI** — Service Level Indicators (measured metrics)

**Key Functions:**
```c
unsigned int quality_model_add_scenario(QualityModel *model, ...);
unsigned int quality_model_add_trade_off(QualityModel *model, ...);
double quality_model_get_overall_score(const QualityModel *model);
bool quality_model_export_report(const QualityModel *model, const char *filename);
```

## Design Principles

1. **C99 Standard** — Portable across compilers and platforms, including embedded systems
2. **Header-only API** — All public types and functions declared in `include/` with `#ifndef` guards
3. **No external dependencies** — Only C standard library (`<stdio.h>`, `<stdlib.h>`, `<string.h>`, `<time.h>`, `<math.h>`)
4. **Static allocation** — Fixed-size arrays with configurable `MAX_*` macros; no dynamic allocation required for core operations
5. **snake_case functions** — All API functions use `module_action_subject` naming
6. **PascalCase types** — All struct, enum, and typedef names use PascalCase
7. **UPPER_SNAKE_CASE macros** — All constants, limits, and configuration values

## Project Structure

```
mini-arch-management/
├── include/               # Public headers
│   ├── adr_system.h
│   ├── c4_model.h
│   ├── arch_review.h
│   ├── tech_debt.h
│   └── quality_attributes.h
├── src/                   # Implementation
│   ├── adr_system.c
│   ├── c4_model.c
│   ├── arch_review.c
│   ├── tech_debt.c
│   └── quality_attributes.c
├── examples/              # Runnable examples
│   ├── example_adr.c
│   ├── example_c4.c
│   └── example_tech_debt.c
├── demos/                 # Demonstration walkthroughs
│   ├── README_ADR.md
│   └── README_C4.md
├── docs/                  # Design and architecture docs
│   ├── ARCHITECTURE.md
│   └── DESIGN.md
├── Makefile
└── README.md
```

## Integration

To use in your own project:

```makefile
ARCH_MGMT_DIR = path/to/mini-arch-management
CFLAGS += -I$(ARCH_MGMT_DIR)/include
LDFLAGS += -L$(ARCH_MGMT_DIR)/bin -larch_mgmt
```

Or compile directly:

```bash
gcc -I include -c src/adr_system.c -o adr_system.o
gcc -I include my_program.c adr_system.o -o my_program
```

## License

MIT
