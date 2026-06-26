# mini-mutation-testing — Mutation Testing Framework

## Goal

Evaluate test suite quality by injecting artificial bugs (mutants) and measuring how many are caught (killed) by existing tests.

## Steps

1. Define a set of mutation operators:
   - Arithmetic: + → -, * → /, > → >=
   - Logical: && → ||, !x → x
   - Conditional: flip branches, remove conditions
   - Value: replace constants, swap parameters
2. Create mutants by applying operators to source code
3. Run the test suite against each mutant
4. If a test fails → mutant is KILLED (test detected the bug)
5. If all tests pass → mutant SURVIVED (gap in test coverage)
6. Calculate mutation score = killed / total * 100

## Key APIs

- `mutation_init()` — Initialize mutation report
- `mutation_add_mutant()` — Register a mutant
- `mutation_kill()` — Mark mutant as killed (test caught it)
- `mutation_score()` — Percentage of mutants killed

## Extensions

- Implement equivalent mutant detection (mutations that don't change behavior)
- Add mutation operator categories (statement, decision, value)
- Generate mutation diff output showing exact code changes
- Track mutation-adequate test coverage (kill ratio per operator type)

## References

- Jia, Y. & Harman, M. (2011) "An Analysis and Survey of the Development of Mutation Testing"
- DeMillo, R.A. et al. (1978) "Hints on Test Data Selection"
