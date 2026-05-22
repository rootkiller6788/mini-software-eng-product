# mini-product-roadmap — Product Roadmap Builder

## Goal

Build a quarterly product roadmap with themes, epics, and features, tracking status from planned through delivery.

## Steps

1. Initialize roadmap with a product vision statement
2. Add 2-3 strategic themes (e.g., "Performance", "User Growth")
3. Add items to each theme with quarter targets and effort estimates
4. Update item statuses as development progresses
5. Calculate items and total effort per quarter
6. Print the roadmap and verify quarter capacity

## Key APIs

- `roadmap_init()` — Set vision, year, quarter
- `roadmap_add_theme()` — Add strategic theme area
- `roadmap_add_item()` — Add epic/feature/story with quarter target
- `roadmap_update_status()` — Move through PLANNED→IN_PROGRESS→DELIVERED
- `roadmap_items_in_quarter()` / `roadmap_total_effort()` — Capacity planning

## Extensions

- Add dependency validation (check depends_on exists)
- Calculate critical path across items
- Support now/next/later time horizons
