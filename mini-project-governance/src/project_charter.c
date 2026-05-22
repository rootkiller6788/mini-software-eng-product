#include "project_charter.h"
#include <stdio.h>
#include <string.h>

void charter_init(ProjectCharter *pc, const char *name, const char *sponsor, const char *manager) {
    memset(pc, 0, sizeof(*pc));
    strncpy(pc->project_name, name, 63); pc->project_name[63] = '\0';
    strncpy(pc->sponsor, sponsor, 63); pc->sponsor[63] = '\0';
    strncpy(pc->manager, manager, 63); pc->manager[63] = '\0';
    pc->status = CS_PROPOSED;
}

int charter_add_objective(ProjectCharter *pc, const char *desc, double target) {
    if (pc->objective_count >= MAX_OBJECTIVES) return -1;
    Objective *o = &pc->objectives[pc->objective_count];
    strncpy(o->description, desc, 255); o->description[255] = '\0';
    o->target = target; o->current = 0; o->measurable = true;
    return pc->objective_count++;
}

int charter_add_constraint(ProjectCharter *pc, const char *desc, const char *category) {
    if (pc->constraint_count >= MAX_CONSTRAINTS) return -1;
    Constraint *c = &pc->constraints[pc->constraint_count];
    strncpy(c->description, desc, 127); c->description[127] = '\0';
    strncpy(c->category, category, 31); c->category[31] = '\0';
    return pc->constraint_count++;
}

int charter_add_scope(ProjectCharter *pc, const char *name, bool in_scope) {
    if (pc->scope_count >= MAX_SCOPE_ITEMS) return -1;
    ScopeItem *s = &pc->scope[pc->scope_count];
    strncpy(s->name, name, 127); s->name[127] = '\0';
    s->in_scope = in_scope; s->exclusion_note[0] = '\0';
    return pc->scope_count++;
}

void charter_update_objective(ProjectCharter *pc, int idx, double current) {
    if (idx >= 0 && idx < pc->objective_count) pc->objectives[idx].current = current;
}

double charter_progress_pct(ProjectCharter *pc) {
    if (pc->objective_count == 0) return 0;
    double total = 0;
    for (int i = 0; i < pc->objective_count; i++)
        total += pc->objectives[i].target > 0 ? pc->objectives[i].current / pc->objectives[i].target : 1.0;
    return 100.0 * total / pc->objective_count;
}

bool charter_is_on_budget(ProjectCharter *pc) {
    return pc->budget_spent <= pc->budget_total;
}

bool charter_is_on_schedule(ProjectCharter *pc) {
    if (pc->timeline_days == 0) return true;
    return pc->days_elapsed <= pc->timeline_days;
}

void charter_print(ProjectCharter *pc) {
    const char *ss[] = {"PROPOSED","APPROVED","ACTIVE","CLOSED","SUSPENDED"};
    printf("=== Project Charter: %s [%s] ===\n", pc->project_name, ss[pc->status]);
    printf("  Sponsor: %s  Manager: %s\n", pc->sponsor, pc->manager);
    printf("  Budget: %d/%d  Schedule: %d/%d days  Progress: %.0f%%\n",
           pc->budget_spent, pc->budget_total, pc->days_elapsed, pc->timeline_days,
           charter_progress_pct(pc));
    printf("  Objectives:\n");
    for (int i = 0; i < pc->objective_count; i++)
        printf("    - %s (%.1f/%.1f)\n", pc->objectives[i].description, pc->objectives[i].current, pc->objectives[i].target);
    printf("  Constraints:\n");
    for (int i = 0; i < pc->constraint_count; i++)
        printf("    - [%s] %s\n", pc->constraints[i].category, pc->constraints[i].description);
    printf("  Scope:\n");
    for (int i = 0; i < pc->scope_count; i++)
        printf("    %s %s\n", pc->scope[i].in_scope ? "[IN]" : "[OUT]", pc->scope[i].name);
}
