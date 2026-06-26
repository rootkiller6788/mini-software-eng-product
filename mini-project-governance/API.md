# API Reference

## 1. Agile Scrum (`agile_scrum.h`)

### Context Management
| Function | Description |
|----------|-------------|
| `scrum_context_create()` | Allocate and initialize a Scrum context |
| `scrum_context_free(ctx)` | Free the Scrum context |

### Sprint Management
| Function | Description |
|----------|-------------|
| `scrum_sprint_create(ctx, name, goal, duration_days)` | Create a new sprint. Returns sprint pointer |
| `scrum_sprint_start(ctx, sprint_id)` | Start a sprint (status → ACTIVE, records start_date) |
| `scrum_sprint_complete(ctx, sprint_id)` | Complete a sprint, calculate velocity |
| `scrum_sprint_find(ctx, sprint_id)` | Find sprint by ID |
| `scrum_sprint_print(sprint)` | Print sprint details |

### Product Backlog
| Function | Description |
|----------|-------------|
| `scrum_backlog_add(ctx, title, desc, points, priority)` | Add item to product backlog |
| `scrum_backlog_remove(ctx, item_id)` | Remove item from backlog |
| `scrum_backlog_refine(ctx, output_path)` | Export refined backlog to file or stdout |
| `scrum_sprint_plan(ctx, sprint_id, item_ids, count)` | Move backlog items into sprint |

### Daily Standup
| Function | Description |
|----------|-------------|
| `scrum_standup_record(ctx, sprint_id, participant_id, name, yesterday, today, blockers)` | Record a daily standup entry |
| `scrum_standup_print(standup)` | Print standup entry |

### Sprint Review & Retrospective
| Function | Description |
|----------|-------------|
| `scrum_review_conduct(ctx, sprint_id, demo_notes, feedback, accepted, rejected)` | Conduct sprint review |
| `scrum_retrospective_create(ctx, sprint_id)` | Create a retrospective session |
| `scrum_retrospective_add_item(retro, text, category)` | Add item to retrospective |
| `scrum_retrospective_add_action(retro, action_id)` | Track action item from retro |

### Analytics
| Function | Description |
|----------|-------------|
| `scrum_velocity_calculate(ctx)` | Calculate velocity report across all sprints |
| `scrum_burndown_generate(ctx, sprint_id, points, max)` | Generate burndown chart data |
| `scrum_burnup_generate(ctx, sprint_id, scope, completed, max)` | Generate burnup chart data |
| `scrum_sprint_health_score(ctx, sprint_id)` | Calculate sprint health (0.0-1.0) |
| `scrum_velocity_print(report)` | Print velocity report |

---

## 2. Kanban Board (`kanban_board.h`)

### Board Management
| Function | Description |
|----------|-------------|
| `kanban_board_create(name)` | Create a new Kanban board with 5 default columns |
| `kanban_board_free(board)` | Free the board |
| `kanban_column_add(board, type, name)` | Add a column to the board |
| `kanban_column_count(board, type)` | Count cards in a column |

### WIP Limits
| Function | Description |
|----------|-------------|
| `kanban_wip_limit_set(board, type, limit)` | Set WIP limit for a column |
| `kanban_wip_check(board, type)` | Check if column is at WIP limit (0=OK, -1=at limit) |

### Cards
| Function | Description |
|----------|-------------|
| `kanban_card_create(board, title, description, priority)` | Create a new card. Returns card ID |
| `kanban_card_move(board, card_id, to_column)` | Move card between columns |
| `kanban_card_block(board, card_id, reason)` | Block a card with reason |
| `kanban_card_unblock(board, card_id)` | Unblock a card |
| `kanban_card_find(board, card_id)` | Find card by ID |

### CFD & Flow Metrics
| Function | Description |
|----------|-------------|
| `kanban_cfd_snapshot(board)` | Take a CFD snapshot |
| `kanban_cfd_generate(board, output, max, count)` | Get CFD history |
| `kanban_flow_metrics_calculate(board)` | Calculate flow metrics (cycle time, lead time, throughput, WIP avg, flow efficiency) |
| `kanban_bottleneck_detect(board, reports, max)` | Detect bottlenecks (>80% WIP utilization) |
| `kanban_cycle_time_calculate(card)` | Calculate cycle time for a card (start→done) |
| `kanban_lead_time_calculate(card)` | Calculate lead time for a card (created→done) |
| `kanban_throughput_calculate(board, days)` | Calculate throughput (cards/day) |

### Display
| Function | Description |
|----------|-------------|
| `kanban_board_print(board)` | Print board state with columns and WIP |
| `kanban_card_print(card)` | Print card details |
| `kanban_metrics_print(metrics)` | Print flow metrics |
| `kanban_bottleneck_print(reports, count)` | Print bottleneck report |
| `kanban_column_type_name(type)` | Get string name of column type |

---

## 3. Project Tracker (`project_tracker.h`)

### Tracker Management
| Function | Description |
|----------|-------------|
| `pt_tracker_create()` | Create a new project tracker |
| `pt_tracker_free(tracker)` | Free the tracker |

### Issue Management
| Function | Description |
|----------|-------------|
| `pt_issue_create(tracker, type, title, desc, priority)` | Create a new issue. Returns issue ID |
| `pt_issue_delete(tracker, issue_id)` | Delete an issue |
| `pt_issue_update(tracker, issue_id, new_status)` | Update issue status |
| `pt_issue_assign(tracker, issue_id, assignee_id)` | Assign issue to a user |
| `pt_issue_set_sprint(tracker, issue_id, sprint_id)` | Assign issue to a sprint |
| `pt_issue_set_points(tracker, issue_id, points)` | Set story points |
| `pt_issue_block(tracker, issue_id, reason)` | Block an issue |
| `pt_issue_unblock(tracker, issue_id)` | Unblock an issue |
| `pt_issue_find(tracker, issue_id)` | Find issue by ID |

### Labels & Components
| Function | Description |
|----------|-------------|
| `pt_label_create(tracker, name, color)` | Create a label (color = 0xRRGGBB) |
| `pt_label_assign_issue(tracker, issue_id, label_id)` | Assign label to issue |
| `pt_component_create(tracker, name, desc)` | Create a component |
| `pt_component_assign_issue(tracker, issue_id, comp_id)` | Assign component to issue |

### Workflow
| Function | Description |
|----------|-------------|
| `pt_workflow_customize(tracker, name, states, count)` | Customize workflow states |
| `pt_workflow_add_transition(tracker, from, to)` | Add transition between states |

### Queries & Views
| Function | Description |
|----------|-------------|
| `pt_filter_jql(tracker, query, result_ids, max_results)` | Execute JQL-like filter query |
| `pt_board_view(tracker)` | Display board view grouped by status |
| `pt_list_view(tracker, filter_status)` | Display list view filtered by status |
| `pt_sprint_metrics_calculate(tracker, sprint_id)` | Calculate sprint metrics |

### Utilities
| Function | Description |
|----------|-------------|
| `pt_issue_type_name(type)` | Get string name of issue type |
| `pt_issue_status_name(status)` | Get string name of status |
| `pt_issue_priority_name(priority)` | Get string name of priority |
| `pt_issue_print(issue)` | Print issue details |
| `pt_workflow_print(workflow)` | Print workflow state machine |
| `pt_sprint_metrics_print(metrics)` | Print sprint metrics |
| `pt_board_print(tracker)` | Print board overview |

---

## 4. OKR Tracker (`okr_tracker.h`)

### Context Management
| Function | Description |
|----------|-------------|
| `okr_context_create()` | Create OKR context |
| `okr_context_free(ctx)` | Free OKR context |

### Objectives & Key Results
| Function | Description |
|----------|-------------|
| `okr_objective_create(ctx, title, desc, level, parent_id, end_date)` | Create an objective. Returns objective ID |
| `okr_key_result_add(ctx, obj_id, title, start, target, unit, weight)` | Add a key result to an objective |
| `okr_key_result_update(ctx, kr_id, new_value)` | Update key result progress value |

### Grading & Check-Ins
| Function | Description |
|----------|-------------|
| `okr_grade_calculate(ctx, objective_id)` | Calculate OKR grade with breakdown |
| `okr_checkin_weekly(ctx, obj_id, week, confidence, notes)` | Record weekly check-in |

### Alignment
| Function | Description |
|----------|-------------|
| `okr_alignment_link(ctx, child_id, parent_id)` | Link child objective to parent |
| `okr_alignment_validate(ctx)` | Validate alignment (returns issue count) |
| `okr_objective_find_at_risk(ctx, result_ids, max)` | Find objectives graded < 0.3 |

### CFR (Conversations, Feedback, Recognition)
| Function | Description |
|----------|-------------|
| `cfr_conversation_record(ctx, obj_id, participant_id, content)` | Record a conversation |
| `cfr_feedback_record(ctx, obj_id, giver, receiver, content)` | Record feedback |
| `cfr_recognition_record(ctx, obj_id, giver, receiver, content)` | Record recognition |

### Analytics & Display
| Function | Description |
|----------|-------------|
| `okr_confidence_trend(ctx, objective_id)` | Get average confidence from check-ins |
| `okr_objective_print(obj)` | Print objective details |
| `okr_key_result_print(kr)` | Print key result with progress % |
| `okr_grade_print(grade)` | Print grade breakdown |
| `okr_checkin_print(checkin)` | Print check-in entry |
| `cfr_entry_print(entry)` | Print CFR entry |
| `okr_context_summary(ctx)` | Print full OKR context summary |

---

## 5. Retrospective (`retrospective.h`)

### Context & Sessions
| Function | Description |
|----------|-------------|
| `retro_context_create()` | Create retrospective context |
| `retro_context_free(ctx)` | Free retrospective context |
| `retro_session_create(ctx, format, sprint_id)` | Create a retro session |

### Retro Cards (3 formats)
| Function | Description |
|----------|-------------|
| `retro_add_item_ssc(session, category, text, author)` | Add Start/Stop/Continue item |
| `retro_add_item_msg(session, category, text, author)` | Add Mad/Sad/Glad item |
| `retro_add_item_4ls(session, category, text, author)` | Add 4Ls (Liked/Learned/Lacked/Longed) item |

### Action Items
| Function | Description |
|----------|-------------|
| `retro_action_item_create(session, desc, assignee, priority)` | Create action item |
| `retro_action_item_track(ctx, action_id, new_status)` | Update action item status |
| `retro_action_item_find_pending(ctx, result_ids, max)` | Find all pending action items |

### Health Check
| Function | Description |
|----------|-------------|
| `retro_health_check_create(sprint_id)` | Create health check |
| `retro_health_question_add(check, question)` | Add a health question |
| `retro_health_answer_record(check, idx, score)` | Record a score (1.0-5.0) |
| `retro_health_score(check)` | Calculate overall health score |

### Morale Survey
| Function | Description |
|----------|-------------|
| `retro_morale_survey_create()` | Create morale survey |
| `retro_morale_question_add(survey, question)` | Add a survey question |
| `retro_morale_answer_record(survey, idx, type)` | Record response (0=agree, 1=neutral, 2=disagree) |
| `retro_morale_index(survey)` | Calculate morale index (0.0-1.0) |

### Psychological Safety
| Function | Description |
|----------|-------------|
| `retro_psych_safety_assess()` | Create psych safety assessment |
| `retro_psych_question_add(assess, statement)` | Add a statement |
| `retro_psych_answer_record(assess, idx, score)` | Record score (1.0-5.0) |
| `retro_psych_safety_index(assess)` | Calculate safety index (0.0-1.0) |
| `retro_psych_safe_threshold(assess, threshold)` | Count areas above threshold |

### Display
| Function | Description |
|----------|-------------|
| `retro_session_print(session)` | Print retro session with all cards and actions |
| `retro_health_print(check)` | Print health check results |
| `retro_morale_print(survey)` | Print morale survey results |
| `retro_psych_print(assess)` | Print psych safety assessment |
| `retro_action_print(action)` | Print single action item |
| `retro_format_name(format)` | Get string name of retro format |
