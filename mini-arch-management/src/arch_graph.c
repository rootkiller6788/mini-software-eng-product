#include "arch_graph.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void arch_graph_init(ArchitectureGraph *g) {
    memset(g, 0, sizeof(*g));
}

int arch_graph_add_module(ArchitectureGraph *g, const char *name, int loc, bool is_abstract) {
    if (g->module_count >= MAX_MODULES) return -1;
    ArchModule *m = &g->modules[g->module_count];
    m->id = g->module_count; m->loc = loc; m->is_abstract = is_abstract;
    m->abstractness = is_abstract ? 1.0 : 0.0;
    strncpy(m->name, name, 63); m->name[63] = '\0';
    return g->module_count++;
}

int arch_graph_add_dependency(ArchitectureGraph *g, int from, int to, DepType type) {
    if (g->dep_count >= MAX_DEPENDENCIES || from < 0 || to < 0 || from >= g->module_count || to >= g->module_count) return -1;
    ArchDependency *d = &g->deps[g->dep_count];
    d->id = g->dep_count; d->type = type; d->from = from; d->to = to; d->weight = 1.0; d->cyclic = false;
    if (from < MAX_MODULES && to < MAX_MODULES) g->adj[from][to] = 1;
    return g->dep_count++;
}

bool arch_graph_detect_cycles(ArchitectureGraph *g, int *cycle_nodes, int *cycle_len) {
    int visited[MAX_MODULES] = {0}, rec_stack[MAX_MODULES] = {0}, parent[MAX_MODULES];
    for (int i = 0; i < MAX_MODULES; i++) parent[i] = -1;
    int stack[MAX_MODULES * 4], top = 0;
    for (int start = 0; start < g->module_count; start++) {
        if (visited[start]) continue;
        stack[top++] = start; visited[start] = 1; rec_stack[start] = 1;
        while (top > 0) {
            int u = stack[top-1]; bool all_done = true;
            for (int v = 0; v < g->module_count; v++) {
                if (g->adj[u][v]) {
                    if (!visited[v]) { stack[top++] = v; visited[v] = 1; rec_stack[v] = 1; parent[v] = u; all_done = false; break; }
                    else if (rec_stack[v] && cycle_nodes && cycle_len) {
                        int idx = 0, cur = u; cycle_nodes[idx++] = v;
                        while (cur != v && idx < MAX_MODULES) { cycle_nodes[idx++] = cur; cur = parent[cur]; }
                        *cycle_len = idx; return true;
                    }
                }
            }
            if (all_done) { rec_stack[u] = 0; top--; }
        }
    }
    return false;
}

int arch_graph_topological_sort(ArchitectureGraph *g, int *order) {
    int in_deg[MAX_MODULES] = {0};
    for (int i = 0; i < g->module_count; i++) for (int j = 0; j < g->module_count; j++) if (g->adj[j][i]) in_deg[i]++;
    int q[MAX_MODULES], front = 0, back = 0, idx = 0;
    for (int i = 0; i < g->module_count; i++) if (in_deg[i] == 0) q[back++] = i;
    while (front < back) { int u = q[front++]; order[idx++] = u;
        for (int v = 0; v < g->module_count; v++) if (g->adj[u][v] && --in_deg[v] == 0) q[back++] = v; }
    return idx;
}

int arch_graph_layer_assign(ArchitectureGraph *g, int *layers) {
    int order[MAX_MODULES]; int n = arch_graph_topological_sort(g, order);
    for (int i = 0; i < n; i++) {
        int u = order[i], max_l = 0;
        for (int j = 0; j < g->module_count; j++) if (g->adj[j][u] && layers[j] + 1 > max_l) max_l = layers[j] + 1;
        layers[u] = max_l;
    }
    return n;
}

int arch_graph_fan_in(ArchitectureGraph *g, int id) { int n = 0; for (int i = 0; i < g->module_count; i++) if (g->adj[i][id]) n++; return n; }
int arch_graph_fan_out(ArchitectureGraph *g, int id) { int n = 0; for (int i = 0; i < g->module_count; i++) if (g->adj[id][i]) n++; return n; }

void arch_graph_print(ArchitectureGraph *g) {
    printf("=== Architecture Graph ===\n");
    printf("  Modules: %d, Dependencies: %d\n", g->module_count, g->dep_count);
    for (int i = 0; i < g->module_count; i++) { ArchModule *m = &g->modules[i]; printf("  %s [%d LOC, Ca=%d Ce=%d]\n", m->name, m->loc, arch_graph_fan_in(g,i), arch_graph_fan_out(g,i)); }
}
