#ifndef PRODUCT_GROWTH_H
#define PRODUCT_GROWTH_H
#include <stdbool.h>
#include <stddef.h>

/* ================================================================
 * L1: Core Definitions — Product Growth Models
 * ================================================================
 * Reference: Eric Ries "The Lean Startup" (2011)
 *            Sean Ellis "Hacking Growth" (2017)
 *            David Skok "For Entrepreneurs" — SaaS Metrics
 */

/* --- Viral Growth Model (K-Factor) --- */
typedef struct {
    double invites_per_user;     /* i: avg invitations sent per user */
    double conversion_rate;      /* % of invitees who convert */
    double cycle_time_days;      /* days per viral cycle */
    double k_factor;             /* i × conv_rate, >1 = viral growth */
    int initial_users;
    int total_users_after_cycles;
    int cycles_simulated;
} ViralModel;

/* --- Customer Lifetime Value (LTV) Models --- */
typedef enum { LTV_SIMPLE, LTV_COHORT, LTV_DISCOUNTED } LtvModelType;

typedef struct {
    double monthly_arpu;         /* Avg Revenue Per User / month */
    double gross_margin_pct;     /* 0-100 */
    double monthly_churn_pct;    /* 0-100 */
    double ltv;                  /* Computed: ARPU × margin / churn */
    double ltv_cac_ratio;        /* LTV / CAC, healthy ≥ 3 */
    double cac;                  /* Customer Acquisition Cost */
    int    payback_months;       /* months to recover CAC */
} CustomerLTV;

/* --- Cohort-based LTV Projection --- */
#define MAX_COHORT_MONTHS 36

typedef struct {
    int    cohort_size;
    double retention[MAX_COHORT_MONTHS];  /* % retained each month */
    double revenue_per_user[MAX_COHORT_MONTHS];
    double cumulative_ltv;     /* sum of discounted revenue */
    double discount_rate;      /* monthly discount rate */
    int    months_projected;
} CohortLTVProjection;

/* --- Net Revenue Retention (NRR) --- */
typedef struct {
    double starting_mrr;        /* Monthly Recurring Revenue at start */
    double expansion_mrr;       /* upsell/cross-sell */
    double contraction_mrr;     /* downgrades */
    double churned_mrr;         /* cancellations */
    double ending_mrr;
    double nrr_pct;             /* >100% = net negative churn */
} NetRevenueRetention;

/* --- AARRR Pirate Metrics (Dave McClure, 2007) --- */
typedef struct {
    /* Acquisition */
    int    visitors;            double visitor_to_lead_pct;
    int    leads;               double lead_to_trial_pct;
    int    trials;
    /* Activation */
    int    activated;           double activation_rate_pct;
    double time_to_aha_sec;     /* time to "aha moment" */
    /* Retention */
    double day1_retention_pct;  double day7_retention_pct;
    double day30_retention_pct; double week12_retention_pct;
    /* Revenue */
    double arpu;                double ltv;
    double mrr;                 double arr;
    /* Referral */
    double nps;                 int    referrals_sent;
    double referral_conversion_pct; double viral_coefficient;
} AARRRMetrics;

/* --- HEART Framework (Google UX, 2010) --- */
typedef struct {
    /* Happiness */
    double satisfaction_score;  /* CSAT or NPS */
    double perceived_ease;      /* 1-7 scale */
    /* Engagement */
    double dau;                 double mau;
    double stickiness;          /* DAU/MAU */
    double sessions_per_user_day;
    /* Adoption */
    int    new_users_period;    double feature_adoption_pct;
    double time_to_first_action_sec;
    /* Retention */
    double cohort_retention[12]; /* monthly */
    double churn_rate_pct;
    /* Task Success */
    double task_completion_rate; double task_time_sec;
    double error_rate_pct;
} HEARTMetrics;

/* --- Product-Market Fit (Sean Ellis Test) --- */
typedef struct {
    int    total_responses;
    int    very_disappointed;
    int    somewhat_disappointed;
    int    not_disappointed;
    double pmf_score;           /* % "very disappointed" if product gone */
    bool   pmf_achieved;        /* ≥ 40% threshold */
} ProductMarketFit;

/* --- API Declarations --- */

/* Viral Model (L5: K-factor algorithm) */
void viral_model_init(ViralModel *vm, double invites, double conv, double cycle_days, int initial);
void viral_model_simulate(ViralModel *vm, int max_cycles);
double viral_model_growth_rate(const ViralModel *vm);
bool viral_model_is_viral(const ViralModel *vm);

/* LTV (L4: SaaS metrics theorem — LTV:CAC ≥ 3) */
void customer_ltv_init(CustomerLTV *clv, double arpu, double margin, double churn, double cac);
void customer_ltv_calculate(CustomerLTV *clv);
bool customer_ltv_is_healthy(const CustomerLTV *clv);
double customer_ltv_payback_months(const CustomerLTV *clv);

/* Cohort LTV Projection (L5: Discounted cash flow) */
void cohort_ltv_init(CohortLTVProjection *cl, int size, double discount);
void cohort_ltv_record(CohortLTVProjection *cl, int month, double retention, double rev_per_user);
void cohort_ltv_project(CohortLTVProjection *cl, int months);
double cohort_ltv_total(const CohortLTVProjection *cl);

/* NRR (L5: Net Revenue Retention algorithm) */
void nrr_init(NetRevenueRetention *nr, double starting_mrr);
void nrr_add_expansion(NetRevenueRetention *nr, double amount);
void nrr_add_contraction(NetRevenueRetention *nr, double amount);
void nrr_add_churn(NetRevenueRetention *nr, double amount);
void nrr_calculate(NetRevenueRetention *nr);
const char *nrr_health_assessment(const NetRevenueRetention *nr);

/* AARRR Metrics (L5: Pirate Metrics framework) */
void aarrr_init(AARRRMetrics *am);
void aarrr_set_acquisition(AARRRMetrics *am, int visitors, int leads, int trials);
void aarrr_set_activation(AARRRMetrics *am, int activated, double time_to_aha);
void aarrr_set_retention(AARRRMetrics *am, double d1, double d7, double d30, double w12);
void aarrr_set_revenue(AARRRMetrics *am, double arpu, double mrr);
void aarrr_calculate_conversions(AARRRMetrics *am);
void aarrr_health_report(const AARRRMetrics *am, char *buffer, size_t sz);

/* HEART (L5: Google UX metrics framework) */
void heart_init(HEARTMetrics *hm);
void heart_set_happiness(HEARTMetrics *hm, double sat, double ease);
void heart_set_engagement(HEARTMetrics *hm, double dau, double mau, double sessions);
void heart_calculate_stickiness(HEARTMetrics *hm);
void heart_overall_health(const HEARTMetrics *hm, char *buffer, size_t sz);

/* PMF (L4: Sean Ellis test — 40% threshold) */
void pmf_init(ProductMarketFit *pmf);
void pmf_add_response(ProductMarketFit *pmf, int disappointment_level); /* 0-2 */
void pmf_evaluate(ProductMarketFit *pmf);
const char *pmf_status(const ProductMarketFit *pmf);

/* L5: Bass Diffusion Model (Frank Bass, 1969) */
typedef struct {
    double innovation_coeff;  /* p: external influence (advertising) */
    double imitation_coeff;   /* q: internal influence (word-of-mouth) */
    double market_size;       /* m: total addressable market */
    double adopters[52];      /* weekly adoption */
    int    weeks_projected;
} BassDiffusion;

void bass_init(BassDiffusion *bd, double p, double q, double m);
void bass_simulate(BassDiffusion *bd, int weeks);
double bass_peak_time(const BassDiffusion *bd);
double bass_peak_adoption(const BassDiffusion *bd);

#endif /* PRODUCT_GROWTH_H */
