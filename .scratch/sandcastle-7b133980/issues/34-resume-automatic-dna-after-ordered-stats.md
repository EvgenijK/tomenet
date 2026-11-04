# 34: Возобновить automatic DNA после ordered stats

**Type:** repair

**Tracker role:** Wayfinder planning/execution history for canonical [SV-B-023](../../../docs/tasks/stage-b/SV-B-023-birth-choices.md); this child does not own Stage B capability IDs.

**What to build:** Когда `%` приводит automatic reincarnation к ordered stats, игрок вручную завершает legal six-stat order, не теряя pending automatic state; при переходе к mode production birth interaction автоматически применяет legal mode DNA и продолжает baseline flow. Поведение работает в ordinary и dedicated creation в обеих compile-time class/race order configurations, не меняя уже проверенную point-buy DNA semantics и не добавляя persistence или final send.

**Blocked by:** 19: Продолжить automatic DNA choices через birth flow (completed); 24: Нормализовать point-buy DNA отдельно для каждого stat (completed).

**Status:** resolved

**Assignee:** codex/sandcastle-7b133980-wave-23-1

Labels: bug, ready-for-agent

**Finding ID:** `D-36ccf3e367566848` (`birth.dna-auto-reincarnation-semantics`), residual repair of the still-open ledger finding.

**Production seam for TDD:** публичный `sv_endpoint_run(SvEndpointOptions)` получает authoritative creation setup и DNA identities; реальный SDL `%`, ручные ordered-stat navigation/confirm events и последующий mode transition проходят native birth adapter и публичную production birth-interaction/input-router boundary. `SvEndpointOutcome` и controlled peer наблюдают pending automatic state, owner, highlights/draft и отсутствие запрещённых outbound bytes. Seam принят P1-AC02, P1-AC08, P1-AC09, P1-AC10, P1-AC11, P1-AC26 и P1-AC28; проверки — `sv-core` и `sv-build`.

- [x] Red production-path scenario с authoritative `creation_flags == 0` доводит `%` до ordered stats и фиксирует baseline suspension: stats DNA не применяется и не помечается illegal, `auto_reincarnation` остаётся pending, а owner/highlight и пустой stat order готовы к обычному ручному вводу. [Contract: P1-AC02, P1-AC08, P1-AC10, P1-AC26, P1-AC28, P2-AC04, P2-AC05; checks: `sv-core`]
- [x] Keyboard и mouse production input позволяют вручную выбрать все шесть legal stat identities; pending automatic state сохраняется после каждого подтверждения, committed order совпадает с вводом игрока, а шестое подтверждение переводит interaction к mode и автоматически применяет legal mode DNA без дополнительного `%`. [Contract: P1-AC02, P1-AC08, P1-AC09, P1-AC10, P1-AC11, P1-AC26, P1-AC28; checks: `sv-core`]
- [x] Missing или illegal mode DNA после ordered stats останавливает automatic continuation на mode по существующим reject rules, не подтверждает illegal value и оставляет согласованные owner, stable highlight и draft для ручного продолжения; legal mode DNA завершает только local draft. [Contract: P1-AC02, P1-AC09, P1-AC10, P1-AC26, P2-AC04; checks: `sv-core`, `controller-review`]
- [x] Production-path regressions покрывают ordinary и dedicated creation с ordered stats в default и `SV_CLASS_BEFORE_RACE=0` configurations; существующие point-buy automatic-DNA scenarios остаются green. Ни один scenario не подменяет interaction fixed-response или test-only implementation. [Contract: P1-AC19, P1-AC20, P1-AC28, P2-AC05; checks: `sv-build`, `sv-core`]
- [x] Repair остаётся по возможности SV-local, не добавляет `birth.restore-dna`/`birth.save-dna`, DNA persistence, partial/completed-character send или ownership SV-B-024; найденные побочные улучшения только записываются отдельно. [Contract: P1-AC02, P1-AC26, P1-AC27, P1-AC29, P2-AC01, P2-AC02, P2-AC03, P2-AC06, P2-AC08, P2-AC09; checks: `sv-core`, `controller-review`]

**Deferred check:** `P1-birth-runtime-evidence` / P1-AC30 остаётся deferred и не считается пройденным этим implementation repair; полная native evidence matrix собирается только на exact accepted contract digest и HEAD во время full acceptance.

## Comments

- 2026-10-07: Claimed by `codex/sandcastle-7b133980-wave-23-1`. TDD seam is the ticket-approved public `sv_endpoint_run(SvEndpointOptions)` path through authoritative creation setup, native SDL birth input and the production interaction/input router.

## Answer

Исправление выполнено локально в SV birth interaction. Automatic DNA на
ordered stats теперь приостанавливается без применения stats DNA, reject evidence
или потери `auto_reincarnation`. Ручные legal stat confirmations сохраняют pending
state; шестое подтверждение готовит mode и возобновляет тот же automatic loop,
который применяет legal mode DNA и продолжает baseline flow. Point-buy ветка не
изменена. Missing и illegal mode DNA используют прежние reject rules и оставляют
готовый committed stat order и стабильный mode highlight для ручного продолжения.

Production seam: публичный `sv_endpoint_run(SvEndpointOptions)`. Controlled peer
передаёт authoritative `creation_flags == 0`, ordinary/dedicated setup и DNA
identities. Настоящие SDL `%`, keyboard и mouse events проходят native birth
adapter и production interaction/input router. `SvEndpointOutcome` наблюдает
pending state, owner, highlights и draft; peer после разрешённого name packet
требует EOF, поэтому partial/completed-character send провалил бы сценарий.
Test-only choice implementation и fixed-response wizard не добавлены;
legacy/shared runtime files не изменялись.

TDD red → green, 2026-10-07:

- `SV_BIRTH_CASE=auto-dna-ordered-suspend python3 -B tests/sv_name_editor_submit_checks.py`
  — **RED**, exit 1: `birth_auto_reincarnation` был сброшен на ordered stats;
  после минимальной suspension repair — **GREEN**, exit 0 в default и
  `SV_CLASS_BEFORE_RACE=0` configurations.
- `SV_BIRTH_CASE=auto-dna-ordered-keyboard python3 -B tests/sv_name_editor_submit_checks.py`
  — **RED**, exit 1: после шестого manual confirm owner оставался `MODE`; после
  automatic continuation repair — **GREEN**, exit 0 в обеих configurations.

Focused verification:

- `auto-dna-ordered-partial`, `auto-dna-ordered-mouse-dedicated`,
  `auto-dna-ordered-mode-missing` и `auto-dna-ordered-mode-illegal` через тот же
  runner — **PASS**, exit 0 каждый в обеих configurations.
- Point-buy regressions `auto-dna-full` и `auto-dna-stats-normalized`, а также
  `auto-dna-dedicated-pvp-maia` — **PASS**, exit 0 каждый в обеих configurations.
- `git diff --check` — **PASS**.

Полные `sv-build`, `sv-core`, controller review и общий build/test/review workflow
не запускались: их выполняет orchestrator после assembly. Диагностики declared
font fallback и SDL `Leaked thread` совпадают с существующим focused-runner
output; assertion, compiler и sanitizer failures в green runs не было. Новых
побочных improvement opportunities не обнаружено.
`P1-birth-runtime-evidence` / P1-AC30 остаётся **deferred; not run** до full
acceptance на exact contract digest и HEAD.
