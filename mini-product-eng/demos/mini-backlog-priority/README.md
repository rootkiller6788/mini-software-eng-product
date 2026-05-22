# mini-backlog-priority — Backlog Prioritization

## Goal

Implement multiple backlog prioritization strategies — by priority, by value/effort ratio, and sprint capacity planning.

## Steps

1. Create a backlog with velocity=20 and a mix of items (stories, bugs, spikes)
2. Sort by priority — critical items first
3. Sort by value/effort ratio (ROI) — maximize business value per point
4. Calculate sprint capacity — how many priority-ordered items fit
5. Accept completed items with actual hours, compare to estimates
6. Block an item and observe it remains un-schedulable

## Key APIs

- `backlog_add()` — Add item with priority, type, points, business value
- `backlog_sort_by_priority()` — qsort by Priority enum
- `backlog_sort_by_value_effort()` — qsort by business_value/story_points descending
- `backlog_sprint_capacity()` — Greedy fill by priority up to velocity
- `backlog_accept()` — Mark done, record actual hours
- `backlog_block()` — Mark blocked

## Extensions

- Add WSJF (Weighted Shortest Job First) = cost_of_delay / job_size
- Implement ICE scoring (Impact, Confidence, Ease)
- Track velocity over multiple sprints with moving average
