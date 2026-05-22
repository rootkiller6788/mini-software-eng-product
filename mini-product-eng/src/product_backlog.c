#include "product_backlog.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void backlog_init(Backlog *bl, int velocity, int sprint_days) {
    memset(bl, 0, sizeof(*bl));
    bl->velocity = velocity; bl->sprint_length_days = sprint_days;
}

int backlog_add(Backlog *bl, const char *id, const char *title, Priority prio,
                BacklogItemType type, int points, int value) {
    if (bl->item_count >= MAX_BACKLOG_ITEMS) return -1;
    BacklogItem *bi = &bl->items[bl->item_count];
    strncpy(bi->id, id, 15); bi->id[15] = '\0';
    strncpy(bi->title, title, 127); bi->title[127] = '\0';
    bi->priority = prio; bi->type = type;
    bi->story_points = points; bi->business_value = value;
    bi->estimated_hours = 0; bi->actual_hours = 0;
    bi->accepted = false; bi->blocked = false;
    bl->total_points += points;
    return bl->item_count++;
}

bool backlog_remove(Backlog *bl, const char *id) {
    for (int i = 0; i < bl->item_count; i++)
        if (strcmp(bl->items[i].id, id) == 0) {
            bl->total_points -= bl->items[i].story_points;
            if (i < bl->item_count - 1)
                memmove(&bl->items[i], &bl->items[i+1], (bl->item_count - i - 1) * sizeof(BacklogItem));
            bl->item_count--; return true;
        }
    return false;
}

static int cmp_priority(const void *a, const void *b) {
    return ((const BacklogItem*)a)->priority - ((const BacklogItem*)b)->priority;
}

static int cmp_value_effort(const void *a, const void *b) {
    const BacklogItem *ia = a, *ib = b;
    double ra = ia->story_points > 0 ? (double)ia->business_value / ia->story_points : 0;
    double rb = ib->story_points > 0 ? (double)ib->business_value / ib->story_points : 0;
    return (ra < rb) - (ra > rb); /* descending */
}

void backlog_sort_by_priority(Backlog *bl) {
    qsort(bl->items, bl->item_count, sizeof(BacklogItem), cmp_priority);
}

void backlog_sort_by_value_effort(Backlog *bl) {
    qsort(bl->items, bl->item_count, sizeof(BacklogItem), cmp_value_effort);
}

int backlog_sprint_capacity(Backlog *bl) {
    int pts = 0, count = 0;
    Backlog sorted = *bl;
    backlog_sort_by_priority(&sorted);
    for (int i = 0; i < sorted.item_count; i++) {
        if (pts + sorted.items[i].story_points <= bl->velocity && !sorted.items[i].accepted) {
            pts += sorted.items[i].story_points; count++;
        }
    }
    return count;
}

void backlog_accept(Backlog *bl, const char *id, int actual_hours) {
    for (int i = 0; i < bl->item_count; i++)
        if (strcmp(bl->items[i].id, id) == 0) {
            bl->items[i].accepted = true; bl->items[i].actual_hours = actual_hours; return;
        }
}

void backlog_block(Backlog *bl, const char *id) {
    for (int i = 0; i < bl->item_count; i++)
        if (strcmp(bl->items[i].id, id) == 0) { bl->items[i].blocked = true; return; }
}

void backlog_print(Backlog *bl) {
    const char *ts[] = {"STORY","BUG","TASK","SPIKE"};
    printf("=== Backlog: %d items (%d pts, velocity=%d) ===\n", bl->item_count, bl->total_points, bl->velocity);
    for (int i = 0; i < bl->item_count && i < 20; i++) {
        BacklogItem *bi = &bl->items[i];
        printf("  %s [P%d] %s (%s) %dpt val=%d%s%s\n",
               bi->id, bi->priority, bi->title, ts[bi->type],
               bi->story_points, bi->business_value,
               bi->accepted ? " ACCEPTED" : "", bi->blocked ? " BLOCKED" : "");
    }
}
