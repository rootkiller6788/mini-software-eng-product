#include "product_lifecycle.h"
#include <stdio.h>

int main(void) {
    ProductLifecycle app;
    product_lifecycle_init(&app, "SuperApp", "Simplify workflow automation for SMBs");

    printf("=== Product Lifecycle Demo ===\n\n");
    printf("Product: %s\n", app.product_name);
    printf("Vision:  %s\n\n", app.vision);

    printf("--- Defining MVP Features ---\n");
    product_lifecycle_add_mvp_feature(&app, "User registration & login", MVP_MUST_HAVE);
    product_lifecycle_add_mvp_feature(&app, "Dashboard with key metrics", MVP_MUST_HAVE);
    product_lifecycle_add_mvp_feature(&app, "Task creation & management", MVP_MUST_HAVE);
    product_lifecycle_add_mvp_feature(&app, "Email notifications", MVP_SHOULD_HAVE);
    product_lifecycle_add_mvp_feature(&app, "Report export (PDF)", MVP_COULD_HAVE);
    product_lifecycle_add_mvp_feature(&app, "Dark mode theme", MVP_WONT_HAVE);

    printf("MVP features defined: %zu\n\n", app.mvp_feature_count);

    const char *priority_labels[] = {"Must", "Should", "Could", "Won't"};
    for (size_t i = 0; i < app.mvp_feature_count; i++) {
        printf("  [%s] %s\n", priority_labels[app.mvp_features[i].priority],
               app.mvp_features[i].feature);
    }

    printf("\n--- Phase Gates Evaluation ---\n");
    for (ProductPhase p = PHASE_DISCOVERY; p < PHASE_COUNT; p++) {
        printf("\n[Phase %d/%d] %s\n", p + 1, PHASE_COUNT, product_lifecycle_phase_name(p));
        printf("  Entry: %s\n", app.gates[p].entry_criteria);
        printf("  Exit:  %s\n", app.gates[p].exit_criteria);

        bool go = (p < PHASE_TESTING);
        product_lifecycle_evaluate_gate(&app, p, go);
        printf("  Decision: %s\n", go ? "GO" : "NO-GO (needs review)");

        if (go) {
            if (product_lifecycle_advance_phase(&app)) {
                printf("  -> Advanced to %s (iteration %d)\n",
                       product_lifecycle_phase_name(app.current_phase), app.iteration);
            }
        }
    }

    printf("\n--- Summary ---\n");
    product_lifecycle_print_summary(&app);

    printf("\nCan regress: %s\n", product_lifecycle_can_regress(&app) ? "Yes" : "No");

    product_lifecycle_reset(&app);
    printf("After reset: phase=%s iteration=%d\n",
           product_lifecycle_phase_name(app.current_phase), app.iteration);

    return 0;
}
