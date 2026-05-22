#include "arch_graph.h"
#include "arch_metrics.h"
#include "arch_adr.h"
#include "arch_pattern.h"
#include "arch_modularity.h"
#include <stdio.h>

int main(void) {
    printf("=== Architecture Management Demo ===\n\n");
    /* Build module graph */
    ArchitectureGraph g; arch_graph_init(&g);
    int ui = arch_graph_add_module(&g, "UI", 2000, false);
    int svc = arch_graph_add_module(&g, "Service", 3500, true);
    int db = arch_graph_add_module(&g, "Database", 1500, false);
    int auth = arch_graph_add_module(&g, "Auth", 800, true);
    int util = arch_graph_add_module(&g, "Utils", 600, false);
    arch_graph_add_dependency(&g, ui, svc, DEP_CALLS);
    arch_graph_add_dependency(&g, svc, db, DEP_CALLS);
    arch_graph_add_dependency(&g, ui, auth, DEP_USES);
    arch_graph_add_dependency(&g, svc, auth, DEP_USES);
    arch_graph_add_dependency(&g, auth, util, DEP_USES);
    arch_graph_print(&g);
    /* Metrics */
    printf("\n=== Coupling Metrics ===\n");
    for (int i = 0; i < g.module_count; i++) {
        ModuleMetrics mm;
        metrics_compute(&mm, arch_graph_fan_in(&g,i), arch_graph_fan_out(&g,i), g.modules[i].is_abstract?1:0, 10, 5);
        metrics_print_module(&mm, g.modules[i].name);
    }
    /* ADR */
    AdrLog adr; adr_init(&adr);
    adr_add(&adr, "Use layered architecture", "Need separation of concerns", "4-layer architecture: UI/Service/Domain/Infra", "Better testability, looser coupling");
    adr_accept(&adr, 0);
    adr_print_all(&adr);
    /* Pattern detection */
    PatternContext pctx; pattern_init_context(&pctx, g.module_count);
    for (int i = 0; i < g.module_count; i++) {
        pctx.fan_in[i] = arch_graph_fan_in(&g,i);
        pctx.fan_out[i] = arch_graph_fan_out(&g,i);
    }
    DetectedPattern dp;
    int layers[64] = {0}; arch_graph_layer_assign(&g, layers);
    for (int i = 0; i < g.module_count; i++) pctx.layers[i] = layers[i];
    pattern_detect_layered(&pctx, &dp);
    printf("\n=== Pattern Detection ===\n");
    pattern_print(&dp);
    return 0;
}
