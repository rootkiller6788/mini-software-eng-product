#ifndef PRODUCT_LIFECYCLE_H
#define PRODUCT_LIFECYCLE_H

#include <stdbool.h>
#include <stddef.h>

typedef enum {
    PHASE_DISCOVERY,
    PHASE_DEFINITION,
    PHASE_DESIGN,
    PHASE_DEVELOPMENT,
    PHASE_TESTING,
    PHASE_LAUNCH,
    PHASE_GROWTH,
    PHASE_MATURITY,
    PHASE_DECLINE,
    PHASE_COUNT
} ProductPhase;

typedef struct {
    const char *name;
    const char *description;
    const char *entry_criteria;
    const char *exit_criteria;
    bool passed;
    char decision[256];
    char notes[512];
} PhaseGate;

typedef enum {
    MVP_MUST_HAVE,
    MVP_SHOULD_HAVE,
    MVP_COULD_HAVE,
    MVP_WONT_HAVE
} MvpPriority;

typedef struct {
    const char *feature;
    MvpPriority priority;
} MvpFeature;

typedef struct {
    ProductPhase current_phase;
    PhaseGate gates[PHASE_COUNT];
    MvpFeature *mvp_features;
    size_t mvp_feature_count;
    char product_name[128];
    char vision[512];
    int iteration;
    bool lifecycle_complete;
} ProductLifecycle;

void        product_lifecycle_init(ProductLifecycle *pl, const char *name, const char *vision);
ProductPhase product_lifecycle_current_phase(const ProductLifecycle *pl);
bool        product_lifecycle_evaluate_gate(ProductLifecycle *pl, ProductPhase phase, bool go);
const char *product_lifecycle_phase_name(ProductPhase phase);
bool        product_lifecycle_advance_phase(ProductLifecycle *pl);
void        product_lifecycle_add_mvp_feature(ProductLifecycle *pl, const char *feature, MvpPriority priority);
bool        product_lifecycle_is_mvp_ready(const ProductLifecycle *pl);
void        product_lifecycle_print_summary(const ProductLifecycle *pl);
bool        product_lifecycle_can_regress(const ProductLifecycle *pl);
void        product_lifecycle_set_phase_description(ProductLifecycle *pl, ProductPhase phase, const char *desc);
void        product_lifecycle_reset(ProductLifecycle *pl);

#endif
