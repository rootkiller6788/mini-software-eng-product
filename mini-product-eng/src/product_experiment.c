#include "product_experiment.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

void experiment_init(Experiment *exp, const char *id, const char *hypothesis) {
    memset(exp, 0, sizeof(*exp));
    strncpy(exp->id, id, 15); exp->id[15] = '\0';
    strncpy(exp->hypothesis, hypothesis, 255); exp->hypothesis[255] = '\0';
    exp->status = EXSTAT_DRAFT; exp->significance_level = 0.05;
    exp->minimum_detectable_effect = 0.02; exp->control_idx = 0;
}

int experiment_add_variant(Experiment *exp, const char *name) {
    if (exp->variant_count >= MAX_VARIANTS) return -1;
    Variant *v = &exp->variants[exp->variant_count];
    strncpy(v->name, name, 31); v->name[31] = '\0';
    v->user_count = 0; v->conversions = 0; v->conversion_rate = 0;
    return exp->variant_count++;
}

void experiment_set_control(Experiment *exp, int variant_idx) {
    if (variant_idx >= 0 && variant_idx < exp->variant_count)
        exp->control_idx = variant_idx;
}

void experiment_assign_users(Experiment *exp, int variant_idx, int users, int conversions) {
    if (variant_idx < 0 || variant_idx >= exp->variant_count) return;
    Variant *v = &exp->variants[variant_idx];
    v->user_count += users; v->conversions += conversions;
    v->conversion_rate = v->user_count > 0 ? (double)v->conversions / v->user_count : 0;
}

bool experiment_is_significant(Experiment *exp) {
    Variant *ctrl = &exp->variants[exp->control_idx];
    double p_ctrl = ctrl->conversion_rate;
    int n_ctrl = ctrl->user_count;
    if (n_ctrl < 30) return false;

    for (int i = 0; i < exp->variant_count; i++) {
        if (i == exp->control_idx) continue;
        Variant *v = &exp->variants[i];
        if (v->user_count < 30) continue;
        double p_var = v->conversion_rate;
        double p_pool = (double)(ctrl->conversions + v->conversions) / (n_ctrl + v->user_count);
        double se = sqrt(p_pool * (1 - p_pool) * (1.0/n_ctrl + 1.0/v->user_count));
        if (fabs(se) < 1e-9) continue;
        double z = fabs(p_var - p_ctrl) / se;
        if (z > 1.96) return true; /* two-tailed 95% confidence */
    }
    return false;
}

int experiment_winning_variant(Experiment *exp) {
    Variant *ctrl = &exp->variants[exp->control_idx];
    int best = exp->control_idx;
    double best_rate = ctrl->conversion_rate;
    for (int i = 0; i < exp->variant_count; i++) {
        if (i == exp->control_idx) continue;
        double lift = exp->variants[i].conversion_rate - ctrl->conversion_rate;
        if (lift > 0 && exp->variants[i].conversion_rate > best_rate) {
            best = i; best_rate = exp->variants[i].conversion_rate;
        }
    }
    if (best == exp->control_idx) return -1;
    if (!experiment_is_significant(exp)) return -1;
    return best;
}

double experiment_lift(Experiment *exp, int variant_idx) {
    if (variant_idx < 0 || variant_idx >= exp->variant_count) return 0;
    Variant *ctrl = &exp->variants[exp->control_idx];
    if (ctrl->conversion_rate < 1e-9) return 0;
    return (exp->variants[variant_idx].conversion_rate - ctrl->conversion_rate) / ctrl->conversion_rate;
}

void experiment_stop(Experiment *exp) { exp->status = EXSTAT_CONCLUDED; }

void experiment_print(Experiment *exp) {
    printf("=== Experiment %s ===\n  Hypothesis: %s\n", exp->id, exp->hypothesis);
    for (int i = 0; i < exp->variant_count; i++) {
        Variant *v = &exp->variants[i];
        printf("  %s%s: n=%d conv=%d rate=%.3f%%",
               v->name, i == exp->control_idx ? " (CTRL)" : "",
               v->user_count, v->conversions, v->conversion_rate * 100);
        if (i != exp->control_idx) printf(" lift=%.1f%%", experiment_lift(exp, i) * 100);
        printf("\n");
    }
    printf("  Significant: %s  Winner: %d\n",
           experiment_is_significant(exp) ? "YES" : "NO",
           experiment_winning_variant(exp));
}
