# mini-stakeholder-raci — Stakeholder Analysis & RACI Matrix

## Goal

Map project stakeholders by power and interest, identify key stakeholders, and build a RACI matrix for clear responsibility assignment.

## Steps

1. Add stakeholders with name, role, influence (power), and interest levels
2. Mark key stakeholders who must be actively managed
3. Print power/interest grid showing stakeholder distribution
4. Define activities (tasks, decisions, deliverables)
5. Assign RACI roles per activity:
   - **R**esponsible: Does the work
   - **A**ccountable: Approves and is answerable
   - **C**onsulted: Provides input
   - **I**nformed: Kept in the loop
6. Print RACI matrix as a table

## Key APIs

- `stakeholder_add()` — Add person with influence/interest
- `stakeholder_set_key()` — Mark as key stakeholder
- `stakeholder_print_matrix()` — Power/interest grid display
- `raci_add()` — Define an activity
- `raci_assign()` — Assign RACI role to stakeholder
- `stakeholder_print_raci()` — Print RACI table

## Extensions

- Add stakeholder communication preferences and frequency
- Implement RACI validation (check each activity has exactly one A, at least one R)
- Support RASCI (add S=Supportive role)
