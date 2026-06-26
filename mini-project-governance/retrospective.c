#include "retrospective.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

static const char *retro_format_names[] = {
    "Start/Stop/Continue",
    "Mad/Sad/Glad",
    "4Ls (Liked/Learned/Lacked/Longed)",
    "Sailboat",
    "KALM (Keep/Add/Less/More)"
};

const char *retro_format_name(RetroFormat format)
{
    if (format >= 0 && format < (int)(sizeof(retro_format_names) /
                                      sizeof(retro_format_names[0])))
        return retro_format_names[format];
    return "Unknown";
}

RetroContext *retro_context_create(void)
{
    RetroContext *ctx = (RetroContext *)calloc(1, sizeof(RetroContext));
    if (!ctx) return NULL;
    ctx->session_count = 0;
    ctx->session_capacity = 16;
    ctx->pending_action_count = 0;
    ctx->pending_action_capacity = 64;
    ctx->sessions = (RetrospectiveSession *)calloc(
        ctx->session_capacity, sizeof(RetrospectiveSession));
    ctx->pending_actions = (ActionItem *)calloc(
        ctx->pending_action_capacity, sizeof(ActionItem));
    return ctx;
}

void retro_context_free(RetroContext *ctx)
{
    if (!ctx) return;
    free(ctx->sessions);
    free(ctx->pending_actions);
    free(ctx);
}

RetrospectiveSession *retro_session_create(RetroContext *ctx,
                                            RetroFormat format, int sprint_id)
{
    if (!ctx || ctx->session_count >= ctx->session_capacity) return NULL;
    RetrospectiveSession *session = &ctx->sessions[ctx->session_count];
    session->id = ctx->session_count + 1;
    session->format = format;
    session->sprint_id = sprint_id;
    session->date = time(NULL);
    session->item_count = 0;
    session->action_item_count = 0;
    session->next_action_id = 1;
    session->engagement_score = 0;
    ctx->session_count++;
    return session;
}

int retro_add_item_ssc(RetrospectiveSession *session, RetroSSC category,
                        const char *text, const char *author)
{
    if (!session || session->item_count >= RETRO_MAX_ITEMS) return -1;
    RetroCard *card = &session->items[session->item_count];
    card->category = (int)category;
    strncpy(card->text, text, RETRO_MAX_TEXT - 1);
    card->text[RETRO_MAX_TEXT - 1] = '\0';
    strncpy(card->author, author, RETRO_MAX_PARTICIPANT_NAME - 1);
    card->author[RETRO_MAX_PARTICIPANT_NAME - 1] = '\0';
    session->item_count++;
    return session->item_count - 1;
}

int retro_add_item_msg(RetrospectiveSession *session, RetroMSG category,
                        const char *text, const char *author)
{
    if (!session || session->item_count >= RETRO_MAX_ITEMS) return -1;
    RetroCard *card = &session->items[session->item_count];
    card->category = (int)category + 10;
    strncpy(card->text, text, RETRO_MAX_TEXT - 1);
    card->text[RETRO_MAX_TEXT - 1] = '\0';
    strncpy(card->author, author, RETRO_MAX_PARTICIPANT_NAME - 1);
    card->author[RETRO_MAX_PARTICIPANT_NAME - 1] = '\0';
    session->item_count++;
    return session->item_count - 1;
}

int retro_add_item_4ls(RetrospectiveSession *session, Retro4L category,
                        const char *text, const char *author)
{
    if (!session || session->item_count >= RETRO_MAX_ITEMS) return -1;
    RetroCard *card = &session->items[session->item_count];
    card->category = (int)category + 20;
    strncpy(card->text, text, RETRO_MAX_TEXT - 1);
    card->text[RETRO_MAX_TEXT - 1] = '\0';
    strncpy(card->author, author, RETRO_MAX_PARTICIPANT_NAME - 1);
    card->author[RETRO_MAX_PARTICIPANT_NAME - 1] = '\0';
    session->item_count++;
    return session->item_count - 1;
}

int retro_action_item_create(RetrospectiveSession *session,
                              const char *description,
                              const char *assignee_name, int priority)
{
    if (!session || session->action_item_count >= RETRO_MAX_ACTION_ITEMS)
        return -1;
    ActionItem *action = &session->action_items[session->action_item_count];
    action->id = session->next_action_id++;
    strncpy(action->description, description, RETRO_MAX_TEXT - 1);
    action->description[RETRO_MAX_TEXT - 1] = '\0';
    action->assignee_id = 0;
    strncpy(action->assignee_name, assignee_name,
            RETRO_MAX_PARTICIPANT_NAME - 1);
    action->assignee_name[RETRO_MAX_PARTICIPANT_NAME - 1] = '\0';
    action->status = ACTION_STATUS_OPEN;
    action->sprint_id = session->sprint_id;
    action->created_at = time(NULL);
    action->completed_at = 0;
    action->priority = priority;
    session->action_item_count++;
    return action->id;
}

int retro_action_item_track(RetroContext *ctx, int action_id,
                             ActionItemStatus new_status)
{
    if (!ctx) return -1;
    for (int s = 0; s < ctx->session_count; s++) {
        RetrospectiveSession *session = &ctx->sessions[s];
        for (int a = 0; a < session->action_item_count; a++) {
            if (session->action_items[a].id == action_id) {
                session->action_items[a].status = new_status;
                if (new_status == ACTION_STATUS_DONE) {
                    session->action_items[a].completed_at = time(NULL);
                }
                return 0;
            }
        }
    }
    return -1;
}

int retro_action_item_find_pending(const RetroContext *ctx,
                                    int *result_ids, int max_results)
{
    if (!ctx || !result_ids) return 0;
    int count = 0;
    for (int s = 0; s < ctx->session_count && count < max_results; s++) {
        const RetrospectiveSession *session = &ctx->sessions[s];
        for (int a = 0; a < session->action_item_count && count < max_results; a++) {
            if (session->action_items[a].status != ACTION_STATUS_DONE &&
                session->action_items[a].status != ACTION_STATUS_WONT_DO) {
                result_ids[count++] = session->action_items[a].id;
            }
        }
    }
    return count;
}

RetroHealthCheck *retro_health_check_create(int sprint_id)
{
    RetroHealthCheck *check = (RetroHealthCheck *)calloc(1, sizeof(RetroHealthCheck));
    if (!check) return NULL;
    check->sprint_id = sprint_id;
    check->date = time(NULL);
    check->question_count = 0;
    check->overall_health = 0;
    return check;
}

int retro_health_question_add(RetroHealthCheck *check, const char *question)
{
    if (!check || check->question_count >= RETRO_MAX_HEALTH_QUESTIONS) return -1;
    HealthCheckQuestion *q = &check->questions[check->question_count];
    strncpy(q->question, question, RETRO_MAX_TEXT - 1);
    q->question[RETRO_MAX_TEXT - 1] = '\0';
    q->score = 0;
    q->response_count = 0;
    check->question_count++;
    return check->question_count - 1;
}

int retro_health_answer_record(RetroHealthCheck *check, int question_index,
                                double score)
{
    if (!check || question_index < 0 || question_index >= check->question_count)
        return -1;
    HealthCheckQuestion *q = &check->questions[question_index];
    q->score = (q->score * q->response_count + score) / (q->response_count + 1);
    q->response_count++;
    return 0;
}

double retro_health_score(const RetroHealthCheck *check)
{
    if (!check || check->question_count == 0) return 0;
    double total = 0;
    int answered = 0;
    for (int i = 0; i < check->question_count; i++) {
        if (check->questions[i].response_count > 0) {
            total += check->questions[i].score;
            answered++;
        }
    }
    return answered > 0 ? total / answered : 0;
}

TeamMoraleSurvey *retro_morale_survey_create(void)
{
    TeamMoraleSurvey *survey = (TeamMoraleSurvey *)calloc(1, sizeof(TeamMoraleSurvey));
    if (!survey) return NULL;
    survey->date = time(NULL);
    survey->question_count = 0;
    survey->morale_index = 0;
    return survey;
}

int retro_morale_question_add(TeamMoraleSurvey *survey, const char *question)
{
    if (!survey || survey->question_count >= RETRO_MAX_SURVEY_QUESTIONS) return -1;
    MoraleSurveyQuestion *q = &survey->questions[survey->question_count];
    strncpy(q->question, question, RETRO_MAX_TEXT - 1);
    q->question[RETRO_MAX_TEXT - 1] = '\0';
    q->agree_count = 0;
    q->neutral_count = 0;
    q->disagree_count = 0;
    q->total_responses = 0;
    survey->question_count++;
    return survey->question_count - 1;
}

int retro_morale_answer_record(TeamMoraleSurvey *survey, int question_index,
                                int response_type)
{
    if (!survey || question_index < 0 || question_index >= survey->question_count)
        return -1;
    MoraleSurveyQuestion *q = &survey->questions[question_index];
    q->total_responses++;
    if (response_type == 0) q->agree_count++;
    else if (response_type == 1) q->neutral_count++;
    else q->disagree_count++;
    return 0;
}

double retro_morale_index(const TeamMoraleSurvey *survey)
{
    if (!survey || survey->question_count == 0) return 0;
    double total_score = 0;
    int answered = 0;
    for (int i = 0; i < survey->question_count; i++) {
        const MoraleSurveyQuestion *q = &survey->questions[i];
        if (q->total_responses > 0) {
            double score = (double)q->agree_count / q->total_responses;
            total_score += score;
            answered++;
        }
    }
    return answered > 0 ? total_score / answered : 0;
}

PsychSafetyAssessment *retro_psych_safety_assess(void)
{
    PsychSafetyAssessment *assessment =
        (PsychSafetyAssessment *)calloc(1, sizeof(PsychSafetyAssessment));
    if (!assessment) return NULL;
    assessment->date = time(NULL);
    assessment->question_count = 0;
    assessment->safety_index = 0;
    assessment->safe_participants = 0;
    assessment->total_participants = 0;
    return assessment;
}

int retro_psych_question_add(PsychSafetyAssessment *assessment,
                              const char *statement)
{
    if (!assessment || assessment->question_count >= RETRO_MAX_PSYCH_QUESTIONS)
        return -1;
    PsychSafetyQuestion *q = &assessment->questions[assessment->question_count];
    strncpy(q->statement, statement, RETRO_MAX_TEXT - 1);
    q->statement[RETRO_MAX_TEXT - 1] = '\0';
    q->avg_score = 0;
    q->response_count = 0;
    q->safe_zone_count = 0;
    assessment->question_count++;
    return assessment->question_count - 1;
}

int retro_psych_answer_record(PsychSafetyAssessment *assessment,
                               int question_index, double score)
{
    if (!assessment || question_index < 0 ||
        question_index >= assessment->question_count) return -1;
    PsychSafetyQuestion *q = &assessment->questions[question_index];
    q->avg_score = (q->avg_score * q->response_count + score) /
                    (q->response_count + 1);
    q->response_count++;
    if (score >= 4.0) q->safe_zone_count++;
    assessment->total_participants = q->response_count;
    return 0;
}

double retro_psych_safety_index(const PsychSafetyAssessment *assessment)
{
    if (!assessment || assessment->question_count == 0) return 0;
    double total = 0;
    for (int i = 0; i < assessment->question_count; i++) {
        total += assessment->questions[i].avg_score;
    }
    double avg = total / assessment->question_count;
    return avg / 5.0;
}

int retro_psych_safe_threshold(const PsychSafetyAssessment *assessment,
                                double threshold)
{
    if (!assessment || assessment->question_count == 0) return 0;
    int safe_count = 0;
    for (int i = 0; i < assessment->question_count; i++) {
        if (assessment->questions[i].avg_score >= threshold) safe_count++;
    }
    return safe_count;
}

void retro_session_print(const RetrospectiveSession *session)
{
    if (!session) return;
    printf("=== Retro #%d [%s] ===\n",
           session->id, retro_format_name(session->format));
    printf("Sprint: %d | Items: %d | Actions: %d\n",
           session->sprint_id, session->item_count, session->action_item_count);
    for (int i = 0; i < session->item_count; i++) {
        printf("  [%d] %s: %s\n",
               session->items[i].category,
               session->items[i].author,
               session->items[i].text);
    }
    printf("Action Items:\n");
    for (int i = 0; i < session->action_item_count; i++) {
        printf("  #%d [%d] %s -> %s\n",
               session->action_items[i].id,
               session->action_items[i].status,
               session->action_items[i].description,
               session->action_items[i].assignee_name);
    }
}

void retro_health_print(const RetroHealthCheck *check)
{
    if (!check) return;
    printf("Retro Health Check (Sprint #%d): %.2f/5.0\n",
           check->sprint_id, check->overall_health);
    for (int i = 0; i < check->question_count; i++) {
        printf("  %s: %.2f (%d responses)\n",
               check->questions[i].question,
               check->questions[i].score,
               check->questions[i].response_count);
    }
}

void retro_morale_print(const TeamMoraleSurvey *survey)
{
    if (!survey) return;
    printf("Team Morale Survey: %.2f\n", survey->morale_index);
    for (int i = 0; i < survey->question_count; i++) {
        const MoraleSurveyQuestion *q = &survey->questions[i];
        printf("  %s: agree=%d neutral=%d disagree=%d\n",
               q->question, q->agree_count, q->neutral_count, q->disagree_count);
    }
}

void retro_psych_print(const PsychSafetyAssessment *assessment)
{
    if (!assessment) return;
    printf("Psychological Safety Assessment: %.2f/1.0 (%.2f/5.0 avg)\n",
           assessment->safety_index,
           assessment->safety_index * 5.0);
    for (int i = 0; i < assessment->question_count; i++) {
        printf("  %s: %.2f (safe=%d/%d)\n",
               assessment->questions[i].statement,
               assessment->questions[i].avg_score,
               assessment->questions[i].safe_zone_count,
               assessment->questions[i].response_count);
    }
}

void retro_action_print(const ActionItem *action)
{
    if (!action) return;
    static const char *status_names[] = {"Open", "InProgress", "Done", "WontDo"};
    printf("Action #%d [%s] %s -> %s (P:%d)\n",
           action->id,
           action->status < 4 ? status_names[action->status] : "?",
           action->description,
           action->assignee_name,
           action->priority);
}
