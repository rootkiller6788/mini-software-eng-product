# mini-ab-experiment — A/B Testing Framework

## Goal

Implement a complete A/B testing framework — variant creation, user assignment, z-test significance, and winner selection.

## Steps

1. Define hypothesis: "New checkout flow increases conversion by 10%"
2. Create control and 2-3 treatment variants
3. Assign users to each variant with conversion counts
4. Run significance test (two-proportion z-test at 95% confidence)
5. Calculate lift vs control for each variant
6. Determine winning variant (statistically significant best performer)
7. Conclude experiment and print results

## Key APIs

- `experiment_init()` — Set hypothesis and significance level
- `experiment_add_variant()` — Create variant bucket
- `experiment_assign_users()` — Allocate users and conversions
- `experiment_is_significant()` — Two-tailed z-test (z > 1.96)
- `experiment_lift()` — Relative improvement over control
- `experiment_winning_variant()` — Best significant variant

## Extensions

- Add minimum sample size calculator (power analysis)
- Implement multi-armed bandit (Thompson sampling)
- Support sequential testing with alpha spending
- Add Bonferroni correction for multiple comparisons
