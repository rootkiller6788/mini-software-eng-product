# Architecture of mini-arch-management

## System Context

`mini-arch-management` is a C99 library designed to be embedded into larger software engineering tool chains. It provides an in-process architecture knowledge base rather than relying on external services.

```
┌────────────────────────────────────────────────────┐
│  CI/CD Pipeline    │  IDE Plugin    │  CLI Tool    │
│  (Generate ADRs)   │  (Live Views)  │  (Reports)   │
└─────────┬──────────┴───────┬────────┴──────┬───────┘
          │                  │               │
          └──────────────────┼───────────────┘
                             │ -I include
                    ┌────────▼─────────┐
                    │ mini-arch-mgmt   │
                    │ (C99 Library)    │
                    └────────┬─────────┘
                             │ stdlib only
                    ┌────────▼─────────┐
                    │  Operating       │
                    │  System          │
                    └──────────────────┘
```

## Module Architecture

The library is organised into five independent but composable modules:

```
┌──────────────────────────────────────────────────┐
│                 Application Layer                 │
│         (Custom tools, reports, plugins)          │
├──────────┬──────────┬──────────┬────────┬────────┤
│ ADR      │ C4       │ Arch     │ Tech   │Quality │
│ System   │ Model    │ Review   │ Debt   │Attribs │
│          │          │          │        │        │
│ adr_*.h  │ c4_*.h   │arch_*    │tech_*  │qa_*.h  │
├──────────┴──────────┴──────────┴────────┴────────┤
│              C99 Standard Library                 │
│     <stdio.h> <stdlib.h> <string.h> <time.h>     │
│                  <math.h> <stdbool.h>             │
└──────────────────────────────────────────────────┘
```

### Module Dependencies

| Module | Depends On | Depended By |
|--------|-----------|-------------|
| `adr_system` | (none) | `c4_model`, `arch_review`, `tech_debt` |
| `c4_model` | `adr_system` | — |
| `arch_review` | `adr_system` | — |
| `tech_debt` | (none) | — |
| `quality_attributes` | (none) | — |

Dependencies are minimised. `c4_model` and `arch_review` reference `adr_system` for ADR linking only; `tech_debt` and `quality_attributes` are fully independent.

## Data Flow

### ADR Lifecycle Flow

```
Create ADR ──► Status: Proposed
                   │
           ┌───────┼────────┐
           ▼       ▼        ▼
       Accept   Reject   Supersede
           │       │        │
           ▼       ▼        ▼
      Accepted  (delete)  Superseded
           │
     ┌─────┴─────┐
     ▼           ▼
 Deprecate   Supersede
     │           │
     ▼           ▼
 Deprecated  Superseded
```

### Review Process Flow

```
Schedule Review ──► Add Stakeholders ──► Add Checklist
                                              │
                         ┌────────────────────┤
                         ▼                    ▼
                    Start Review    Schedule Meeting(s)
                         │                    │
                         ▼                    ▼
                    Check Items        Collect Findings
                         │                    │
                         └────────┬───────────┘
                                  ▼
                          Complete Review
                                  │
                                  ▼
                         Export Report
```

### Technical Debt Quantification Flow

```
Identify Code Smell ──► Map to Debt Item ──► Classify Quadrant
                                                   │
                          ┌────────────────────────┤
                          ▼                        ▼
                    Quantify Principal    Set Interest Rate
                          │                        │
                          └───────────┬────────────┘
                                      ▼
                              Acknowledge Debt
                                      │
                                      ▼
                               Plan Payback
                                      │
                                      ▼
                            Start Repayment
                                      │
                                      ▼
                            Resolve / Accept
                                      │
                                      ▼
                           Update Interest Calc
```

## Memory Model

### Static Allocation

All data structures use fixed-size arrays with compile-time configurable limits defined by `MAX_*_LENGTH` and `MAX_*_COUNT` macros. This approach:

1. Eliminates dynamic allocation in steady state
2. Provides predictable memory usage
3. Suitable for resource-constrained environments
4. Simplifies integration — no custom allocator needed

**Estimated memory footprint for a typical configuration:**

| Structure | Size (bytes) |
|-----------|-------------|
| `AdrLog` (256 entries) | ~1.5 MB |
| `C4Model` (32 systems, 64 containers, 128 components, 256 code, 512 rels) | ~2.2 MB |
| `ArchReview` (64 checklist, 64 findings, 32 stakeholders) | ~1.6 MB |
| `TechDebtRegister` (256 items) | ~1.2 MB |
| `QualityModel` (16 profiles, 64 scenarios, 64 trade-offs, 64 SLO, 64 SLI) | ~600 KB |

Total library heap usage: ~7 MB for a fully populated model. Applications can reduce this by lowering `MAX_*` macros.

### Ownership Model

- `AdrLog`, `C4Model`, `ArchReview`, `TechDebtRegister`, and `QualityModel` own their contents
- Users may allocate these on the stack or heap (using `*_create()` / `*_destroy()`)
- Pointers returned by `*_find()` functions point into the owning structure — no separate lifetimes to manage

```c
/* Stack allocation — no free() needed */
AdrLog log;
adr_log_init(&log, "my-project");

/* Heap allocation — must call destroy */
AdrLog *log = adr_log_create("my-project");
adr_log_destroy(log);
```

## Thread Safety

**Current status:** Single-threaded. The library is not thread-safe by default.

For multi-threaded use, wrap API calls with external synchronisation. Future versions may add:
- Per-module `mutex` fields (opt-in via `MAX_*_THREAD_SAFE` macro)
- Read-copy-update (RCU) for read-heavy ADR/C4 workloads

## Error Handling

The library uses a consistent error reporting pattern:

| Return Type | Success | Failure |
|------------|---------|---------|
| `unsigned int` (ID) | Non-zero ID | 0 |
| `bool` | `true` | `false` |
| Pointer | Non-NULL | `NULL` |

No `errno` is set. All functions are signal-safe.

### Common Failure Reasons

1. **Capacity exhausted** — `MAX_*` limit reached
2. **Invalid ID** — Referenced element not found
3. **State transition violation** — Attempting invalid status change
4. **NULL pointer** — Required pointer argument is NULL
5. **Validation failure** — Referential integrity check failed

## Extensibility

### Adding a New Module

1. Create `include/new_module.h` with `#ifndef NEW_MODULE_H` guard
2. Implement `src/new_module.c` with matching function signatures
3. Add compile rules to `Makefile`
4. Add API overview to `README.md`
5. Add example in `examples/`

### Configuring Limits

Override `MAX_*` macros before including headers:

```c
#define ADR_MAX_ENTRIES 512  /* Double default capacity */
#include "adr_system.h"
```

## Platform Support

| Platform | Status |
|----------|--------|
| Linux (GCC/Clang) | Supported |
| macOS (GCC/Clang) | Supported |
| Windows (MinGW/MSVC) | Supported (MinGW recommended) |
| Embedded (ARM GCC) | Supported (adjust MAX_* for memory) |
| WASM (Emscripten) | Supported (no filesystem export) |

C99 is the minimum standard. C11 features (`stdatomic.h`, `_Generic`) are used only when `C11_EXTENSIONS` is defined.
