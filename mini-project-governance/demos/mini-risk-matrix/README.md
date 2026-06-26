# mini-risk-matrix — Risk Registry & PxI Matrix

## Goal

Build a risk registry with probability x impact scoring, mitigation tracking, exposure calculation, and a visual risk matrix heatmap.

## Steps

1. Initialize registry with risk appetite threshold
2. Add risks with probability (Rare→Almost Certain) and impact (Low→Critical)
3. Calculate risk score = probability * impact for each risk
4. Set mitigation plans and owners for top risks
5. Update risk status (Open→Mitigated→Closed→Accepted)
6. Calculate exposure (sum of open risk scores)
7. Check if within appetite
8. Print PxI matrix heatmap showing risk concentration

## Key APIs

- `risk_add()` — Add risk with probability and severity
- `risk_set_mitigation()` — Document mitigation and owner
- `risk_update_status()` — Track risk lifecycle
- `risk_exposure()` — Sum of open risk scores
- `risk_within_appetite()` — Compare exposure to threshold
- `risk_top_n()` — Get highest-risk items for action
- `risk_print_matrix()` — Visual PxI heatmap grid

## Extensions

- Add risk velocity (how quickly is risk score changing)
- Implement Monte Carlo simulation for aggregate risk
- Track realized risks (actual vs predicted)
