#include "product_roadmap.h"
#include <stdio.h>
#include <string.h>

void roadmap_init(ProductRoadmap *pr, const char *vision, int year, int quarter) {
    memset(pr, 0, sizeof(*pr));
    strncpy(pr->vision, vision, 511); pr->vision[511] = '\0';
    pr->current_year = year; pr->current_quarter = quarter;
}

int roadmap_add_theme(ProductRoadmap *pr, const char *name, const char *desc) {
    if (pr->theme_count >= MAX_THEMES) return -1;
    Theme *t = &pr->themes[pr->theme_count];
    strncpy(t->name, name, 63); t->name[63] = '\0';
    strncpy(t->description, desc, 255); t->description[255] = '\0';
    t->item_count = 0;
    return pr->theme_count++;
}

int roadmap_add_item(ProductRoadmap *pr, int theme_idx, const char *id, const char *title,
                     RoadmapItemType type, int target_quarter, int effort) {
    if (theme_idx < 0 || theme_idx >= pr->theme_count) return -1;
    Theme *t = &pr->themes[theme_idx];
    if (t->item_count >= MAX_ITEMS_PER_THEME) return -1;
    RoadmapItem *ri = &t->items[t->item_count];
    strncpy(ri->id, id, 15); ri->id[15] = '\0';
    strncpy(ri->title, title, 127); ri->title[127] = '\0';
    ri->type = type; ri->quarter_target = target_quarter;
    ri->effort_weeks = effort; ri->status = RSTATUS_PLANNED;
    ri->depends_on[0] = '\0';
    return t->item_count++;
}

bool roadmap_update_status(ProductRoadmap *pr, const char *item_id, RoadmapStatus new_status) {
    for (int i = 0; i < pr->theme_count; i++)
        for (int j = 0; j < pr->themes[i].item_count; j++)
            if (strcmp(pr->themes[i].items[j].id, item_id) == 0) {
                pr->themes[i].items[j].status = new_status; return true;
            }
    return false;
}

int roadmap_items_in_quarter(ProductRoadmap *pr, int quarter) {
    int count = 0;
    for (int i = 0; i < pr->theme_count; i++)
        for (int j = 0; j < pr->themes[i].item_count; j++)
            if (pr->themes[i].items[j].quarter_target == quarter) count++;
    return count;
}

int roadmap_total_effort(ProductRoadmap *pr, int quarter) {
    int total = 0;
    for (int i = 0; i < pr->theme_count; i++)
        for (int j = 0; j < pr->themes[i].item_count; j++)
            if (pr->themes[i].items[j].quarter_target == quarter)
                total += pr->themes[i].items[j].effort_weeks;
    return total;
}

void roadmap_print(ProductRoadmap *pr) {
    const char *ts[] = {"EPIC","FEATURE","STORY","DEBT"};
    const char *ss[] = {"PLAN","PROG","DONE","CANC"};
    printf("=== Product Roadmap: Q%d %d ===\n%s\n", pr->current_quarter, pr->current_year, pr->vision);
    for (int i = 0; i < pr->theme_count; i++) {
        printf("Theme: %s\n", pr->themes[i].name);
        for (int j = 0; j < pr->themes[i].item_count; j++) {
            RoadmapItem *ri = &pr->themes[i].items[j];
            printf("  [%s] %s (%s) Q%d %dw [%s]\n", ri->id, ri->title, ts[ri->type], ri->quarter_target, ri->effort_weeks, ss[ri->status]);
        }
    }
}
