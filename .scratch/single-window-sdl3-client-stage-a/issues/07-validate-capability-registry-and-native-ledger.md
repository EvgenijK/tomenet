# 07: Validate a source-backed capability registry and native ledger

**What to build:** An implementer can validate canonical capability data and explicit native coverage/stage allocation through a standalone data-tool interface. A source-backed initial Stage A scenario slice demonstrates the complete workflow without claiming the full registry is finished.

**Blocked by:** None (can start immediately).

**Status:** completed (2026-09-22)

- [x] Implement the approved normalized capabilities, surfaces, actions, states, bindings, inputContexts and relations, with stable globally unique typed IDs and the approved relationship meanings.
- [x] Distinguish behavior, protocol, UX and acceptance provenance; active capabilities require behavior sources, and source records support repository/revision/locator plus content fingerprints.
- [x] Validate reference existence and types, binding context/keyset/platform restrictions, legal lifecycle/replacements and absence of replacement cycles; IDs are never reused and replacement does not inherit acceptance.
- [x] Keep native implementation, evidence and A–F allocation in consumer artifacts; each active in-scope outcome has a row with one acceptance stage, prerequisites, applicable conditions and evidence obligations.
- [x] Use schema version for structure and SHA-256 of actual manifest bytes for content identity. Capabilities represent independently verifiable outcomes rather than arbitrary packet fields, buttons or broad families.
- [x] Populate the selected foundation outcomes and related entities from source with truthful pending dispositions. Schema validity does not certify runtime behavior or completeness of the remaining baseline.
- [x] Negative fixtures reject duplicate/unknown IDs, invalid endpoint types, missing provenance/coverage/stage assignments and replacement cycles; reports identify offending entities/sources and return an appropriate process result.
- [x] Publish validator usage and valid/invalid fixtures. The tool accepts data and returns reports without adding a second game-core seam.


## Implementation

Canonical data, schemas, validator usage and verification: `docs/capabilities/README.md`.
Four source-backed outcomes remain pending; complete registry reconciliation and
native evidence acceptance remain assigned to later tickets. Linux regression
run: 22/22 passed; independent Standards and Spec review: zero unresolved findings.
