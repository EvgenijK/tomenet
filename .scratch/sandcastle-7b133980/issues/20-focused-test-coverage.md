# 20: Verify focused production coverage

Type: implementation
**Tracker role:** Wayfinder planning/execution history for canonical [SV-B-023](../../../docs/tasks/stage-b/SV-B-023-birth-choices.md); this child does not own Stage B capability IDs.
Status: resolved
Assignee: codex/sandcastle-7b133980-wave-14-1
Labels: enhancement, ready-for-agent

Test the completed SV change through production paths. Inspect the active tickets and add focused missing tests if needed. Run the relevant focused checks. Keep all explicit deferred acceptance checks pending. The orchestrator runs the full headless gate after assembly.

- [x] Relevant production paths are exercised and results recorded under Answer.
- [x] Any new tests are committed, with no test-only behavior or claims of unrun acceptance.

## Comments

- 2026-10-07: Claimed by `codex/sandcastle-7b133980-wave-14-1`; the
  contract-approved production seam is the public
  `sv_endpoint_run(SvEndpointOptions)` flow through decoded setup, the native SDL
  birth adapter and production interaction/input router.

## Answer

Проверены все implementation tickets 01–19 effort: они имеют `Status: resolved`.
Изменения repair tickets 14–19 уже представлены focused scenarios для
exclusive birth input context, stats re-entry, hidden derived class resolution,
empty class random rejection, first-run/Archer body legality и automatic DNA
continuation. Сопоставление этих scenarios с исходной production matrix не
выявило отсутствующего meaningful coverage, поэтому новые tests и production
changes не потребовались.

Проверенный public production seam: `sv_endpoint_run(SvEndpointOptions)` получает
настоящий decoded server setup и authoritative character/session state, создаёт
session-owned birth interaction, а keyboard/mouse input проходит через
`sv_native_birth_event()` и `sv_birth_command()`. `SvEndpointOutcome` наблюдает
owner, legal lists, highlights, draft и lifecycle. Controlled peer после
единственного допустимого name packet требует EOF, поэтому partial или completed
character send ломает scenario. Test-only choice engine, fixed-response wizard
или отдельная реализация поведения не используются.

Focused verification, 2026-10-07, production implementation HEAD
`886f65318b` (последующий commit меняет только этот ticket):

- `python3 -B tests/sv_name_editor_submit_checks.py` — **PASS**, exit 0;
  default и `SV_CLASS_BEFORE_RACE=0` binaries собраны с
  `-Wall -Wextra -Werror` и ASan/UBSan, вся production endpoint birth matrix
  прошла, включая новые slices tickets 14–19 и ранее принятый baseline.
- `git diff --check` — **PASS**, exit 0.

Это coverage-only ticket уже реализованного поведения. Проверка прошла с первого
запуска, behavior fix не понадобился, поэтому TDD red → green claim не делается.
Новые tests не добавлялись; существующие tests вызывают production SV path.
Повторяющиеся diagnostics о declared font fallback и `Leaked thread` уже
записаны в `docs/sv-improvements.md`; новых improvement opportunities не
обнаружено.

Полные `sv-build`, `sv-core`, controller review и весь build/test/review workflow
не запускались: assembled gates выполняет orchestrator. В частности,
`P1-birth-runtime-evidence` / P1-AC30 остаётся **deferred; not run** по исходной
acceptance-phase причине. Никакая full-acceptance evidence не объявляется
выполненной: её необходимо произвести и проверить на точных accepted contract
digest `690a04cefb7fd0e1615dce46d1a538e1dcbd856232af1a1373be93f987095c82`
и HEAD full acceptance.
