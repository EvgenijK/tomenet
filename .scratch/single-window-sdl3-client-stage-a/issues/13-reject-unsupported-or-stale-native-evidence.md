# 13: Reject unsupported or stale native evidence

**What to build:** A reviewer can distinguish implemented, pending, accepted, stale and unavailable native claims. Acceptance is tied to precisely scoped evidence and cannot survive relevant source drift or terminal fallback.

**Blocked by:** 07: Validate a source-backed capability registry and native ledger.

**Status:** completed (2026-09-22)

- [x] Native evidence identifies scenario, expected outcomes, actual result, executable/configuration, source/fixture/resource fingerprints and environment; reports distinguish malformed claims, missing coverage and stale/unavailable evidence.
- [x] Reject unsupported acceptance, missing evidence and substitution of schema validity, HTML implementation or human UX approval for native behavior evidence.
- [x] Detect relevant source/dependency additions, removals and changes even at unchanged Git HEAD; invalidate affected evidence, or the broader affected scope when impact is unknown.
- [x] Record development fallback routes only for explicitly named future flows, with owner, replacement stage and removal criterion; an empty route set is valid and pending flows are not invented working routes.
- [x] Accepted flow checks fail on fallback entry. Provide the claim/runtime-check contract so later integrated scenarios can supply safe route IDs/reasons/counts without recording prompts, secrets or arbitrary payloads.
- [x] If reuse introduces a temporary adapter, require named flow/context routing through the single input router, preserved queue/request/parent/reply semantics and continuing network/model/timer/render work; unknown gestures and native errors cannot trigger catch-all fallback.
- [x] Negative fixtures exercise unsupported claims, stale/missing/unavailable evidence and fallback violations through the validator interface, without requiring the complete native scenario or fabricating runtime evidence.


## Implementation

Contract and validation: `docs/capabilities/native-evidence.md`. Canonical evidence
and fallback routes remain empty; no runtime acceptance is fabricated.

Validation: 17 evidence, 12 registry and 13 reconciliation tests passed;
21 runtime regression invocations passed. Standards 0, Spec 0 findings.
