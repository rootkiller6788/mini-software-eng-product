#include "gov_velocity.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

/* ============================================================
 * L2: Velocity & Forecasting Implementation
 * L4: Cone of Uncertainty, Parkinson's Law, Brooks's Law
 * L5: Rolling average, Monte Carlo simulation, burndown
 * ============================================================ */

void velocity_init(VelocityProfile *vp, const char *team_name, int rolling_window) {
    memset(vp, 0, sizeof(*vp));
    strncpy(vp->team_name, team_name, VELOCITY_NAME_LEN - 1);
    vp->team_name[VELOCITY_NAME_LEN - 1] = '\0';
    vp->rolling_window = rolling_window > 0 ? rolling_window : 3;
    vp->best_sprint = 0;
    vp->worst_sprint = 1e9;
    vp->improving = false;
    vp->trend_slope = 0;
}

int velocity_record(VelocityProfile *vp, int sprint_num, int planned_points,
                    int completed_points, int planned_items, int completed_items,
                    double capacity_days, double days_lost, int carry_over) {
    if (vp->record_count >= VELOCITY_MAX_SPRINTS) return -1;
    VelocityRecord *vr = &vp->records[vp->record_count];
    vr->sprint_number = sprint_num;
    vr->planned_points = planned_points;
    vr->completed_points = completed_points;
    vr->planned_items = planned_items;
    vr->completed_items = completed_items;
    vr->focus_factor = planned_points > 0 ? (double)completed_points / planned_points : 1.0;
    vr->carry_over = carry_over;
    vr->added_mid_sprint = 0;
    vr->team_capacity_days = capacity_days;
    vr->days_lost = days_lost;

    /* Update rolling stats */
    int w = vp->rolling_window;
    if (vp->record_count >= w) {
        double sum = 0;
        for (int i = vp->record_count - w + 1; i <= vp->record_count; i++) {
            sum += vp->records[i].completed_points;
        }
        vp->rolling_avg[vp->record_count] = sum / w;
        double sum_sq = 0;
        for (int i = vp->record_count - w + 1; i <= vp->record_count; i++) {
            double d = vp->records[i].completed_points - vp->rolling_avg[vp->record_count];
            sum_sq += d * d;
        }
        vp->rolling_std[vp->record_count] = sqrt(sum_sq / w);
    } else {
        double sum = 0;
        for (int i = 0; i <= vp->record_count; i++) sum += vp->records[i].completed_points;
        vp->rolling_avg[vp->record_count] = sum / (vp->record_count + 1);
        vp->rolling_std[vp->record_count] = 0;
    }

    /* All-time stats */
    double all_sum = 0;
    for (int i = 0; i <= vp->record_count; i++) all_sum += vp->records[i].completed_points;
    vp->all_time_avg = all_sum / (vp->record_count + 1);
    if (completed_points > vp->best_sprint) vp->best_sprint = completed_points;
    if (completed_points < vp->worst_sprint) vp->worst_sprint = completed_points;

    /* All-time std */
    double sq_sum = 0;
    for (int i = 0; i <= vp->record_count; i++) {
        double d = vp->records[i].completed_points - vp->all_time_avg;
        sq_sum += d * d;
    }
    vp->all_time_std = vp->record_count > 0 ? sqrt(sq_sum / vp->record_count) : 0;

    /* Compute median */
    int n = vp->record_count + 1;
    int *pts = (int *)malloc(n * sizeof(int));
    if (pts) {
        for (int i = 0; i < n; i++) pts[i] = vp->records[i].completed_points;
        /* Simple bubble sort for median */
        for (int i = 0; i < n - 1; i++)
            for (int j = 0; j < n - i - 1; j++)
                if (pts[j] > pts[j + 1]) { int t = pts[j]; pts[j] = pts[j + 1]; pts[j + 1] = t; }
        if (n % 2 == 0) vp->all_time_median = (pts[n/2 - 1] + pts[n/2]) / 2.0;
        else vp->all_time_median = pts[n/2];
        free(pts);
    }

    /* Velocity trend */
    velocity_calc_trend(vp);
    return vp->record_count++;
}

double velocity_rolling_avg(VelocityProfile *vp) {
    if (vp->record_count == 0) return 0;
    return vp->rolling_avg[vp->record_count - 1];
}

double velocity_rolling_std(VelocityProfile *vp) {
    if (vp->record_count == 0) return 0;
    return vp->rolling_std[vp->record_count - 1];
}

double velocity_median(VelocityProfile *vp) {
    return vp->all_time_median;
}

/* L5: Linear regression for velocity trend */
void velocity_calc_trend(VelocityProfile *vp) {
    int n = vp->record_count + 1;
    if (n < 3) return;
    double sum_x = 0, sum_y = 0, sum_xy = 0, sum_xx = 0;
    for (int i = 0; i < n; i++) {
        sum_x += i;
        sum_y += vp->records[i].completed_points;
        sum_xy += i * vp->records[i].completed_points;
        sum_xx += i * i;
    }
    double denom = n * sum_xx - sum_x * sum_x;
    if (fabs(denom) > 1e-10) {
        vp->trend_slope = (n * sum_xy - sum_x * sum_y) / denom;
        vp->improving = vp->trend_slope > 0;
    }
}

double velocity_volatility(VelocityProfile *vp) {
    if (vp->all_time_avg <= 0) return 0;
    return vp->all_time_std / vp->all_time_avg;
}

bool velocity_is_stable(VelocityProfile *vp, double threshold) {
    return velocity_volatility(vp) <= threshold;
}

/* L5: Burndown calculations */
void burndown_calc_ideal(double total_points, int sprint_days, double *output) {
    for (int d = 0; d < sprint_days && d < 30; d++) {
        output[d] = total_points * (1.0 - (double)d / sprint_days);
    }
}

void burndown_calc_actual(double *actuals, int days, double *remaining) {
    for (int d = 0; d < days; d++) remaining[d] = actuals[d];
}

bool burndown_on_track(double *ideal, double *actual, int current_day, double tolerance) {
    if (current_day < 0 || ideal[current_day] <= 0) return true;
    double diff_pct = (ideal[current_day] - actual[current_day]) / ideal[current_day];
    return diff_pct <= tolerance;
}

double burndown_projected_completion(double *actual, int days, double total) {
    if (days < 2) return -1;
    if (total <= 0) return -1;
    /* Fit linear slope through last 3 actual points */
    int n = days < 3 ? days : 3;
    double sum_x = 0, sum_y = 0, sum_xy = 0, sum_xx = 0;
    for (int i = days - n; i < days; i++) {
        sum_x += i; sum_y += actual[i];
        sum_xy += i * actual[i]; sum_xx += i * i;
    }
    double denom = n * sum_xx - sum_x * sum_x;
    if (fabs(denom) < 1e-10) return total / (actual[days - 1] > 0 ? actual[days - 1] : 1) * days;
    double slope = (n * sum_xy - sum_x * sum_y) / denom;
    if (slope >= 0) return -1; /* Not decreasing */
    /* Days until remaining reaches 0 */
    double projected_day = -actual[days - 1] / slope;
    return projected_day > 0 ? projected_day : total;
}

/* ---- Forecasting ---- */
void forecast_init(Forecast *f, ForecastMethod method) {
    memset(f, 0, sizeof(*f));
    f->method = method;
}

void forecast_rolling(VelocityProfile *vp, Forecast *f, int remaining_points) {
    double avg = velocity_rolling_avg(vp);
    double std = velocity_rolling_std(vp);
    if (avg <= 0) avg = 10;
    f->predicted_points = avg;
    f->confidence_50 = remaining_points / avg;
    f->confidence_80 = remaining_points / (avg - 0.84 * std > 0 ? avg - 0.84 * std : 1);
    f->confidence_95 = remaining_points / (avg - 1.645 * std > 0 ? avg - 1.645 * std : 1);
    double coeffs[] = {0.0, 0.524, 0.842, 1.282, 1.645};
    for (int i = 0; i < 5; i++) {
        double adj = avg - coeffs[i] * std;
        f->sprints_to_complete[i] = adj > 0 ? (int)ceil(remaining_points / adj) : 999;
    }
}

void forecast_weighted(VelocityProfile *vp, Forecast *f, int remaining_points, double decay) {
    if (vp->record_count == 0) { forecast_rolling(vp, f, remaining_points); return; }
    double weighted_sum = 0, weight_total = 0;
    double w = 1.0;
    for (int i = vp->record_count - 1; i >= 0 && w > 0.01; i--) {
        weighted_sum += vp->records[i].completed_points * w;
        weight_total += w;
        w *= decay;
    }
    double weighted_avg = weight_total > 0 ? weighted_sum / weight_total : vp->all_time_avg;
    f->predicted_points = weighted_avg;
    f->confidence_50 = remaining_points / (weighted_avg > 0 ? weighted_avg : 1);
    for (int i = 0; i < 5; i++) {
        f->sprints_to_complete[i] = weighted_avg > 0 ? (int)ceil(remaining_points / weighted_avg) : 999;
    }
}

/* L5: Monte Carlo simulation for sprint forecasting
 * Simulate many possible futures by randomly sampling historical throughput
 */
void forecast_monte_carlo(VelocityProfile *vp, Forecast *f, int remaining_points, MonteCarloConfig *mc) {
    if (!mc || vp->record_count == 0 || remaining_points <= 0) {
        forecast_rolling(vp, f, remaining_points);
        return;
    }
    int sims = mc->num_simulations > 0 ? mc->num_simulations : 10000;
    int max_sp = mc->num_sprints > 0 ? mc->num_sprints : 50;
    int *results = (int *)calloc(sims, sizeof(int));
    if (!results) { forecast_rolling(vp, f, remaining_points); return; }

    /* Seed random */
    srand((unsigned int)time(NULL));

    for (int s = 0; s < sims; s++) {
        int remaining = remaining_points;
        int sprints = 0;
        while (remaining > 0 && sprints < max_sp) {
            /* Randomly sample from historical data */
            int idx = rand() % vp->record_count;
            int throughput = vp->records[idx].completed_points;
            if (throughput <= 0) throughput = 1;
            remaining -= throughput;
            sprints++;
        }
        results[s] = sprints;
    }

    /* Sort results to get percentiles */
    { /* Simple bubble sort for int array (avoid nested function) */
        for (int i = 0; i < sims - 1; i++)
            for (int j = 0; j < sims - i - 1; j++)
                if (results[j] > results[j + 1]) {
                    int t = results[j]; results[j] = results[j + 1]; results[j + 1] = t;
                }
    }

    f->confidence_50 = results[sims * 50 / 100];
    f->confidence_80 = results[sims * 80 / 100];
    f->confidence_95 = results[sims * 95 / 100];
    f->predicted_points = velocity_rolling_avg(vp);

    free(results);
}

void forecast_percentile(VelocityProfile *vp, Forecast *f, int remaining_points) {
    if (vp->record_count == 0) { forecast_rolling(vp, f, remaining_points); return; }
    /* Build sorted array of velocities */
    int n = vp->record_count;
    int *vels = (int *)malloc(n * sizeof(int));
    if (!vels) { forecast_rolling(vp, f, remaining_points); return; }
    for (int i = 0; i < n; i++) vels[i] = vp->records[i].completed_points;
    /* Sort ascending */
    for (int i = 0; i < n - 1; i++)
        for (int j = 0; j < n - i - 1; j++)
            if (vels[j] > vels[j + 1]) { int t = vels[j]; vels[j] = vels[j + 1]; vels[j + 1] = t; }

    int p50_idx = (int)(n * 0.5);
    int p80_idx = (int)(n * 0.2);  /* For "how many sprints at 80th percentile" we use lower bound */
    int p95_idx = (int)(n * 0.05);
    if (p50_idx >= n) p50_idx = n - 1;
    if (p80_idx >= n) p80_idx = n - 1;
    if (p95_idx >= n) p95_idx = n - 1;
    if (p50_idx < 0) p50_idx = 0;
    if (p80_idx < 0) p80_idx = 0;
    if (p95_idx < 0) p95_idx = 0;

    double v50 = vels[p50_idx] > 0 ? vels[p50_idx] : 1;
    double v80 = vels[p80_idx] > 0 ? vels[p80_idx] : 1;
    double v95 = vels[p95_idx] > 0 ? vels[p95_idx] : 1;

    f->predicted_points = velocity_rolling_avg(vp);
    f->confidence_50 = ceil(remaining_points / v50);
    f->confidence_80 = ceil(remaining_points / v80);
    f->confidence_95 = ceil(remaining_points / v95);

    free(vels);
}

/* L4: Cone of Uncertainty — estimation accuracy improves over project lifecycle
 * At inception: 4x, at requirements: 2x, at design: 1.5x, at implementation: 1.25x
 * Returns multiplier for estimation range
 */
double cone_of_uncertainty(double phase_pct) {
    if (phase_pct < 0) phase_pct = 0;
    if (phase_pct > 1) phase_pct = 1;
    /* Exponential decay from 4x to 1x */
    return 4.0 * exp(-2.0 * phase_pct) + 0.8;
}

/* L4: Parkinson's Law — work expands to fill available time
 * expansion = actual_days / estimate
 */
double parkinsons_law_expansion(double estimate, double actual_days) {
    if (estimate <= 0) return 1.0;
    return actual_days / estimate;
}

/* L4: Brooks's Law — adding manpower to a late project makes it later
 * Additional delay = ramp_up_time * new_members * (team_size + new_members) / team_size
 */
double brooks_law_delay(int team_size, int new_members, double original_duration) {
    if (team_size <= 0 || new_members <= 0) return original_duration;
    double ramp_up_factor = 0.2; /* 20% of original duration for ramp-up */
    double communication_overhead = (double)(team_size + new_members) / team_size;
    return original_duration * (1.0 + ramp_up_factor * new_members * communication_overhead);
}

void velocity_print_profile(VelocityProfile *vp) {
    printf("=== Velocity Profile: %s ===\n", vp->team_name);
    printf("  Sprints: %d | All-time Avg: %.1f | Median: %.1f | Std: %.1f\n",
           vp->record_count, vp->all_time_avg, vp->all_time_median, vp->all_time_std);
    printf("  Best: %.0f | Worst: %.0f | Volatility: %.2f | Stable: %s\n",
           vp->best_sprint, vp->worst_sprint, velocity_volatility(vp),
           velocity_is_stable(vp, 0.3) ? "YES" : "NO");
    printf("  Trend: %s (slope=%.2f)\n", vp->improving ? "IMPROVING" : "DECLINING", vp->trend_slope);
    for (int i = 0; i < vp->record_count; i++) {
        printf("  Sprint %2d: planned=%3d completed=%3d focus=%.2f\n",
               vp->records[i].sprint_number, vp->records[i].planned_points,
               vp->records[i].completed_points, vp->records[i].focus_factor);
    }
}

void velocity_print_forecast(Forecast *f) {
    printf("=== Forecast (Method: %d) ===\n", f->method);
    printf("  Predicted next sprint: %.1f pts\n", f->predicted_points);
    printf("  Sprints to complete: P50=%.0f P80=%.0f P95=%.0f\n",
           f->confidence_50, f->confidence_80, f->confidence_95);
}

void burndown_print_chart(double *ideal, double *actual, int days) {
    printf("=== Burndown Chart ===\n");
    printf("  Day | Ideal  | Actual\n");
    for (int d = 0; d < days; d++) {
        printf("  %3d | %6.1f | %6.1f\n", d, ideal[d], actual[d]);
    }
}