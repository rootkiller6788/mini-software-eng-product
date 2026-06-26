#include "gov_scrum.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

void scrum_init(ScrumBoard *sb) {
    memset(sb, 0, sizeof(*sb));
    sb->current_sprint = -1;
    sb->avg_velocity = 0.0;
    sb->velocity_stdev = 0.0;
    sb->velocity_samples = 0;
}
void scrum_team_init(ScrumTeam *t) {
    memset(t, 0, sizeof(*t));
    t->total_capacity = 0;
}
int scrum_add_member(ScrumTeam *t, const char *name, ScrumRole role, int capacity) {
    if (t->count >= SCRUM_MAX_TEAM_MEMBERS || capacity < 0 || capacity > 100) return -1;
    TeamMember *m = &t->members[t->count];
    strncpy(m->name, name, SCRUM_NAME_LEN - 1);
    m->name[SCRUM_NAME_LEN - 1] = '\0';
    m->role = role;
    m->capacity_pct = capacity;
    m->hourly_cost = 0.0;
    m->active = true;
    return t->count++;
}
void scrum_team_capacity(ScrumTeam *t) {
    t->total_capacity = 0;
    for (int i = 0; i < t->count; i++) {
        if (t->members[i].active) t->total_capacity += t->members[i].capacity_pct;
    }
}
static void pbi_init_item(ProductBacklogItem *pbi) {
    memset(pbi, 0, sizeof(*pbi));
    pbi->state = PBI_NEW;
    pbi->sprint_number = -1;
    pbi->created_at = (uint32_t)time(NULL);
    pbi->dep_count = 0;
    pbi->wsjf_score = 0.0;
    pbi->priority = 100;
}
int pbi_add(ProductBacklog *pb, const char *id, const char *title,
            PBIType type, int points, int value, int hours) {
    if (pb->count >= SCRUM_MAX_PBI) return -1;
    if (points < 1) return -1;
    ProductBacklogItem *p = &pb->items[pb->count];
    pbi_init_item(p);
    strncpy(p->id, id, SCRUM_ID_LEN - 1); p->id[SCRUM_ID_LEN - 1] = '\0';
    strncpy(p->title, title, SCRUM_NAME_LEN - 1); p->title[SCRUM_NAME_LEN - 1] = '\0';
    p->type = type;
    p->story_points = points;
    p->business_value = value > 0 ? value : 1;
    p->effort_hours = hours;
    pb->total_points += points;
    pb->total_value += value;
    return pb->count++;
}
int pbi_find(ProductBacklog *pb, const char *id) {
    for (int i = 0; i < pb->count; i++) {
        if (strcmp(pb->items[i].id, id) == 0) return i;
    }
    return -1;
}
bool pbi_move_state(ProductBacklog *pb, const char *id, PBIState new_state) {
    int idx = pbi_find(pb, id);
    if (idx < 0) return false;
    ProductBacklogItem *p = &pb->items[idx];
    if (p->state == PBI_DONE && new_state != PBI_ACCEPTED && new_state != PBI_DONE) return false;
    if (p->state == PBI_BLOCKED && new_state == PBI_ACCEPTED) return false;
    p->state = new_state;
    if (new_state == PBI_ACCEPTED) {
        p->completed_at = (uint32_t)time(NULL);
    }
    return true;
}
static int pbi_cmp_priority(const void *a, const void *b) {
    const ProductBacklogItem *pa = (const ProductBacklogItem *)a;
    const ProductBacklogItem *pb2 = (const ProductBacklogItem *)b;
    return pa->priority - pb2->priority;
}
void pbi_sort_by_priority(ProductBacklog *pb) {
    qsort(pb->items, pb->count, sizeof(ProductBacklogItem), pbi_cmp_priority);
}
void pbi_calc_wsjf(ProductBacklog *pb) {
    for (int i = 0; i < pb->count; i++) {
        ProductBacklogItem *p = &pb->items[i];
        double cod = (double)p->business_value +
                     (p->type == PBI_BUG ? 8.0 : 1.0) +
                     (p->type == PBI_TECH_DEBT ? 3.0 : 0.0);
        double sz = (double)(p->story_points > 0 ? p->story_points : 1);
        p->wsjf_score = cod / sz;
    }
}
static int pbi_cmp_wsjf(const void *a, const void *b) {
    const ProductBacklogItem *pa = (const ProductBacklogItem *)a;
    const ProductBacklogItem *pb2 = (const ProductBacklogItem *)b;
    if (pa->wsjf_score < pb2->wsjf_score) return 1;
    if (pa->wsjf_score > pb2->wsjf_score) return -1;
    return 0;
}
void pbi_sort_by_wsjf(ProductBacklog *pb) {
    pbi_calc_wsjf(pb);
    qsort(pb->items, pb->count, sizeof(ProductBacklogItem), pbi_cmp_wsjf);
}
void pbi_add_dependency(ProductBacklog *pb, const char *from, const char *to) {
    int fi = pbi_find(pb, from);
    int ti = pbi_find(pb, to);
    if (fi < 0 || ti < 0 || fi == ti) return;
    if (pb->items[fi].dep_count >= 8) return;
    for (int i = 0; i < pb->items[ti].dep_count; i++) {
        if (pb->items[ti].dependencies[i] == fi) return;
    }
    pb->items[fi].dependencies[pb->items[fi].dep_count++] = ti;
}
int sprint_create(ScrumBoard *sb, const char *goal, int duration_days) {
    if (sb->sprint_count >= SCRUM_MAX_SPRINTS) return -1;
    if (duration_days < 1 || duration_days > 30) return -1;
    Sprint *sp = &sb->sprints[sb->sprint_count];
    memset(sp, 0, sizeof(*sp));
    sp->sprint_number = sb->sprint_count + 1;
    strncpy(sp->goal, goal, SCRUM_DESC_LEN - 1);
    sp->goal[SCRUM_DESC_LEN - 1] = '\0';
    sp->duration_days = duration_days;
    sp->state = SPRINT_PLANNING;
    return sb->sprint_count++;
}
int sprint_load_items(ScrumBoard *sb, int sprint_idx, int max_points) {
    if (sprint_idx < 0 || sprint_idx >= sb->sprint_count) return -1;
    Sprint *sp = &sb->sprints[sprint_idx];
    ProductBacklog *pb = &sb->backlog;
    pbi_sort_by_priority(pb);
    int loaded_points = 0, loaded_count = 0;
    for (int i = 0; i < pb->count && loaded_points < max_points; i++) {
        ProductBacklogItem *p = &pb->items[i];
        if (p->state == PBI_IN_PROGRESS || p->state == PBI_ACCEPTED) continue;
        if (p->sprint_number != -1) continue;
        bool deps_met = true;
        for (int d = 0; d < p->dep_count; d++) {
            if (pb->items[p->dependencies[d]].state != PBI_ACCEPTED) { deps_met = false; break; }
        }
        if (!deps_met) continue;
        if (loaded_points + p->story_points <= max_points) {
            p->sprint_number = sp->sprint_number;
            p->state = PBI_READY;
            loaded_points += p->story_points;
            loaded_count++;
        }
    }
    sp->planned_points = loaded_points;
    sp->planned_items = loaded_count;
    return loaded_count;
}
bool sprint_start(ScrumBoard *sb, int sprint_idx) {
    if (sprint_idx < 0 || sprint_idx >= sb->sprint_count) return false;
    if (sb->sprints[sprint_idx].state != SPRINT_PLANNING) return false;
    sb->sprints[sprint_idx].state = SPRINT_ACTIVE;
    sb->sprints[sprint_idx].start_date = (uint32_t)time(NULL);
    sb->current_sprint = sprint_idx;
    return true;
}
bool sprint_finish(ScrumBoard *sb, int sprint_idx) {
    if (sprint_idx < 0 || sprint_idx >= sb->sprint_count) return false;
    Sprint *sp = &sb->sprints[sprint_idx];
    if (sp->state != SPRINT_ACTIVE) return false;
    sp->state = SPRINT_REVIEW;
    sp->end_date = (uint32_t)time(NULL);
    sprint_calc_velocity(sp);
    sb->velocity_samples++;
    double old_avg = sb->avg_velocity;
    sb->avg_velocity = old_avg + (sp->velocity - old_avg) / sb->velocity_samples;
    double sum_sq = 0; int cnt = 0;
    for (int i = 0; i < sb->sprint_count; i++) {
        if (sb->sprints[i].state >= SPRINT_REVIEW) {
            double d = sb->sprints[i].velocity - sb->avg_velocity;
            sum_sq += d * d; cnt++;
        }
    }
    sb->velocity_stdev = cnt > 1 ? sqrt(sum_sq / (cnt - 1)) : 0.0;
    return true;
}
bool sprint_plan(ScrumBoard *sb, int sprint_idx) {
    if (sprint_idx < 0 || sprint_idx >= sb->sprint_count) return false;
    double capacity = sb->avg_velocity > 0 ? sb->avg_velocity : 20.0;
    int loaded = sprint_load_items(sb, sprint_idx, (int)ceil(capacity));
    return loaded >= 0;
}
void sprint_record_burndown(Sprint *sp, int day, double remaining) {
    if (day < 0 || day >= 30) return;
    if (sp->burndown_days == 0) {
        for (int d = 0; d < sp->duration_days && d < 30; d++) {
            sp->burndown_ideal[d] = (double)sp->planned_points * (1.0 - (double)d / sp->duration_days);
        }
    }
    if (day < 30) {
        sp->burndown_actual[day] = remaining;
        if (day >= sp->burndown_days) sp->burndown_days = day + 1;
    }
}
void sprint_calc_velocity(Sprint *sp) {
    sp->velocity = (double)sp->completed_points;
    sp->goal_met = sp->completed_points >= sp->planned_points;
}
int scrum_wip_count(ScrumBoard *sb) {
    int wip = 0;
    for (int i = 0; i < sb->backlog.count; i++)
        if (sb->backlog.items[i].state == PBI_IN_PROGRESS) wip++;
    return wip;
}
double scrum_predict_completion(ScrumBoard *sb, int remaining_points) {
    if (sb->avg_velocity <= 0) return -1.0;
    return (double)remaining_points / sb->avg_velocity;
}
void scrum_print_sprint_report(ScrumBoard *sb, int sprint_idx) {
    if (sprint_idx < 0 || sprint_idx >= sb->sprint_count) { printf("No such sprint.\n"); return; }
    Sprint *sp = &sb->sprints[sprint_idx];
    printf("=== Sprint %d Report ===\n", sp->sprint_number);
    printf("  Goal: %s\n", sp->goal);
    printf("  State: %d | Duration: %d days\n", sp->state, sp->duration_days);
    printf("  Planned: %d pts / %d items\n", sp->planned_points, sp->planned_items);
    printf("  Completed: %d pts / %d items\n", sp->completed_points, sp->completed_items);
    printf("  Velocity: %.1f pts/sprint\n", sp->velocity);
    printf("  Goal Met: %s\n", sp->goal_met ? "YES" : "NO");
}
void scrum_print_velocity_chart(ScrumBoard *sb) {
    printf("=== Velocity Chart ===\n");
    printf("  Avg: %.1f  StDev: %.1f  Samples: %d\n",
           sb->avg_velocity, sb->velocity_stdev, sb->velocity_samples);
    for (int i = 0; i < sb->sprint_count; i++) {
        Sprint *sp = &sb->sprints[i];
        printf("  Sprint %2d: %3d pts [", sp->sprint_number, sp->completed_points);
        int bar = sp->completed_points / 2;
        for (int j = 0; j < bar && j < 30; j++) printf("#");
        printf("]\n");
    }
}