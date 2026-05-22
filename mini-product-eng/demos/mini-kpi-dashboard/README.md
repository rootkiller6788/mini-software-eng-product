# mini-kpi-dashboard — KPI Dashboard

## Goal

Build a product KPI dashboard with trend analysis, threshold alerts, and standard SaaS metrics (NPS, retention, churn, MAU).

## Steps

1. Initialize dashboard and add 5+ KPIs with targets and warning/critical thresholds
2. Feed weekly data and compute trends with linear regression
3. Calculate standard product metrics:
   - NPS = (promoters - detractors) / total * 100
   - Retention D7/D30 = active / initial * 100
   - Churn = (start - end + new) / avg * 100
4. Check on-track status per KPI
5. Print dashboard showing all metrics and trends

## Key APIs

- `kpi_add()` — Define KPI with target and thresholds
- `kpi_update()` — Record a new data point
- `kpi_trend()` — Linear regression slope over time
- `kpi_on_track()` — Check against target and thresholds
- `kpi_calc_nps()` / `kpi_calc_retention()` / `kpi_calc_churn()` — Standard formulas

## Extensions

- Add forecasting (linear extrapolation to target)
- Implement OKR grading (0.0-1.0 based on stretch goals)
- Add North Star metric identification
