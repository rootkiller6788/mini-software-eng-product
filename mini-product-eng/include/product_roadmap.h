#ifndef PRODUCT_ROADMAP_H
#define PRODUCT_ROADMAP_H
#include <stdbool.h>
#include <stdint.h>

#define MAX_THEMES 16
#define MAX_ITEMS_PER_THEME 32

typedef enum { RITEM_EPIC, RITEM_FEATURE, RITEM_STORY, RITEM_TECH_DEBT } RoadmapItemType;
typedef enum { RSTATUS_PLANNED, RSTATUS_IN_PROGRESS, RSTATUS_DELIVERED, RSTATUS_CANCELLED } RoadmapStatus;

typedef struct {
    char id[16]; char title[128]; RoadmapItemType type; RoadmapStatus status;
    int quarter_target; /* Q1-Q4 */ int effort_weeks; char depends_on[16];
} RoadmapItem;

typedef struct {
    char name[64]; char description[256];
    RoadmapItem items[MAX_ITEMS_PER_THEME]; int item_count;
} Theme;

typedef struct {
    Theme themes[MAX_THEMES]; int theme_count;
    int current_quarter; int current_year;
    char vision[512];
} ProductRoadmap;

void roadmap_init(ProductRoadmap *pr, const char *vision, int year, int quarter);
int  roadmap_add_theme(ProductRoadmap *pr, const char *name, const char *desc);
int  roadmap_add_item(ProductRoadmap *pr, int theme_idx, const char *id, const char *title, RoadmapItemType type, int target_quarter, int effort);
bool roadmap_update_status(ProductRoadmap *pr, const char *item_id, RoadmapStatus new_status);
int  roadmap_items_in_quarter(ProductRoadmap *pr, int quarter);
int  roadmap_total_effort(ProductRoadmap *pr, int quarter);
void roadmap_print(ProductRoadmap *pr);
#endif
