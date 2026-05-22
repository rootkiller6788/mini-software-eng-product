#ifndef ROADMAP_PLAN_H
#define ROADMAP_PLAN_H

#include <stdbool.h>
#include <stddef.h>

typedef enum {
    TIME_HORIZON_NOW,
    TIME_HORIZON_NEXT,
    TIME_HORIZON_LATER,
    TIME_HORIZON_COUNT
} TimeHorizon;

typedef struct {
    double reach;
    double impact;
    double confidence;
    double effort;
    double rice_score;
    char feature_name[128];
    char description[256];
    TimeHorizon horizon;
    bool committed;
} RiceFeature;

typedef struct {
    double importance;
    double satisfaction_gap;
    double opportunity_score;
    char opportunity_name[128];
} OpportunityScore;

typedef struct {
    char theme_name[128];
    RiceFeature *features;
    size_t feature_count;
    TimeHorizon horizon;
    char okr_objective[256];
    char okr_key_results[5][128];
    int kr_count;
} RoadmapTheme;

typedef struct {
    RoadmapTheme *themes;
    size_t theme_count;
    RiceFeature *backlog;
    size_t backlog_count;
    char stakeholder_message[1024];
    int quarter;
    int year;
} ProductRoadmap;

void   roadmap_init(ProductRoadmap *r, int quarter, int year);
void   roadmap_add_feature(ProductRoadmap *r, const char *name, const char *desc,
                           double reach, double impact, double confidence, double effort);
double roadmap_rice_score(double reach, double impact, double confidence, double effort);
void   roadmap_prioritize_by_rice(ProductRoadmap *r);
void   roadmap_assign_horizon(RiceFeature *f, double rice_threshold_now, double rice_threshold_next);
void   roadmap_add_theme(ProductRoadmap *r, const char *theme_name, TimeHorizon horizon);
void   roadmap_set_okr(RoadmapTheme *t, const char *objective, const char **key_results, int kr_count);
void   roadmap_add_feature_to_theme(RoadmapTheme *t, const RiceFeature *f);
double roadmap_calc_opportunity_score(double importance, double satisfaction_gap);
void   roadmap_generate_stakeholder_message(ProductRoadmap *r, char *buffer, size_t buf_size);
int    roadmap_count_by_horizon(const ProductRoadmap *r, TimeHorizon hz);
void   roadmap_auto_commit_now(ProductRoadmap *r, int max_features);
void   roadmap_free(ProductRoadmap *r);

#endif
