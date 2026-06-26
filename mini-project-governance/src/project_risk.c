#include "project_risk.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void risk_registry_init(RiskRegistry *rr, int appetite) {
    memset(rr, 0, sizeof(*rr));
    rr->risk_appetite = appetite;
}

int risk_add(RiskRegistry *rr, const char *id, const char *desc, RiskProbability prob, RiskSeverity sev) {
    if (rr->risk_count >= MAX_RISKS) return -1;
    Risk *r = &rr->risks[rr->risk_count];
    strncpy(r->id, id, 15); r->id[15] = '\0';
    strncpy(r->description, desc, 255); r->description[255] = '\0';
    r->probability = prob; r->impact = sev;
    r->risk_score = (int)prob * (int)sev;
    r->status = RSTAT_OPEN;
    r->mitigation[0] = '\0'; r->owner[0] = '\0'; r->trigger_condition[0] = '\0';
    return rr->risk_count++;
}

void risk_update_status(RiskRegistry *rr, const char *id, RiskStatus status) {
    for (int i = 0; i < rr->risk_count; i++)
        if (strcmp(rr->risks[i].id, id) == 0) { rr->risks[i].status = status; return; }
}

void risk_set_mitigation(RiskRegistry *rr, const char *id, const char *mitigation, const char *owner) {
    for (int i = 0; i < rr->risk_count; i++)
        if (strcmp(rr->risks[i].id, id) == 0) {
            strncpy(rr->risks[i].mitigation, mitigation, 255);
            strncpy(rr->risks[i].owner, owner, 63);
            return;
        }
}

int risk_top_n(RiskRegistry *rr, int n, int *indices) {
    Risk sorted[MAX_RISKS];
    int idx_map[MAX_RISKS];
    memcpy(sorted, rr->risks, rr->risk_count * sizeof(Risk));
    for (int i = 0; i < rr->risk_count; i++) idx_map[i] = i;

    /* sort indices by risk score */
    for (int i = 0; i < rr->risk_count - 1; i++)
        for (int j = i + 1; j < rr->risk_count; j++)
            if (sorted[j].risk_score > sorted[i].risk_score) {
                Risk tr = sorted[i]; sorted[i] = sorted[j]; sorted[j] = tr;
                int ti = idx_map[i]; idx_map[i] = idx_map[j]; idx_map[j] = ti;
            }

    int count = n < rr->risk_count ? n : rr->risk_count;
    for (int i = 0; i < count; i++) indices[i] = idx_map[i];
    return count;
}

int risk_exposure(RiskRegistry *rr) {
    int total = 0;
    for (int i = 0; i < rr->risk_count; i++)
        if (rr->risks[i].status == RSTAT_OPEN) total += rr->risks[i].risk_score;
    return total;
}

bool risk_within_appetite(RiskRegistry *rr) {
    return risk_exposure(rr) <= rr->risk_appetite;
}

void risk_print(RiskRegistry *rr) {
    const char *ps[] = {"","RARE","UNLIKELY","POSSIBLE","LIKELY","ALMOST CERTAIN"};
    const char *ss[] = {"","LOW","MED","HIGH","CRIT"};
    printf("=== Risk Registry: %d risks (exposure=%d, appetite=%d) ===\n",
           rr->risk_count, risk_exposure(rr), rr->risk_appetite);
    for (int i = 0; i < rr->risk_count; i++) {
        Risk *r = &rr->risks[i];
        printf("  %s [P:%s I:%s S:%d] %s\n", r->id, ps[r->probability], ss[r->impact], r->risk_score, r->description);
        if (r->mitigation[0]) printf("    Mitigation: %s (owner: %s)\n", r->mitigation, r->owner);
    }
}

void risk_print_matrix(RiskRegistry *rr) {
    printf("=== Risk Matrix (P x I) ===\n");
    printf("        LOW  MED  HIGH CRIT\n");
    for (int p = RPROB_ALMOST_CERTAIN; p >= RPROB_RARE; p--) {
        const char *pl[] = {"","RARE","UNLK","POSS","LIKL","ACRT"};
        printf("  %-4s ", pl[p]);
        for (int s = RSEV_LOW; s <= RSEV_CRITICAL; s++) {
            int cnt = 0;
            for (int i = 0; i < rr->risk_count; i++)
                if ((int)rr->risks[i].probability == p && (int)rr->risks[i].impact == s) cnt++;
            printf(" %3d ", cnt);
        }
        printf("\n");
    }
}
