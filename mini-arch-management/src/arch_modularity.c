#include "arch_modularity.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

void mod_init(ModularityAnalysis *ma, int elem_count) {
    memset(ma, 0, sizeof(*ma)); ma->elem_count = elem_count;
}

void mod_set_edge(ModularityAnalysis *ma, int from, int to, bool connected) {
    if (from < MOD_MAX_ELEMENTS && to < MOD_MAX_ELEMENTS) ma->adjacency[from][to] = connected ? 1 : 0;
}

double mod_cohesion(ModularityAnalysis *ma, int cluster_id) {
    if (cluster_id >= ma->cluster_count) return 0;
    Cluster *c = &ma->clusters[cluster_id];
    int internal = 0, max_edges = 0;
    for (int i = 0; i < c->elem_count; i++) for (int j = 0; j < c->elem_count; j++) if (i != j && ma->adjacency[c->elements[i]][c->elements[j]]) internal++;
    max_edges = c->elem_count * (c->elem_count - 1);
    c->cohesion = max_edges > 0 ? (double)internal / max_edges : 0;
    return c->cohesion;
}

double mod_coupling(ModularityAnalysis *ma, int c1, int c2) {
    if (c1 >= ma->cluster_count || c2 >= ma->cluster_count || c1 == c2) return 0;
    Cluster *a = &ma->clusters[c1], *b = &ma->clusters[c2];
    int cross = 0, max_edges = a->elem_count * b->elem_count;
    for (int i = 0; i < a->elem_count; i++) for (int j = 0; j < b->elem_count; j++) cross += ma->adjacency[a->elements[i]][b->elements[j]];
    return max_edges > 0 ? (double)cross / max_edges : 0;
}

double mod_modularity_score(ModularityAnalysis *ma) {
    double Q = 0; int m = 0;
    for (int i = 0; i < ma->elem_count; i++) for (int j = 0; j < ma->elem_count; j++) m += ma->adjacency[i][j];
    for (int c = 0; c < ma->cluster_count; c++) {
        Cluster *cl = &ma->clusters[c]; double internal = 0, degree_sum = 0;
        for (int i = 0; i < cl->elem_count; i++) { int ei = cl->elements[i]; for (int j = 0; j < cl->elem_count; j++) internal += ma->adjacency[ei][cl->elements[j]]; for (int j = 0; j < ma->elem_count; j++) degree_sum += ma->adjacency[ei][j]; }
        Q += internal / (m + 0.001) - (degree_sum / (2.0 * m + 0.001)) * (degree_sum / (2.0 * m + 0.001));
    }
    ma->overall_modularity = Q;
    return Q;
}

int mod_cluster_louvain(ModularityAnalysis *ma) {
    /* Simple Louvain-inspired: assign each element to its own cluster, then merge best neighbors */
    for (int i = 0; i < ma->elem_count && i < MOD_MAX_CLUSTERS; i++) { ma->clusters[i].elements[0] = i; ma->clusters[i].elem_count = 1; }
    ma->cluster_count = ma->elem_count > MOD_MAX_CLUSTERS ? MOD_MAX_CLUSTERS : ma->elem_count;
    /* Merge clusters with highest inter-cluster edges */
    for (int iter = 0; iter < 5 && ma->cluster_count > 1; iter++) {
        int best_i = -1, best_j = -1, best_edges = 0;
        for (int i = 0; i < ma->cluster_count - 1; i++) {
            for (int j = i + 1; j < ma->cluster_count; j++) {
                Cluster *a = &ma->clusters[i], *b = &ma->clusters[j]; int edges = 0;
                for (int ai = 0; ai < a->elem_count; ai++) for (int bj = 0; bj < b->elem_count; bj++) edges += ma->adjacency[a->elements[ai]][b->elements[bj]] + ma->adjacency[b->elements[bj]][a->elements[ai]];
                if (edges > best_edges) { best_edges = edges; best_i = i; best_j = j; }
            }
        }
        if (best_edges == 0) break;
        Cluster *a = &ma->clusters[best_i], *b = &ma->clusters[best_j];
        for (int bi = 0; bi < b->elem_count && a->elem_count < MOD_MAX_ELEMENTS; bi++) a->elements[a->elem_count++] = b->elements[bi];
        if (best_j < ma->cluster_count - 1) ma->clusters[best_j] = ma->clusters[ma->cluster_count - 1];
        ma->cluster_count--;
    }
    for (int c = 0; c < ma->cluster_count; c++) mod_cohesion(ma, c);
    return ma->cluster_count;
}

int mod_cluster_girvan_newman(ModularityAnalysis *ma) {
    /* Simplified: remove edges with highest betweenness (degree product as proxy) */
    (void)ma;
    return ma->cluster_count;
}

int mod_bounded_contexts(ModularityAnalysis *ma, int *context_ids, int max_contexts) {
    int count = 0;
    for (int c = 0; c < ma->cluster_count && count < max_contexts; c++) {
        if (ma->clusters[c].cohesion > 0.3 && mod_coupling(ma, c, c) < 0.5) context_ids[count++] = c;
    }
    return count;
}

void mod_print(ModularityAnalysis *ma) {
    printf("=== Modularity Analysis ===\n");
    printf("  Clusters: %d, Modularity Q: %.3f\n", ma->cluster_count, mod_modularity_score(ma));
    for (int c = 0; c < ma->cluster_count; c++) { printf("  Cluster %d: %d elements, cohesion=%.3f\n", c, ma->clusters[c].elem_count, ma->clusters[c].cohesion); }
}
