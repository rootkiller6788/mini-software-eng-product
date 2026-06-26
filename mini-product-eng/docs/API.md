# mini-product-eng API Reference

## product_lifecycle.h — 产品生命周期

| Function | Description |
|---|---|
| `product_lifecycle_init(pl, name, vision)` | Initialize lifecycle with product name and vision |
| `product_lifecycle_current_phase(pl)` | Get current phase enum |
| `product_lifecycle_evaluate_gate(pl, phase, go)` | Set go/no-go decision for a phase gate |
| `product_lifecycle_phase_name(phase)` | Get human-readable phase name |
| `product_lifecycle_advance_phase(pl)` | Move to next phase (requires gate passed) |
| `product_lifecycle_add_mvp_feature(pl, feature, priority)` | Add MVP feature with MoSCoW priority |
| `product_lifecycle_is_mvp_ready(pl)` | Check if MVP criteria are met |
| `product_lifecycle_print_summary(pl)` | Print lifecycle state to stdout |
| `product_lifecycle_can_regress(pl)` | Check if regression to previous phase is possible |
| `product_lifecycle_set_phase_description(pl, phase, desc)` | Set custom description for a phase |
| `product_lifecycle_reset(pl)` | Free resources and reinitialize |

### Data Types

**ProductPhase**: `DISCOVERY → DEFINITION → DESIGN → DEVELOPMENT → TESTING → LAUNCH → GROWTH → MATURITY → DECLINE`

**MvpPriority**: `MVP_MUST_HAVE, MVP_SHOULD_HAVE, MVP_COULD_HAVE, MVP_WONT_HAVE`

---

## roadmap_plan.h — 路线图规划

| Function | Description |
|---|---|
| `roadmap_init(r, quarter, year)` | Initialize roadmap with quarter and year |
| `roadmap_add_feature(r, name, desc, reach, impact, confidence, effort)` | Add feature with RICE parameters |
| `roadmap_rice_score(reach, impact, confidence, effort)` | Calculate RICE score: `(R * I * C) / E` |
| `roadmap_prioritize_by_rice(r)` | Sort backlog by RICE score and assign horizons |
| `roadmap_assign_horizon(f, now_threshold, next_threshold)` | Assign time horizon based on RICE score |
| `roadmap_add_theme(r, name, horizon)` | Add a thematic roadmap section |
| `roadmap_set_okr(t, objective, key_results, kr_count)` | Set OKR for a theme |
| `roadmap_add_feature_to_theme(t, f)` | Associate a feature with a theme |
| `roadmap_calc_opportunity_score(importance, satisfaction_gap)` | Calculate opportunity score: `imp + max(imp - gap, 0)` |
| `roadmap_generate_stakeholder_message(r, buf, size)` | Generate stakeholder-friendly roadmap summary |
| `roadmap_count_by_horizon(r, hz)` | Count features in a given horizon |
| `roadmap_auto_commit_now(r, max_features)` | Auto-commit up to N NOW features |
| `roadmap_free(r)` | Free all roadmap resources |

---

## spec_design.h — 规格与设计

| Function | Description |
|---|---|
| `user_story_init(us, role, want, so_that)` | Initialize user story: As a, I want, so that |
| `user_story_add_acceptance(us, given, when, then)` | Add acceptance criteria |
| `user_story_format(us, buf, size)` | Format user story to string |
| `prd_init(p, title, problem)` | Initialize PRD document |
| `prd_add_story(p, story)` | Add user story to PRD |
| `prd_total_story_points(p)` | Sum story points across all stories |
| `prd_print_summary(p)` | Print PRD summary |
| `tech_spec_add_endpoint(ts, endpoint, method, status)` | Add API endpoint to tech spec |
| `tech_spec_add_model(ts, entity)` | Add data model entity |
| `tech_spec_add_mockup(ts, screen, layout)` | Add UI mockup record |
| `tech_spec_print_summary(ts)` | Print tech spec summary |
| `design_review_init(dr)` | Initialize design review checklist (all false) |
| `design_review_all_passed(dr)` | Check all 10 checklist items |
| `design_review_score(dr)` | Get 0.0–1.0 completion ratio |
| `design_review_list_pending(dr, buf, size)` | List incomplete checklist items |
| `spec_free_all(prd, ts)` | Free PRD and tech spec resources |

---

## stakeholder_mgmt.h — 干系人管理

| Function | Description |
|---|---|
| `stakeholder_init(s, name, title, power, interest)` | Initialize stakeholder with 0–1 power/interest |
| `stakeholder_quadrant(power, interest)` | Get quadrant label (Key Players, Meet Needs, etc.) |
| `stakeholder_set_strategy(s)` | Auto-assign engagement strategy based on quadrant |
| `stakeholder_map_print(stakeholders, count, buf, size)` | Generate ASCII stakeholder map |
| `raci_matrix_init(rm)` | Initialize RACI matrix |
| `raci_add_item(rm, task, responsible, accountable)` | Add task row with R and A |
| `raci_add_consulted(rm, index, person)` | Add C person to task at index |
| `raci_add_informed(rm, index, person)` | Add I person to task at index |
| `raci_has_gaps(rm, buf, size)` | Check for tasks without C or I assignment |
| `raci_free(rm)` | Free RACI matrix resources |
| `comm_plan_init(cp, stakeholder, channel, freq)` | Initialize communication plan entry |
| `comm_plan_generate(stakeholders, count, plans)` | Auto-generate comm plans from stakeholder list |
| `comm_plan_print(plans, count)` | Print communication plans |
| `escalation_init(e, issue, severity)` | Initialize escalation issue |
| `escalation_resolve(e, decision)` | Resolve escalation with decision path |
| `escalation_requires_attention(e, threshold)` | Check if unresolved escalation meets severity threshold |
| `escalation_print(e)` | Print escalation status |

---

## release_mgmt.h — 发布管理

| Function | Description |
|---|---|
| `semver_parse(str, sv)` | Parse "MAJOR.MINOR.PATCH[-prerelease][+build]" |
| `semver_compare(a, b)` | Compare two versions: -1, 0, or 1 |
| `semver_bump_major(sv)` | Increment major, reset minor/patch/prerelease |
| `semver_bump_minor(sv)` | Increment minor, reset patch/prerelease |
| `semver_bump_patch(sv)` | Increment patch, reset prerelease |
| `semver_to_string(sv, buf, size)` | Format SemVer to string |
| `semver_is_prerelease(sv)` | Check if version has prerelease tag |
| `semver_is_stable(sv)` | Check if version is stable (major>0, no prerelease) |
| `release_train_init(rt, name, major, minor, patch, sprints)` | Initialize release train |
| `release_train_is_ready(rt)` | Check if locked and has sprints |
| `release_train_lock(rt)` | Lock the release train scope |
| `release_train_print(rt)` | Print release train details |
| `feature_flag_init(ff, name, owner, desc)` | Initialize feature flag |
| `feature_flag_enable(ff)` | Enable the feature flag |
| `feature_flag_disable(ff)` | Disable and reset rollout |
| `feature_flag_set_rollout(ff, pct)` | Set rollout percentage (0–100) |
| `feature_flag_kill_switch(ff)` | Activate emergency kill switch |
| `feature_flag_is_active_for_user(ff, user_id)` | Check if feature is active for a given user |
| `release_checklist_init(items, count)` | Initialize checklist array |
| `release_checklist_all_done(items, count)` | Check all items complete |
| `release_checklist_progress(items, count)` | Get 0.0–1.0 completion ratio |
| `rollback_plan_init(rp, trigger, steps, min)` | Initialize rollback plan |
| `rollback_plan_validate(rp)` | Validate plan completeness |
| `release_stage_name(stage)` | Get stage name: Pre-Alpha, Alpha, Beta, RC, GA |
| `release_stage_can_advance(current, next)` | Check if next stage is valid |
| `announcement_init(a, title, date)` | Initialize release announcement |
| `announcement_add_highlight(a, highlight)` | Add highlight bullet point |
| `announcement_generate(a, buf, size)` | Generate formatted announcement text |
