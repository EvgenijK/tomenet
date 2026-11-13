# 45: Verify focused production coverage

Type: implementation
Tracker role: Wayfinder planning/execution history for canonical [SV-B-023](../../../docs/tasks/stage-b/SV-B-023-birth-choices.md); this child does not own Stage B capability IDs.
Status: resolved
Assignee: codex/sandcastle-7b133980-wave-30-1
Labels: enhancement, ready-for-agent

Test the completed SV change through production paths. Inspect the active tickets and add focused missing tests if needed. Run the relevant focused checks. Keep all explicit deferred acceptance checks pending. The orchestrator runs the full headless gate after assembly.

- [x] Relevant production paths are exercised and results recorded under Answer.
- [x] Any new tests are committed, with no test-only behavior or claims of unrun acceptance.

## Comments

- 2026-10-08: Claimed on `codex/sandcastle-7b133980-wave-30-1` before
  coverage inspection. The contract-approved production seam is public
  `sv_endpoint_run(SvEndpointOptions)` through authoritative decoded setup,
  native SDL birth input and the production interaction/input router. Review
  of resolved records 43–44 found one insensitive case: the existing class
  scenario sent Shift+Caps+Q only after lowercase q had already selected the
  same identity, so rejection of the parity event produced the same outcome.

## Answer

Проверены resolved production repairs 43–44. Добавлен focused scenario
`case-sensitive-shift-caps-class-binding`, который закрывает найденный пробел:
Shift+Caps+Q без предварительного выбора должен действовать как lowercase q,
выбрать и подтвердить 17-ю identity из server-provided class list, не завершить
creation и не отметить input illegal.

Production seam: публичный `sv_endpoint_run(SvEndpointOptions)`. Controlled
peer передаёт authoritative 20-item class list; настоящий
`SDL_EVENT_KEY_DOWN` с SDL keycode Q и одновременно Shift/Caps проходит
`sv_native_birth_event()` и production birth interaction/input router.
`SvEndpointOutcome` подтверждает class identity 16, stable highlight,
keyboard navigation, отсутствие illegal rejection и последующий Ctrl-Q без
partial/completed-character send. Peer после разрешённого name packet требует
EOF, поэтому любой дополнительный outbound packet провалил бы scenario. Тест
исполняется в обеих compile-time class/race order configurations.

Это coverage-only тест уже реализованного поведения. Он прошёл с первого
запуска; production defect не обнаружен, production/legacy/shared runtime code
не менялся, поэтому TDD red → green claim не делается. Test-only behavior path
и fixed-response wizard не добавлены.

Focused verification, 2026-10-08 (каждая команда — **PASS**, exit 0 в default
и `SV_CLASS_BEFORE_RACE=0` configurations с `-Wall -Wextra -Werror`, ASan и
UBSan):

- `SV_BIRTH_CASE=case-sensitive-shift-caps-class-binding python3 -B
  tests/sv_name_editor_submit_checks.py`;
- `SV_BIRTH_CASE=case-sensitive-class-binding python3 -B
  tests/sv_name_editor_submit_checks.py`;
- `SV_BIRTH_CASE=case-sensitive-mode-ordinary-hellish python3 -B
  tests/sv_name_editor_submit_checks.py`;
- `SV_BIRTH_CASE=unknown-setup-pvp-maia-back-confirm python3 -B
  tests/sv_name_editor_submit_checks.py`;
- `SV_BIRTH_CASE=unknown-setup-exclusive-pvp-maia-back-random python3 -B
  tests/sv_name_editor_submit_checks.py`;
- `git diff --check` — **PASS**, exit 0.

Повторяющиеся diagnostics declared font fallback и SDL `Leaked thread`
совпадают с существующим focused-runner output; assertion, compiler и sanitizer
failures не было. Новых unrelated improvement opportunities не обнаружено.

Полные `sv-build`, `sv-core`, controller review и общий build/test/review
workflow не запускались: их выполняет orchestrator после assembly.
`P1-birth-runtime-evidence` / P1-AC30 для accepted contract digest
`690a04cefb7fd0e1615dce46d1a538e1dcbd856232af1a1373be93f987095c82`
остаётся **deferred; not run** до full acceptance и не объявляется выполненным.
