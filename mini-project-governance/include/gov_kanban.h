#ifndef GOV_KANBAN_H
#define GOV_KANBAN_H
#include <stdint.h>
#include <stdbool.h>
#include <time.h>

/* ============================================================
 * L1: Kanban System — Core Definitions
 * L4: Little's Law — WIP = Throughput × Cycle Time
 * Reference: Kanban Method (Anderson), Toyota Production System
 * ============================================================ */

#define KANBAN_MAX_COLUMNS     12
#define KANBAN_MAX_CARDS       128
#define KANBAN_NAME_LEN        64
#define KANBAN_DESC_LEN        128
#define KANBAN_ID_LEN          32

/* ---- Card Types & Classes of Service ---- */
typedef enum {
    COS_STANDARD = 0,      /* normal priority, FIFO */
    COS_FIXED_DATE,        /* must complete by deadline */
    COS_EXPEDITE,          /* highest priority, preempt */
    COS_INTANGIBLE         /* discovery/analysis work */
} ClassOfService;

typedef struct {
    char id[KANBAN_ID_LEN];
    char title[KANBAN_NAME_LEN];
    char description[KANBAN_DESC_LEN];
    ClassOfService cos;
    int column_index;          /* current column position */
    int priority;
    int story_points;
    uint32_t created_at;
    uint32_t started_at;
    uint32_t finished_at;
    uint32_t deadline;         /* for COS_FIXED_DATE */
    int blocked_days;
    bool blocked;
    char block_reason[KANBAN_DESC_LEN];
} KanbanCard;

/* ---- Kanban Column ---- */
typedef struct {
    char name[KANBAN_NAME_LEN];
    int wip_limit;             /* Work-In-Progress limit */
    int card_count;
    int card_indices[KANBAN_MAX_CARDS];
    bool is_done_column;
} KanbanColumn;

/* ---- Kanban Board ---- */
typedef struct {
    KanbanColumn columns[KANBAN_MAX_COLUMNS];
    int column_count;
    KanbanCard cards[KANBAN_MAX_CARDS];
    int card_count;
    int total_completed;
    uint32_t start_date;
    /* Flow metrics — L4: Little's Law */
    double avg_cycle_time;        /* avg time from start to finish */
    double avg_throughput;        /* cards completed per day */
    double avg_wip;               /* average cards in progress */
    int cycle_time_samples;
} KanbanBoard;

/* ---- Cumulative Flow Data ---- */
typedef struct {
    int day;
    int counts[KANBAN_MAX_COLUMNS];
} CfdSnapshot;

typedef struct {
    CfdSnapshot snapshots[90];
    int snapshot_count;
} CumulativeFlowDiagram;

/* ---- API ---- */
void kanban_init(KanbanBoard *kb);
int  kanban_add_column(KanbanBoard *kb, const char *name, int wip_limit, bool is_done);
bool kanban_set_wip_limit(KanbanBoard *kb, int col_idx, int limit);

int  kanban_add_card(KanbanBoard *kb, const char *id, const char *title,
                     ClassOfService cos, int points, uint32_t deadline);
bool kanban_move_card(KanbanBoard *kb, const char *card_id, int to_col);
int  kanban_find_card(KanbanBoard *kb, const char *card_id);

bool kanban_block_card(KanbanBoard *kb, const char *card_id, const char *reason);
bool kanban_unblock_card(KanbanBoard *kb, const char *card_id);

/* L4: Little's Law verification */
void kanban_calc_flow_metrics(KanbanBoard *kb);
bool kanban_verify_littles_law(KanbanBoard *kb, double tolerance);
double kanban_expected_cycle_time(KanbanBoard *kb, double current_wip);

/* Board queries */
int  kanban_wip_total(KanbanBoard *kb);
int  kanban_wip_in_column(KanbanBoard *kb, int col_idx);
bool kanban_column_blocked(KanbanBoard *kb, int col_idx);
int  kanban_bottleneck_column(KanbanBoard *kb);

/* Cumulative Flow */
void cfd_init(CumulativeFlowDiagram *cfd);
void cfd_record(CumulativeFlowDiagram *cfd, KanbanBoard *kb, int day);
void cfd_print(CumulativeFlowDiagram *cfd);

/* Print */
void kanban_print_board(KanbanBoard *kb);
void kanban_print_metrics(KanbanBoard *kb);

#endif
