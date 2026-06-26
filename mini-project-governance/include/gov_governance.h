#ifndef GOV_GOVERNANCE_H
#define GOV_GOVERNANCE_H
#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include "gov_scrum.h"
#include "gov_kanban.h"
#include "gov_okr.h"
#include "gov_tracker.h"
#include "gov_velocity.h"
#include "gov_retro.h"

/* ============================================================
 * L1: Governance Orchestration — Unified API
 * L3: Cross-system integration structures
 * L6: End-to-end sprint management demo
 * ============================================================ */

#define GOV_MAX_PROGRAMS      16
#define GOV_NAME_LEN          128
#define GOV_DESC_LEN          512

/* ---- Governance Framework ---- */
typedef enum {
    GOV_SCRUM = 0,
    GOV_KANBAN,
    GOV_SCRUMBAN,
    GOV_SAFE,            /* Scaled Agile Framework */
    GOV_WATERFALL_HYBRID
} GovernanceFramework;

/* ---- Risk ---- */
typedef enum {
    RISK_LOW = 0,
    RISK_MEDIUM,
    RISK_HIGH,
    RISK_CRITICAL
} RiskLevel;

typedef struct {
    char id[32];
    char description[GOV_DESC_LEN];
    RiskLevel level;
    RiskLevel residual_level;
    double probability;      /* 0.0 - 1.0 */
    double impact;           /* 1-10 */
    double exposure;         /* probability × impact */
    char mitigation[GOV_DESC_LEN];
    char owner[GOV_NAME_LEN];
    bool active;
    uint32_t identified_at;
    uint32_t resolved_at;
} GovernanceRisk;

/* ---- Stakeholder ---- */
typedef enum {
    STAKE_RACI_R,       /* Responsible */
    STAKE_RACI_A,       /* Accountable */
    STAKE_RACI_C,       /* Consulted */
    STAKE_RACI_I        /* Informed */
} RaciRole;

typedef struct {
    char name[GOV_NAME_LEN];
    RaciRole role;
    double influence;      /* 1-10 */
    double interest;       /* 1-10 */
    char communication[GOV_DESC_LEN];
} Stakeholder;

/* ---- Program / Large Initiative ---- */
typedef struct {
    char name[GOV_NAME_LEN];
    char mission[GOV_DESC_LEN];
    GovernanceFramework framework;
    ScrumBoard scrum_board;
    KanbanBoard kanban_board;
    OkrDashboard okr_dashboard;
    IssueTracker tracker;
    VelocityProfile velocity;
    RetroLog retro_log;
    GovernanceRisk risks[32];
    int risk_count;
    Stakeholder stakeholders[16];
    int stakeholder_count;
    double budget_total;
    double budget_spent;
    uint32_t start_date;
    uint32_t target_date;
    bool active;
} GovernanceProgram;

/* ---- L4: Governance Metrics ---- */
typedef struct {
    double on_time_delivery_pct;
    double budget_adherence_pct;
    double quality_index;
    double stakeholder_satisfaction;
    double team_health_index;
    double risk_exposure;
    double okr_attainment;
    double predictability;      /* velocity stability */
} GovernanceScorecard;

/* ---- L6: Sprint Simulation Config ---- */
typedef struct {
    int num_sprints;
    int sprint_duration_days;
    int team_size;
    double avg_velocity;
    double velocity_std;
    double scope_creep_rate;    /* % new items added mid-sprint */
    double defect_rate;         /* % of items needing rework */
    int total_backlog_points;
} SprintSimConfig;

typedef struct {
    int sprint_num;
    int planned;
    int completed;
    int added;
    int defects_found;
    double happiness;
} SprintSimResult;

/* ---- API ---- */
void gov_program_init(GovernanceProgram *gp, const char *name, const char *mission,
                      GovernanceFramework framework);

/* Risk management */
int  gov_add_risk(GovernanceProgram *gp, const char *id, const char *desc,
                  double prob, double impact, const char *owner, const char *mitigation);
void gov_update_risk_exposure(GovernanceProgram *gp);
double gov_total_risk_exposure(GovernanceProgram *gp);
int  gov_top_risks(GovernanceProgram *gp, int *results, int max);

/* Stakeholder management */
int  gov_add_stakeholder(GovernanceProgram *gp, const char *name, RaciRole role,
                         double influence, double interest);
void gov_stakeholder_matrix(GovernanceProgram *gp);  /* Influence vs Interest */

/* L4: Earned Value Management */
typedef struct {
    double planned_value;     /* PV */
    double earned_value;      /* EV */
    double actual_cost;       /* AC */
    double budget_at_completion; /* BAC */
    double sv;                /* Schedule Variance = EV - PV */
    double cv;                /* Cost Variance = EV - AC */
    double spi;               /* Schedule Performance Index = EV / PV */
    double cpi;               /* Cost Performance Index = EV / AC */
    double eac;               /* Estimate at Completion */
    double etc;               /* Estimate to Complete */
} EarnedValue;

void evm_init(EarnedValue *ev, double bac);
void evm_update(EarnedValue *ev, double planned, double earned, double actual);
bool evm_on_schedule(EarnedValue *ev);
bool evm_on_budget(EarnedValue *ev);
void evm_forecast(EarnedValue *ev);

/* L6: Sprint simulation */
void sprint_sim_init(SprintSimConfig *cfg);
void sprint_sim_run(SprintSimConfig *cfg, SprintSimResult *results, int max_sprints);
void sprint_sim_print(SprintSimResult *results, int count);

/* Scorecard */
void gov_calc_scorecard(GovernanceProgram *gp, GovernanceScorecard *sc);
void gov_print_scorecard(GovernanceScorecard *sc);
void gov_print_risks(GovernanceProgram *gp);
void gov_print_dashboard(GovernanceProgram *gp);

/* L9: Industry Frontier — AI-assisted sprint planning 
 * Documented conceptual model (not implemented):
 * - ML-based story point estimation from historical data
 * - NLP-based PRD-to-backlog-item decomposition
 * - Anomaly detection for sprint burndown patterns
 */

#endif
