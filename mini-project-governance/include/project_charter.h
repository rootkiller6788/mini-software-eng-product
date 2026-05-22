#ifndef PROJECT_CHARTER_H
#define PROJECT_CHARTER_H
#include <stdbool.h>

#define MAX_OBJECTIVES 16
#define MAX_CONSTRAINTS 8
#define MAX_SCOPE_ITEMS 32

typedef enum { CS_PROPOSED, CS_APPROVED, CS_ACTIVE, CS_CLOSED, CS_SUSPENDED } CharterStatus;

typedef struct { char description[256]; bool measurable; double target; double current; } Objective;

typedef struct { char description[128]; char category[32]; /* budget, time, quality, scope, legal */ } Constraint;

typedef struct { char name[128]; bool in_scope; char exclusion_note[256]; } ScopeItem;

typedef struct {
    char project_name[64]; char sponsor[64]; char manager[64];
    CharterStatus status;
    Objective objectives[MAX_OBJECTIVES]; int objective_count;
    Constraint constraints[MAX_CONSTRAINTS]; int constraint_count;
    ScopeItem scope[MAX_SCOPE_ITEMS]; int scope_count;
    int budget_total; int budget_spent;
    int timeline_days; int days_elapsed;
} ProjectCharter;

void charter_init(ProjectCharter *pc, const char *name, const char *sponsor, const char *manager);
int  charter_add_objective(ProjectCharter *pc, const char *desc, double target);
int  charter_add_constraint(ProjectCharter *pc, const char *desc, const char *category);
int  charter_add_scope(ProjectCharter *pc, const char *name, bool in_scope);
void charter_update_objective(ProjectCharter *pc, int idx, double current);
double charter_progress_pct(ProjectCharter *pc); /* avg of objectives current/target */
bool charter_is_on_budget(ProjectCharter *pc);
bool charter_is_on_schedule(ProjectCharter *pc);
void charter_print(ProjectCharter *pc);
#endif
