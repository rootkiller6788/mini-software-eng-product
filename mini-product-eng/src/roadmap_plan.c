#include "roadmap_plan.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *g_horizon_labels[] = { "NOW", "NEXT", "LATER" };

void roadmap_init(ProductRoadmap *r, int quarter, int year) {
    if (!r) return;
    memset(r, 0, sizeof(*r));
    r->quarter = quarter;
    r->year = year;
}

double roadmap_rice_score(double reach, double impact, double confidence, double effort) {
    if (effort <= 0.0) effort = 0.01;
    return (reach * impact * confidence) / effort;
}

void roadmap_add_feature(ProductRoadmap *r, const char *name, const char *desc,
                         double reach, double impact, double confidence, double effort) {
    if (!r || !name) return;
    r->backlog_count++;
    RiceFeature *tmp = realloc(r->backlog, r->backlog_count * sizeof(RiceFeature));
    if (!tmp) return;
    r->backlog = tmp;
    RiceFeature *f = &r->backlog[r->backlog_count - 1];
    memset(f, 0, sizeof(*f));
    strncpy(f->feature_name, name, sizeof(f->feature_name) - 1);
    f->feature_name[sizeof(f->feature_name) - 1] = '\0';
    if (desc) {
        strncpy(f->description, desc, sizeof(f->description) - 1);
        f->description[sizeof(f->description) - 1] = '\0';
    }
    f->reach = reach;
    f->impact = impact;
    f->confidence = confidence;
    f->effort = effort;
    f->rice_score = roadmap_rice_score(reach, impact, confidence, effort);
    f->horizon = TIME_HORIZON_LATER;
}

static int rice_compare_desc(const void *a, const void *b) {
    const RiceFeature *fa = (const RiceFeature *)a;
    const RiceFeature *fb = (const RiceFeature *)b;
    if (fb->rice_score > fa->rice_score) return 1;
    if (fb->rice_score < fa->rice_score) return -1;
    return 0;
}

void roadmap_prioritize_by_rice(ProductRoadmap *r) {
    if (!r || r->backlog_count == 0) return;
    qsort(r->backlog, r->backlog_count, sizeof(RiceFeature), rice_compare_desc);
    double threshold_now = 5.0;
    double threshold_next = 2.0;
    for (size_t i = 0; i < r->backlog_count; i++) {
        roadmap_assign_horizon(&r->backlog[i], threshold_now, threshold_next);
    }
}

void roadmap_assign_horizon(RiceFeature *f, double rice_threshold_now, double rice_threshold_next) {
    if (!f) return;
    if (f->rice_score >= rice_threshold_now) {
        f->horizon = TIME_HORIZON_NOW;
    } else if (f->rice_score >= rice_threshold_next) {
        f->horizon = TIME_HORIZON_NEXT;
    } else {
        f->horizon = TIME_HORIZON_LATER;
    }
}

void roadmap_add_theme(ProductRoadmap *r, const char *theme_name, TimeHorizon horizon) {
    if (!r || !theme_name) return;
    r->theme_count++;
    RoadmapTheme *tmp = realloc(r->themes, r->theme_count * sizeof(RoadmapTheme));
    if (!tmp) return;
    r->themes = tmp;
    RoadmapTheme *t = &r->themes[r->theme_count - 1];
    memset(t, 0, sizeof(*t));
    strncpy(t->theme_name, theme_name, sizeof(t->theme_name) - 1);
    t->theme_name[sizeof(t->theme_name) - 1] = '\0';
    t->horizon = horizon;
}

void roadmap_set_okr(RoadmapTheme *t, const char *objective, const char **key_results, int kr_count) {
    if (!t || !objective) return;
    strncpy(t->okr_objective, objective, sizeof(t->okr_objective) - 1);
    t->okr_objective[sizeof(t->okr_objective) - 1] = '\0';
    t->kr_count = (kr_count < 5) ? kr_count : 5;
    for (int i = 0; i < t->kr_count; i++) {
        if (key_results && key_results[i]) {
            strncpy(t->okr_key_results[i], key_results[i], sizeof(t->okr_key_results[i]) - 1);
            t->okr_key_results[i][sizeof(t->okr_key_results[i]) - 1] = '\0';
        }
    }
}

void roadmap_add_feature_to_theme(RoadmapTheme *t, const RiceFeature *f) {
    if (!t || !f) return;
    t->feature_count++;
    RiceFeature *tmp = realloc(t->features, t->feature_count * sizeof(RiceFeature));
    if (!tmp) return;
    t->features = tmp;
    memcpy(&t->features[t->feature_count - 1], f, sizeof(RiceFeature));
}

double roadmap_calc_opportunity_score(double importance, double satisfaction_gap) {
    return importance + fmax(importance - satisfaction_gap, 0.0);
}

int roadmap_count_by_horizon(const ProductRoadmap *r, TimeHorizon hz) {
    if (!r) return 0;
    int count = 0;
    for (size_t i = 0; i < r->backlog_count; i++) {
        if (r->backlog[i].horizon == hz) count++;
    }
    return count;
}

void roadmap_auto_commit_now(ProductRoadmap *r, int max_features) {
    if (!r || max_features <= 0) return;
    int committed = 0;
    for (size_t i = 0; i < r->backlog_count && committed < max_features; i++) {
        if (r->backlog[i].horizon == TIME_HORIZON_NOW && !r->backlog[i].committed) {
            r->backlog[i].committed = true;
            committed++;
        }
    }
}

void roadmap_generate_stakeholder_message(ProductRoadmap *r, char *buffer, size_t buf_size) {
    if (!r || !buffer || buf_size == 0) return;
    int written = 0;
    written = snprintf(buffer, buf_size,
        "=== Product Roadmap Q%d %d ===\n\n", r->quarter, r->year);

    if (r->theme_count > 0) {
        written += snprintf(buffer + written, buf_size - written,
            "Strategic Themes (%zu total):\n", r->theme_count);
        for (size_t i = 0; i < r->theme_count; i++) {
            RoadmapTheme *t = &r->themes[i];
            written += snprintf(buffer + written, buf_size - written,
                "  [%s] %s\n", g_horizon_labels[t->horizon], t->theme_name);
            if (t->okr_objective[0]) {
                written += snprintf(buffer + written, buf_size - written,
                    "  Objective: %s\n", t->okr_objective);
            }
            for (int j = 0; j < t->kr_count; j++) {
                if (t->okr_key_results[j][0]) {
                    written += snprintf(buffer + written, buf_size - written,
                        "    KR%d: %s\n", j + 1, t->okr_key_results[j]);
                }
            }
        }
        written += snprintf(buffer + written, buf_size - written, "\n");
    }

    written += snprintf(buffer + written, buf_size - written,
        "Prioritized Feature Backlog (by RICE score):\n");
    for (size_t i = 0; i < r->backlog_count && i < 10; i++) {
        RiceFeature *f = &r->backlog[i];
        const char *hz = g_horizon_labels[f->horizon];
        written += snprintf(buffer + written, buf_size - written,
            "  %s [%s] RICE=%.2f (R:%.0f I:%.0f C:%.0f%% E:%.0f)\n",
            f->feature_name, hz, f->rice_score,
            f->reach, f->impact, f->confidence * 100.0, f->effort);
    }

    if (written > 0 && (size_t)written < sizeof(r->stakeholder_message)) {
        strncpy(r->stakeholder_message, buffer, sizeof(r->stakeholder_message) - 1);
    }
}

void roadmap_free(ProductRoadmap *r) {
    if (!r) return;
    for (size_t i = 0; i < r->theme_count; i++) {
        free(r->themes[i].features);
    }
    free(r->themes);
    free(r->backlog);
    memset(r, 0, sizeof(*r));
}
