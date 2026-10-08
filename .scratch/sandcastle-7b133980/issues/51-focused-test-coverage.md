# 51: Verify focused production coverage

Type: implementation
Status: resolved
Assignee: codex/sandcastle-7b133980-wave-34-1
Labels: enhancement

Test the completed SV change through production paths. Inspect the active tickets and add focused missing tests if needed. Run the relevant focused checks. Keep all explicit deferred acceptance checks pending. The orchestrator runs the full headless gate after assembly.

- [x] Relevant production paths are exercised and results recorded under Answer.
- [x] Any new tests are committed, with no test-only behavior or claims of unrun acceptance.

## Comments

- 2026-10-08: Claimed on `codex/sandcastle-7b133980-wave-34-1` before
  coverage inspection. The accepted production seam is public
  `sv_endpoint_run(SvEndpointOptions)` through authoritative decoded setup,
  realistic native SDL birth events and the production interaction/input
  router. It observes owner, stable selection, draft, Guide lifecycle and
  outbound-send suppression; it does not replace deferred native runtime
  evidence P1-AC30.

## Answer

Проверены resolved batch-12 records 48–50. Ticket 48 изменил production native
adapter и production-path scenarios для реальных Shift-пунктуационных SDL
bindings; tickets 49–50 изменили только `docs/sv-improvements.md` и оставили
предлагаемые DNA aggregate и random-step helper отдельной будущей работой.

Production seam: публичный `sv_endpoint_run(SvEndpointOptions)`. Controlled
peer передаёт authoritative server setup и DNA draft; реальные
`SDL_EVENT_KEY_DOWN` с physical scancode, base keycode и modifiers проходят
`sv_native_birth_event()` и public birth interaction/input-router boundary.
`SvEndpointOutcome` наблюдает owner, stable highlight, draft и Guide lifecycle.
Peer после разрешённого name packet требует EOF, поэтому запрещённый partial или
completed-character send провалил бы scenario.

Существующая матрица чувствительно покрывает все изменённые bindings: Shift+/
на каждом canonical Guide owner, Shift+= для выбранного point-buy stat, Shift+8
random, Shift+3 staged DNA, Shift+5 automatic DNA, unshifted/unsupported/mismatched
events и сохранённый keypad-multiply binding. Дополнительного missing coverage
не обнаружено, поэтому новые тесты и production/legacy/shared code в этом ticket
не добавлялись. Это coverage-only verification уже реализованного поведения;
TDD red → green не заявляется, test-only behavior path не создавался.

Focused verification, 2026-10-08 — все команды **PASS**, exit 0; runner
компилировал production seam с `-Wall -Wextra -Werror`, ASan и UBSan:

- `SV_BIRTH_CASE=guide-all-owners python3 -B tests/sv_name_editor_submit_checks.py`
  — exact placeholder и same-caller/draft preservation в обеих compile orders;
- `SV_BIRTH_CASE=stats-point-navigate python3 -B tests/sv_name_editor_submit_checks.py`
  — Shift+=, rejected unshifted/modified forms и stable point-buy selection в
  обеих compile orders;
- `SV_BIRTH_CASE=random python3 -B tests/sv_name_editor_submit_checks.py` и
  `SV_BIRTH_CASE=alternate-random python3 -B tests/sv_name_editor_submit_checks.py`
  — Shift+8 по default и alternate class/race orders;
- `SV_BIRTH_CASE=dna python3 -B tests/sv_name_editor_submit_checks.py` и
  `SV_BIRTH_CASE=alternate-dna python3 -B tests/sv_name_editor_submit_checks.py`
  — Shift+3 по default и alternate class/race orders;
- `SV_BIRTH_CASE=auto-dna-full python3 -B tests/sv_name_editor_submit_checks.py`
  — Shift+5 automatic DNA traversal в обеих compile orders;
- `SV_BIRTH_CASE=birth-context-escape python3 -B tests/sv_name_editor_submit_checks.py`
  — unshifted bases, unsupported modifiers и mismatched scancode не меняют
  owner/highlight/draft в обеих compile orders;
- `SV_BIRTH_CASE=keypad-random python3 -B tests/sv_name_editor_submit_checks.py`
  — keypad multiply random regression;
- focused structural assertions for the two improvement records — **PASS**:
  each heading occurs once and contains exactly one Status, Problem, Affected
  code, Proposal and Required checks field while keeping future checks unclaimed;
- `git diff --check` — **PASS**.

Повторяющиеся declared-font fallback и SDL `Leaked thread` diagnostics совпали
с известным focused-runner output; assertion, compiler и sanitizer failures не
было. Полные `sv-build`, `sv-core`, controller review и общий workflow не
запускались: их выполняет orchestrator после assembly.
`P1-birth-runtime-evidence` / P1-AC30 для accepted contract digest
`690a04cefb7fd0e1615dce46d1a538e1dcbd856232af1a1373be93f987095c82`
остаётся **deferred; not run** у named full-acceptance producer/verifier и этим
ticket не объявляется выполненным.
