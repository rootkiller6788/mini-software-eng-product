#include "kanban_board.h"

#include <stdio.h>
#include <stdlib.h>

int main(void)
{
    printf("=== Kanban Board Example ===\n\n");

    KanbanBoard *board = kanban_board_create("Team Alpha Board");
    if (!board) {
        printf("Failed to create board\n");
        return 1;
    }

    kanban_wip_limit_set(board, KANBAN_COL_TODO, 5);
    kanban_wip_limit_set(board, KANBAN_COL_IN_PROGRESS, 3);
    kanban_wip_limit_set(board, KANBAN_COL_REVIEW, 2);

    int c1 = kanban_card_create(board, "Fix login timeout", "Users get disconnected", 1);
    int c2 = kanban_card_create(board, "Add export CSV", "Export reports to CSV", 2);
    int c3 = kanban_card_create(board, "Update dependencies", "Security patches", 3);
    int c4 = kanban_card_create(board, "Refactor auth module", "Clean up old code", 2);
    int c5 = kanban_card_create(board, "Write integration tests", "API integration tests", 2);

    kanban_card_move(board, c1, KANBAN_COL_TODO);
    kanban_card_move(board, c2, KANBAN_COL_TODO);
    kanban_card_move(board, c3, KANBAN_COL_TODO);
    kanban_card_move(board, c4, KANBAN_COL_TODO);

    kanban_card_move(board, c1, KANBAN_COL_IN_PROGRESS);
    kanban_card_move(board, c2, KANBAN_COL_IN_PROGRESS);
    kanban_card_move(board, c3, KANBAN_COL_IN_PROGRESS);

    int cfd_check = kanban_cfd_snapshot(board);
    printf("CFD snapshot: %s\n", cfd_check == 0 ? "OK" : "Failed");

    kanban_card_move(board, c1, KANBAN_COL_REVIEW);
    kanban_card_move(board, c1, KANBAN_COL_DONE);

    kanban_card_move(board, c4, KANBAN_COL_IN_PROGRESS);
    kanban_card_block(board, c3, "Waiting for security team review");

    kanban_cfd_snapshot(board);

    printf("\n");
    kanban_board_print(board);

    printf("\n");
    KanbanCard *card = kanban_card_find(board, c1);
    if (card) kanban_card_print(card);

    printf("\nWIP check for In Progress: %s\n",
           kanban_wip_check(board, KANBAN_COL_IN_PROGRESS) == 0 ? "OK" : "AT LIMIT");

    printf("\n");
    KanbanFlowMetrics metrics = kanban_flow_metrics_calculate(board);
    kanban_metrics_print(&metrics);

    printf("\n");
    KanbanBottleneckReport reports[8];
    int bn_count = kanban_bottleneck_detect(board, reports, 8);
    kanban_bottleneck_print(reports, bn_count);

    printf("\nThroughput: %.2f cards/day\n",
           kanban_throughput_calculate(board, 7));

    printf("\nCFD Data:\n");
    KanbanCFDPoint cfd_points[32];
    int cfd_count = 0;
    kanban_cfd_generate(board, cfd_points, 32, &cfd_count);
    for (int i = 0; i < cfd_count; i++) {
        printf("  Snapshot %d: Backlog=%d, Todo=%d, InProgress=%d, Review=%d, Done=%d\n",
               i,
               cfd_points[i].column_counts[KANBAN_COL_BACKLOG],
               cfd_points[i].column_counts[KANBAN_COL_TODO],
               cfd_points[i].column_counts[KANBAN_COL_IN_PROGRESS],
               cfd_points[i].column_counts[KANBAN_COL_REVIEW],
               cfd_points[i].column_counts[KANBAN_COL_DONE]);
    }

    kanban_board_free(board);
    printf("\nDone.\n");
    return 0;
}
