# 02: Display decoded HP through the Session presentation model

**What to build:** A reviewer sees HP/status changes in the native shell when synthetic server bytes enter production decoding. The Session presentation model publishes coherent state using the approved core/model/UI boundary.

**Blocked by:** 01: Launch an isolated single-window SV client.

**Status:** implemented

- [x] Separate the selected HP decode/apply path from terminal presentation before exposing it to SV; preserve applicable legacy client behavior and reuse suitable existing storage rather than creating independently mutable duplicates.
- [x] Prepared bytes for both applicable HP wire layouts run through production packet parsing and dispatch with an explicitly selected server version; visible and semantic results preserve HP value/marker semantics.
- [x] For every split boundary, incomplete input leaves state and side effects unchanged; completion applies once and a following known packet remains consumable in order. Malformed inputs with defined failure contracts retain those contracts.
- [x] Native UI consumes coherent read-only semantic views and never interprets protocol-version predicates, legacy globals or terminal buffers.
- [x] Assertions observe independently specified model outcomes, visible status and successful native frame submission; no second decoder or test-only presentation model substitutes for production behavior.
- [x] The synthetic peer and controlled time/RNG enter only at external seams. Replaced model/view storage has explicit ownership and lifetime, and the accepted HP flow cannot enter terminal fallback.
- [x] Document reproducible scenario checks and rerun relevant legacy build/behavior checks for shared-code changes.


## Implementation and verification — 2026-09-20

Shared transactional HP decoding feeds the native Session model and preserves
legacy presentation/alert behavior. Both wire layouts and all split boundaries
passed 78 cases and 156 real native frame submissions on Linux software/OpenGL
and Wine software. The actual legacy Receive_hp regression harness and full
Linux SDL3 build passed. Eight existing Linux shell smoke checks also passed.

Reproducible commands, ownership/failure contracts and evidence limits:
[Native HP guide](../../../docs/sv-hp.md). Fedora41 runtime compatibility, actual
Windows and human UX approval remain unverified; this is not complete Stage A
acceptance. No commit was created for ticket 02.
