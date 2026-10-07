# 19: Продолжить automatic DNA choices через birth flow

**Type:** implementation

**Tracker role:** Wayfinder planning/execution history for canonical [SV-B-023](../../../docs/tasks/stage-b/SV-B-023-birth-choices.md); this child does not own Stage B capability IDs.

**What to build:** `%` включает baseline automatic reincarnation: production birth interaction последовательно применяет legal DNA identity на каждом доступном owner, пропускает только действительно unavailable steps и безопасно останавливается на missing/incompatible DNA, оставляя игроку согласованный ручной выбор без persistence.

**Blocked by:** 14: Изолировать production birth input context.

**Status:** resolved

**Assignee:** codex/sandcastle-7b133980-wave-13-3

Labels: bug, ready-for-agent

**Finding ID:** `D-36ccf3e367566848` (`birth.dna-auto-reincarnation-semantics`).

**Production seam for TDD:** публичный `sv_endpoint_run(SvEndpointOptions)` получает доступные DNA identities и реальный `%` SDL key event; native birth adapter создаёт semantic automatic-DNA command, а production `SvBirth`/input router ведёт его через owners. Outcome и socket boundary наблюдают auto state, owner/highlights/draft и отсутствие send. Seam принят P1-AC02, P1-AC03–P1-AC10 и P1-AC28; проверка — `sv-core`.

- [x] `%` распознаётся только в active birth input context как semantic automatic-DNA command и включает `auto_reincarnation`; команда проходит generation/active validation до любой state mutation. [Contract: P1-AC10, P1-AC22, P1-AC28, P2-AC04; checks: `sv-core`, `controller-review`]
- [x] Automatic mode последовательно применяет legal DNA для каждого доступного sex/class/race/trait/body/stats/mode owner, сохраняя те же compatibility, stable identity и confirmation semantics, что одиночный `#`. [Contract: P1-AC03, P1-AC04, P1-AC05, P1-AC06, P1-AC07, P1-AC08, P1-AC09, P1-AC10, P1-AC26; checks: `sv-core`]
- [x] Missing, stale или incompatible DNA безопасно выключает automatic continuation на текущем owner, не подтверждает illegal value и оставляет согласованные highlight/draft для ручного keyboard/mouse продолжения. [Contract: P1-AC02, P1-AC10, P1-AC11, P2-AC04; checks: `sv-core`]
- [x] Production-path scenarios проверяют полный `%` flow, остановку и ручное продолжение в обеих compile configurations, включая unavailable trait/body и dedicated IDDC/PvP branches; никакая ветвь не использует fixed-response/test-only wizard. [Contract: P1-AC19, P1-AC20, P1-AC28, P2-AC05; checks: `sv-build`, `sv-core`]
- [x] Automatic DNA меняет только текущий local draft: `birth.restore-dna`/`birth.save-dna`, DNA persistence, partial/completed send и ownership SV-B-024 не добавляются. [Contract: P1-AC02, P1-AC26, P1-AC27, P1-AC29; checks: `sv-core`, `controller-review`]
- [x] Реализация остаётся SV-local и сохраняет behavior baseline без расширения shared/legacy client. [Contract: P2-AC01, P2-AC02, P2-AC04, P2-AC09; checks: `controller-review`]

**Deferred check:** `P1-birth-runtime-evidence` / P1-AC30 остаётся deferred и не считается пройденным этим implementation ticket; evidence собирается только на full acceptance для exact accepted contract digest и HEAD.

## Answer

Production seam: публичный `sv_endpoint_run(SvEndpointOptions)` получает
authoritative decoded setup и DNA identities; реальный SDL `%` event проходит
через `sv_native_birth_event()` как semantic `SV_BIRTH_AUTOMATIC_DNA`, затем
production `sv_birth_command()` последовательно применяет существующую
`SV_BIRTH_DNA` команду. Поэтому automatic flow использует ровно те же legal-list,
compatibility, hidden-derived identity и confirmation rules, что одиночный `#`.
`SvEndpointOutcome` наблюдает auto state, owners, highlights и local draft.

Generation/active/quit и допустимый birth owner проверяются до включения
`auto_reincarnation`. Каждый успешный DNA confirmation переводит interaction к
следующему production owner; `prepare_traits()` и `prepare_bodies()` по-прежнему
единственные владельцы unavailable-step skipping. Missing или illegal identity
останавливает цикл на текущем owner через существующий DNA reject path, выключает
auto state и не меняет legal highlight/draft. Keyboard input после остановки
продолжает тот же interaction. Dedicated IDDC и PvP Maia flow также проходят этот
router; после PvP mode automatic continuation применяет DNA к post-mode trait.

Изменения ограничены SV: добавлены semantic command, SDL binding, outcome evidence
и подсказки `% automatic DNA`. Legacy/shared gameplay code не менялся. Managed
peer после единственного разрешённого name packet требует EOF, поэтому все
automatic, stop и manual-continuation scenarios доказывают отсутствие partial или
completed-character send. Persistence, `birth.restore-dna`, `birth.save-dna` и
ownership SV-B-024 не добавлялись.

TDD red → green через согласованный production seam:

- `SV_BIRTH_CASE=auto-dna-full python3 -B tests/sv_name_editor_submit_checks.py`
  — **RED**, exit 1: после реального `%` interaction оставался на sex owner, и
  assertion ожидания `SV_BIRTH_READY` падал. После semantic command и production
  owner continuation та же команда — **GREEN**, exit 0 для default и
  `SV_CLASS_BEFORE_RACE=0` binaries.
- Затем тем же seam добавлены focused cases
  `auto-dna-incompatible-class`, `auto-dna-missing-sex`,
  `auto-dna-stale-event`, `auto-dna-skip-trait-body`,
  `auto-dna-dedicated-iddc` и `auto-dna-dedicated-pvp-maia`. Они проверяют safe
  stop, ручное продолжение, отсутствие mutation от stale event, unavailable
  skipping и dedicated/post-mode branches в обеих compile configurations.

Focused verification, 2026-10-07:

- `python3 -B tests/sv_name_editor_submit_checks.py` — **PASS**, exit 0; полная
  production endpoint birth matrix для обеих compile configurations, собранная с
  ASan/UBSan, включая новые automatic cases и весь ранее принятый baseline.
- `git diff --check` — **PASS**, exit 0.

Полный orchestrator build/test/review workflow не запускался. Новых improvement
opportunities за пределами текущей SV-local реализации не обнаружено.
`P1-birth-runtime-evidence` остаётся **deferred; not run**: по контракту evidence
собирается только на full acceptance для exact digest
`690a04cefb7fd0e1615dce46d1a538e1dcbd856232af1a1373be93f987095c82` и exact
тогдашнего HEAD.
