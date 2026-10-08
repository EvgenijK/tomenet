# 47: Verify focused production coverage

Type: implementation
Tracker role: Wayfinder planning/execution history for canonical [SV-B-023](../../../docs/tasks/stage-b/SV-B-023-birth-choices.md); this child does not own Stage B capability IDs.
Status: resolved
Assignee: codex/sandcastle-7b133980-wave-32-1
Labels: enhancement, ready-for-agent

Test the completed SV change through production paths. Inspect the active tickets and add focused missing tests if needed. Run the relevant focused checks. Keep all explicit deferred acceptance checks pending. The orchestrator runs the full headless gate after assembly.

- [x] Relevant production paths are exercised and results recorded under Answer.
- [x] Any new tests are committed, with no test-only behavior or claims of unrun acceptance.

## Comments

- 2026-10-08: Claimed on `codex/sandcastle-7b133980-wave-32-1` before
  coverage inspection. The accepted production seam is public
  `sv_endpoint_run(SvEndpointOptions)` through authoritative decoded setup,
  native SDL birth input and the production interaction/input router. It
  observes owner, stable selection, draft and outbound-send suppression; it
  does not replace deferred native runtime evidence P1-AC30.

## Answer

Проверены batch 8 repairs 43–44 и последующие focused records 45–46. Ticket 45
уже закрыл единственный найденный пробел чувствительным production-path scenario
`case-sensitive-shift-caps-class-binding`: Shift+Caps+Q без предварительного
выбора подтверждает 17-ю server-provided class identity как lowercase `q`.
Ticket 46 изменил только `docs/sv-improvements.md`, оставив централизацию raw
scenario descriptors отдельной будущей работой; production/test behavior там не
менялся. Поэтому нового missing coverage, требующего ещё одного теста, не
обнаружено, и production, legacy/shared и test code в этом тикете не менялись.

Production seam: публичный `sv_endpoint_run(SvEndpointOptions)`. Controlled peer
передаёт authoritative server setup; настоящие SDL keyboard events и backtracking
проходят `sv_native_birth_event()` и production birth interaction/input router.
`SvEndpointOutcome` проверяет owner, stable highlighted identity и draft. Peer
после разрешённого name packet требует EOF, поэтому partial/completed-character
send провалил бы scenario. Каждый focused command ниже компилирует и запускает
default и `SV_CLASS_BEFORE_RACE=0` production paths с `-Wall -Wextra -Werror`,
ASan и UBSan.

Focused verification, 2026-10-08 — все команды **PASS**, exit 0:

- `SV_BIRTH_CASE=case-sensitive-shift-caps-class-binding python3 -B
  tests/sv_name_editor_submit_checks.py` — Shift/Caps parity, lowercase list
  identity, stable highlight, Ctrl-Q termination и send suppression;
- `SV_BIRTH_CASE=case-sensitive-mode-ordinary-hellish python3 -B
  tests/sv_name_editor_submit_checks.py` — uppercase Hellish bindings и
  case-sensitive mode selection;
- `SV_BIRTH_CASE=unknown-setup-pvp-maia-back-confirm python3 -B
  tests/sv_name_editor_submit_checks.py` — ordinary unknown-setup PvP Maia
  Backspace, mode re-entry and legal confirm without a zero-choice stats owner;
- `SV_BIRTH_CASE=unknown-setup-exclusive-pvp-maia-back-random python3 -B
  tests/sv_name_editor_submit_checks.py` — dedicated PvP handoff, Backspace and
  random post-mode Maia trait completion;
- focused structural inspection confirms the ticket-46 improvement entry remains
  one separately proposed record and does not claim its future checks passed;
- `git diff --check` — **PASS**, exit 0 after resolution edits.

Повторяющиеся diagnostics declared font fallback и SDL `Leaked thread`
совпадают с существующим focused-runner output; assertion, compiler и sanitizer
failures не было. Это coverage-only verification уже реализованного поведения:
новый тест не потребовался, defect не обнаружен, поэтому TDD red → green claim
не делается и test-only behavior path не добавлен.

Полные `sv-build`, `sv-core`, controller review и общий build/test/review workflow
не запускались: их выполняет orchestrator после assembly.
`P1-birth-runtime-evidence` / P1-AC30 для accepted contract digest
`690a04cefb7fd0e1615dce46d1a538e1dcbd856232af1a1373be93f987095c82`
остаётся **deferred; not run** у named producer/verifier до full acceptance и
этим тикетом не объявляется выполненным.
