#ifndef GOV_VELOCITY_H
#define GOV_VELOCITY_H
#include <stdint.h>
#include <stdbool.h>

/* ============================================================
 * L1: Velocity & Forecasting — Core Definitions
 * L4: Cone of Uncertainty, Parkinson's Law, Brooks's Law
 * L5: Forecasting Algorithms: Rolling Average, Monte Carlo
 * Reference: Agile Estimating and Planning (Cohn)
 * ============================================================ */

#define VELOCITY_MAX_SPRINTS    32
#define VELOCITY_MAX_TEAMS      4
#define VELOCITY_NAME_LEN       64

/* ---- Velocity Record ---- */
typedef struct {
    int sprint_number;
    int planned_points;
    int completed_points;
    int planned_items;
    int completed_items;
    double focus_factor;       /* completed / planned */
    int carry_over;            /* incomplete items */
    int added_mid_sprint;      /* scope creep */
    double team_capacity_days;
    double days_lost;          /* holidays, incidents */
} VelocityRecord;

/* ---- Team Velocity Profile ---- */
typedef struct {
    char team_name[VELOCITY_NAME_LEN];
    VelocityRecord records[VELOCITY_MAX_SPRINTS];
    int record_count;
    /* Rolling statistics */
    double rolling_avg[VELOCITY_MAX_SPRINTS];  /* 3-sprint rolling avg */
    double rolling_std[VELOCITY_MAX_SPRINTS];
    int rolling_window;
    double all_time_avg;
    double all_time_median;
    double all_time_std;
    double best_sprint;
    double worst_sprint;
    /* Trend */
    double trend_slope;        /* velocity trend direction */
    bool improving;
} VelocityProfile;

/* ---- Forecast Methods (L5) ---- */
typedef enum {
    FORECAST_ROLLING_AVG = 0,  /* simple rolling average */
    FORECAST_WEIGHTED,         /* exponentially weighted */
    FORECAST_MONTE_CARLO,      /* Monte Carlo simulation */
    FORECAST_PERCENTILE        /* historical percentile */
} ForecastMethod;

typedef struct {
    ForecastMethod method;
    double predicted_points;       /* next sprint prediction */
    double confidence_50;          /* P50 estimate */
    double confidence_80;          /* P80 estimate */
    double confidence_95;          /* P95 estimate */
    int sprints_to_complete[5];    /* at P50,P70,P80,P90,P95 */
    double completion_date[5];     /* estimated dates */
    int total_remaining_points;
} Forecast;

/* ---- Monte Carlo Simulation Config ---- */
typedef struct {
    int num_simulations;       /* default 10000 */
    int num_sprints;           /* max sprints to simulate */
    int *historical_throughput;
    int history_len;
} MonteCarloConfig;

/* ---- API ---- */
void velocity_init(VelocityProfile *vp, const char *team_name, int rolling_window);
int  velocity_record(VelocityProfile *vp, int sprint_num, int planned_points,
                     int completed_points, int planned_items, int completed_items,
                     double capacity_days, double days_lost, int carry_over);

/* Statistics */
double velocity_rolling_avg(VelocityProfile *vp);
double velocity_rolling_std(VelocityProfile *vp);
double velocity_median(VelocityProfile *vp);
void  velocity_calc_trend(VelocityProfile *vp);
double velocity_volatility(VelocityProfile *vp);  /* coefficient of variation */
bool velocity_is_stable(VelocityProfile *vp, double threshold);

/* L5: Burndown Analysis */
void burndown_calc_ideal(double total_points, int sprint_days, double *output);
void burndown_calc_actual(double *actuals, int days, double *remaining);
bool burndown_on_track(double *ideal, double *actual, int current_day, double tolerance);
double burndown_projected_completion(double *actual, int days, double total);

/* L5: Forecasting */
void forecast_init(Forecast *f, ForecastMethod method);
void forecast_rolling(VelocityProfile *vp, Forecast *f, int remaining_points);
void forecast_weighted(VelocityProfile *vp, Forecast *f, int remaining_points, double decay);
void forecast_monte_carlo(VelocityProfile *vp, Forecast *f, int remaining_points, MonteCarloConfig *mc);
void forecast_percentile(VelocityProfile *vp, Forecast *f, int remaining_points);

/* L4: Cone of Uncertainty — estimation accuracy over time */
double cone_of_uncertainty(double phase_pct);
double parkinsons_law_expansion(double estimate, double actual_days);
double brooks_law_delay(int team_size, int new_members, double original_duration);

/* Print */
void velocity_print_profile(VelocityProfile *vp);
void velocity_print_forecast(Forecast *f);
void burndown_print_chart(double *ideal, double *actual, int days);

#endif
