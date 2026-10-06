# 09: Register item, combat, spell and store outcomes

**What to build:** The registry and native ledger describe complete item, combat, spell and ordinary store outcomes so their future native implementations can be accepted individually.

**Blocked by:** 07: Validate a source-backed capability registry and native ledger.

**Status:** implemented

- [x] Reconcile this domain's baseline, input contexts, slash commands, registered versioned packet paths/responses and text/byte obligations at atomic outcome granularity.
- [x] Include slot identity, multi-step selection and transactions, macro/input sequences, cancellation, server abort, errors and correct parent restoration; shared selection primitives do not certify all callers.
- [x] Populate related surfaces/actions/states/bindings/input contexts with behavior provenance and applicable protocol/UX/acceptance sources.
- [x] Map every relevant inventory row or supply a source-backed disposition; retain applicable version, keyset, build and platform conditions.
- [x] Each active in-scope outcome has explicit native coverage, one A–F acceptance stage, prerequisites and required scenario evidence; earlier flow prerequisites cannot be silently deferred.
- [x] Keep unimplemented outcomes pending, preserve shared IDs and cross-domain references, and validate all added data without claiming synthetic request evidence covers full gameplay commands.


## Result — 2026-09-22

Added 156 source-backed outcomes (269 total), all pending. See tracked
`docs/capabilities/item-reconciliation.md` for mappings, 22/22 Linux regression
results, independent Standards/Spec review and explicit evidence limits.
