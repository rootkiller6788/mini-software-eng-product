#include "gov_governance.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

/* ============================================================
 * L2: Governance Orchestration Implementation
 * Core concepts: Program management, risk, stakeholder,
 *                earned value management (EVM), sprint simulation
 * L4: EVM formulas (SPI, CPI, EAC)
 * L6: Sprint simulation demo — end-to-end governance
 * ============================================================ */

void gov_program_init(GovernanceProgram *gp, const char *name, const char *mission,
                      GovernanceFramework framework) {
    memset(gp, 0, sizeof(*gp));
    strncpy(gp->name, name, GOV_NAME_LEN - 1);
    gp->name[GOV_NAME_LEN - 1] = '\0';
    strncpy(gp->mission, mission, GOV_DESC_LEN - 1);
    gp->mission[GOV_DESC_LEN - 1] = '\0';
    gp->framework = framework;
    gp->start_date = (uint32_t)time(NULL);
    gp->active = true;
    gp->budget_total = 0;
    gp->budget_spent = 0;
    gp->risk_count = 0;
    gp->stakeholder_count = 0;

    /* Init subsystems */
    scrum_init(&gp->scrum_board);
    kanban_init(&gp->kanban_board);
    okr_init(&gp->okr_dashboard, 2026, 1);
    tracker_init(&gp->tracker);
    velocity_init(&gp->velocity, name, 3);
    retro_init(&gp->retro_log);
}

/* ---- Risk Management ---- */
int gov_add_risk(GovernanceProgram *gp, const char *id, const char *desc,
                 double prob, double impact, const char *owner, const char *mitigation) {
    if (gp->risk_count >= 32) return -1;
    GovernanceRisk *r = &gp->risks[gp->risk_count];
    memset(r, 0, sizeof(*r));
    strncpy(r->id, id, 31); r->id[31] = '\0';
    strncpy(r->description, desc, GOV_DESC_LEN - 1);
    r->description[GOV_DESC_LEN - 1] = '\0';
    r->probability = prob;
    r->impact = impact;
    r->exposure = prob * impact;
    r->residual_level = (RiskLevel)(int)(r->exposure / 2.5);
    if (r->residual_level > RISK_CRITICAL) r->residual_level = RISK_CRITICAL;
    strncpy(r->owner, owner, GOV_NAME_LEN - 1);
    r->owner[GOV_NAME_LEN - 1] = '\0';
    strncpy(r->mitigation, mitigation, GOV_DESC_LEN - 1);
    r->mitigation[GOV_DESC_LEN - 1] = '\0';
    r->active = true;
    r->identified_at = (uint32_t)time(NULL);
    return gp->risk_count++;
}

void gov_update_risk_exposure(GovernanceProgram *gp) {
    for (int i = 0; i < gp->risk_count; i++) {
        gp->risks[i].exposure = gp->risks[i].probability * gp->risks[i].impact;
    }
}

double gov_total_risk_exposure(GovernanceProgram *gp) {
    double total = 0;
    for (int i = 0; i < gp->risk_count; i++) {
        if (gp->risks[i].active) total += gp->risks[i].exposure;
    }
    return total;
}

/* L5: Top risks by exposure (selection sort, descending) */
int gov_top_risks(GovernanceProgram *gp, int *results, int max) {
    if (gp->risk_count == 0) return 0;
    int *indices = (int *)malloc(gp->risk_count * sizeof(int));
    if (!indices) return 0;
    for (int i = 0; i < gp->risk_count; i++) indices[i] = i;
    for (int i = 0; i < gp->risk_count - 1; i++) {
        int max_i = i;
        for (int j = i + 1; j < gp->risk_count; j++) {
            if (gp->risks[indices[j]].exposure > gp->risks[indices[max_i]].exposure)
                max_i = j;
        }
        int t = indices[i]; indices[i] = indices[max_i]; indices[max_i] = t;
    }
    int count = 0;
    for (int i = 0; i < gp->risk_count && count < max; i++) {
        if (gp->risks[indices[i]].active) results[count++] = indices[i];
    }
    free(indices);
    return count;
}

/* ---- Stakeholder Management ---- */
int gov_add_stakeholder(GovernanceProgram *gp, const char *name, RaciRole role,
                        double influence, double interest) {
    if (gp->stakeholder_count >= 16) return -1;
    Stakeholder *s = &gp->stakeholders[gp->stakeholder_count];
    memset(s, 0, sizeof(*s));
    strncpy(s->name, name, GOV_NAME_LEN - 1);
    s->name[GOV_NAME_LEN - 1] = '\0';
    s->role = role;
    s->influence = influence;
    s->interest = interest;
    return gp->stakeholder_count++;
}

void gov_stakeholder_matrix(GovernanceProgram *gp) {
    printf("=== Stakeholder Matrix (Influence vs Interest) ===\n");
    printf("  High Influence/High Interest (Manage Closely):\n");
    for (int i = 0; i < gp->stakeholder_count; i++) {
        if (gp->stakeholders[i].influence >= 7 && gp->stakeholders[i].interest >= 7)
            printf("    - %s\n", gp->stakeholders[i].name);
    }
    printf("  High Influence/Low Interest (Keep Satisfied):\n");
    for (int i = 0; i < gp->stakeholder_count; i++) {
        if (gp->stakeholders[i].influence >= 7 && gp->stakeholders[i].interest < 7)
            printf("    - %s\n", gp->stakeholders[i].name);
    }
    printf("  Low Influence/High Interest (Keep Informed):\n");
    for (int i = 0; i < gp->stakeholder_count; i++) {
        if (gp->stakeholders[i].influence < 7 && gp->stakeholders[i].interest >= 7)
            printf("    - %s\n", gp->stakeholders[i].name);
    }
}

/* ---- Earned Value Management (L4) ---- */
void evm_init(EarnedValue *ev, double bac) {
    memset(ev, 0, sizeof(*ev));
    ev->budget_at_completion = bac;
}

void evm_update(EarnedValue *ev, double planned, double earned, double actual) {
    ev->planned_value = planned;
    ev->earned_value = earned;
    ev->actual_cost = actual;
    ev->sv = ev->earned_value - ev->planned_value;
    ev->cv = ev->earned_value - ev->actual_cost;
    ev->spi = ev->planned_value > 0 ? ev->earned_value / ev->planned_value : 0;
    ev->cpi = ev->actual_cost > 0 ? ev->earned_value / ev->actual_cost : 0;
    /* EAC = BAC / CPI */
    ev->eac = ev->cpi > 0 ? ev->budget_at_completion / ev->cpi : ev->budget_at_completion;
    ev->etc = ev->eac - ev->actual_cost;
}

bool evm_on_schedule(EarnedValue *ev) {
    return ev->spi >= 0.95; /* Within 5% of plan */
}

bool evm_on_budget(EarnedValue *ev) {
    return ev->cpi >= 0.95;
}

void evm_forecast(EarnedValue *ev) {
    printf("=== Earned Value Management ===\n");
    printf("  PV: %.1f | EV: %.1f | AC: %.1f\n", ev->planned_value, ev->earned_value, ev->actual_cost);
    printf("  SV: %.1f | CV: %.1f\n", ev->sv, ev->cv);
    printf("  SPI: %.2f (%s)\n", ev->spi, evm_on_schedule(ev) ? "ON TRACK" : "BEHIND");
    printf("  CPI: %.2f (%s)\n", ev->cpi, evm_on_budget(ev) ? "ON BUDGET" : "OVER BUDGET");
    printf("  BAC: %.1f | EAC: %.1f | ETC: %.1f\n", ev->budget_at_completion, ev->eac, ev->etc);
}

/* ---- L6: Sprint Simulation ---- */
void sprint_sim_init(SprintSimConfig *cfg) {
    memset(cfg, 0, sizeof(*cfg));
    cfg->num_sprints = 5;
    cfg->sprint_duration_days = 10;
    cfg->team_size = 5;
    cfg->avg_velocity = 25;
    cfg->velocity_std = 5;
    cfg->scope_creep_rate = 0.1;
    cfg->defect_rate = 0.15;
    cfg->total_backlog_points = 200;
}

/* L5: Monte Carlo-style sprint simulation with configurable parameters */
void sprint_sim_run(SprintSimConfig *cfg, SprintSimResult *results, int max_sprints) {
    int sprints = cfg->num_sprints < max_sprints ? cfg->num_sprints : max_sprints;
    double remaining = (double)cfg->total_backlog_points;
    srand((unsigned int)time(NULL));

    for (int s = 0; s < sprints; s++) {
        SprintSimResult *r = &results[s];
        r->sprint_num = s + 1;

        /* Sample velocity with normal-ish distribution (Box-Muller simplified) */
        double u1 = (double)rand() / RAND_MAX;
        double u2 = (double)rand() / RAND_MAX;
        if (u1 < 0.01) u1 = 0.01; /* Avoid log(0) */
        double z = sqrt(-2.0 * log(u1)) * cos(2.0 * 3.14159265 * u2);
        double velocity = cfg->avg_velocity + z * cfg->velocity_std;
        if (velocity < 5) velocity = 5;

        int planned = (int)(velocity + 0.5);
        if (planned > remaining) planned = (int)remaining;

        /* Scope creep: mid-sprint additions */
        int added = (int)(planned * cfg->scope_creep_rate);
        int total = planned + added;

        /* Defects: some percentage of items need rework */
        int defects = (int)(total * cfg->defect_rate);
        int completed = total - defects;
        if (completed < 0) completed = 0;

        r->planned = planned;
        r->completed = completed;
        r->added = added;
        r->defects_found = defects;
        /* Happiness degrades with defects and improves with completion */
        r->happiness = 4.0 - (double)defects / (total > 0 ? total : 1) * 2.0;
        if (r->happiness < 1.0) r->happiness = 1.0;

        remaining -= completed;
        if (remaining < 0) remaining = 0;
    }
}

void sprint_sim_print(SprintSimResult *results, int count) {
    printf("=== Sprint Simulation Results ===\n");
    printf("  Sprint | Planned | Completed | Added | Defects | Happiness\n");
    int total_planned = 0, total_completed = 0;
    for (int i = 0; i < count; i++) {
        SprintSimResult *r = &results[i];
        total_planned += r->planned;
        total_completed += r->completed;
        printf("  %6d | %7d | %9d | %5d | %7d | %.1f\n",
               r->sprint_num, r->planned, r->completed, r->added, r->defects_found, r->happiness);
    }
    printf("  ----------------------------------------\n");
    printf("  TOTAL  | %7d | %9d | Focus Factor: %.2f\n",
           total_planned, total_completed,
           total_planned > 0 ? (double)total_completed / total_planned : 0);
}

/* ---- Scorecard (L4: Balanced metrics) ---- */
void gov_calc_scorecard(GovernanceProgram *gp, GovernanceScorecard *sc) {
    memset(sc, 0, sizeof(*sc));

    /* On-time delivery: % of sprints that met goals */
    int on_time = 0, total_sprints = 0;
    for (int i = 0; i < gp->scrum_board.sprint_count; i++) {
        total_sprints++;
        if (gp->scrum_board.sprints[i].goal_met) on_time++;
    }
    sc->on_time_delivery_pct = total_sprints > 0 ? (double)on_time / total_sprints * 100 : 0;

    /* Budget adherence */
    sc->budget_adherence_pct = gp->budget_total > 0 ?
        (1.0 - gp->budget_spent / gp->budget_total) * 100 : 100;

    /* OKR attainment */
    sc->okr_attainment = okr_company_score(&gp->okr_dashboard) * 100;

    /* Predictability (velocity stability) */
    sc->predictability = velocity_is_stable(&gp->velocity, 0.3) ? 100 :
        (1.0 - velocity_volatility(&gp->velocity)) * 100;
    if (sc->predictability < 0) sc->predictability = 0;

    /* Team health */
    if (gp->retro_log.trend_points > 0) {
        double h_sum = 0;
        for (int i = 0; i < gp->retro_log.trend_points; i++)
            h_sum += gp->retro_log.team_happiness_trend[i];
        sc->team_health_index = (h_sum / gp->retro_log.trend_points) / 5.0 * 100;
    }

    /* Risk exposure */
    double max_possible = (double)gp->risk_count * 10.0;
    sc->risk_exposure = max_possible > 0 ?
        (1.0 - gov_total_risk_exposure(gp) / max_possible) * 100 : 100;
}

void gov_print_scorecard(GovernanceScorecard *sc) {
    printf("=== Governance Scorecard ===\n");
    printf("  On-Time Delivery:     %.0f%%\n", sc->on_time_delivery_pct);
    printf("  Budget Adherence:     %.0f%%\n", sc->budget_adherence_pct);
    printf("  OKR Attainment:       %.0f%%\n", sc->okr_attainment);
    printf("  Predictability:       %.0f%%\n", sc->predictability);
    printf("  Team Health Index:    %.0f%%\n", sc->team_health_index);
    printf("  Risk Exposure:        %.0f%%\n", sc->risk_exposure);
    double overall = (sc->on_time_delivery_pct + sc->budget_adherence_pct +
                      sc->okr_attainment + sc->predictability +
                      sc->team_health_index + sc->risk_exposure) / 6.0;
    printf("  OVERALL SCORE:        %.0f%% %s\n", overall,
           overall >= 80 ? "GREEN" : overall >= 60 ? "YELLOW" : "RED");
}

void gov_print_risks(GovernanceProgram *gp) {
    printf("=== Risk Register (%d risks, Total Exposure: %.1f) ===\n",
           gp->risk_count, gov_total_risk_exposure(gp));
    int top[10];
    int n = gov_top_risks(gp, top, 10);
    for (int i = 0; i < n; i++) {
        GovernanceRisk *r = &gp->risks[top[i]];
        printf("  [%s] P=%.2f I=%.1f E=%.1f — %s\n", r->id,
               r->probability, r->impact, r->exposure, r->description);
        printf("    Owner: %s | Mitigation: %s\n", r->owner, r->mitigation);
    }
}

void gov_print_dashboard(GovernanceProgram *gp) {
    printf("========================================\n");
    printf("  GOVERNANCE DASHBOARD: %s\n", gp->name);
    printf("========================================\n");
    printf("  Framework: %d | Mission: %s\n", gp->framework, gp->mission);
    printf("  Budget: %.1f / %.1f\n", gp->budget_spent, gp->budget_total);
    printf("\n");

    /* Sprint info */
    if (gp->scrum_board.sprint_count > 0) {
        scrum_print_sprint_report(&gp->scrum_board,
            gp->scrum_board.current_sprint >= 0 ? gp->scrum_board.current_sprint : 0);
        scrum_print_velocity_chart(&gp->scrum_board);
    }

    /* OKR */
    okr_print_dashboard(&gp->okr_dashboard);

    /* Risks */
    gov_print_risks(gp);

    /* Kanban metrics */
    kanban_print_metrics(&gp->kanban_board);

    /* Scorecard */
    GovernanceScorecard sc;
    gov_calc_scorecard(gp, &sc);
    gov_print_scorecard(&sc);
}