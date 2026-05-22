#include "project_stakeholder.h"
#include <stdio.h>
#include <string.h>

void stakeholder_map_init(StakeholderMap *sm) { memset(sm, 0, sizeof(*sm)); }

int stakeholder_add(StakeholderMap *sm, const char *name, const char *role,
                    StakeholderInfluence infl, StakeholderInterest inter) {
    if (sm->stakeholder_count >= MAX_STAKEHOLDERS) return -1;
    Stakeholder *s = &sm->stakeholders[sm->stakeholder_count];
    strncpy(s->name, name, 63); s->name[63] = '\0';
    strncpy(s->role, role, 63); s->role[63] = '\0';
    s->org[0] = '\0';
    s->influence = infl; s->interest = inter;
    s->is_sponsor = false; s->is_key = false;
    s->communication_need[0] = '\0'; s->notes[0] = '\0';
    return sm->stakeholder_count++;
}

void stakeholder_set_key(StakeholderMap *sm, int idx, bool is_key) {
    if (idx >= 0 && idx < sm->stakeholder_count) sm->stakeholders[idx].is_key = is_key;
}

int raci_add(StakeholderMap *sm, const char *activity) {
    if (sm->raci_count >= MAX_RACI_ITEMS) return -1;
    RaciItem *ri = &sm->raci[sm->raci_count];
    strncpy(ri->activity, activity, 127); ri->activity[127] = '\0';
    ri->responsible_idx = -1;
    for (int i = 0; i < MAX_STAKEHOLDERS; i++) ri->roles[i] = -1;
    return sm->raci_count++;
}

void raci_assign(StakeholderMap *sm, int raci_idx, int stakeholder_idx, RaciRole role) {
    if (raci_idx < 0 || raci_idx >= sm->raci_count) return;
    if (stakeholder_idx < 0 || stakeholder_idx >= sm->stakeholder_count) return;
    sm->raci[raci_idx].roles[stakeholder_idx] = role;
    if (role == RACI_R) sm->raci[raci_idx].responsible_idx = stakeholder_idx;
}

int stakeholder_count_by_influence(StakeholderMap *sm, StakeholderInfluence infl) {
    int count = 0;
    for (int i = 0; i < sm->stakeholder_count; i++)
        if (sm->stakeholders[i].influence == infl) count++;
    return count;
}

void stakeholder_print_matrix(StakeholderMap *sm) {
    printf("=== Stakeholder Power/Interest Grid ===\n");
    printf("          Low Int  Med Int  High Int\n");
    for (int infl = SINFL_HIGH; infl >= SINFL_LOW; infl--) {
        const char *il[] = {"","LOW ","MED ","HIGH"};
        printf("  %s PWR", il[infl]);
        for (int inter = SINTEREST_LOW; inter <= SINTEREST_HIGH; inter++) {
            int cnt = 0;
            for (int i = 0; i < sm->stakeholder_count; i++)
                if ((int)sm->stakeholders[i].influence == infl && (int)sm->stakeholders[i].interest == inter) cnt++;
            printf("   %d     ", cnt);
        }
        printf("\n");
    }
    printf("  Key stakeholders:\n");
    for (int i = 0; i < sm->stakeholder_count; i++)
        if (sm->stakeholders[i].is_key)
            printf("    - %s (%s) [PWR:%d INT:%d]\n",
                   sm->stakeholders[i].name, sm->stakeholders[i].role,
                   sm->stakeholders[i].influence, sm->stakeholders[i].interest);
}

void stakeholder_print_raci(StakeholderMap *sm) {
    const char *rl[] = {"","R","A","C","I"};
    printf("=== RACI Matrix ===\n");
    printf("%-20s", "Activity");
    for (int i = 0; i < sm->stakeholder_count; i++)
        printf(" %-8s", sm->stakeholders[i].name);
    printf("\n");
    for (int i = 0; i < sm->raci_count; i++) {
        printf("%-20s", sm->raci[i].activity);
        for (int j = 0; j < sm->stakeholder_count; j++)
            printf(" %-8s", sm->raci[i].roles[j] >= 0 ? rl[sm->raci[i].roles[j]+1] : "-");
        printf("\n");
    }
}
