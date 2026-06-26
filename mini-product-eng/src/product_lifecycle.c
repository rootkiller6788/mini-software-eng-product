#include "product_lifecycle.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *g_phase_names[] = {
    "Discovery", "Definition", "Design", "Development",
    "Testing", "Launch", "Growth", "Maturity", "Decline"
};

static const char *g_entry_criteria[] = {
    "Problem identified, initial user research complete",
    "Validated problem, target market defined, business case approved",
    "Solution approach approved, UX research complete",
    "Design reviewed and approved, technical architecture defined",
    "Code complete, unit tests passing, integration environment ready",
    "QA sign-off, performance benchmarks met, security audit passed",
    "Post-launch monitoring stable, KPIs trending positive",
    "Market saturation, feature parity with competitors",
    "New alternatives emerging, user engagement declining"
};

static const char *g_exit_criteria[] = {
    "Problem validation, market size estimation, stakeholder alignment",
    "PRD approved, success metrics defined, resource allocation secured",
    "High-fidelity mockups approved, technical spec finalized, story points estimated",
    "All MVP features implemented, code review complete, CI/CD green",
    "Bug burndown acceptable, UAT passed, deployment runbook ready",
    "Monitoring dashboards active, support team trained, rollback plan tested",
    "Market share growing, NPS > 40, churn rate < 5%",
    "Revenue stable, cost optimization opportunities, extension ecosystem",
    "Migration path defined, customer communication sent, sunset timeline"
};

void product_lifecycle_init(ProductLifecycle *pl, const char *name, const char *vision) {
    if (!pl) return;
    memset(pl, 0, sizeof(*pl));
    pl->current_phase = PHASE_DISCOVERY;
    pl->iteration = 1;
    pl->lifecycle_complete = false;
    if (name) {
        strncpy(pl->product_name, name, sizeof(pl->product_name) - 1);
        pl->product_name[sizeof(pl->product_name) - 1] = '\0';
    }
    if (vision) {
        strncpy(pl->vision, vision, sizeof(pl->vision) - 1);
        pl->vision[sizeof(pl->vision) - 1] = '\0';
    }
    for (int i = 0; i < PHASE_COUNT; i++) {
        pl->gates[i].name = g_phase_names[i];
        pl->gates[i].entry_criteria = g_entry_criteria[i];
        pl->gates[i].exit_criteria = g_exit_criteria[i];
    }
}

ProductPhase product_lifecycle_current_phase(const ProductLifecycle *pl) {
    if (!pl) return PHASE_DISCOVERY;
    return pl->current_phase;
}

bool product_lifecycle_evaluate_gate(ProductLifecycle *pl, ProductPhase phase, bool go) {
    if (!pl || phase < 0 || phase >= PHASE_COUNT) return false;
    PhaseGate *gate = &pl->gates[phase];
    gate->passed = go;
    if (go) {
        snprintf(gate->decision, sizeof(gate->decision), "GO");
        snprintf(gate->notes, sizeof(gate->notes),
                 "Phase gate passed for: %s", g_phase_names[phase]);
    } else {
        snprintf(gate->decision, sizeof(gate->decision), "NO-GO");
        snprintf(gate->notes, sizeof(gate->notes),
                 "Phase gate blocked for: %s — rework required", g_phase_names[phase]);
    }
    return go;
}

const char *product_lifecycle_phase_name(ProductPhase phase) {
    if (phase < 0 || phase >= PHASE_COUNT) return "Unknown";
    return g_phase_names[phase];
}

bool product_lifecycle_advance_phase(ProductLifecycle *pl) {
    if (!pl || pl->lifecycle_complete) return false;
    if (pl->current_phase >= PHASE_DECLINE) {
        pl->lifecycle_complete = true;
        return false;
    }
    PhaseGate *gate = &pl->gates[pl->current_phase];
    if (!gate->passed) {
        fprintf(stderr, "Phase gate not passed for: %s\n", g_phase_names[pl->current_phase]);
        return false;
    }
    snprintf(gate->description ? gate->description : gate->notes,
             sizeof(gate->notes), "Advanced on iteration %d", pl->iteration);
    pl->current_phase = (ProductPhase)(pl->current_phase + 1);
    pl->iteration++;
    return true;
}

bool product_lifecycle_can_regress(const ProductLifecycle *pl) {
    if (!pl || pl->lifecycle_complete) return false;
    return pl->current_phase > PHASE_DISCOVERY;
}

void product_lifecycle_add_mvp_feature(ProductLifecycle *pl, const char *feature, MvpPriority priority) {
    if (!pl || !feature) return;
    pl->mvp_feature_count++;
    MvpFeature *tmp = realloc(pl->mvp_features, pl->mvp_feature_count * sizeof(MvpFeature));
    if (!tmp) return;
    pl->mvp_features = tmp;
    MvpFeature *f = &pl->mvp_features[pl->mvp_feature_count - 1];
    memset(f, 0, sizeof(*f));
    f->feature = _strdup(feature);
    f->priority = priority;
}

bool product_lifecycle_is_mvp_ready(const ProductLifecycle *pl) {
    if (!pl || pl->mvp_feature_count == 0) return false;
    int must_count = 0;
    for (size_t i = 0; i < pl->mvp_feature_count; i++) {
        if (pl->mvp_features[i].priority == MVP_MUST_HAVE) must_count++;
    }
    return must_count > 0 && pl->current_phase >= PHASE_TESTING;
}

void product_lifecycle_print_summary(const ProductLifecycle *pl) {
    if (!pl) return;
    printf("Product: %s\n", pl->product_name);
    printf("Vision:  %s\n", pl->vision);
    printf("Phase:   %s (iteration %d)\n", g_phase_names[pl->current_phase], pl->iteration);
    printf("MVP Features: %zu\n", pl->mvp_feature_count);
    printf("MVP Ready: %s\n", product_lifecycle_is_mvp_ready(pl) ? "Yes" : "No");
    printf("Lifecycle complete: %s\n", pl->lifecycle_complete ? "Yes" : "No");
    printf("Phase Gates:\n");
    for (int i = 0; i < PHASE_COUNT; i++) {
        const char *marker = (i == pl->current_phase) ? " [CURRENT]" : "";
        printf("  %s%s: %s\n", g_phase_names[i], marker, pl->gates[i].decision);
    }
}

void product_lifecycle_set_phase_description(ProductLifecycle *pl, ProductPhase phase, const char *desc) {
    if (!pl || !desc || phase < 0 || phase >= PHASE_COUNT) return;
    pl->gates[phase].description = _strdup(desc);
}

void product_lifecycle_reset(ProductLifecycle *pl) {
    if (!pl) return;
    for (size_t i = 0; i < pl->mvp_feature_count; i++) {
        free((void *)pl->mvp_features[i].feature);
    }
    for (int i = 0; i < PHASE_COUNT; i++) {
        free((void *)pl->gates[i].description);
    }
    free(pl->mvp_features);
    product_lifecycle_init(pl, pl->product_name, pl->vision);
}
