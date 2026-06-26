#include "okr_tracker.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

OKRContext *okr_context_create(void)
{
    OKRContext *ctx = (OKRContext *)calloc(1, sizeof(OKRContext));
    if (!ctx) return NULL;
    ctx->objective_count = 0;
    ctx->key_result_count = 0;
    ctx->checkin_count = 0;
    ctx->cfr_count = 0;
    ctx->next_id = 1;
    ctx->objective_capacity = OKR_MAX_OBJECTIVES;
    ctx->key_result_capacity = OKR_MAX_KEY_RESULTS;
    ctx->objectives = (OKRObjective *)calloc(
        ctx->objective_capacity, sizeof(OKRObjective));
    ctx->key_results = (OKRKeyResult *)calloc(
        ctx->key_result_capacity, sizeof(OKRKeyResult));
    return ctx;
}

void okr_context_free(OKRContext *ctx)
{
    if (!ctx) return;
    free(ctx->objectives);
    free(ctx->key_results);
    free(ctx);
}

int okr_objective_create(OKRContext *ctx, const char *title,
                          const char *description, OKRLevel level,
                          int parent_objective_id, time_t end_date)
{
    if (!ctx || ctx->objective_count >= ctx->objective_capacity) return -1;
    OKRObjective *obj = &ctx->objectives[ctx->objective_count];
    obj->id = ctx->next_id++;
    strncpy(obj->title, title, OKR_MAX_TITLE - 1);
    obj->title[OKR_MAX_TITLE - 1] = '\0';
    strncpy(obj->description, description, OKR_MAX_DESC - 1);
    obj->description[OKR_MAX_DESC - 1] = '\0';
    obj->level = level;
    obj->status = OKR_STATUS_ACTIVE;
    obj->parent_objective_id = parent_objective_id;
    obj->start_date = time(NULL);
    obj->end_date = end_date;
    obj->key_result_count = 0;
    obj->overall_grade = 0;
    ctx->objective_count++;
    return obj->id;
}

int okr_key_result_add(OKRContext *ctx, int objective_id,
                        const char *title, double start_value,
                        double target_value, const char *unit,
                        double weight)
{
    if (!ctx || ctx->key_result_count >= ctx->key_result_capacity) return -1;
    int obj_idx = -1;
    for (int i = 0; i < ctx->objective_count; i++) {
        if (ctx->objectives[i].id == objective_id) { obj_idx = i; break; }
    }
    if (obj_idx < 0) return -1;
    OKRObjective *obj = &ctx->objectives[obj_idx];
    if (obj->key_result_count >= 8) return -1;

    OKRKeyResult *kr = &ctx->key_results[ctx->key_result_count];
    kr->id = ctx->next_id++;
    kr->objective_id = objective_id;
    strncpy(kr->title, title, OKR_MAX_TITLE - 1);
    kr->title[OKR_MAX_TITLE - 1] = '\0';
    kr->start_value = start_value;
    kr->target_value = target_value;
    kr->current_value = start_value;
    strncpy(kr->unit, unit, OKR_MAX_UNIT - 1);
    kr->unit[OKR_MAX_UNIT - 1] = '\0';
    kr->weight = weight;
    obj->key_result_ids[obj->key_result_count++] = kr->id;
    ctx->key_result_count++;
    return kr->id;
}

int okr_key_result_update(OKRContext *ctx, int key_result_id,
                           double new_value)
{
    if (!ctx) return -1;
    for (int i = 0; i < ctx->key_result_count; i++) {
        if (ctx->key_results[i].id == key_result_id) {
            ctx->key_results[i].current_value = new_value;
            return 0;
        }
    }
    return -1;
}

OKRGradeBreakdown okr_grade_calculate(const OKRContext *ctx,
                                       int objective_id)
{
    OKRGradeBreakdown grade = {0, 0, 0, 0};
    if (!ctx) return grade;
    int obj_idx = -1;
    for (int i = 0; i < ctx->objective_count; i++) {
        if (ctx->objectives[i].id == objective_id) { obj_idx = i; break; }
    }
    if (obj_idx < 0) return grade;
    OKRObjective *obj = &ctx->objectives[obj_idx];
    if (obj->key_result_count == 0) return grade;

    double total_weight = 0;
    double weighted_sum = 0;
    for (int j = 0; j < obj->key_result_count; j++) {
        for (int k = 0; k < ctx->key_result_count; k++) {
            if (ctx->key_results[k].id == obj->key_result_ids[j]) {
                OKRKeyResult *kr = &ctx->key_results[k];
                double range = kr->target_value - kr->start_value;
                double progress = 0;
                if (fabs(range) > 1e-9) {
                    progress = (kr->current_value - kr->start_value) / range;
                }
                if (progress < 0) progress = 0;
                if (progress > 1.0) progress = 1.0;
                weighted_sum += progress * kr->weight;
                total_weight += kr->weight;
                break;
            }
        }
    }
    if (total_weight > 0) {
        grade.achievement = weighted_sum / total_weight;
    }
    grade.overall = grade.achievement;
    grade.progress = grade.achievement;
    grade.learning = 0.5;
    return grade;
}

int okr_checkin_weekly(OKRContext *ctx, int objective_id,
                        int week_number, double confidence,
                        const char *notes)
{
    if (!ctx || ctx->checkin_count >= OKR_MAX_CHECKINS * 4) return -1;
    OKRCheckin *ci = &ctx->checkins[ctx->checkin_count];
    ci->id = ctx->next_id++;
    ci->objective_id = objective_id;
    ci->key_result_id = -1;
    ci->week_number = week_number;
    ci->checkin_date = time(NULL);
    ci->confidence = confidence;
    strncpy(ci->notes, notes, OKR_MAX_DESC - 1);
    ci->notes[OKR_MAX_DESC - 1] = '\0';
    ctx->checkin_count++;
    return ci->id;
}

int okr_alignment_link(OKRContext *ctx, int child_objective_id,
                        int parent_objective_id)
{
    if (!ctx) return -1;
    for (int i = 0; i < ctx->objective_count; i++) {
        if (ctx->objectives[i].id == child_objective_id) {
            ctx->objectives[i].parent_objective_id = parent_objective_id;
            return 0;
        }
    }
    return -1;
}

int okr_alignment_validate(const OKRContext *ctx)
{
    if (!ctx) return 0;
    int issues = 0;
    for (int i = 0; i < ctx->objective_count; i++) {
        const OKRObjective *obj = &ctx->objectives[i];
        if (obj->parent_objective_id > 0) {
            int found_parent = 0;
            for (int j = 0; j < ctx->objective_count; j++) {
                if (ctx->objectives[j].id == obj->parent_objective_id) {
                    found_parent = 1;
                    break;
                }
            }
            if (!found_parent) issues++;
        }
    }
    return issues;
}

int okr_objective_find_at_risk(OKRContext *ctx, int *result_ids,
                                int max_results)
{
    if (!ctx || !result_ids) return 0;
    int count = 0;
    for (int i = 0; i < ctx->objective_count && count < max_results; i++) {
        OKRGradeBreakdown grade = okr_grade_calculate(ctx, ctx->objectives[i].id);
        if (grade.overall < 0.3) {
            result_ids[count++] = ctx->objectives[i].id;
        }
    }
    return count;
}

int cfr_conversation_record(OKRContext *ctx, int objective_id,
                             int participant_id, const char *content)
{
    if (!ctx || ctx->cfr_count >= OKR_MAX_CFR_ENTRIES) return -1;
    CFREntry *entry = &ctx->cfr_entries[ctx->cfr_count];
    entry->id = ctx->next_id++;
    entry->objective_id = objective_id;
    entry->type = CFR_TYPE_CONVERSATION;
    entry->participant_id = participant_id;
    entry->target_participant_id = 0;
    strncpy(entry->content, content, OKR_MAX_DESC - 1);
    entry->content[OKR_MAX_DESC - 1] = '\0';
    entry->date = time(NULL);
    ctx->cfr_count++;
    return entry->id;
}

int cfr_feedback_record(OKRContext *ctx, int objective_id,
                         int giver_id, int receiver_id,
                         const char *content)
{
    if (!ctx || ctx->cfr_count >= OKR_MAX_CFR_ENTRIES) return -1;
    CFREntry *entry = &ctx->cfr_entries[ctx->cfr_count];
    entry->id = ctx->next_id++;
    entry->objective_id = objective_id;
    entry->type = CFR_TYPE_FEEDBACK;
    entry->participant_id = giver_id;
    entry->target_participant_id = receiver_id;
    strncpy(entry->content, content, OKR_MAX_DESC - 1);
    entry->content[OKR_MAX_DESC - 1] = '\0';
    entry->date = time(NULL);
    ctx->cfr_count++;
    return entry->id;
}

int cfr_recognition_record(OKRContext *ctx, int objective_id,
                            int giver_id, int receiver_id,
                            const char *content)
{
    if (!ctx || ctx->cfr_count >= OKR_MAX_CFR_ENTRIES) return -1;
    CFREntry *entry = &ctx->cfr_entries[ctx->cfr_count];
    entry->id = ctx->next_id++;
    entry->objective_id = objective_id;
    entry->type = CFR_TYPE_RECOGNITION;
    entry->participant_id = giver_id;
    entry->target_participant_id = receiver_id;
    strncpy(entry->content, content, OKR_MAX_DESC - 1);
    entry->content[OKR_MAX_DESC - 1] = '\0';
    entry->date = time(NULL);
    ctx->cfr_count++;
    return entry->id;
}

double okr_confidence_trend(const OKRContext *ctx, int objective_id)
{
    if (!ctx) return 0;
    double total = 0;
    int count = 0;
    for (int i = 0; i < ctx->checkin_count; i++) {
        if (ctx->checkins[i].objective_id == objective_id) {
            total += ctx->checkins[i].confidence;
            count++;
        }
    }
    return count > 0 ? total / count : 0;
}

void okr_objective_print(const OKRObjective *obj)
{
    if (!obj) return;
    printf("Objective #%d: %s\n", obj->id, obj->title);
    printf("  Level: %d | Status: %d | Grade: %.2f\n",
           obj->level, obj->status, obj->overall_grade);
    printf("  %s\n", obj->description);
    printf("  Key Results: %d\n", obj->key_result_count);
}

void okr_key_result_print(const OKRKeyResult *kr)
{
    if (!kr) return;
    double range = kr->target_value - kr->start_value;
    double pct = 0;
    if (fabs(range) > 1e-9) {
        pct = (kr->current_value - kr->start_value) / range * 100;
    }
    printf("  KR #%d: %s (%.1f%%) [%.2f->%.2f %s]\n",
           kr->id, kr->title, pct,
           kr->start_value, kr->target_value, kr->unit);
}

void okr_grade_print(const OKRGradeBreakdown *grade)
{
    if (!grade) return;
    printf("OKR Grade: %.2f (achievement=%.2f, progress=%.2f)\n",
           grade->overall, grade->achievement, grade->progress);
}

void okr_checkin_print(const OKRCheckin *checkin)
{
    if (!checkin) return;
    printf("Checkin #%d (week %d): confidence=%.2f | %s\n",
           checkin->id, checkin->week_number,
           checkin->confidence, checkin->notes);
}

void cfr_entry_print(const CFREntry *entry)
{
    if (!entry) return;
    static const char *type_names[] = {"Conversation", "Feedback", "Recognition"};
    printf("[%s] #%d: %s\n",
           entry->type < 3 ? type_names[entry->type] : "Unknown",
           entry->id, entry->content);
}

void okr_context_summary(const OKRContext *ctx)
{
    if (!ctx) return;
    printf("=== OKR Context Summary ===\n");
    printf("Objectives: %d | Key Results: %d\n",
           ctx->objective_count, ctx->key_result_count);
    printf("Checkins: %d | CFR Entries: %d\n",
           ctx->checkin_count, ctx->cfr_count);
    for (int i = 0; i < ctx->objective_count; i++) {
        okr_objective_print(&ctx->objectives[i]);
    }
}
