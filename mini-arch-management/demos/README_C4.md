# C4 Model Demo — Architecture Visualisation

## Introduction

The C4 model is a hierarchical approach to visualising software architecture, created by Simon Brown. `mini-arch-management` implements all four C4 levels and provides PlantUML export.

## Quick Start

```bash
cd examples
make
./bin/examples/example_c4
```

## The Four Levels

### Level 1: Context Diagram

The highest level shows the system as a single box surrounded by users and external systems:

```c
C4Model model;
c4_model_init(&model, "E-Commerce Platform", "Online retail system");

c4_model_add_person(&model, "Customer", "Online shopper", false);
c4_model_add_person(&model, "Warehouse Staff", "Fulfilment team", false);

unsigned int ecommerce = c4_model_add_system(&model,
    "E-Commerce Platform", "Web-based retail system", "Cloud", false);

c4_model_add_system(&model,
    "Payment Gateway", "External payment processor", "External", true);
c4_model_add_system(&model,
    "Shipping Provider API", "External logistics integration",
    "External", true);

c4_model_add_relationship(&model,
    "Customer", "E-Commerce Platform", C4_REL_USES,
    "Browses products, places orders", "HTTPS");
c4_model_add_relationship(&model,
    "E-Commerce Platform", "Payment Gateway", C4_REL_USES,
    "Processes payments", "REST API");
c4_model_add_relationship(&model,
    "E-Commerce Platform", "Shipping Provider API", C4_REL_USES,
    "Creates shipments, tracks deliveries", "REST API");
```

**When to use:** Presenting to non-technical stakeholders, system landscape overview, architecture decision context.

### Level 2: Container Diagram

Zoom in to show applications, data stores, and their interactions:

```c
unsigned int web_app = c4_model_add_container(&model,
    "Web Application", "React SPA served by Nginx",
    "React + Nginx", ecommerce);

unsigned int api = c4_model_add_container(&model,
    "REST API", "Backend API serving all frontend requests",
    "Java + Spring Boot", ecommerce);

unsigned int order_db = c4_model_add_container(&model,
    "Order Database", "Stores orders, line items, and status",
    "PostgreSQL 15", ecommerce);

unsigned int product_db = c4_model_add_container(&model,
    "Product Database", "Stores catalogue, pricing, and inventory",
    "MongoDB 6", ecommerce);

unsigned int cache = c4_model_add_container(&model,
    "Redis Cache", "Caches product listings and session data",
    "Redis 7", ecommerce);

c4_model_add_relationship(&model,
    "Web Application", "REST API", C4_REL_CALLS,
    "Makes API calls", "HTTPS/REST");

c4_model_add_relationship(&model,
    "REST API", "Order Database", C4_REL_STORED_IN,
    "Reads/writes order data", "JDBC");

c4_model_add_relationship(&model,
    "REST API", "Cache", C4_REL_STORED_IN,
    "Stores/retrieves cached data", "Lettuce client");
```

**When to use:** Technical team discussions, deployment planning, technology stack decisions.

### Level 3: Component Diagram

Decompose a container into components:

```c
unsigned int product_ctrl = c4_model_add_component(&model,
    "Product Controller", "REST endpoints for product CRUD",
    "Spring MVC", api);

unsigned int product_svc = c4_model_add_component(&model,
    "Product Service", "Business logic for product operations",
    "Spring Service", api);

unsigned int product_repo = c4_model_add_component(&model,
    "Product Repository", "Data access for product persistence",
    "Spring Data JPA", api);

unsigned int search_svc = c4_model_add_component(&model,
    "Search Service", "Full-text search with faceting",
    "Elasticsearch Client", api);

c4_model_add_relationship(&model,
    "Product Controller", "Product Service",
    C4_REL_CALLS, "Delegates business logic", "Java method call");

c4_model_add_relationship(&model,
    "Product Service", "Product Repository",
    C4_REL_CALLS, "Persists and retrieves products", "JPA");

c4_model_add_relationship(&model,
    "Product Service", "Search Service",
    C4_REL_CALLS, "Indexes and queries product catalogue",
    "Elasticsearch REST");
```

**When to use:** Detailed design reviews, onboarding new developers, identifying refactoring targets.

### Level 4: Code Diagram

Drill into individual classes/interfaces:

```c
c4_model_add_code_element(&model,
    "ProductController", "Maps HTTP requests to service calls",
    "Java", product_ctrl);

c4_model_add_code_element(&model,
    "ProductServiceImpl", "Implements product business rules",
    "Java", product_svc);

c4_model_add_code_element(&model,
    "ProductRepository", "JPA repository with custom queries",
    "Java", product_repo);

c4_model_add_code_element(&model,
    "ElasticsearchProductDao", "Elasticsearch index management",
    "Java", search_svc);
```

**When to use:** Code reviews, architecture compliance checks, detailed documentation.

## Relationship Types

Nine relationship types model different interaction patterns:

| Type | Code | Description |
|------|------|-------------|
| Uses | `C4_REL_USES` | Generic usage relationship |
| Depends on | `C4_REL_DEPENDS_ON` | Compile-time or runtime dependency |
| Calls | `C4_REL_CALLS` | Synchronous RPC/API call |
| Publishes to | `C4_REL_PUBLISHES_TO` | Publishes events or messages |
| Subscribes to | `C4_REL_SUBSCRIBES_TO` | Consumes events or messages |
| Sends to | `C4_REL_SENDS_TO` | Asynchronous message sending |
| Stored in | `C4_REL_STORED_IN` | Data persistence relationship |
| Deployed on | `C4_REL_DEPLOYED_ON` | Deployment target relationship |
| Owned by | `C4_REL_OWNED_BY` | Organisational ownership |

## Linking Architecture Decisions

Connect C4 elements to ADRs to trace design rationale:

```c
c4_model_link_adr_to_container(&model, order_db, 3,
    "ADR #3: PostgreSQL selected for ACID compliance on order data");
c4_model_link_adr_to_component(&model, search_svc, 5,
    "ADR #5: Elasticsearch over Solr for real-time indexing");
c4_model_link_adr_to_system(&model, ecommerce, 1,
    "ADR #1: Monolith-first strategy with modular boundaries");
```

## Exporting PlantUML

Generate diagrams in PlantUML format for rendering:

```c
c4_model_export_plantuml(&model, "context.puml", C4_LEVEL_CONTEXT);
c4_model_export_plantuml(&model, "container.puml", C4_LEVEL_CONTAINER);
```

The output includes proper C4 PlantUML syntax:

```plantuml
@startuml
title E-Commerce Platform - Context

Person(Customer, "Customer", "Online shopper")

System(E-Commerce_Platform, "E-Commerce Platform", "Web-based retail system")

System_Ext(Payment_Gateway, "Payment Gateway", "External payment processor")

Rel(Customer, E-Commerce_Platform, "Uses", "Browses products, places orders")
Rel(E-Commerce_Platform, Payment_Gateway, "Uses", "Processes payments")
@enduml
```

You can render these with the PlantUML CLI, VS Code extension, or the [PlantUML online server](https://www.plantuml.com/plantuml/).

## Validation

Check C4 model integrity, including referential integrity for parent-child relationships:

```c
if (c4_model_validate(&model)) {
    printf("C4 model is valid\n");
}
```

Validation checks:
- Counts do not exceed `MAX_*` limits
- Every container references a valid system
- Every component references a valid container
- Every code element references a valid component

## Query Functions

```c
C4System *sys = c4_model_find_system(&model, system_id);
C4Container *c = c4_model_find_container(&model, container_id);
C4Component *comp = c4_model_find_component(&model, component_id);

size_t n = c4_model_get_container_count_for_system(&model, system_id);
size_t m = c4_model_get_component_count_for_container(&model, container_id);
```

## Printing

Convenience functions for stdout output:

```c
c4_model_print_context(&model);    /* System + user overview */
c4_model_print_container(&model);  /* All containers grouped by system */
c4_model_print_component(&model, container_id); /* Components of a container */
c4_model_print_code(&model, component_id);    /* Code elements of a component */
```

## Best Practices

1. **Start at the top** — always begin with Context, then drill down as needed
2. **Don't show everything** — each diagram should tell one coherent story
3. **Consistent naming** — use the same names across all levels
4. **Include technology choices** — especially at Container and Component levels
5. **Annotate relationships** — describe the purpose, not just the protocol
6. **Link to ADRs** — connect architectural elements to the decisions that created them
7. **Keep diagrams current** — regenerate from code during CI/CD
8. **Use PlantUML as code** — version control `.puml` files alongside source code
9. **Boundary labels** — group containers within the same system visually
10. **External flags** — clearly mark systems and users that are outside your control

## Further Reading

- [The C4 Model for Visualising Software Architecture](https://c4model.com/) — Simon Brown
- [PlantUML C4 Library](https://github.com/plantuml-stdlib/C4-PlantUML)
- [Structurizr DSL](https://structurizr.com/dsl) — C4 model as code
