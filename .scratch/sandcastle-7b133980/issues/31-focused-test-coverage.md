# 31: Verify focused production coverage

Type: implementation
**Tracker role:** Wayfinder planning/execution history for canonical [SV-B-023](../../../docs/tasks/stage-b/SV-B-023-birth-choices.md); this child does not own Stage B capability IDs.
Status: resolved
Assignee: codex/sandcastle-7b133980-wave-20-1
Labels: enhancement, ready-for-agent

Test the completed SV change through production paths. Inspect the active tickets and add focused missing tests if needed. Run the relevant focused checks. Keep all explicit deferred acceptance checks pending. The orchestrator runs the full headless gate after assembly.

- [x] Relevant production paths are exercised and results recorded under Answer.
- [x] Any new tests are committed, with no test-only behavior or claims of unrun acceptance.

## Comments

- 2026-10-07: Claimed by `codex/sandcastle-7b133980-wave-20-1` before
  coverage inspection. The contract-approved production seam is
  `sv_endpoint_run(SvEndpointOptions)`: decoded server setup enters the
  session-owned birth interaction, and real SDL events pass through the native
  adapter and production input router.

## Answer

Inspected resolved behavior tickets 27–29 and tracker-only ticket 30. Their
production-path scenarios already provide focused coverage for stable hidden
DNA race rejection, unknown creation setup stats skipping/backtracking, and
long class/race/trait list viewport navigation. Ticket 30 changed only the
Wayfinder ledger. No missing meaningful coverage or behavior defect was found,
so no new test or production-code change was needed and no test-first loop is
claimed.

The exercised public seam is `sv_endpoint_run(SvEndpointOptions)`. Controlled
peers provide authoritative decoded creation setup and server-provided choice
lists. SDL keyboard, mouse, wheel, focus, resize, stale-generation and quit
events pass through `sv_native_birth_event()` and the production interaction
router. `SvEndpointOutcome` observes owner, highlighted identity, committed
draft and lifecycle; the peer accepts only the allowed name packet and requires
EOF afterwards, so a partial or completed birth send fails the scenarios. There
is no test-only choice implementation or fixed-response wizard.

Focused verification, 2026-10-07:

- `python3 -B tests/sv_name_editor_submit_checks.py` — **PASS**, exit 0. The
  complete focused production endpoint birth matrix passed in both compile-time
  class/race orders with `-Wall -Wextra -Werror` and ASan/UBSan. This includes
  the ticket 27 compatible/rejected staged and automatic DNA cases; ticket 28
  unknown-setup mode, Backspace/re-entry, ready, stale and disconnect cases;
  ticket 29 long class/race/trait keyboard, wheel, mouse, disabled-row,
  focus/resize and stable-highlight cases; and the existing quit/no-extra-send
  regression matrix.
- `git diff --check` — **PASS**, exit 0 before resolution edits.

The recurring fallback-font and `Leaked thread` diagnostics match the known
focused-runner output; there were no assertion, compiler or sanitizer failures.
No unrelated legacy/shared improvement opportunity was found.

The orchestrator-owned full build/test/review workflow, `sv-build`, and
`sv-core` were not run here. `P1-birth-runtime-evidence` / P1-AC30 at contract
digest `690a04cefb7fd0e1615dce46d1a538e1dcbd856232af1a1373be93f987095c82`
remains **deferred; not run** for full acceptance and is not claimed complete.
