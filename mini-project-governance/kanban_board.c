#include "kanban_board.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static const char *column_names[] = {
    "Backlog", "To Do", "In Progress", "Review", "Done",
    "Custom 1", "Custom 2", "Custom 3"
};

const char *kanban_column_type_name(KanbanColumnType type)
{
    if (type >= 0 && type < (int)(sizeof(column_names) / sizeof(column_names[0])))
        return column_names[type];
    return "Unknown";
}

KanbanBoard *kanban_board_create(const char *name)
{
    KanbanBoard *board = (KanbanBoard *)calloc(1, sizeof(KanbanBoard));
    if (!board) return NULL;
    strncpy(board->name, name, KANBAN_MAX_TITLE - 1);
    board->name[KANBAN_MAX_TITLE - 1] = '\0';
    board->column_count = 0;
    board->card_count = 0;
    board->next_card_id = 1;
    board->cfd_count = 0;
    board->created_at = time(NULL);

    const char *default_names[] = {"Backlog", "To Do", "In Progress", "Review", "Done"};
    for (int i = 0; i < 5; i++) {
        kanban_column_add(board, (KanbanColumnType)i, default_names[i]);
    }
    return board;
}

void kanban_board_free(KanbanBoard *board)
{
    if (board) free(board);
}

int kanban_column_add(KanbanBoard *board, KanbanColumnType type,
                       const char *name)
{
    if (!board || board->column_count >= KANBAN_MAX_COLUMNS) return -1;
    for (int i = 0; i < board->column_count; i++) {
        if (board->columns[i].type == type) return -1;
    }
    KanbanColumn *col = &board->columns[board->column_count];
    col->type = type;
    strncpy(col->name, name, KANBAN_MAX_COLUMN_NAME - 1);
    col->name[KANBAN_MAX_COLUMN_NAME - 1] = '\0';
    col->wip_limit = KANBAN_DEFAULT_WIP_LIMIT;
    col->card_count = 0;
    board->column_count++;
    return 0;
}

int kanban_wip_limit_set(KanbanBoard *board, KanbanColumnType type, int limit)
{
    if (!board) return -1;
    for (int i = 0; i < board->column_count; i++) {
        if (board->columns[i].type == type) {
            board->columns[i].wip_limit = limit;
            return 0;
        }
    }
    return -1;
}

int kanban_wip_check(const KanbanBoard *board, KanbanColumnType type)
{
    if (!board) return -1;
    int current = kanban_column_count(board, type);
    for (int i = 0; i < board->column_count; i++) {
        if (board->columns[i].type == type) {
            if (current >= board->columns[i].wip_limit) return -1;
            return 0;
        }
    }
    return -1;
}

int kanban_card_create(KanbanBoard *board, const char *title,
                        const char *description, int priority)
{
    if (!board || board->card_count >= KANBAN_MAX_CARDS) return -1;
    KanbanCard *card = &board->cards[board->card_count];
    card->id = board->next_card_id++;
    strncpy(card->title, title, KANBAN_MAX_TITLE - 1);
    card->title[KANBAN_MAX_TITLE - 1] = '\0';
    strncpy(card->description, description, KANBAN_MAX_DESC - 1);
    card->description[KANBAN_MAX_DESC - 1] = '\0';
    card->column = KANBAN_COL_BACKLOG;
    card->priority = priority;
    card->created_at = time(NULL);
    card->started_at = 0;
    card->completed_at = 0;
    card->blocked = 0;
    card->block_reason[0] = '\0';
    card->assignee_id = 0;
    card->label_count = 0;
    board->card_count++;
    return card->id;
}

int kanban_card_move(KanbanBoard *board, int card_id, KanbanColumnType to_column)
{
    if (!board) return -1;
    KanbanCard *card = kanban_card_find(board, card_id);
    if (!card) return -1;
    KanbanColumnType from = card->column;
    card->column = to_column;
    if (to_column == KANBAN_COL_IN_PROGRESS && card->started_at == 0) {
        card->started_at = time(NULL);
    }
    if (to_column == KANBAN_COL_DONE) {
        card->completed_at = time(NULL);
    }
    kanban_cfd_snapshot(board);
    (void)from;
    return 0;
}

int kanban_card_block(KanbanBoard *board, int card_id, const char *reason)
{
    KanbanCard *card = kanban_card_find(board, card_id);
    if (!card) return -1;
    card->blocked = 1;
    strncpy(card->block_reason, reason, KANBAN_MAX_DESC - 1);
    card->block_reason[KANBAN_MAX_DESC - 1] = '\0';
    return 0;
}

int kanban_card_unblock(KanbanBoard *board, int card_id)
{
    KanbanCard *card = kanban_card_find(board, card_id);
    if (!card) return -1;
    card->blocked = 0;
    card->block_reason[0] = '\0';
    return 0;
}

KanbanCard *kanban_card_find(KanbanBoard *board, int card_id)
{
    if (!board) return NULL;
    for (int i = 0; i < board->card_count; i++) {
        if (board->cards[i].id == card_id) return &board->cards[i];
    }
    return NULL;
}

int kanban_cfd_snapshot(KanbanBoard *board)
{
    if (!board || board->cfd_count >= KANBAN_MAX_CFD_DAYS) return -1;
    KanbanCFDPoint *point = &board->cfd_history[board->cfd_count];
    point->date = time(NULL);
    for (int c = 0; c <= KANBAN_COL_DONE; c++) {
        point->column_counts[c] = kanban_column_count(board, (KanbanColumnType)c);
    }
    board->cfd_count++;
    return 0;
}

int kanban_cfd_generate(const KanbanBoard *board, KanbanCFDPoint *output,
                         int max_points, int *count)
{
    if (!board || !output || !count) return 0;
    int n = board->cfd_count;
    if (n > max_points) n = max_points;
    for (int i = 0; i < n; i++) {
        output[i] = board->cfd_history[i];
    }
    *count = n;
    return 0;
}

KanbanFlowMetrics kanban_flow_metrics_calculate(const KanbanBoard *board)
{
    KanbanFlowMetrics metrics = {0};
    if (!board || board->card_count == 0) return metrics;
    double total_cycle = 0, total_lead = 0;
    int completed = 0;
    time_t earliest = board->cards[0].created_at;
    for (int i = 0; i < board->card_count; i++) {
        if (board->cards[i].completed_at > 0) {
            completed++;
            double cycle = kanban_cycle_time_calculate(&board->cards[i]);
            double lead = kanban_lead_time_calculate(&board->cards[i]);
            total_cycle += cycle;
            total_lead += lead;
        }
        if (board->cards[i].created_at < earliest)
            earliest = board->cards[i].created_at;
    }
    if (completed > 0) {
        metrics.cycle_time_days = total_cycle / completed;
        metrics.lead_time_days = total_lead / completed;
    }
    metrics.total_cards_completed = completed;
    metrics.measurement_start = earliest;
    metrics.measurement_end = time(NULL);
    double period_days = (double)(metrics.measurement_end - earliest) / 86400.0;
    if (period_days > 0) {
        metrics.throughput_per_week = (completed / period_days) * 7.0;
    }
    double wip_total = 0;
    int wip_cols = 0;
    for (int i = 0; i < board->column_count; i++) {
        if (board->columns[i].type == KANBAN_COL_IN_PROGRESS ||
            board->columns[i].type == KANBAN_COL_REVIEW) {
            wip_total += kanban_column_count(board, board->columns[i].type);
            wip_cols++;
        }
    }
    metrics.wip_avg = wip_cols > 0 ? wip_total / wip_cols : 0;
    if (metrics.wip_avg > 0 && metrics.cycle_time_days > 0) {
        metrics.flow_efficiency = metrics.throughput_per_week /
            (metrics.wip_avg * 7.0 / metrics.cycle_time_days);
    }
    return metrics;
}

int kanban_bottleneck_detect(const KanbanBoard *board,
                              KanbanBottleneckReport *reports, int max_reports)
{
    if (!board || !reports) return 0;
    int count = 0;
    for (int i = 0; i < board->column_count && count < max_reports; i++) {
        KanbanColumnType type = board->columns[i].type;
        int cards = kanban_column_count(board, type);
        reports[count].column_type = type;
        reports[count].cards_accumulated = cards;
        reports[count].wip_limit = board->columns[i].wip_limit;
        reports[count].congestion_score =
            board->columns[i].wip_limit > 0
                ? (double)cards / board->columns[i].wip_limit
                : 0;
        reports[count].is_bottleneck =
            (reports[count].congestion_score >= 0.8) ? 1 : 0;
        count++;
    }
    return count;
}

double kanban_cycle_time_calculate(const KanbanCard *card)
{
    if (!card || card->started_at == 0 || card->completed_at == 0) return 0;
    return difftime(card->completed_at, card->started_at) / 86400.0;
}

double kanban_lead_time_calculate(const KanbanCard *card)
{
    if (!card || card->completed_at == 0) return 0;
    return difftime(card->completed_at, card->created_at) / 86400.0;
}

double kanban_throughput_calculate(const KanbanBoard *board, int days)
{
    if (!board || days <= 0) return 0;
    int done = kanban_column_count(board, KANBAN_COL_DONE);
    return (double)done / days;
}

int kanban_column_count(const KanbanBoard *board, KanbanColumnType type)
{
    int count = 0;
    if (!board) return 0;
    for (int i = 0; i < board->card_count; i++) {
        if (board->cards[i].column == type) count++;
    }
    return count;
}

void kanban_board_print(const KanbanBoard *board)
{
    if (!board) return;
    printf("Kanban Board: %s\n", board->name);
    printf("Columns: %d | Cards: %d | WIP Limits:\n",
           board->column_count, board->card_count);
    for (int i = 0; i < board->column_count; i++) {
        int count = kanban_column_count(board, board->columns[i].type);
        printf("  [%s] %d/%d cards",
               board->columns[i].name, count,
               board->columns[i].wip_limit);
        if (count >= board->columns[i].wip_limit)
            printf(" *** WIP LIMIT REACHED ***");
        printf("\n");
    }
}

void kanban_card_print(const KanbanCard *card)
{
    if (!card) return;
    printf("Card #%d: %s [%s]\n",
           card->id, card->title, kanban_column_type_name(card->column));
    if (card->blocked) printf("  BLOCKED: %s\n", card->block_reason);
}

void kanban_metrics_print(const KanbanFlowMetrics *metrics)
{
    if (!metrics) return;
    printf("Flow Metrics:\n");
    printf("  Cycle Time: %.2f days\n", metrics->cycle_time_days);
    printf("  Lead Time: %.2f days\n", metrics->lead_time_days);
    printf("  Throughput: %.2f / week\n", metrics->throughput_per_week);
    printf("  WIP Avg: %.2f\n", metrics->wip_avg);
    printf("  Completed: %d cards\n", metrics->total_cards_completed);
}

void kanban_bottleneck_print(const KanbanBottleneckReport *report, int count)
{
    printf("Bottleneck Detection (%d columns):\n", count);
    for (int i = 0; i < count; i++) {
        printf("  %s: congestion=%.2f, cards=%d, wip=%d %s\n",
               kanban_column_type_name(report[i].column_type),
               report[i].congestion_score,
               report[i].cards_accumulated,
               report[i].wip_limit,
               report[i].is_bottleneck ? "** BOTTLENECK **" : "");
    }
}
