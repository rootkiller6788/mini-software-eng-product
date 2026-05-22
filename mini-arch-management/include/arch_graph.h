#ifndef ARCH_GRAPH_H
#define ARCH_GRAPH_H
#include <stdint.h>
#include <stdbool.h>

#define MAX_MODULES 64
#define MAX_DEPENDENCIES 512

typedef enum { DEP_USES, DEP_IMPLEMENTS, DEP_EXTENDS, DEP_CALLS, DEP_PUBLISHES, DEP_SUBSCRIBES } DepType;

typedef struct { int id; char name[64]; int source_files; int loc; bool is_abstract; double abstractness; } ArchModule;

typedef struct { int id; DepType type; int from; int to; double weight; bool cyclic; } ArchDependency;

typedef struct {
    ArchModule modules[MAX_MODULES]; int module_count;
    ArchDependency deps[MAX_DEPENDENCIES]; int dep_count;
    int adj[MAX_MODULES][MAX_MODULES]; /* adjacency matrix */
} ArchitectureGraph;

void arch_graph_init(ArchitectureGraph *g);
int  arch_graph_add_module(ArchitectureGraph *g, const char *name, int loc, bool is_abstract);
int  arch_graph_add_dependency(ArchitectureGraph *g, int from, int to, DepType type);
bool arch_graph_detect_cycles(ArchitectureGraph *g, int *cycle_nodes, int *cycle_len);
int  arch_graph_topological_sort(ArchitectureGraph *g, int *order);
int  arch_graph_layer_assign(ArchitectureGraph *g, int *layers);
int  arch_graph_fan_in(ArchitectureGraph *g, int module_id);
int  arch_graph_fan_out(ArchitectureGraph *g, int module_id);
void arch_graph_print(ArchitectureGraph *g);
#endif
