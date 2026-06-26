#include "product_economics.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ================================================================
 * L5: Pricing Strategy Implementations
 * ================================================================ */

void pricing_init_cost_plus(PricingModel *pm, double cost, double margin) {
    if (!pm) return;
    memset(pm, 0, sizeof(*pm));
    pm->strategy = PRICE_COST_PLUS;
    pm->cost_per_unit = cost;
    pm->target_margin_pct = margin;
}

void pricing_init_value_based(PricingModel *pm, double perceived_value) {
    if (!pm) return;
    memset(pm, 0, sizeof(*pm));
    pm->strategy = PRICE_VALUE_BASED;
    pm->perceived_value = perceived_value;
}

void pricing_init_competitive(PricingModel *pm, double competitor_price, double cost) {
    if (!pm) return;
    memset(pm, 0, sizeof(*pm));
    pm->strategy = PRICE_COMPETITIVE;
    pm->competitor_price = competitor_price;
    pm->cost_per_unit = cost;
}

void pricing_calculate(PricingModel *pm) {
    if (!pm) return;
    switch (pm->strategy) {
        case PRICE_COST_PLUS:
            pm->calculated_price = pm->cost_per_unit * (1.0 + pm->target_margin_pct / 100.0);
            break;
        case PRICE_VALUE_BASED:
            /* Price at 10-20% below perceived value for adoption */
            pm->calculated_price = pm->perceived_value * 0.85;
            break;
        case PRICE_COMPETITIVE:
            /* Match competitor unless cost prohibits */
            if (pm->competitor_price > pm->cost_per_unit * 1.2) {
                pm->calculated_price = pm->competitor_price * 0.95;
            } else {
                pm->calculated_price = pm->cost_per_unit * 1.3;
            }
            break;
        case PRICE_PENETRATION:
            pm->calculated_price = pm->cost_per_unit * 1.1;
            break;
        case PRICE_SKIMMING:
            pm->calculated_price = pm->cost_per_unit * 3.0;
            break;
        case PRICE_FREEMIUM:
            pm->calculated_price = 0.0;
            break;
        case PRICE_DYNAMIC:
            pm->calculated_price = pm->cost_per_unit * 1.5;
            break;
        default:
            pm->calculated_price = pm->cost_per_unit * 1.5;
    }
}

const char *pricing_strategy_name(PricingStrategy s) {
    switch (s) {
        case PRICE_COST_PLUS:    return "Cost-Plus";
        case PRICE_VALUE_BASED:  return "Value-Based";
        case PRICE_COMPETITIVE:  return "Competitive";
        case PRICE_PENETRATION:  return "Penetration";
        case PRICE_SKIMMING:     return "Skimming";
        case PRICE_FREEMIUM:     return "Freemium";
        case PRICE_DYNAMIC:      return "Dynamic";
        default:                 return "Unknown";
    }
}

/* ================================================================
 * L5: Unit Economics — SaaS Metrics
 * Reference: David Skok "SaaS Metrics 2.0"
 * ================================================================ */

void unit_economics_init(UnitEconomics *ue) {
    if (ue) memset(ue, 0, sizeof(*ue));
}

void unit_economics_calculate(UnitEconomics *ue) {
    if (!ue) return;
    if (ue->total_users > 0) {
        ue->conversion_to_paid_pct = 100.0 * (double)ue->paid_users / (double)ue->total_users;
    }
    ue->contribution_margin = (ue->arpu * ue->gross_margin_pct / 100.0) - ue->cac;
}

bool unit_economics_is_sustainable(const UnitEconomics *ue) {
    if (!ue || ue->cac <= 0.0) return false;
    double churn = ue->monthly_churn_pct > 0.0 ? ue->monthly_churn_pct / 100.0 : 0.05;
    double ltv = ue->arpu * (ue->gross_margin_pct / 100.0) / churn;
    double ltv_cac = ltv / ue->cac;
    return ltv_cac >= 3.0;
}

double unit_economics_cac_payback_months(const UnitEconomics *ue) {
    if (!ue || ue->arpu <= 0.0) return 999.0;
    double monthly_margin = ue->arpu * ue->gross_margin_pct / 100.0;
    if (monthly_margin <= 0.0) return 999.0;
    return ue->cac / monthly_margin;
}

/* ================================================================
 * L5: Breakeven Analysis
 * breakeven_units = fixed_costs / (price - variable_cost)
 * ================================================================ */

void breakeven_init(BreakevenAnalysis *ba, double fixed, double variable, double price) {
    if (!ba) return;
    memset(ba, 0, sizeof(*ba));
    ba->fixed_costs = fixed;
    ba->variable_cost_per_unit = variable;
    ba->price_per_unit = price;
}

void breakeven_calculate(BreakevenAnalysis *ba) {
    if (!ba) return;
    double contribution = ba->price_per_unit - ba->variable_cost_per_unit;
    if (contribution <= 0.0) {
        ba->breakeven_units = -1;
        ba->breakeven_revenue = -1.0;
        return;
    }
    ba->breakeven_units = (int)ceil(ba->fixed_costs / contribution);
    ba->breakeven_revenue = ba->breakeven_units * ba->price_per_unit;
}

int breakeven_units_for_profit(BreakevenAnalysis *ba, double target_profit) {
    if (!ba) return -1;
    double contribution = ba->price_per_unit - ba->variable_cost_per_unit;
    if (contribution <= 0.0) return -1;
    ba->target_profit = target_profit;
    ba->units_for_target = (int)ceil((ba->fixed_costs + target_profit) / contribution);
    return ba->units_for_target;
}

/* ================================================================
 * L5: Revenue Forecasting — Linear & Exponential Regression
 * Reference: Hyndman & Athanasopoulos "Forecasting: Principles and Practice"
 *
 * Linear: y = a + b*x    (simple linear regression)
 * Exponential: y = a * e^(b*x)  (log-linear regression)
 * ================================================================ */

void revenue_forecast_init(RevenueForecast *rf, ForecastMethod method) {
    if (!rf) return;
    memset(rf, 0, sizeof(*rf));
    rf->method = method;
}

void revenue_forecast_add_historical(RevenueForecast *rf, double revenue) {
    if (!rf || rf->history_count >= 24) return;
    rf->historical[rf->history_count++] = revenue;
}

static void forecast_linear_regression(const double *y, int n,
    double *out_slope, double *out_intercept, double *out_r2) {
    if (n < 2) { *out_slope = 0; *out_intercept = y ? y[0] : 0; *out_r2 = 0; return; }
    double sum_x = 0, sum_y = 0, sum_xy = 0, sum_x2 = 0, sum_y2 = 0;
    for (int i = 0; i < n; i++) {
        sum_x += i; sum_y += y[i];
        sum_xy += i * y[i]; sum_x2 += i * i; sum_y2 += y[i] * y[i];
    }
    double denom = n * sum_x2 - sum_x * sum_x;
    if (fabs(denom) < 1e-12) { *out_slope = 0; *out_intercept = sum_y / n; *out_r2 = 0; return; }
    double slope = (n * sum_xy - sum_x * sum_y) / denom;
    double intercept = (sum_y - slope * sum_x) / n;
    *out_slope = slope;
    *out_intercept = intercept;
    /* R-squared */
    double ss_res = 0, ss_tot = 0;
    double y_mean = sum_y / n;
    for (int i = 0; i < n; i++) {
        double pred = intercept + slope * i;
        ss_res += (y[i] - pred) * (y[i] - pred);
        ss_tot += (y[i] - y_mean) * (y[i] - y_mean);
    }
    *out_r2 = (ss_tot > 1e-12) ? 1.0 - ss_res / ss_tot : 0.0;
}

static void forecast_exp_regression(const double *y, int n,
    double *out_growth, double *out_initial, double *out_r2) {
    if (n < 2) { *out_growth = 0; *out_initial = y ? y[0] : 0; *out_r2 = 0; return; }
    double *log_y = (double *)malloc(n * sizeof(double));
    if (!log_y) return;
    int valid = 0;
    for (int i = 0; i < n; i++) {
        if (y[i] > 0) { log_y[valid++] = log(y[i]); }
    }
    double slope = 0, intercept = 0, r2 = 0;
    if (valid >= 2) {
        forecast_linear_regression(log_y, valid, &slope, &intercept, &r2);
    }
    free(log_y);
    *out_growth = slope;
    *out_initial = exp(intercept);
    *out_r2 = r2;
}

void revenue_forecast_predict(RevenueForecast *rf, int months) {
    if (!rf || months <= 0) return;
    int n = rf->history_count;
    if (n < 2) return;
    int fc = months < 12 ? months : 12;
    rf->forecast_count = fc;

    if (rf->method == FORECAST_LINEAR) {
        double slope, intercept, r2;
        forecast_linear_regression(rf->historical, n, &slope, &intercept, &r2);
        rf->growth_rate = slope;
        rf->r_squared = r2;
        for (int i = 0; i < fc; i++) {
            double pred = intercept + slope * (n + i);
            rf->forecast[i] = pred > 0 ? pred : 0;
        }
    } else if (rf->method == FORECAST_EXPONENTIAL) {
        double growth, initial, r2;
        forecast_exp_regression(rf->historical, n, &growth, &initial, &r2);
        rf->growth_rate = growth;
        rf->r_squared = r2;
        for (int i = 0; i < fc; i++) {
            rf->forecast[i] = initial * exp(growth * (n + i));
        }
    } else {
        /* Seasonal: simple moving average */
        double sum = 0; int window = n < 4 ? n : 4;
        for (int i = n - window; i < n; i++) sum += rf->historical[i];
        double avg = sum / window;
        for (int i = 0; i < fc; i++) rf->forecast[i] = avg;
    }
}

double revenue_forecast_total(const RevenueForecast *rf) {
    if (!rf) return 0.0;
    double total = 0.0;
    for (int i = 0; i < rf->forecast_count; i++) total += rf->forecast[i];
    return total;
}

double revenue_forecast_annual_run_rate(const RevenueForecast *rf) {
    if (!rf) return 0.0;
    if (rf->history_count > 0 && rf->forecast_count > 0) {
        return rf->forecast[0] * 12.0;
    }
    if (rf->history_count > 0) return rf->historical[rf->history_count - 1] * 12.0;
    return 0.0;
}

/* ================================================================
 * L5: Demand Curve & Price Optimization
 * Arc elasticity: E = (dQ/Q_avg) / (dP/P_avg)
 * ================================================================ */

void demand_curve_init(DemandCurve *dc) {
    if (dc) memset(dc, 0, sizeof(*dc));
}

void demand_curve_add_point(DemandCurve *dc, double price, int demand) {
    if (!dc || dc->point_count >= 20 || price < 0 || demand < 0) return;
    dc->price_points[dc->point_count] = price;
    dc->demand_at_points[dc->point_count] = demand;
    dc->point_count++;
}

void demand_curve_calculate_elasticity(DemandCurve *dc) {
    if (!dc || dc->point_count < 2) return;
    /* Use first and last points for arc elasticity */
    double p1 = dc->price_points[0];
    double q1 = (double)dc->demand_at_points[0];
    double p2 = dc->price_points[dc->point_count - 1];
    double q2 = (double)dc->demand_at_points[dc->point_count - 1];
    double dq = q2 - q1;
    double dp = p2 - p1;
    double q_avg = (q1 + q2) / 2.0;
    double p_avg = (p1 + p2) / 2.0;
    if (fabs(q_avg) < 1e-9 || fabs(p_avg) < 1e-9) { dc->elasticity = 0; return; }
    dc->elasticity = (dq / q_avg) / (dp / p_avg);
}

void demand_curve_find_optimal(DemandCurve *dc) {
    if (!dc || dc->point_count == 0) return;
    double best_rev = 0.0, best_price = 0.0;
    for (int i = 0; i < dc->point_count; i++) {
        double rev = dc->price_points[i] * (double)dc->demand_at_points[i];
        if (rev > best_rev) { best_rev = rev; best_price = dc->price_points[i]; }
    }
    dc->optimal_price = best_price;
    dc->max_revenue = best_rev;
}

double demand_curve_revenue_at(const DemandCurve *dc, double price) {
    if (!dc) return 0.0;
    /* Linear interpolation between nearest points */
    if (dc->point_count < 1) return 0.0;
    for (int i = 1; i < dc->point_count; i++) {
        if (price <= dc->price_points[i] && price >= dc->price_points[i-1]) {
            double ratio = (price - dc->price_points[i-1]) / (dc->price_points[i] - dc->price_points[i-1]);
            double demand = dc->demand_at_points[i-1] + ratio * (dc->demand_at_points[i] - dc->demand_at_points[i-1]);
            return price * demand;
        }
    }
    /* Extrapolate from last point */
    return price * (double)dc->demand_at_points[dc->point_count - 1];
}

/* ================================================================
 * L4: Pareto Principle (80/20 Rule)
 * Reference: Vilfredo Pareto "Cours d'Economie Politique" (1896)
 * J.M. Juran "Quality Control Handbook" (1951)
 * ================================================================ */

static int compare_double_desc(const void *a, const void *b) {
    double da = *(const double *)a;
    double db = *(const double *)b;
    if (da < db) return 1;
    if (da > db) return -1;
    return 0;
}

void pareto_analyze(ParetoAnalysis *pa, const double *values, int count) {
    if (!pa || !values || count <= 0) return;
    memset(pa, 0, sizeof(*pa));

    double *sorted = (double *)malloc(count * sizeof(double));
    if (!sorted) return;
    memcpy(sorted, values, count * sizeof(double));
    qsort(sorted, count, sizeof(double), compare_double_desc);

    pa->total_value = 0.0;
    for (int i = 0; i < count; i++) pa->total_value += sorted[i];

    int top20 = count / 5;
    if (top20 < 1) top20 = 1;
    pa->top20_value = 0.0;
    for (int i = 0; i < top20; i++) pa->top20_value += sorted[i];

    if (pa->total_value > 1e-9) {
        pa->top20_pct_of_total = 100.0 * pa->top20_value / pa->total_value;
    }
    pa->pareto_index = pa->top20_pct_of_total / 80.0; /* 1.0 = perfect Pareto */
    free(sorted);
}

bool pareto_is_strong(const ParetoAnalysis *pa) {
    if (!pa) return false;
    return pa->top20_pct_of_total >= 75.0;
}

/* ================================================================
 * L4: Metcalfe's Law — Network Effects
 * Reference: Robert Metcalfe (1980) — V proportional to n^2
 *            Briscoe, Odlyzko, Tilly (2006) — V proportional to n log n
 * ================================================================ */

void network_effect_calculate(NetworkEffect *ne, int users, double val_per_conn) {
    if (!ne) return;
    memset(ne, 0, sizeof(*ne));
    ne->num_users = users;
    ne->value_per_connection = val_per_conn;
    double n = (double)users;
    ne->metcalfe_value = val_per_conn * n * (n - 1.0) / 2.0;
    ne->briscoe_value = val_per_conn * n * log(n > 1.0 ? n : 2.71828);
    ne->network_value = ne->metcalfe_value;
}

double network_effect_marginal_value(const NetworkEffect *ne, int additional_users) {
    if (!ne || additional_users <= 0) return 0.0;
    double n = (double)ne->num_users;
    double n_new = n + (double)additional_users;
    /* Metcalfe: marginal = v * [n_new*(n_new-1) - n*(n-1)] / 2 */
    double old_val = n * (n - 1.0) / 2.0;
    double new_val = n_new * (n_new - 1.0) / 2.0;
    return ne->value_per_connection * (new_val - old_val);
}
