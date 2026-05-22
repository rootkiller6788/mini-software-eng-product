#ifndef PROJECT_RISK_H
#define PROJECT_RISK_H
#include <stdbool.h>

#define MAX_RISKS 64

typedef enum { RSEV_LOW=1, RSEV_MEDIUM=2, RSEV_HIGH=3, RSEV_CRITICAL=4 } RiskSeverity;
typedef enum { RPROB_RARE=1, RPROB_UNLIKELY=2, RPROB_POSSIBLE=3, RPROB_LIKELY=4, RPROB_ALMOST_CERTAIN=5 } RiskProbability;
typedef enum { RSTAT_OPEN, RSTAT_MITIGATED, RSTAT_CLOSED, RSTAT_ACCEPTED } RiskStatus;

typedef struct {
    char id[16]; char description[256];
    RiskProbability probability; RiskSeverity impact;
    int risk_score; /* probability * impact */
    RiskStatus status;
    char mitigation[256]; char owner[64];
    char trigger_condition[128]; /* what triggers this risk */
} Risk;

typedef struct {
    Risk risks[MAX_RISKS]; int risk_count;
    int risk_appetite; /* max acceptable risk score sum */
} RiskRegistry;

void risk_registry_init(RiskRegistry *rr, int appetite);
int  risk_add(RiskRegistry *rr, const char *id, const char *desc, RiskProbability prob, RiskSeverity sev);
void risk_update_status(RiskRegistry *rr, const char *id, RiskStatus status);
void risk_set_mitigation(RiskRegistry *rr, const char *id, const char *mitigation, const char *owner);
int  risk_top_n(RiskRegistry *rr, int n, int *indices); /* top N by risk score, returns count */
int  risk_exposure(RiskRegistry *rr); /* sum of open risk scores */
bool risk_within_appetite(RiskRegistry *rr);
void risk_print(RiskRegistry *rr);
void risk_print_matrix(RiskRegistry *rr); /* PxI heatmap */
#endif
