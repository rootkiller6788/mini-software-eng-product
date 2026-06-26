#include "gov_retro.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

/* ============================================================
 * L2: Retrospective System Implementation
 * Core concepts: Structured retrospectives, action tracking,
 *                team health monitoring, 5 Whys root cause analysis
 * L8: 5 Whys root cause analysis
 * ============================================================ */

void retro_init(RetroLog *log) {
    memset(log, 0, sizeof(*log));
    log->session_count = 0;
    log->open_action_count = 0;
    log->trend_points = 0;
}

int retro_create_session(RetroLog *log, int sprint_number, const char *goal,
                         RetroFormat format, const char *facilitator) {
    if (log->session_count >= RETRO_MAX_SPRINTS) return -1;
    RetroSession *rs = &log->sessions[log->session_count];
    memset(rs, 0, sizeof(*rs));
    rs->sprint_number = sprint_number;
    strncpy(rs->sprint_goal, goal, RETRO_DESC_LEN - 1);
    rs->sprint_goal[RETRO_DESC_LEN - 1] = '\0';
    rs->format = format;
    strncpy(rs->facilitator, facilitator, RETRO_NAME_LEN - 1);
    rs->facilitator[RETRO_NAME_LEN - 1] = '\0';
    rs->session_date = (uint32_t)time(NULL);
    rs->completed = false;
    rs->item_count = 0;
    rs->action_count = 0;
    rs->health_count = 0;
    rs->total_votes_cast = 0;
    return log->session_count++;
}

int retro_add_item(RetroLog *log, int session_idx, const char *content,
                   const char *author, RetroCategory cat) {
    if (session_idx < 0 || session_idx >= log->session_count) return -1;
    RetroSession *rs = &log->sessions[session_idx];
    if (rs->item_count >= RETRO_MAX_ITEMS) return -1;
    if (rs->completed) return -1;
    RetroItem *ri = &rs->items[rs->item_count];
    memset(ri, 0, sizeof(*ri));
    snprintf(ri->id, RETRO_ID_LEN, "RI-%d-%d", session_idx, rs->item_count);
    strncpy(ri->content, content, RETRO_DESC_LEN - 1);
    ri->content[RETRO_DESC_LEN - 1] = '\0';
    strncpy(ri->author, author, RETRO_NAME_LEN - 1);
    ri->author[RETRO_NAME_LEN - 1] = '\0';
    ri->category = cat;
    ri->votes = 0;
    ri->discussed = false;
    ri->has_action_item = false;
    ri->action_item_id = -1;
    return rs->item_count++;
}

/* L5: Dot voting — each vote increments a retro item */
void retro_vote(RetroLog *log, int session_idx, int item_idx) {
    if (session_idx < 0 || session_idx >= log->session_count) return;
    RetroSession *rs = &log->sessions[session_idx];
    if (item_idx < 0 || item_idx >= rs->item_count) return;
    if (rs->completed) return;
    rs->items[item_idx].votes++;
    rs->total_votes_cast++;
}

/* L5: Get top-voted items (descending by votes) */
int retro_top_voted(RetroLog *log, int session_idx, int *results, int max) {
    if (session_idx < 0 || session_idx >= log->session_count) return 0;
    RetroSession *rs = &log->sessions[session_idx];
    if (rs->item_count == 0) return 0;

    /* Create index array and sort by votes desc */
    int *indices = (int *)malloc(rs->item_count * sizeof(int));
    if (!indices) return 0;
    for (int i = 0; i < rs->item_count; i++) indices[i] = i;

    /* Simple selection sort by votes desc */
    for (int i = 0; i < rs->item_count - 1; i++) {
        int max_idx = i;
        for (int j = i + 1; j < rs->item_count; j++) {
            if (rs->items[indices[j]].votes > rs->items[indices[max_idx]].votes) max_idx = j;
        }
        int t = indices[i]; indices[i] = indices[max_idx]; indices[max_idx] = t;
    }

    int count = 0;
    for (int i = 0; i < rs->item_count && count < max; i++) {
        if (rs->items[indices[i]].votes > 0) results[count++] = indices[i];
    }
    free(indices);
    return count;
}

int retro_add_action(RetroLog *log, int session_idx, const char *desc,
                     const char *owner, uint32_t due_date, double impact, double effort) {
    if (session_idx < 0 || session_idx >= log->session_count) return -1;
    RetroSession *rs = &log->sessions[session_idx];
    if (rs->action_count >= RETRO_MAX_ACTIONS) return -1;
    ActionItem *ai = &rs->actions[rs->action_count];
    memset(ai, 0, sizeof(*ai));
    snprintf(ai->id, RETRO_ID_LEN, "ACT-%d-%d", session_idx, rs->action_count);
    strncpy(ai->description, desc, RETRO_DESC_LEN - 1);
    ai->description[RETRO_DESC_LEN - 1] = '\0';
    strncpy(ai->owner, owner, RETRO_NAME_LEN - 1);
    ai->owner[RETRO_NAME_LEN - 1] = '\0';
    ai->status = ACTION_OPEN;
    ai->sprint_created = rs->sprint_number;
    ai->created_at = (uint32_t)time(NULL);
    ai->due_date = due_date;
    ai->impact_score = impact;
    ai->effort_score = effort;

    /* Also add to global open actions list */
    if (log->open_action_count < RETRO_MAX_ACTIONS) {
        log->open_actions[log->open_action_count] = *ai;
        log->open_action_count++;
    }
    return rs->action_count++;
}

int retro_find_action(RetroLog *log, const char *action_id) {
    for (int i = 0; i < log->open_action_count; i++) {
        if (strcmp(log->open_actions[i].id, action_id) == 0) return i;
    }
    return -1;
}

bool retro_complete_action(RetroLog *log, const char *action_id) {
    int idx = retro_find_action(log, action_id);
    if (idx < 0) return false;
    log->open_actions[idx].status = ACTION_DONE;
    log->open_actions[idx].completed_at = (uint32_t)time(NULL);
    /* Remove from open list by compacting */
    memmove(&log->open_actions[idx], &log->open_actions[idx + 1],
            (log->open_action_count - idx - 1) * sizeof(ActionItem));
    log->open_action_count--;
    return true;
}

bool retro_reopen_action(RetroLog *log, const char *action_id) {
    /* Find in all sessions */
    for (int s = 0; s < log->session_count; s++) {
        for (int a = 0; a < log->sessions[s].action_count; a++) {
            if (strcmp(log->sessions[s].actions[a].id, action_id) == 0) {
                log->sessions[s].actions[a].status = ACTION_OPEN;
                log->sessions[s].actions[a].retry_count++;
                /* Re-add to open list */
                if (log->open_action_count < RETRO_MAX_ACTIONS) {
                    log->open_actions[log->open_action_count++] = log->sessions[s].actions[a];
                }
                return true;
            }
        }
    }
    return false;
}

int retro_pending_actions(RetroLog *log, int *results, int max) {
    int count = 0;
    for (int i = 0; i < log->open_action_count && count < max; i++) {
        if (log->open_actions[i].status == ACTION_OPEN ||
            log->open_actions[i].status == ACTION_IN_PROGRESS) {
            results[count++] = i;
        }
    }
    return count;
}

/* Health Check */
void retro_add_health(RetroLog *log, int session_idx, const char *participant,
                      double happiness, double engagement, double clarity,
                      double autonomy, double mastery, double team_trust) {
    if (session_idx < 0 || session_idx >= log->session_count) return;
    RetroSession *rs = &log->sessions[session_idx];
    if (rs->health_count >= RETRO_MAX_PARTICIPANTS) return;
    HealthCheck *hc = &rs->health_checks[rs->health_count];
    memset(hc, 0, sizeof(*hc));
    strncpy(hc->participant, participant, RETRO_NAME_LEN - 1);
    hc->participant[RETRO_NAME_LEN - 1] = '\0';
    hc->happiness = happiness;
    hc->engagement = engagement;
    hc->clarity = clarity;
    hc->autonomy = autonomy;
    hc->mastery = mastery;
    hc->team_trust = team_trust;
    hc->timestamp = (uint32_t)time(NULL);
    rs->health_count++;
}

/* L5: Compute average health metrics for a session */
void retro_calc_health_avg(RetroLog *log, int session_idx) {
    if (session_idx < 0 || session_idx >= log->session_count) return;
    RetroSession *rs = &log->sessions[session_idx];
    if (rs->health_count == 0) return;
    double h = 0, e = 0;
    for (int i = 0; i < rs->health_count; i++) {
        h += rs->health_checks[i].happiness;
        e += rs->health_checks[i].engagement;
    }
    rs->avg_happiness = h / rs->health_count;
    rs->avg_engagement = e / rs->health_count;
}

/* Track happiness trend over time */
void retro_update_happiness_trend(RetroLog *log) {
    log->trend_points = 0;
    for (int i = 0; i < log->session_count && i < RETRO_MAX_SPRINTS; i++) {
        retro_calc_health_avg(log, i);
        log->team_happiness_trend[i] = log->sessions[i].avg_happiness;
        log->trend_points++;
    }
}

/* L5: Detect declining team health (negative slope over window) */
bool retro_health_declining(RetroLog *log, int window) {
    if (log->session_count < window) return false;
    retro_update_happiness_trend(log);
    int n = log->trend_points;
    if (n < window) return false;
    /* Simple linear regression over last `window` points */
    double sum_x = 0, sum_y = 0, sum_xy = 0, sum_xx = 0;
    int start = n - window;
    for (int i = start; i < n; i++) {
        double x = i - start;
        double y = log->team_happiness_trend[i];
        sum_x += x; sum_y += y; sum_xy += x * y; sum_xx += x * x;
    }
    double slope = (window * sum_xy - sum_x * sum_y) / (window * sum_xx - sum_x * sum_x + 1e-10);
    return slope < -0.1;
}

/* 5 Whys Root Cause Analysis */
void five_whys_init(FiveWhys *fw, const char *problem) {
    memset(fw, 0, sizeof(*fw));
    strncpy(fw->problem, problem, RETRO_DESC_LEN - 1);
    fw->problem[RETRO_DESC_LEN - 1] = '\0';
    fw->depth_reached = 0;
}

bool five_whys_add(FiveWhys *fw, int level, const char *because) {
    if (level < 0 || level >= 5) return false;
    if (level > fw->depth_reached + 1) return false; /* Must fill sequentially */
    strncpy(fw->why_answers[level], because, RETRO_DESC_LEN - 1);
    fw->why_answers[level][RETRO_DESC_LEN - 1] = '\0';
    if (level + 1 > fw->depth_reached) fw->depth_reached = level + 1;
    return true;
}

void five_whys_solve(FiveWhys *fw, const char *root_cause, const char *countermeasure) {
    strncpy(fw->root_cause, root_cause, RETRO_DESC_LEN - 1);
    fw->root_cause[RETRO_DESC_LEN - 1] = '\0';
    strncpy(fw->countermeasure, countermeasure, RETRO_DESC_LEN - 1);
    fw->countermeasure[RETRO_DESC_LEN - 1] = '\0';
}

void retro_print_session(RetroLog *log, int session_idx) {
    if (session_idx < 0 || session_idx >= log->session_count) {
        printf("Session not found.\n"); return;
    }
    RetroSession *rs = &log->sessions[session_idx];
    printf("=== Retro Sprint %d ===\n", rs->sprint_number);
    printf("  Format: %d | Facilitator: %s\n", rs->format, rs->facilitator);
    printf("  Items: %d | Actions: %d | Votes: %d\n",
           rs->item_count, rs->action_count, rs->total_votes_cast);
    printf("  Health: %.1f happiness | %.1f engagement\n", rs->avg_happiness, rs->avg_engagement);

    /* Group items by category */
    const char *cats[] = {"Well","Wrong","Improve","Appreciate","Action","Puzzle"};
    for (int c = 0; c <= (int)RETRO_CAT_PUZZLE; c++) {
        int cnt = 0;
        for (int i = 0; i < rs->item_count; i++)
            if ((int)rs->items[i].category == c) cnt++;
        if (cnt > 0) {
            printf("  [%s] (%d):\n", cats[c], cnt);
            for (int i = 0; i < rs->item_count; i++) {
                if ((int)rs->items[i].category == c)
                    printf("    - [%d votes] %s\n", rs->items[i].votes, rs->items[i].content);
            }
        }
    }

    if (rs->action_count > 0) {
        printf("  Actions:\n");
        for (int i = 0; i < rs->action_count; i++) {
            ActionItem *a = &rs->actions[i];
            printf("    - [%d] %s (owner: %s, impact: %.0f)\n",
                   a->status, a->description, a->owner, a->impact_score);
        }
    }
}

void retro_print_summary(RetroLog *log) {
    printf("=== Retrospective Summary ===\n");
    printf("  Sessions: %d | Open Actions: %d\n", log->session_count, log->open_action_count);
    retro_update_happiness_trend(log);
    printf("  Health Trend: ");
    for (int i = 0; i < log->trend_points; i++)
        printf("%.1f ", log->team_happiness_trend[i]);
    printf("\n");
    bool declining = retro_health_declining(log, 3);
    printf("  Health Status: %s\n", declining ? "DECLINING - ACTION NEEDED!" : "STABLE");

    printf("  Open Actions:\n");
    for (int i = 0; i < log->open_action_count; i++) {
        ActionItem *a = &log->open_actions[i];
        printf("    - %s: %s [owner: %s]\n", a->id, a->description, a->owner);
    }
}

void retro_print_health_trend(RetroLog *log) {
    retro_update_happiness_trend(log);
    printf("=== Team Health Trend ===\n");
    for (int i = 0; i < log->trend_points; i++) {
        printf("  Sprint %d: Happiness=%.1f Engagement=%.1f\n",
               log->sessions[i].sprint_number,
               log->team_happiness_trend[i],
               log->sessions[i].avg_engagement);
    }
}

void five_whys_print(FiveWhys *fw) {
    printf("=== 5 Whys Analysis ===\n");
    printf("  Problem: %s\n", fw->problem);
    for (int i = 0; i < fw->depth_reached; i++) {
        printf("  Why #%d: %s\n", i + 1, fw->why_answers[i]);
    }
    printf("  Root Cause: %s\n", fw->root_cause);
    printf("  Countermeasure: %s\n", fw->countermeasure);
}