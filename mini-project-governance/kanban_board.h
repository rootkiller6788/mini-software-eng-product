#ifndef KANBAN_BOARD_H
#define KANBAN_BOARD_H

#include <stddef.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

#define KANBAN_MAX_CARDS 2048
#define KANBAN_MAX_TITLE 256
#define KANBAN_MAX_DESC 1024
#define KANBAN_MAX_COLUMNS 16
#define KANBAN_MAX_COLUMN_NAME 64
#define KANBAN_MAX_CFD_DAYS 365
#define KANBAN_DEFAULT_WIP_LIMIT 5

typedef enum {
    KANBAN_COL_BACKLOG = 0,
    KANBAN_COL_TODO,
    KANBAN_COL_IN_PROGRESS,
    KANBAN_COL_REVIEW,
    KANBAN_COL_DONE,
    KANBAN_COL_CUSTOM_1,
    KANBAN_COL_CUSTOM_2,
    KANBAN_COL_CUSTOM_3
} KanbanColumnType;

typedef struct {
    KanbanColumnType type;
    char name[KANBAN_MAX_COLUMN_NAME];
    int wip_limit;
    int card_count;
} KanbanColumn;

typedef struct {
    int id;
    char title[KANBAN_MAX_TITLE];
    char description[KANBAN_MAX_DESC];
    KanbanColumnType column;
    int priority;
    time_t created_at;
    time_t started_at;
    time_t completed_at;
    int blocked;
    char block_reason[KANBAN_MAX_DESC];
    int assignee_id;
    int labels[8];
    int label_count;
} KanbanCard;

typedef struct {
    time_t date;
    int column_counts[KANBAN_COL_DONE + 1];
} KanbanCFDPoint;

typedef struct {
    double cycle_time_days;
    double lead_time_days;
    double throughput_per_week;
    double wip_avg;
    double flow_efficiency;
    int total_cards_completed;
    time_t measurement_start;
    time_t measurement_end;
} KanbanFlowMetrics;

typedef struct {
    KanbanColumnType column_type;
    double congestion_score;
    int cards_accumulated;
    int wip_limit;
    int is_bottleneck;
} KanbanBottleneckReport;

typedef struct {
    char name[KANBAN_MAX_TITLE];
    KanbanColumn columns[KANBAN_MAX_COLUMNS];
    int column_count;
    KanbanCard cards[KANBAN_MAX_CARDS];
    int card_count;
    int next_card_id;
    KanbanCFDPoint cfd_history[KANBAN_MAX_CFD_DAYS];
    int cfd_count;
    time_t created_at;
} KanbanBoard;

KanbanBoard *kanban_board_create(const char *name);
void kanban_board_free(KanbanBoard *board);

int kanban_column_add(KanbanBoard *board, KanbanColumnType type,
                       const char *name);
int kanban_wip_limit_set(KanbanBoard *board, KanbanColumnType type, int limit);
int kanban_wip_check(const KanbanBoard *board, KanbanColumnType type);

int kanban_card_create(KanbanBoard *board, const char *title,
                        const char *description, int priority);
int kanban_card_move(KanbanBoard *board, int card_id, KanbanColumnType to_column);
int kanban_card_block(KanbanBoard *board, int card_id, const char *reason);
int kanban_card_unblock(KanbanBoard *board, int card_id);
KanbanCard *kanban_card_find(KanbanBoard *board, int card_id);

int kanban_cfd_snapshot(KanbanBoard *board);
int kanban_cfd_generate(const KanbanBoard *board, KanbanCFDPoint *output,
                         int max_points, int *count);

KanbanFlowMetrics kanban_flow_metrics_calculate(const KanbanBoard *board);

int kanban_bottleneck_detect(const KanbanBoard *board,
                              KanbanBottleneckReport *reports, int max_reports);

double kanban_cycle_time_calculate(const KanbanCard *card);
double kanban_lead_time_calculate(const KanbanCard *card);
double kanban_throughput_calculate(const KanbanBoard *board, int days);

int kanban_column_count(const KanbanBoard *board, KanbanColumnType type);

void kanban_board_print(const KanbanBoard *board);
void kanban_card_print(const KanbanCard *card);
void kanban_metrics_print(const KanbanFlowMetrics *metrics);
void kanban_bottleneck_print(const KanbanBottleneckReport *report, int count);

const char *kanban_column_type_name(KanbanColumnType type);

#ifdef __cplusplus
}
#endif

#endif
