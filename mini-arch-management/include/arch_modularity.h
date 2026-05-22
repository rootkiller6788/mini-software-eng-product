#ifndef ARCH_MODULARITY_H
#define ARCH_MODULARITY_H
#include <stdbool.h>

#define MOD_MAX_CLUSTERS 16
#define MOD_MAX_ELEMENTS 64

typedef struct {
    int elements[MOD_MAX_ELEMENTS]; int elem_count;
    double cohesion;          /* intra-cluster edge density */
    double coupling;          /* inter-cluster edge density */
    double modularity_score;  /* Q = cohesion - coupling */
} Cluster;

typedef struct {
    Cluster clusters[MOD_MAX_CLUSTERS]; int cluster_count;
    double overall_modularity;
    int adjacency[MOD_MAX_ELEMENTS][MOD_MAX_ELEMENTS];
    int elem_count;
} ModularityAnalysis;

void mod_init(ModularityAnalysis *ma, int elem_count);
void mod_set_edge(ModularityAnalysis *ma, int from, int to, bool connected);
int  mod_cluster_louvain(ModularityAnalysis *ma);  /* Louvain-inspired clustering */
int  mod_cluster_girvan_newman(ModularityAnalysis *ma); /* edge-betweenness */
double mod_cohesion(ModularityAnalysis *ma, int cluster_id);
double mod_coupling(ModularityAnalysis *ma, int c1, int c2);
double mod_modularity_score(ModularityAnalysis *ma);
int  mod_bounded_contexts(ModularityAnalysis *ma, int *context_ids, int max_contexts);
void mod_print(ModularityAnalysis *ma);
#endif
