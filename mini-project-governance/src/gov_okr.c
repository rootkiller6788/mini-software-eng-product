#include "gov_okr.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

/* ============================================================
 * L2: OKR System Implementation
 * Core concepts: Objectives, Key Results, grading, alignment
 * L3: OKR Tree with cascading alignment
 * L8: Advanced — OKR alignment tree traversal and scoring
 * ============================================================ */

void okr_init(OkrDashboard *dash, int year, int quarter) {
    memset(dash, 0, sizeof(*dash));
    dash->year = year;
    dash->quarter_number = quarter;
    dash->obj_count = 0;
    dash->kr_count = 0;
}

int okr_add_objective(OkrDashboard *dash, const char *id, const char *title,
                      const char *desc, bool company_level,
                      uint32_t start, uint32_t end) {
    if (dash->obj_count >= OKR_MAX_OBJECTIVES) return -1;
    Objective *o = &dash->objectives[dash->obj_count];
    memset(o, 0, sizeof(*o));
    strncpy(o->id, id, OKR_ID_LEN - 1); o->id[OKR_ID_LEN - 1] = '\0';
    strncpy(o->title, title, OKR_NAME_LEN - 1); o->title[OKR_NAME_LEN - 1] = '\0';
    strncpy(o->description, desc, OKR_DESC_LEN - 1); o->description[OKR_DESC_LEN - 1] = '\0';
    o->is_company_level = company_level;
    o->start_date = start;
    o->end_date = end;
    o->progress = 0.0;
    o->confidence = CONF_ON_TRACK;
    o->kr_count = 0;
    o->alignment_count = 0;
    return dash->obj_count++;
}

int okr_find_objective(OkrDashboard *dash, const char *id) {
    for (int i = 0; i < dash->obj_count; i++) {
        if (strcmp(dash->objectives[i].id, id) == 0) return i;
    }
    return -1;
}

int okr_add_key_result(OkrDashboard *dash, const char *id, const char *desc,
                       KRType type, double target, double start,
                       int obj_idx, int weight) {
    if (dash->kr_count >= OKR_MAX_KEY_RESULTS) return -1;
    if (obj_idx < 0 || obj_idx >= dash->obj_count) return -1;
    if (dash->objectives[obj_idx].kr_count >= 8) return -1;
    if (target <= 0) return -1;

    KeyResult *kr = &dash->key_results[dash->kr_count];
    memset(kr, 0, sizeof(*kr));
    strncpy(kr->id, id, OKR_ID_LEN - 1); kr->id[OKR_ID_LEN - 1] = '\0';
    strncpy(kr->description, desc, OKR_DESC_LEN - 1); kr->description[OKR_DESC_LEN - 1] = '\0';
    kr->type = type;
    kr->target = target;
    kr->start = start;
    kr->current = start;
    kr->objective_id = obj_idx;
    kr->weight_pct = weight > 0 ? weight : 100;
    kr->confidence = CONF_ON_TRACK;
    kr->created_at = (uint32_t)time(NULL);
    kr->last_checkin = kr->created_at;

    Objective *o = &dash->objectives[obj_idx];
    o->key_result_ids[o->kr_count++] = dash->kr_count;
    return dash->kr_count++;
}

int okr_find_kr(OkrDashboard *dash, const char *id) {
    for (int i = 0; i < dash->kr_count; i++) {
        if (strcmp(dash->key_results[i].id, id) == 0) return i;
    }
    return -1;
}

bool okr_update_kr(OkrDashboard *dash, const char *kr_id, double new_value) {
    int idx = okr_find_kr(dash, kr_id);
    if (idx < 0) return false;
    KeyResult *kr = &dash->key_results[idx];
    kr->current = new_value;
    kr->last_checkin = (uint32_t)time(NULL);
    /* Recalculate parent objective */
    okr_recalc_progress(dash, kr->objective_id);
    return true;
}

/* L5: Weighted progress calculation for objectives */
void okr_recalc_progress(OkrDashboard *dash, int obj_idx) {
    if (obj_idx < 0 || obj_idx >= dash->obj_count) return;
    Objective *o = &dash->objectives[obj_idx];
    if (o->kr_count == 0) { o->progress = 0; return; }
    double weighted_sum = 0;
    double total_weight = 0;
    for (int i = 0; i < o->kr_count; i++) {
        KeyResult *kr = &dash->key_results[o->key_result_ids[i]];
        double kr_progress = 0;
        if (kr->target != kr->start) {
            kr_progress = (kr->current - kr->start) / (kr->target - kr->start);
        }
        if (kr_progress < 0) kr_progress = 0;
        if (kr_progress > 1.0) kr_progress = 1.0;
        weighted_sum += kr_progress * kr->weight_pct;
        total_weight += kr->weight_pct;
    }
    o->progress = total_weight > 0 ? weighted_sum / total_weight : 0;
    /* Assess confidence based on time elapsed vs progress */
    uint32_t now = (uint32_t)time(NULL);
    double elapsed_pct = 0;
    if (o->end_date > o->start_date) {
        elapsed_pct = (double)(now - o->start_date) / (double)(o->end_date - o->start_date);
    }
    if (elapsed_pct < 0) elapsed_pct = 0;
    if (elapsed_pct > 1.0) elapsed_pct = 1.0;
    o->confidence = okr_assess_confidence(o->progress, elapsed_pct);
}

void okr_recalc_all(OkrDashboard *dash) {
    for (int i = 0; i < dash->obj_count; i++) {
        okr_recalc_progress(dash, i);
    }
}

/* L4: Confidence assessment formula
 * Google's OKR grading: 0.0-1.0 scale
 * 0.6-0.7 = "good" (stretch goals)
 * > 0.7 = achieved
 * Progress vs elapsed time comparison
 */
OkrConfidence okr_assess_confidence(double progress, double elapsed_pct) {
    if (progress >= 1.0) return CONF_COMPLETED;
    if (elapsed_pct <= 0) return CONF_ON_TRACK;
    double pace_ratio = progress / elapsed_pct;
    if (pace_ratio >= 0.7) return CONF_ON_TRACK;
    if (pace_ratio >= 0.4) return CONF_AT_RISK;
    return CONF_OFF_TRACK;
}

/* L3: OKR Alignment — connect child objective to parent */
bool okr_align(OkrDashboard *dash, const char *parent_id, const char *child_id) {
    int pi = okr_find_objective(dash, parent_id);
    int ci = okr_find_objective(dash, child_id);
    if (pi < 0 || ci < 0 || pi == ci) return false;
    Objective *p = &dash->objectives[pi];
    if (p->alignment_count >= OKR_MAX_ALIGNMENTS) return false;
    /* Prevent circular alignment (simple parent check) */
    for (int i = 0; i < p->alignment_count; i++) {
        if (p->alignments[i] == ci) return false;
    }
    p->alignments[p->alignment_count++] = ci;
    return true;
}

/* L5: Company-level OKR score — weighted average of all objectives */
double okr_company_score(OkrDashboard *dash) {
    double total = 0;
    int count = 0;
    for (int i = 0; i < dash->obj_count; i++) {
        if (dash->objectives[i].is_company_level) {
            total += dash->objectives[i].progress;
            count++;
        }
    }
    if (count == 0) {
        /* If no company-level, average all */
        for (int i = 0; i < dash->obj_count; i++) {
            total += dash->objectives[i].progress;
            count++;
        }
    }
    return count > 0 ? total / count : 0;
}

/* L8: OKR Alignment Tree — builds hierarchy from aligned objectives */
OkrTree *okr_build_tree(OkrDashboard *dash) {
    if (dash->obj_count == 0) return NULL;
    OkrTree *tree = (OkrTree *)malloc(sizeof(OkrTree));
    if (!tree) return NULL;
    memset(tree, 0, sizeof(*tree));

    /* Find root (company-level objective without parent in alignment) */
    int root_idx = -1;
    for (int i = 0; i < dash->obj_count; i++) {
        if (dash->objectives[i].is_company_level) {
            root_idx = i;
            break;
        }
    }
    if (root_idx < 0) root_idx = 0;

    /* Build nodes recursively using alignment data */
    OkrNode **node_map = (OkrNode **)calloc(dash->obj_count, sizeof(OkrNode *));
    if (!node_map) { free(tree); return NULL; }

    for (int i = 0; i < dash->obj_count; i++) {
        node_map[i] = (OkrNode *)malloc(sizeof(OkrNode));
        if (!node_map[i]) {
            for (int j = 0; j < i; j++) free(node_map[j]);
            free(node_map); free(tree); return NULL;
        }
        memset(node_map[i], 0, sizeof(OkrNode));
        node_map[i]->obj = &dash->objectives[i];
    }

    /* Wire up parent-child relationships */
    for (int i = 0; i < dash->obj_count; i++) {
        Objective *o = &dash->objectives[i];
        for (int j = 0; j < o->alignment_count; j++) {
            int child_idx = o->alignments[j];
            if (child_idx >= 0 && child_idx < dash->obj_count && child_idx != i) {
                OkrNode *parent = node_map[i];
                OkrNode *child = node_map[child_idx];
                if (parent->child_count < 8) {
                    parent->children[parent->child_count++] = child;
                    child->parent = parent;
                }
            }
        }
    }

    tree->root = node_map[root_idx];
    tree->total_nodes = dash->obj_count;

    /* Compute max depth */
    int max_depth = 0;
    for (int i = 0; i < dash->obj_count; i++) {
        int depth = 0;
        OkrNode *n = node_map[i];
        while (n->parent) { depth++; n = n->parent; }
        if (depth > max_depth) max_depth = depth;
    }
    tree->max_depth = max_depth;

    free(node_map);
    return tree;
}

/* Recursive scoring of tree nodes */
static void tree_score_recursive(OkrNode *node, double *total, int *count) {
    if (!node) return;
    (*total) += node->obj->progress;
    (*count)++;
    for (int i = 0; i < node->child_count; i++) {
        tree_score_recursive(node->children[i], total, count);
    }
}

void okr_tree_score(OkrTree *tree) {
    if (!tree || !tree->root) return;
    double total = 0;
    int count = 0;
    tree_score_recursive(tree->root, &total, &count);
}

/* Static helper: recursively free OKR tree nodes */
static void okr_free_nodes(OkrNode *n) {
    if (!n) return;
    for (int i = 0; i < n->child_count; i++) okr_free_nodes(n->children[i]);
    free(n);
}

void okr_tree_free(OkrTree *tree) {
    if (!tree) return;
    okr_free_nodes(tree->root);
    free(tree);
}

/* Static helper: recursively print OKR tree nodes */
static void okr_print_node(OkrNode *n, int d, int max_depth) {
    if (!n || d > max_depth + 2) return;
    for (int i = 0; i < d; i++) printf("  ");
    printf("%s [%.0f%%] %s\n", n->obj->id, n->obj->progress * 100,
           n->obj->confidence == CONF_ON_TRACK ? "ON_TRACK" :
           n->obj->confidence == CONF_AT_RISK ? "AT_RISK" :
           n->obj->confidence == CONF_COMPLETED ? "DONE" : "OFF_TRACK");
    for (int i = 0; i < n->child_count; i++) okr_print_node(n->children[i], d + 1, max_depth);
}

void okr_print_tree(OkrTree *tree, int depth) {
    if (!tree || !tree->root) { printf("Empty OKR tree.\n"); return; }
    okr_print_node(tree->root, 0, depth);
}

void okr_print_dashboard(OkrDashboard *dash) {
    printf("=== OKR Dashboard Q%d %d ===\n", dash->quarter_number, dash->year);
    printf("  Objectives: %d | Key Results: %d\n", dash->obj_count, dash->kr_count);
    printf("  Company Score: %.1f%%\n", okr_company_score(dash) * 100);
    for (int i = 0; i < dash->obj_count; i++) {
        Objective *o = &dash->objectives[i];
        printf("\n  O%d: %s [%.0f%%] %s\n", i, o->title, o->progress * 100,
               o->is_company_level ? "(Company)" : "");
        for (int j = 0; j < o->kr_count; j++) {
            KeyResult *kr = &dash->key_results[o->key_result_ids[j]];
            double kr_pct = 0;
            if (kr->target != kr->start)
                kr_pct = (kr->current - kr->start) / (kr->target - kr->start) * 100;
            printf("    KR: %s [%.0f%%] %.1f/%.1f (%d%% weight)\n",
                   kr->description, kr_pct, kr->current, kr->target, kr->weight_pct);
        }
    }
}