#ifndef PRODUCT_EXPERIMENT_H
#define PRODUCT_EXPERIMENT_H
#include <stdbool.h>

#define MAX_VARIANTS 4
#define MAX_EXPERIMENT_USERS 1024

typedef enum { EXSTAT_DRAFT, EXSTAT_RUNNING, EXSTAT_STOPPED, EXSTAT_CONCLUDED } ExperimentStatus;

typedef struct {
    char name[32]; int user_count; int conversions;
    double conversion_rate; /* conversions / user_count */
    double confidence_interval;
} Variant;

typedef struct {
    char id[16]; char hypothesis[256];
    Variant variants[MAX_VARIANTS]; int variant_count;
    int control_idx; /* index of control variant */
    ExperimentStatus status;
    double significance_level; /* e.g. 0.05 */
    double minimum_detectable_effect;
} Experiment;

void experiment_init(Experiment *exp, const char *id, const char *hypothesis);
int  experiment_add_variant(Experiment *exp, const char *name);
void experiment_set_control(Experiment *exp, int variant_idx);
void experiment_assign_users(Experiment *exp, int variant_idx, int users, int conversions);
bool experiment_is_significant(Experiment *exp); /* z-test for proportions */
int  experiment_winning_variant(Experiment *exp); /* -1 if none significant */
double experiment_lift(Experiment *exp, int variant_idx); /* vs control */
void experiment_stop(Experiment *exp);
void experiment_print(Experiment *exp);
#endif
