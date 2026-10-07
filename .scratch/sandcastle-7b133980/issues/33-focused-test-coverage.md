# 33: Verify focused production coverage

Type: implementation
**Tracker role:** Wayfinder planning/execution history for canonical [SV-B-023](../../../docs/tasks/stage-b/SV-B-023-birth-choices.md); this child does not own Stage B capability IDs.
Status: resolved
Assignee: codex/sandcastle-7b133980-wave-22-1
Labels: enhancement, ready-for-agent

Test the completed SV change through production paths. Inspect the active tickets and add focused missing tests if needed. Run the relevant focused checks. Keep all explicit deferred acceptance checks pending. The orchestrator runs the full headless gate after assembly.

- [x] Relevant production paths are exercised and results recorded under Answer.
- [x] Any new tests are committed, with no test-only behavior or claims of unrun acceptance.

## Comments

- 2026-10-07: Claimed by `codex/sandcastle-7b133980-wave-22-1` before
  coverage inspection. The contract-approved production seam is
  `sv_endpoint_run(SvEndpointOptions)` through authoritative decoded setup,
  native SDL birth input and the production interaction/input router.

## Answer

Проверены resolved records 01–32 и production delta после focused record 31.
Repair 32 уже покрывал снятие post-mode PvP Maia context до body/class,
восстановление ordinary trait и повторный вход в post-mode trait, но ни один
сценарий не проходил восстановленный owner graph полностью до race. Добавлены
два focused regression scenarios: ordinary `mode-pvp-maia-race-reenter` и
production dedicated `exclusive-pvp-maia-race-reenter`.

Оба сценария используют публичный production seam
`sv_endpoint_run(SvEndpointOptions)`. Controlled peer передаёт authoritative
server-provided setup и ordinary/dedicated PvP intent; реальные SDL events идут
через `sv_native_birth_event()` и production interaction/input router. Flow
проходит Maia mode → обязательный post-mode trait → Backspace к stats → полный
ordinary owner graph через доступные body/class и trait до race, выбирает
non-Maia race и снова доходит до ready mode. Outcome проверяет новый race,
пересчитанный unavailable trait, body и mode draft. Peer после разрешённого name
packet требует EOF, поэтому partial/completed-character send провалил бы тест.
Отдельной test-only реализации выбора и fixed-response wizard нет.

Это coverage-only изменение уже реализованного поведения. Оба новых сценария
прошли с первого запуска; production defect не обнаружен, production code не
менялся и red → green claim не делается.

Focused verification, 2026-10-07:

- `SV_BIRTH_CASE=mode-pvp-maia-race-reenter python3 -B tests/sv_name_editor_submit_checks.py`
  — **PASS**, exit 0 в default и `SV_CLASS_BEFORE_RACE=0` configurations.
- `SV_BIRTH_CASE=exclusive-pvp-maia-race-reenter python3 -B tests/sv_name_editor_submit_checks.py`
  — **PASS**, exit 0 в обеих compile configurations.
- `python3 -B tests/sv_name_editor_submit_checks.py` — **PASS**, exit 0;
  полная focused production endpoint birth matrix прошла в обеих compile
  configurations с `-Wall -Wextra -Werror` и ASan/UBSan.
- `git diff --check` — **PASS**, exit 0 после resolution edits.

Повторяющиеся diagnostics declared font fallback и SDL `Leaked thread`
совпадают с уже известным focused-runner output; assertion, compiler и sanitizer
failures не было. Новых unrelated legacy/shared improvement opportunities не
обнаружено.

Полные `sv-build`, `sv-core`, controller review и общий build/test/review
workflow не запускались: их выполняет orchestrator после assembly.
`P1-birth-runtime-evidence` / P1-AC30 для accepted contract digest
`690a04cefb7fd0e1615dce46d1a538e1dcbd856232af1a1373be93f987095c82`
остаётся **deferred; not run** до full acceptance и не объявляется выполненным.
