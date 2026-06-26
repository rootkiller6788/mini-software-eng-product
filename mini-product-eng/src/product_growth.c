#include "product_growth.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

/* ================================================================
 * L5: Viral Growth Model ?? K-factor algorithm
 * Reference: David Skok "The Science of Viral Marketing"
 *
 * The viral coefficient K = i ?? conv_rate determines growth mode:
 *   K < 1: sub-linear (eventually saturates)
 *   K = 1: linear growth
 *   K > 1: exponential (viral)
 * ================================================================ */

void viral_model_init(ViralModel *vm, double invites, double conv, double cycle_days, int initial) {
    if (!vm) return;
    memset(vm, 0, sizeof(*vm));
    vm->invites_per_user = invites;
    vm->conversion_rate = conv;
    vm->cycle_time_days = cycle_days;
    vm->initial_users = initial;
    vm->k_factor = invites * conv;
    vm->total_users_after_cycles = initial;
    vm->cycles_simulated = 0;
}

void viral_model_simulate(ViralModel *vm, int max_cycles) {
    if (!vm || max_cycles <= 0) return;
    double k = vm->k_factor;
    int u0 = vm->initial_users;
    double total;

    if (fabs(k - 1.0) < 1e-9) {
        total = u0 * (double)(max_cycles + 1);
    } else {
        double kn1 = pow(k, max_cycles + 1);
        total = u0 * (kn1 - 1.0) / (k - 1.0);
    }
    vm->total_users_after_cycles = (int)total;
    vm->cycles_simulated = max_cycles;
}

double viral_model_growth_rate(const ViralModel *vm) {
    if (!vm || vm->cycles_simulated <= 0 || vm->initial_users <= 0) return 0.0;
    double end = (double)vm->total_users_after_cycles;
    double start = (double)vm->initial_users;
    return (end - start) / start;
}

bool viral_model_is_viral(const ViralModel *vm) {
    return vm && vm->k_factor > 1.0;
}

/* ================================================================
 * L4: Customer Lifetime Value ?? SaaS Metrics Theorem
 * LTV = ARPU ?? Gross Margin / Churn Rate
 * LTV:CAC ratio >= 3 is considered healthy
 * CAC Payback <= 12 months benchmark
 * ================================================================ */

void customer_ltv_init(CustomerLTV *clv, double arpu, double margin, double churn, double cac) {
    if (!clv) return;
    memset(clv, 0, sizeof(*clv));
    clv->monthly_arpu = arpu;
    clv->gross_margin_pct = margin;
    clv->monthly_churn_pct = churn;
    clv->cac = cac;
}

void customer_ltv_calculate(CustomerLTV *clv) {
    if (!clv) return;
    double margin_decimal = clv->gross_margin_pct / 100.0;
    double churn_decimal = clv->monthly_churn_pct / 100.0;

    if (churn_decimal > 1e-9) {
        clv->ltv = clv->monthly_arpu * margin_decimal / churn_decimal;
    } else {
        clv->ltv = clv->monthly_arpu * margin_decimal * 60.0;
    }

    if (clv->cac > 1e-9) {
        clv->ltv_cac_ratio = clv->ltv / clv->cac;
    } else {
        clv->ltv_cac_ratio = 999.0;
    }

    double monthly_contribution = clv->monthly_arpu * margin_decimal;
    if (monthly_contribution > 1e-9) {
        clv->payback_months = (int)ceil(clv->cac / monthly_contribution);
    } else {
        clv->payback_months = 999;
    }
}

bool customer_ltv_is_healthy(const CustomerLTV *clv) {
    if (!clv) return false;
    return clv->ltv_cac_ratio >= 3.0 && clv->payback_months <= 12;
}

double customer_ltv_payback_months(const CustomerLTV *clv) {
    return clv ? (double)clv->payback_months : 0.0;
}

/* ================================================================
 * L5: Cohort LTV Projection ?? Discounted Cash Flow
 * Reference: Peter Fader "Customer Centricity" (Wharton)
 * CLV = sum(retention(t) * revenue(t) / (1 + d)^t)
 * ================================================================ */

void cohort_ltv_init(CohortLTVProjection *cl, int size, double discount) {
    if (!cl) return;
    memset(cl, 0, sizeof(*cl));
    cl->cohort_size = size;
    cl->discount_rate = discount;
}

void cohort_ltv_record(CohortLTVProjection *cl, int month, double retention, double rev_per_user) {
    if (!cl || month < 0 || month >= MAX_COHORT_MONTHS) return;
    cl->retention[month] = retention;
    cl->revenue_per_user[month] = rev_per_user;
}

void cohort_ltv_project(CohortLTVProjection *cl, int months) {
    if (!cl || months <= 0) return;
    int n = months < MAX_COHORT_MONTHS ? months : MAX_COHORT_MONTHS;
    cl->months_projected = n;
    double cumulative = 0.0;
    for (int t = 0; t < n; t++) {
        double discount_factor = pow(1.0 + cl->discount_rate, (double)t);
        double net_rev = cl->revenue_per_user[t] * (cl->retention[t] / 100.0);
        cumulative += net_rev / discount_factor;
    }
    cl->cumulative_ltv = cumulative;
}

double cohort_ltv_total(const CohortLTVProjection *cl) {
    return cl ? cl->cumulative_ltv : 0.0;
}

/* ================================================================
 * L5: Net Revenue Retention (NRR)
 * NRR = (Starting MRR + Expansion - Contraction - Churn) / Starting MRR
 * NRR > 100% = net negative churn (gold standard)
 * ================================================================ */

void nrr_init(NetRevenueRetention *nr, double starting_mrr) {
    if (!nr) return;
    memset(nr, 0, sizeof(*nr));
    nr->starting_mrr = starting_mrr;
}

void nrr_add_expansion(NetRevenueRetention *nr, double amount) {
    if (nr) nr->expansion_mrr += amount;
}

void nrr_add_contraction(NetRevenueRetention *nr, double amount) {
    if (nr) nr->contraction_mrr += amount;
}

void nrr_add_churn(NetRevenueRetention *nr, double amount) {
    if (nr) nr->churned_mrr += amount;
}

void nrr_calculate(NetRevenueRetention *nr) {
    if (!nr || nr->starting_mrr <= 0.0) return;
    nr->ending_mrr = nr->starting_mrr + nr->expansion_mrr
                   - nr->contraction_mrr - nr->churned_mrr;
    nr->nrr_pct = (nr->ending_mrr / nr->starting_mrr) * 100.0;
}

const char *nrr_health_assessment(const NetRevenueRetention *nr) {
    if (!nr) return "Unknown";
    if (nr->nrr_pct >= 120.0) return "Best-in-class";
    if (nr->nrr_pct >= 110.0) return "Excellent";
    if (nr->nrr_pct >= 100.0) return "Healthy (net negative churn)";
    if (nr->nrr_pct >= 90.0)  return "Concerning";
    return "Critical ?? shrinking";
}

/* ================================================================
 * L5: AARRR Pirate Metrics (Dave McClure, 500 Startups, 2007)
 * ================================================================ */

void aarrr_init(AARRRMetrics *am) {
    if (am) memset(am, 0, sizeof(*am));
}

void aarrr_set_acquisition(AARRRMetrics *am, int visitors, int leads, int trials) {
    if (!am) return;
    am->visitors = visitors;
    am->leads = leads;
    am->trials = trials;
}

void aarrr_set_activation(AARRRMetrics *am, int activated, double time_to_aha) {
    if (!am) return;
    am->activated = activated;
    am->time_to_aha_sec = time_to_aha;
}

void aarrr_set_retention(AARRRMetrics *am, double d1, double d7, double d30, double w12) {
    if (!am) return;
    am->day1_retention_pct = d1;
    am->day7_retention_pct = d7;
    am->day30_retention_pct = d30;
    am->week12_retention_pct = w12;
}

void aarrr_set_revenue(AARRRMetrics *am, double arpu, double mrr) {
    if (!am) return;
    am->arpu = arpu;
    am->mrr = mrr;
    am->arr = mrr * 12.0;
}

void aarrr_calculate_conversions(AARRRMetrics *am) {
    if (!am) return;
    if (am->visitors > 0)
        am->visitor_to_lead_pct = 100.0 * (double)am->leads / (double)am->visitors;
    if (am->leads > 0)
        am->lead_to_trial_pct = 100.0 * (double)am->trials / (double)am->leads;
    if (am->trials > 0)
        am->activation_rate_pct = 100.0 * (double)am->activated / (double)am->trials;
    if (am->activated > 0 && am->referrals_sent > 0)
        am->referral_conversion_pct = 100.0 * (double)am->referrals_sent / (double)am->activated;
}

void aarrr_health_report(const AARRRMetrics *am, char *buffer, size_t sz) {
    if (!am || !buffer || sz == 0) return;
    snprintf(buffer, sz,
        "AARRR PIRATE METRICS REPORT\n"
        "===========================\n"
        "Acquisition: %d visitors -> %d leads (%.1f%%) -> %d trials (%.1f%%)\n"
        "Activation:  %d activated (%.1f%%)  Aha=%.0fs\n"
        "Retention:   D1=%.1f%%  D7=%.1f%%  D30=%.1f%%  W12=%.1f%%\n"
        "Revenue:     ARPU=$%.2f  MRR=$%.0f  ARR=$%.0f\n"
        "Referral:    NPS=%.1f  Referrals=%d  Conv=%.1f%%  K=%.2f\n",
        am->visitors, am->leads, am->visitor_to_lead_pct,
        am->trials, am->lead_to_trial_pct,
        am->activated, am->activation_rate_pct, am->time_to_aha_sec,
        am->day1_retention_pct, am->day7_retention_pct,
        am->day30_retention_pct, am->week12_retention_pct,
        am->arpu, am->mrr, am->arr,
        am->nps, am->referrals_sent,
        am->referral_conversion_pct, am->viral_coefficient);
}

/* ================================================================
 * L5: HEART Framework (Google UX Research, Kerry Rodden et al., 2010)
 * ================================================================ */

void heart_init(HEARTMetrics *hm) {
    if (hm) memset(hm, 0, sizeof(*hm));
}

void heart_set_happiness(HEARTMetrics *hm, double sat, double ease) {
    if (!hm) return;
    hm->satisfaction_score = sat;
    hm->perceived_ease = ease;
}

void heart_set_engagement(HEARTMetrics *hm, double dau, double mau, double sessions) {
    if (!hm) return;
    hm->dau = dau;
    hm->mau = mau;
    hm->sessions_per_user_day = sessions;
}

void heart_calculate_stickiness(HEARTMetrics *hm) {
    if (!hm || hm->mau <= 0.0) return;
    hm->stickiness = hm->dau / hm->mau;
}

void heart_overall_health(const HEARTMetrics *hm, char *buffer, size_t sz) {
    if (!hm || !buffer || sz == 0) return;
    const char *stickiness_label;
    if (hm->stickiness >= 0.50) stickiness_label = "Excellent (>50%)";
    else if (hm->stickiness >= 0.20) stickiness_label = "Good (20-50%)";
    else stickiness_label = "Low (<20%)";

    snprintf(buffer, sz,
        "HEART FRAMEWORK\n"
        "Happiness: SAT=%.1f  Ease=%.1f/7\n"
        "Engagement: DAU=%.0f  MAU=%.0f  Stickiness=%.1f%% [%s]\n"
        "Adoption:   New=%d  Feature=%.1f%%  Time2First=%.0fs\n"
        "Retention:  Churn=%.1f%%\n"
        "Task:       Completion=%.1f%%  Time=%.0fs  Error=%.1f%%\n",
        hm->satisfaction_score, hm->perceived_ease,
        hm->dau, hm->mau, hm->stickiness * 100.0, stickiness_label,
        hm->new_users_period, hm->feature_adoption_pct,
        hm->time_to_first_action_sec,
        hm->churn_rate_pct,
        hm->task_completion_rate, hm->task_time_sec, hm->error_rate_pct);
}

/* ================================================================
 * L4: Product-Market Fit (Sean Ellis Test)
 * Threshold: >= 40% "very disappointed" = PMF achieved
 * ================================================================ */

void pmf_init(ProductMarketFit *pmf) {
    if (pmf) memset(pmf, 0, sizeof(*pmf));
}

void pmf_add_response(ProductMarketFit *pmf, int disappointment_level) {
    if (!pmf) return;
    pmf->total_responses++;
    if (disappointment_level == 0) pmf->very_disappointed++;
    else if (disappointment_level == 1) pmf->somewhat_disappointed++;
    else pmf->not_disappointed++;
}

void pmf_evaluate(ProductMarketFit *pmf) {
    if (!pmf || pmf->total_responses == 0) return;
    pmf->pmf_score = 100.0 * (double)pmf->very_disappointed / (double)pmf->total_responses;
    pmf->pmf_achieved = (pmf->pmf_score >= 40.0);
}

const char *pmf_status(const ProductMarketFit *pmf) {
    if (!pmf || pmf->total_responses == 0) return "Not measured";
    return pmf->pmf_achieved ? "PMF Achieved!" : "PMF Not Yet Reached";
}

/* ================================================================
 * L5: Bass Diffusion Model (Frank Bass, 1969)
 * New adopters: S(t) = [p + q * F(t)] * [m - Y(t)]
 * Peak time: t* = ln(q/p) / (p + q)
 * ================================================================ */

void bass_init(BassDiffusion *bd, double p, double q, double m) {
    if (!bd) return;
    memset(bd, 0, sizeof(*bd));
    bd->innovation_coeff = p;
    bd->imitation_coeff = q;
    bd->market_size = m;
}

void bass_simulate(BassDiffusion *bd, int weeks) {
    if (!bd || weeks <= 0) return;
    int n = weeks < 52 ? weeks : 52;
    bd->weeks_projected = n;
    double cumulative = 0.0;
    double p = bd->innovation_coeff;
    double q = bd->imitation_coeff;
    double m = bd->market_size;
    for (int t = 0; t < n; t++) {
        double f_t = cumulative / m;
        double potential = m - cumulative;
        double hazard = p + q * f_t;
        double new_adopters = hazard * potential;
        if (new_adopters < 0.0) new_adopters = 0.0;
        bd->adopters[t] = new_adopters;
        cumulative += new_adopters;
    }
}

double bass_peak_time(const BassDiffusion *bd) {
    if (!bd) return -1.0;
    double p = bd->innovation_coeff;
    double q = bd->imitation_coeff;
    if (p <= 0.0 || q <= 0.0 || q <= p) return -1.0;
    return log(q / p) / (p + q);
}

double bass_peak_adoption(const BassDiffusion *bd) {
    if (!bd || bd->weeks_projected <= 0) return 0.0;
    double max_a = 0.0;
    for (int t = 0; t < bd->weeks_projected; t++) {
        if (bd->adopters[t] > max_a) max_a = bd->adopters[t];
    }
    return max_a;
}
