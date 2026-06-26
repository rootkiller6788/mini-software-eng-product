#include "gov_kanban.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

/* ============================================================
 * L2: Kanban System Implementation
 * Core concepts: WIP limits, pull system, flow metrics
 * L4: Little's Law verification: WIP = Throughput * CycleTime
 * L5: Cumulative Flow Diagram, Bottleneck detection
 * ============================================================ */

void kanban_init(KanbanBoard *kb) {
    memset(kb, 0, sizeof(*kb));
    kb->start_date = (uint32_t)time(NULL);
    kb->avg_cycle_time = 0;
    kb->avg_throughput = 0;
    kb->avg_wip = 0;
    kb->cycle_time_samples = 0;
}

int kanban_add_column(KanbanBoard *kb, const char *name, int wip_limit, bool is_done) {
    if (kb->column_count >= KANBAN_MAX_COLUMNS) return -1;
    if (wip_limit < 1 && !is_done) wip_limit = 5;
    KanbanColumn *col = &kb->columns[kb->column_count];
    strncpy(col->name, name, KANBAN_NAME_LEN - 1);
    col->name[KANBAN_NAME_LEN - 1] = '\0';
    col->wip_limit = wip_limit;
    col->card_count = 0;
    col->is_done_column = is_done;
    return kb->column_count++;
}

bool kanban_set_wip_limit(KanbanBoard *kb, int col_idx, int limit) {
    if (col_idx < 0 || col_idx >= kb->column_count || limit < 0) return false;
    kb->columns[col_idx].wip_limit = limit;
    return true;
}

int kanban_add_card(KanbanBoard *kb, const char *id, const char *title,
                    ClassOfService cos, int points, uint32_t deadline) {
    if (kb->card_count >= KANBAN_MAX_CARDS) return -1;
    KanbanCard *c = &kb->cards[kb->card_count];
    memset(c, 0, sizeof(*c));
    strncpy(c->id, id, KANBAN_ID_LEN - 1); c->id[KANBAN_ID_LEN - 1] = '\0';
    strncpy(c->title, title, KANBAN_NAME_LEN - 1); c->title[KANBAN_NAME_LEN - 1] = '\0';
    c->cos = cos;
    c->story_points = points;
    c->column_index = 0;
    c->deadline = deadline;
    c->created_at = (uint32_t)time(NULL);
    c->blocked = false;
    c->blocked_days = 0;
    if (kb->column_count > 0) {
        kb->columns[0].card_indices[kb->columns[0].card_count++] = kb->card_count;
        c->column_index = 0;
    }
    return kb->card_count++;
}

int kanban_find_card(KanbanBoard *kb, const char *card_id) {
    for (int i = 0; i < kb->card_count; i++) {
        if (strcmp(kb->cards[i].id, card_id) == 0) return i;
    }
    return -1;
}

bool kanban_move_card(KanbanBoard *kb, const char *card_id, int to_col) {
    int ci = kanban_find_card(kb, card_id);
    if (ci < 0 || to_col < 0 || to_col >= kb->column_count) return false;
    KanbanCard *c = &kb->cards[ci];
    int from_col = c->column_index;

    /* Check WIP limit for target column (unless expedite or done column) */
    if (!kb->columns[to_col].is_done_column && c->cos != COS_EXPEDITE) {
        if (kb->columns[to_col].card_count >= kb->columns[to_col].wip_limit) {
            return false; /* WIP limit reached */
        }
    }

    /* Remove from source column */
    KanbanColumn *src = &kb->columns[from_col];
    for (int i = 0; i < src->card_count; i++) {
        if (src->card_indices[i] == ci) {
            memmove(&src->card_indices[i], &src->card_indices[i + 1],
                    (src->card_count - i - 1) * sizeof(int));
            src->card_count--;
            break;
        }
    }

    /* Add to target column */
    KanbanColumn *dst = &kb->columns[to_col];
    dst->card_indices[dst->card_count++] = ci;
    c->column_index = to_col;

    /* Track cycle time */
    if (from_col == 0 && to_col > 0 && c->started_at == 0) {
        c->started_at = (uint32_t)time(NULL);
    }

    /* Track completion */
    if (dst->is_done_column) {
        c->finished_at = (uint32_t)time(NULL);
        kb->total_completed++;
        double cycle_time = (double)(c->finished_at - c->started_at) / 86400.0;
        if (cycle_time > 0) {
            kb->cycle_time_samples++;
            kb->avg_cycle_time = kb->avg_cycle_time +
                (cycle_time - kb->avg_cycle_time) / kb->cycle_time_samples;
        }
    }
    return true;
}

bool kanban_block_card(KanbanBoard *kb, const char *card_id, const char *reason) {
    int ci = kanban_find_card(kb, card_id);
    if (ci < 0) return false;
    kb->cards[ci].blocked = true;
    strncpy(kb->cards[ci].block_reason, reason, KANBAN_DESC_LEN - 1);
    kb->cards[ci].block_reason[KANBAN_DESC_LEN - 1] = '\0';
    return true;
}

bool kanban_unblock_card(KanbanBoard *kb, const char *card_id) {
    int ci = kanban_find_card(kb, card_id);
    if (ci < 0) return false;
    kb->cards[ci].blocked = false;
    return true;
}

/* L4: Little's Law — WIP = Throughput * Cycle Time
 * Verification: compute WIP from throughput and cycle time,
 * compare with actual WIP count, return true if within tolerance
 */
void kanban_calc_flow_metrics(KanbanBoard *kb) {
    /* Throughput: cards completed per day */
    double elapsed_days = (double)(time(NULL) - kb->start_date) / 86400.0;
    if (elapsed_days > 0) {
        kb->avg_throughput = (double)kb->total_completed / elapsed_days;
    }
    /* Average WIP */
    int total_wip = 0;
    int non_done_cols = 0;
    for (int i = 0; i < kb->column_count; i++) {
        if (!kb->columns[i].is_done_column) {
            total_wip += kb->columns[i].card_count;
            non_done_cols++;
        }
    }
    kb->avg_wip = (double)total_wip;
}

bool kanban_verify_littles_law(KanbanBoard *kb, double tolerance) {
    kanban_calc_flow_metrics(kb);
    if (kb->avg_throughput <= 0 || kb->avg_cycle_time <= 0) return true;
    double predicted_wip = kb->avg_throughput * kb->avg_cycle_time;
    double diff = fabs(predicted_wip - kb->avg_wip) / (kb->avg_wip > 0 ? kb->avg_wip : 1.0);
    return diff <= tolerance;
}

double kanban_expected_cycle_time(KanbanBoard *kb, double current_wip) {
    if (kb->avg_throughput <= 0) return -1.0;
    return current_wip / kb->avg_throughput;
}

int kanban_wip_total(KanbanBoard *kb) {
    int wip = 0;
    for (int i = 0; i < kb->column_count; i++) {
        if (!kb->columns[i].is_done_column) wip += kb->columns[i].card_count;
    }
    return wip;
}

int kanban_wip_in_column(KanbanBoard *kb, int col_idx) {
    if (col_idx < 0 || col_idx >= kb->column_count) return -1;
    return kb->columns[col_idx].card_count;
}

bool kanban_column_blocked(KanbanBoard *kb, int col_idx) {
    if (col_idx < 0 || col_idx >= kb->column_count) return false;
    KanbanColumn *col = &kb->columns[col_idx];
    for (int i = 0; i < col->card_count; i++) {
        if (kb->cards[col->card_indices[i]].blocked) return true;
    }
    return false;
}

/* L5: Bottleneck detection — find column with highest WIP/limit ratio */
int kanban_bottleneck_column(KanbanBoard *kb) {
    int bottleneck = -1;
    double max_ratio = 0;
    for (int i = 0; i < kb->column_count; i++) {
        if (kb->columns[i].is_done_column) continue;
        if (kb->columns[i].wip_limit <= 0) continue;
        double ratio = (double)kb->columns[i].card_count / kb->columns[i].wip_limit;
        if (ratio > max_ratio) {
            max_ratio = ratio;
            bottleneck = i;
        }
    }
    return bottleneck;
}

/* Cumulative Flow Diagram */
void cfd_init(CumulativeFlowDiagram *cfd) {
    memset(cfd, 0, sizeof(*cfd));
}

void cfd_record(CumulativeFlowDiagram *cfd, KanbanBoard *kb, int day) {
    if (cfd->snapshot_count >= 90) return;
    CfdSnapshot *snap = &cfd->snapshots[cfd->snapshot_count];
    snap->day = day;
    int cumulative = 0;
    for (int i = 0; i < kb->column_count && i < KANBAN_MAX_COLUMNS; i++) {
        cumulative += kb->columns[i].card_count;
        snap->counts[i] = cumulative;
    }
    cfd->snapshot_count++;
}

void cfd_print(CumulativeFlowDiagram *cfd) {
    printf("=== Cumulative Flow Diagram ===\n");
    printf("  Day |");
    for (int i = 0; i < KANBAN_MAX_COLUMNS && cfd->snapshots[0].counts[i] >= 0; i++)
        printf(" Col%-2d|", i);
    printf("\n");
    for (int d = 0; d < cfd->snapshot_count; d++) {
        printf("  %3d |", cfd->snapshots[d].day);
        for (int i = 0; i < KANBAN_MAX_COLUMNS && i < 8; i++)
            printf(" %4d |", cfd->snapshots[d].counts[i]);
        printf("\n");
    }
}

void kanban_print_board(KanbanBoard *kb) {
    printf("=== Kanban Board (%d columns, %d cards) ===\n", kb->column_count, kb->card_count);
    for (int i = 0; i < kb->column_count; i++) {
        KanbanColumn *col = &kb->columns[i];
        printf("  [%s] WIP: %d/%d %s\n", col->name, col->card_count, col->wip_limit,
               col->is_done_column ? "(DONE)" : "");
        for (int j = 0; j < col->card_count; j++) {
            KanbanCard *c = &kb->cards[col->card_indices[j]];
            printf("    - %s [%s] %s pts=%d\n", c->id, c->title,
                   c->cos == COS_EXPEDITE ? "EXPEDITE" : "",
                   c->story_points);
            if (c->blocked) printf("      BLOCKED: %s\n", c->block_reason);
        }
    }
}

void kanban_print_metrics(KanbanBoard *kb) {
    kanban_calc_flow_metrics(kb);
    printf("=== Flow Metrics ===\n");
    printf("  Avg Cycle Time: %.1f days\n", kb->avg_cycle_time);
    printf("  Avg Throughput: %.2f cards/day\n", kb->avg_throughput);
    printf("  Current WIP: %d\n", kanban_wip_total(kb));
    printf("  Total Completed: %d\n", kb->total_completed);
    printf("  Little's Law: WIP=%.1f, Tput*CT=%.1f\n",
           kb->avg_wip, kb->avg_throughput * kb->avg_cycle_time);
    int bn = kanban_bottleneck_column(kb);
    if (bn >= 0) printf("  Bottleneck: Column %d (%s)\n", bn, kb->columns[bn].name);
}