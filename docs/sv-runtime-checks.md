# SV scenario runtime checks

Ticket 17 adds an application-owned, owner-thread collector. `sv_app_check_run`
executes one synchronous scenario callback; only its checked successful return
completes the scope. The scenario must process its required events, cancellations,
child interactions and exact serialized reply checks before returning success.
Early failure returns an incomplete result. A crash produces no completed record.
There is no public asynchronous finish operation: old session work cannot finish a
new scope. Nested scopes are bounded to eight; a failed child poisons its parent.
Destroying the application during a scope is rejected.

The collector outlives session resets. Every active scope counts each terminal
handoff attempt, so a reset or child scope cannot erase earlier entries.
`sv_app_terminal_fallback` is the sole allowed terminal handoff boundary; it
records before any future transfer of control. Today it always returns
`SV_INVALID`: no terminal adapter is linked. The single bounded route is
`route.terminal-handoff`, with the existing evidence reason `future-flow`.
This is deliberately conservative: even a denied attempt prevents acceptance.
Any future permitted adapter must dispatch inside this boundary, after recording,
and must add an allocated route to the evidence registry. Do not add direct calls
from UI, input or scenarios to a terminal backend.

The native executable wraps HP, messages, requests, lifecycle, geometry, timing
and architecture scenarios. Their existing packet, pixel, cancellation, macro,
reset and reply assertions remain the completion authority. A deliberate delayed
timing failure has its own `scenario.stage-a.timing-delayed` ID. Plain shell
startup makes no completed scenario claim.

With `SV_RUNTIME_REPORT` set, the executable appends one JSON object per returned
scope to that dedicated JSONL file. The record contains only scenario ID,
completion, fallback total and bounded route/reason/count. It contains no prompts,
credentials, input history or payloads. Write failure fails the native invocation.
The console counter is diagnostic only.

`tools/run_stage_a.py` gives every command a fresh artifact path. Candidate
runtime completion requires exactly the expected records for every constituent
scenario, completed positive scopes and successful wrapper checks. Missing,
malformed, duplicate, mismatched or inconsistent records fail closed. The timing
negative control must be present and incomplete; it cannot replace the positive
scope. All entries, including entries in the negative control, are retained.
The composite candidate scenario aggregates these measured scopes; its report
includes the resulting runtime check and the constituent artifact filenames.
This does not establish platform coverage, dependency closure or human approval.
Those remain separate checker blockers and ticket 18 work.

## Verification

`python3 -B tests/sv_runtime_checks.py` exercises production application replies,
cancellation, incomplete child, reset, stale-generation rejection and a nonzero
entry under ASan/UBSan. `tests/sv_runtime_producer_checks.py` checks fail-closed
composition. `tests/sv_evidence_checks.py` checks rejection of incomplete and
named nonzero routes, and verifies that completion only removes `evidence-runtime`.

The changed Linux and MinGW executables invalidate earlier runtime and human
observations. Ticket 16's SDK and Wine procedure must be repeated on these bytes;
see the ticket 17 acceptance report for the actual results. No ticket 16 human or
runtime approval is inherited.

## Ticket 17 final run — 2026-09-23

Reproduced with the ticket 16 final SDK and a fresh isolated Wine prefix:

```sh
/tmp/sv15-venv/bin/python -B tools/run_stage_a.py --output /tmp/sv17-final \
  --mingw-sdk /tmp/sv16-sdk-final --sdk-downloads /tmp/sv16-downloads
```

58 of 60 commands passed. Linux software/OpenGL, Wine software, native HP,
message, request, lifecycle, geometry and shell checks passed; exact replies and
macro order stayed intact. Five sanitizer suites passed, including real collector
results passed through the production evidence checker. Producer tests passed.
Registry (12), reconciliation (13), evidence (19), and HTML (15) tests passed;
canonical registry and HTML sync/check are valid. No capability was promoted.

The two retained failures are:

- Wine Direct3D timing: first prompt took **36.486 ms / 20 ms**. This repeats the
  separately recorded ticket 16 issue. Its production runtime record is correctly
  **incomplete**, with zero fallback entries; the wrapper fails before running the
  delayed negative control. All other Direct3D scenarios passed.
- Candidate evidence validation: **24 `evidence-dependencies` errors** remain for
  ticket 18. All eight Linux candidate records have measured completed runtime
  checks with zero entries. There are **no `evidence-runtime` errors**. The
  candidate check is a real blocking result, not a passing negative test.

Fresh executable SHA-256:

- Linux: `03ded8efc9341285b600e275a7672d7616a315fbe6f6967869bff5a1eba24b8b`.
- MinGW/staged Wine: `28ae163437886b8de2153a375d55dfdb6ea337639ae1a3d1591328b4ec3f5ea4`.

The [report](acceptance/stage-a-ticket-17-2026-09-23.json) records every command and
result. The [archive](acceptance/stage-a-ticket-17-2026-09-23.tar.gz) retains logs,
production JSONL measurements, candidates, build/SDK metadata and a hashed file
inventory, without executables or Wine prefixes. Archive SHA-256:
`0aba165e19e2adba11da73b876c7f1f1f3b1f2334f553cfe9cb3a526b0abd728`.
All report source fingerprints were checked before archiving. Only the reviewed
SV source fingerprints were refreshed in the canonical inventory. Overall Stage A
acceptance and human/platform evidence remain blocked; earlier binary observations
are not reusable for these new bytes.

## Standards

Independent review against `a1b3c3abb55b6fd66fe0b577d634b6ba62810f53` found no
blocking documented-standard violations or actionable baseline smells. SV
isolation and bounded, application-owned lifecycle state are preserved.

## Spec

Independent review found no blocking implementation findings. The follow-up
review confirmed the decode-error, nested nonzero, real-checker tests and the
invalidation documentation. The final regression results above complete the
requested ticket-16 rerun, while retaining its known Direct3D timing failure.

Review totals: Standards **0**, Spec **0** implementation findings; neither axis
waives the two measured acceptance blockers.
