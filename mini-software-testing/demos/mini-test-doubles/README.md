# mini-test-doubles — Test Doubles (Stubs, Mocks, Fakes, Spies)

## Goal

Implement the four types of test doubles — Stub, Mock, Fake, and Spy — to isolate code under test from its dependencies.

## Steps

1. **Stub**: Create a stub for `database_query()` that always returns predefined data
2. **Mock**: Create a mock for `email_service.send()` with expected call parameters and verify
3. **Fake**: Create a fake `cache` with simple in-memory state (a real but lightweight impl)
4. **Spy**: Create a spy on `logger.log()` to record all calls and their arguments
5. Verify that mock expected calls were made with `double_mock_verify()`
6. Check spy call count and inspect recorded calls

## Key APIs

- `double_add_stub()` — Fixed return value double
- `double_add_mock()` — Expectation-based mock
- `double_mock_expect()` — Define expected call + return value
- `double_add_fake()` — Lightweight real implementation with state
- `double_add_spy()` — Call-recording wrapper
- `double_spy_record()` — Record a call with args
- `double_mock_verify()` — All expected calls satisfied?

## Extensions

- Add stub with callback (stub calls a function when invoked)
- Implement mock argument matchers (any(), contains(), etc.)
- Add spy return value manipulation
