#include "agile_scrum.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

ScrumContext *scrum_context_create(void)
{
    ScrumContext *ctx = (ScrumContext *)calloc(1, sizeof(ScrumContext));
    if (!ctx) return NULL;
    ctx->team_velocity_avg = 0;
    ctx->standup_count = 0;
    ctx->sprint_count = 0;
    ctx->product_backlog.count = 0;
    return ctx;
}

void scrum_context_free(ScrumContext *ctx)
{
    if (ctx) free(ctx);
}

ScrumSprint *scrum_sprint_create(ScrumContext *ctx, const char *name,
                                  const char *goal, int duration_days)
{
    if (!ctx || ctx->sprint_count >= 64) return NULL;
    ScrumSprint *s = &ctx->sprints[ctx->sprint_count];
    s->id = ctx->sprint_count + 1;
    strncpy(s->name, name, SCRUM_MAX_SPRINT_NAME - 1);
    s->name[SCRUM_MAX_SPRINT_NAME - 1] = '\0';
    strncpy(s->goal, goal, SCRUM_MAX_GOAL_TEXT - 1);
    s->goal[SCRUM_MAX_GOAL_TEXT - 1] = '\0';
    s->duration_days = duration_days;
    s->status = SCRUM_STATUS_PLANNING;
    s->story_points_planned = 0;
    s->story_points_completed = 0;
    s->backlog_item_count = 0;
    s->velocity = 0;
    s->start_date = 0;
    s->end_date = 0;
    ctx->sprint_count++;
    return s;
}

int scrum_sprint_start(ScrumContext *ctx, int sprint_id)
{
    if (!ctx || sprint_id < 1) return -1;
    ScrumSprint *s = scrum_sprint_find(ctx, sprint_id);
    if (!s) return -1;
    s->status = SCRUM_STATUS_ACTIVE;
    s->start_date = time(NULL);
    s->end_date = s->start_date + (time_t)(s->duration_days * 86400);
    return 0;
}

int scrum_sprint_complete(ScrumContext *ctx, int sprint_id)
{
    if (!ctx) return -1;
    ScrumSprint *s = scrum_sprint_find(ctx, sprint_id);
    if (!s) return -1;
    s->status = SCRUM_STATUS_COMPLETED;
    if (s->story_points_planned > 0) {
        s->velocity = (double)s->story_points_completed / s->story_points_planned;
    }
    if (ctx->sprint_count > 0) {
        double total = 0;
        int completed = 0;
        for (int i = 0; i < ctx->sprint_count; i++) {
            if (ctx->sprints[i].status == SCRUM_STATUS_COMPLETED &&
                ctx->sprints[i].story_points_planned > 0) {
                total += ctx->sprints[i].story_points_completed;
                completed++;
            }
        }
        if (completed > 0) {
            ctx->team_velocity_avg = total / completed;
        }
    }
    return 0;
}

int scrum_backlog_add(ScrumContext *ctx, const char *title, const char *desc,
                       int story_points, ScrumPriority priority)
{
    if (!ctx || ctx->product_backlog.count >= SCRUM_MAX_BACKLOG_ITEMS)
        return -1;
    ScrumBacklogItem *item =
        &ctx->product_backlog.items[ctx->product_backlog.count];
    item->id = ctx->product_backlog.count + 1;
    strncpy(item->title, title, SCRUM_MAX_ITEM_TITLE - 1);
    item->title[SCRUM_MAX_ITEM_TITLE - 1] = '\0';
    strncpy(item->description, desc, SCRUM_MAX_ITEM_DESC - 1);
    item->description[SCRUM_MAX_ITEM_DESC - 1] = '\0';
    item->story_points = story_points;
    item->priority = priority;
    item->status = SCRUM_ITEM_TODO;
    item->assignee_id = 0;
    item->created_at = time(NULL);
    item->updated_at = time(NULL);
    ctx->product_backlog.count++;
    return item->id;
}

int scrum_backlog_remove(ScrumContext *ctx, int item_id)
{
    if (!ctx || item_id < 1 || item_id > ctx->product_backlog.count)
        return -1;
    int idx = item_id - 1;
    ctx->product_backlog.items[idx].status = SCRUM_ITEM_REMOVED;
    if (idx < ctx->product_backlog.count - 1) {
        memmove(&ctx->product_backlog.items[idx],
                &ctx->product_backlog.items[idx + 1],
                (ctx->product_backlog.count - idx - 1) *
                    sizeof(ScrumBacklogItem));
    }
    ctx->product_backlog.count--;
    return 0;
}

int scrum_backlog_refine(ScrumContext *ctx, const char *output_path)
{
    if (!ctx) return -1;
    FILE *f = output_path ? fopen(output_path, "w") : stdout;
    if (!f && output_path) return -1;
    if (!f) f = stdout;
    for (int i = 0; i < ctx->product_backlog.count; i++) {
        ScrumBacklogItem *item = &ctx->product_backlog.items[i];
        fprintf(f, "[%d] %s (SP:%d, P:%d) %s\n",
                item->id, item->title, item->story_points,
                item->priority, item->description);
    }
    if (output_path) fclose(f);
    return 0;
}

int scrum_sprint_plan(ScrumContext *ctx, int sprint_id,
                       const int *item_ids, int item_count)
{
    if (!ctx) return -1;
    ScrumSprint *s = scrum_sprint_find(ctx, sprint_id);
    if (!s || item_count > SCRUM_MAX_SPRINT_ITEMS) return -1;
    s->story_points_planned = 0;
    for (int i = 0; i < item_count; i++) {
        int idx = item_ids[i] - 1;
        if (idx < 0 || idx >= ctx->product_backlog.count) continue;
        ScrumBacklogItem *item = &ctx->product_backlog.items[idx];
        s->backlog_item_ids[i] = item->id;
        s->backlog_item_count++;
        s->story_points_planned += item->story_points;
        item->status = SCRUM_ITEM_IN_PROGRESS;
    }
    return 0;
}

ScrumSprint *scrum_sprint_find(ScrumContext *ctx, int sprint_id)
{
    if (!ctx) return NULL;
    for (int i = 0; i < ctx->sprint_count; i++) {
        if (ctx->sprints[i].id == sprint_id) return &ctx->sprints[i];
    }
    return NULL;
}

int scrum_standup_record(ScrumContext *ctx, int sprint_id, int participant_id,
                          const char *name, const char *yesterday,
                          const char *today, const char *blockers)
{
    if (!ctx || ctx->standup_count >= SCRUM_MAX_STANDUP_ENTRIES) return -1;
    ScrumStandup *su = &ctx->standup_log[ctx->standup_count];
    su->sprint_id = sprint_id;
    su->participant_id = participant_id;
    strncpy(su->participant_name, name, SCRUM_MAX_PARTICIPANT_NAME - 1);
    su->participant_name[SCRUM_MAX_PARTICIPANT_NAME - 1] = '\0';
    su->standup_date = time(NULL);
    strncpy(su->yesterday, yesterday, SCRUM_MAX_ITEM_DESC - 1);
    su->yesterday[SCRUM_MAX_ITEM_DESC - 1] = '\0';
    strncpy(su->today, today, SCRUM_MAX_ITEM_DESC - 1);
    su->today[SCRUM_MAX_ITEM_DESC - 1] = '\0';
    strncpy(su->blockers, blockers, SCRUM_MAX_BLOCKER_TEXT - 1);
    su->blockers[SCRUM_MAX_BLOCKER_TEXT - 1] = '\0';
    ctx->standup_count++;
    return 0;
}

ScrumSprintReview *scrum_review_conduct(ScrumContext *ctx, int sprint_id,
                                         const char *demo_notes,
                                         const char *feedback,
                                         int accepted, int rejected)
{
    if (!ctx) return NULL;
    ScrumSprint *s = scrum_sprint_find(ctx, sprint_id);
    if (!s) return NULL;
    static ScrumSprintReview review;
    review.sprint_id = sprint_id;
    review.review_date = time(NULL);
    strncpy(review.demo_notes, demo_notes, SCRUM_MAX_ITEM_DESC - 1);
    review.demo_notes[SCRUM_MAX_ITEM_DESC - 1] = '\0';
    strncpy(review.stakeholder_feedback, feedback, SCRUM_MAX_FEEDBACK_TEXT - 1);
    review.stakeholder_feedback[SCRUM_MAX_FEEDBACK_TEXT - 1] = '\0';
    review.items_accepted = accepted;
    review.items_rejected = rejected;
    review.items_total = accepted + rejected;
    s->story_points_completed += accepted;
    return &review;
}

ScrumRetrospective *scrum_retrospective_create(ScrumContext *ctx,
                                                int sprint_id)
{
    if (!ctx) return NULL;
    static ScrumRetrospective retro;
    retro.sprint_id = sprint_id;
    retro.retro_date = time(NULL);
    retro.item_count = 0;
    retro.action_item_count = 0;
    return &retro;
}

int scrum_retrospective_add_item(ScrumRetrospective *retro,
                                  const char *text, int category)
{
    if (!retro || retro->item_count >= SCRUM_MAX_RETRO_ITEMS) return -1;
    ScrumRetroItem *ri = &retro->items[retro->item_count];
    strncpy(ri->item, text, SCRUM_MAX_ACTION_ITEM - 1);
    ri->item[SCRUM_MAX_ACTION_ITEM - 1] = '\0';
    ri->category = category;
    ri->action_item_id = -1;
    retro->item_count++;
    return retro->item_count - 1;
}

int scrum_retrospective_add_action(ScrumRetrospective *retro, int action_id)
{
    if (!retro || retro->action_item_count >= SCRUM_MAX_RETRO_ITEMS) return -1;
    retro->action_item_ids[retro->action_item_count] = action_id;
    retro->action_item_count++;
    return 0;
}

ScrumVelocityReport scrum_velocity_calculate(const ScrumContext *ctx)
{
    ScrumVelocityReport report = {0};
    if (!ctx) return report;
    report.total_sprints = 0;
    report.total_points_completed = 0;
    report.total_points_planned = 0;
    for (int i = 0; i < ctx->sprint_count; i++) {
        if (ctx->sprints[i].status == SCRUM_STATUS_COMPLETED) {
            report.total_sprints++;
            report.total_points_completed +=
                ctx->sprints[i].story_points_completed;
            report.total_points_planned +=
                ctx->sprints[i].story_points_planned;
        }
    }
    if (report.total_sprints > 0) {
        report.avg_velocity =
            (double)report.total_points_completed / report.total_sprints;
    }
    if (ctx->sprint_count > 0) {
        const ScrumSprint *last = &ctx->sprints[ctx->sprint_count - 1];
        report.last_sprint_velocity = last->velocity;
    }
    return report;
}

int scrum_burndown_generate(const ScrumContext *ctx, int sprint_id,
                             ScrumBurndownPoint *points, int max_points)
{
    if (!ctx || !points) return 0;
    const ScrumSprint *s = NULL;
    for (int i = 0; i < ctx->sprint_count; i++) {
        if (ctx->sprints[i].id == sprint_id) {
            s = &ctx->sprints[i];
            break;
        }
    }
    if (!s || s->duration_days <= 0) return 0;
    int days = s->duration_days;
    if (days > max_points) days = max_points;
    double ideal_burn = (double)s->story_points_planned / s->duration_days;
    for (int d = 0; d < days; d++) {
        points[d].sprint_id = sprint_id;
        points[d].day_index = d;
        double remaining = (double)s->story_points_planned - ideal_burn * (d + 1);
        if (remaining < 0) remaining = 0;
        points[d].points_remaining = (int)ceil(remaining);
    }
    return days;
}

int scrum_burnup_generate(const ScrumContext *ctx, int sprint_id,
                           int *scope_points, int *completed_points,
                           int max_points)
{
    if (!ctx || !scope_points || !completed_points) return 0;
    const ScrumSprint *s = NULL;
    for (int i = 0; i < ctx->sprint_count; i++) {
        if (ctx->sprints[i].id == sprint_id) {
            s = &ctx->sprints[i];
            break;
        }
    }
    if (!s) return 0;
    int days = s->duration_days;
    if (days > max_points) days = max_points;
    for (int d = 0; d < days; d++) {
        scope_points[d] = s->story_points_planned;
        double fraction = (double)(d + 1) / s->duration_days;
        completed_points[d] = (int)(s->story_points_completed * fraction);
    }
    return days;
}

double scrum_sprint_health_score(const ScrumContext *ctx, int sprint_id)
{
    if (!ctx) return 0;
    const ScrumSprint *s = NULL;
    for (int i = 0; i < ctx->sprint_count; i++) {
        if (ctx->sprints[i].id == sprint_id) {
            s = &ctx->sprints[i];
            break;
        }
    }
    if (!s || s->story_points_planned <= 0) return 0;
    return (double)s->story_points_completed / s->story_points_planned;
}

void scrum_sprint_print(const ScrumSprint *sprint)
{
    if (!sprint) return;
    printf("Sprint #%d: %s\n", sprint->id, sprint->name);
    printf("  Goal: %s\n", sprint->goal);
    printf("  Duration: %d days\n", sprint->duration_days);
    printf("  Status: %d\n", sprint->status);
    printf("  Points: %d planned / %d completed\n",
           sprint->story_points_planned, sprint->story_points_completed);
    printf("  Velocity: %.2f\n", sprint->velocity);
}

void scrum_backlog_print(const ScrumContext *ctx)
{
    if (!ctx) return;
    printf("Product Backlog (%d items):\n", ctx->product_backlog.count);
    for (int i = 0; i < ctx->product_backlog.count; i++) {
        const ScrumBacklogItem *item = &ctx->product_backlog.items[i];
        printf("  [%d] %s (%d SP) P:%d\n",
               item->id, item->title, item->story_points, item->priority);
    }
}

void scrum_standup_print(const ScrumStandup *standup)
{
    if (!standup) return;
    printf("Standup - %s (Sprint #%d):\n",
           standup->participant_name, standup->sprint_id);
    printf("  Yesterday: %s\n", standup->yesterday);
    printf("  Today: %s\n", standup->today);
    printf("  Blockers: %s\n", standup->blockers);
}

void scrum_velocity_print(const ScrumVelocityReport *report)
{
    if (!report) return;
    printf("Velocity Report:\n");
    printf("  Sprints: %d\n", report->total_sprints);
    printf("  Avg Velocity: %.2f SP/sprint\n", report->avg_velocity);
    printf("  Last Sprint: %.2f\n", report->last_sprint_velocity);
    printf("  Total Completed: %d / %d\n",
           report->total_points_completed, report->total_points_planned);
}
