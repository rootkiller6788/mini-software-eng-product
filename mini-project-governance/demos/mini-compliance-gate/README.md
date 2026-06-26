# mini-compliance-gate — Stage Gate Compliance

## Goal

Implement a compliance framework with stage gates, required/optional checks, gate reviews, and an immutable audit trail.

## Steps

1. Initialize compliance plan with standard (e.g., SOC2, ISO 27001, GDPR)
2. Define stage gates (e.g., Design Review, Pre-Deploy, Launch Readiness)
3. Add compliance checks per gate, marking required vs optional
4. Pass checks with evidence references (document names, report links)
5. Review a gate: if all required checks pass, gate status = PASSED
6. Log every action to the audit trail with before/after state
7. Check if all gates passed (ready for release)

## Key APIs

- `compliance_add_gate()` — Define a stage gate
- `compliance_add_check()` — Add check to gate (required or optional)
- `compliance_pass_check()` — Mark check as passed with evidence
- `compliance_gate_ready()` — Check if all required checks pass
- `compliance_review_gate()` — Formal review, sets PASSED/FAILED
- `compliance_audit_log()` — Record action in immutable trail
- `compliance_all_gates_passed()` — State of all gates

## Extensions

- Add due dates to gates with escalation logic
- Implement role-based gate approvers
- Support re-review after previously failed gate
- Generate compliance report in markdown/PDF format
