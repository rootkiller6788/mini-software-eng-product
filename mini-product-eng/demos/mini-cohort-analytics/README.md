# mini-cohort-analytics — Cohort & Funnel Analytics

## Goal

Implement user cohort retention tables and conversion funnel analysis to understand user behavior over time.

## Steps

1. Create weekly cohorts with initial user counts
2. Record daily retention for each cohort (how many return each day)
3. Print retention table (cohorts x days matrix)
4. Build a multi-step conversion funnel (e.g., Visit → Signup → Complete → Verify)
5. Calculate drop-off percentage at each step
6. Compute overall funnel conversion rate
7. Identify the worst-performing step for optimization

## Key APIs

- `analytics_add_cohort()` — Define a user cohort
- `analytics_record_retention()` — Record active users on day N
- `analytics_retention_at()` — Query retention at specific day
- `analytics_add_funnel()` — Create conversion funnel
- `analytics_funnel_add_step()` — Add step with entered/completed counts
- `analytics_funnel_calculate()` — Compute overall conversion and drop-offs

## Extensions

- Implement rolling retention (30-day window)
- Add segmented funnels (by platform, region, acquisition channel)
- Calculate LTV from retention curve (area under curve * ARPU)
- Build AARRR pirate metrics (Acquisition, Activation, Retention, Revenue, Referral)
