#ifndef PRODUCT_ECONOMICS_H
#define PRODUCT_ECONOMICS_H
#include <stdbool.h>

/* ================================================================
 * L1: Core Definitions — Product Economics & Pricing
 * ================================================================
 * Reference: Thomas Nagle "The Strategy and Tactics of Pricing"
 *            McKinsey "Pricing Maturity Model"
 */

typedef enum {
    PRICE_COST_PLUS,
    PRICE_VALUE_BASED,
    PRICE_COMPETITIVE,
    PRICE_PENETRATION,
    PRICE_SKIMMING,
    PRICE_FREEMIUM,
    PRICE_DYNAMIC
} PricingStrategy;

typedef struct {
    PricingStrategy strategy;
    double cost_per_unit;
    double target_margin_pct;
    double perceived_value;
    double competitor_price;
    double calculated_price;
    double price_elasticity;
    int    demand_at_price;
    int    base_demand;
    double base_price;
} PricingModel;

typedef struct {
    double cac;
    double cpc;
    double cpm;
    double cpl;
    double arpu;
    double arppu;
    double monthly_churn_pct;
    int    paid_users;
    int    total_users;
    double conversion_to_paid_pct;
    double gross_margin_pct;
    double contribution_margin;
} UnitEconomics;

typedef struct {
    double fixed_costs;
    double variable_cost_per_unit;
    double price_per_unit;
    int    breakeven_units;
    double breakeven_revenue;
    double target_profit;
    int    units_for_target;
} BreakevenAnalysis;

typedef enum { FORECAST_LINEAR, FORECAST_EXPONENTIAL, FORECAST_SEASONAL } ForecastMethod;

typedef struct {
    double historical[24];
    int    history_count;
    double forecast[12];
    int    forecast_count;
    ForecastMethod method;
    double r_squared;
    double growth_rate;
} RevenueForecast;

typedef struct {
    double price_points[20];
    int    demand_at_points[20];
    int    point_count;
    double elasticity;
    double optimal_price;
    double max_revenue;
} DemandCurve;

void pricing_init_cost_plus(PricingModel *pm, double cost, double margin);
void pricing_init_value_based(PricingModel *pm, double perceived_value);
void pricing_init_competitive(PricingModel *pm, double competitor_price, double cost);
void pricing_calculate(PricingModel *pm);
const char *pricing_strategy_name(PricingStrategy s);

void unit_economics_init(UnitEconomics *ue);
void unit_economics_calculate(UnitEconomics *ue);
bool unit_economics_is_sustainable(const UnitEconomics *ue);
double unit_economics_cac_payback_months(const UnitEconomics *ue);

void breakeven_init(BreakevenAnalysis *ba, double fixed, double variable, double price);
void breakeven_calculate(BreakevenAnalysis *ba);
int breakeven_units_for_profit(BreakevenAnalysis *ba, double target_profit);

void revenue_forecast_init(RevenueForecast *rf, ForecastMethod method);
void revenue_forecast_add_historical(RevenueForecast *rf, double revenue);
void revenue_forecast_predict(RevenueForecast *rf, int months);
double revenue_forecast_total(const RevenueForecast *rf);
double revenue_forecast_annual_run_rate(const RevenueForecast *rf);

void demand_curve_init(DemandCurve *dc);
void demand_curve_add_point(DemandCurve *dc, double price, int demand);
void demand_curve_calculate_elasticity(DemandCurve *dc);
void demand_curve_find_optimal(DemandCurve *dc);
double demand_curve_revenue_at(const DemandCurve *dc, double price);

/* L4: Pareto Principle (80/20 rule) */
typedef struct {
    double total_value;
    double top20_value;
    double top20_pct_of_total;
    double pareto_index;
} ParetoAnalysis;

void pareto_analyze(ParetoAnalysis *pa, const double *values, int count);
bool pareto_is_strong(const ParetoAnalysis *pa);

/* L4: Metcalfe's Law — Network Value proportional to n^2 */
typedef struct {
    int    num_users;
    double value_per_connection;
    double network_value;
    double metcalfe_value;
    double briscoe_value;
} NetworkEffect;

void network_effect_calculate(NetworkEffect *ne, int users, double val_per_conn);
double network_effect_marginal_value(const NetworkEffect *ne, int additional_users);

#endif /* PRODUCT_ECONOMICS_H */
