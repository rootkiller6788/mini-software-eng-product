#ifndef PRODUCT_BACKLOG_H
#define PRODUCT_BACKLOG_H
#include <stdbool.h>

#define MAX_BACKLOG_ITEMS 128

typedef enum { PRIO_CRITICAL=1, PRIO_HIGH=2, PRIO_MEDIUM=3, PRIO_LOW=4, PRIO_NICE_TO_HAVE=5 } Priority;
typedef enum { BLITEM_STORY, BLITEM_BUG, BLITEM_TASK, BLITEM_SPIKE } BacklogItemType;

typedef struct {
    char id[16]; char title[128]; Priority priority; BacklogItemType type;
    int story_points; int business_value; /* 1-10 */
    int estimated_hours; int actual_hours;
    bool accepted; bool blocked;
} BacklogItem;

typedef struct {
    BacklogItem items[MAX_BACKLOG_ITEMS]; int item_count;
    int total_points; int velocity; /* points per sprint */
    int sprint_length_days;
} Backlog;

void backlog_init(Backlog *bl, int velocity, int sprint_days);
int  backlog_add(Backlog *bl, const char *id, const char *title, Priority prio, BacklogItemType type, int points, int value);
bool backlog_remove(Backlog *bl, const char *id);
void backlog_sort_by_priority(Backlog *bl);
void backlog_sort_by_value_effort(Backlog *bl);  /* business_value / story_points */
int  backlog_sprint_capacity(Backlog *bl);       /* items fitting in velocity */
void backlog_accept(Backlog *bl, const char *id, int actual_hours);
void backlog_block(Backlog *bl, const char *id);
void backlog_print(Backlog *bl);
#endif
