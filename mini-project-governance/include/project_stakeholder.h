#ifndef PROJECT_STAKEHOLDER_H
#define PROJECT_STAKEHOLDER_H
#include <stdbool.h>

#define MAX_STAKEHOLDERS 32
#define MAX_RACI_ITEMS 16

typedef enum { RACI_R, RACI_A, RACI_C, RACI_I } RaciRole;
typedef enum { SINFL_HIGH=3, SINFL_MEDIUM=2, SINFL_LOW=1 } StakeholderInfluence;
typedef enum { SINTEREST_HIGH=3, SINTEREST_MEDIUM=2, SINTEREST_LOW=1 } StakeholderInterest;

typedef struct {
    char name[64]; char role[64]; char org[64];
    StakeholderInfluence influence; StakeholderInterest interest;
    bool is_sponsor; bool is_key; /* key stakeholder */
    char communication_need[128]; char notes[256];
} Stakeholder;

typedef struct {
    char activity[128];
    RaciRole roles[MAX_STAKEHOLDERS]; /* parallel array to StakeholderMap.stakeholders */
    int responsible_idx; /* index of the "R" person */
} RaciItem;

typedef struct {
    Stakeholder stakeholders[MAX_STAKEHOLDERS]; int stakeholder_count;
    RaciItem raci[MAX_RACI_ITEMS]; int raci_count;
} StakeholderMap;

void stakeholder_map_init(StakeholderMap *sm);
int  stakeholder_add(StakeholderMap *sm, const char *name, const char *role, StakeholderInfluence infl, StakeholderInterest inter);
void stakeholder_set_key(StakeholderMap *sm, int idx, bool is_key);
int  raci_add(StakeholderMap *sm, const char *activity);
void raci_assign(StakeholderMap *sm, int raci_idx, int stakeholder_idx, RaciRole role);
int  stakeholder_count_by_influence(StakeholderMap *sm, StakeholderInfluence infl);
void stakeholder_print_matrix(StakeholderMap *sm); /* power/interest grid */
void stakeholder_print_raci(StakeholderMap *sm);
#endif
