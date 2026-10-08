# 52: Принять trait-derived hidden classes из DNA

**Type:** repair

**Tracker role:** Wayfinder planning/execution history for canonical [SV-B-023](../../../docs/tasks/stage-b/SV-B-023-birth-choices.md); this child does not own Stage B capability IDs.

**What to build:** Staged и automatic DNA принимают legal server-provided Hell Knight и Corrupted Priest, показывают соответствующий visible base class и продолжают через независимые Maia race и Corrupted trait choices. Полный draft сохраняет согласованную комбинацию base class/race/trait, из которой authoritative final handling выводит исходную hidden class, в обеих compile-time class/race order configurations; несовместимая комбинация безопасно останавливает DNA continuation без изменения уже подтверждённого draft и без partial или completed send.

**Blocked by:** 21: Сохранить hidden DNA class до подтверждения race (completed); 19: Продолжить automatic DNA choices через birth flow (completed).

**Status:** resolved

**Assignee:** codex/sandcastle-7b133980-wave-35-1

Labels: bug, ready-for-agent

**Finding ID:** `D-6ea1c7fe7649ba64` (`birth-hidden-dna-trait-derived-class-rejected`), one coherent repair of trait-derived hidden-class normalization and continuation.

**Production seam for TDD:** публичный `sv_endpoint_run(SvEndpointOptions)` получает authoritative server-provided class/race/trait setup и DNA identities; реальные staged/automatic DNA SDL events проходят native birth adapter и публичную production birth-interaction/input-router boundary. `SvEndpointOutcome` и controlled peer наблюдают owner, legal identities, stable highlight, authoritative draft и отсутствие запрещённых outbound bytes. Seam принят P1-AC02, P1-AC04, P1-AC05, P1-AC06, P1-AC10, P1-AC26 и P1-AC28; проверки — `sv-core` и `sv-build`.

- [x] Red production-path cases для Hell Knight и Corrupted Priest показывают текущий reject на class owner; green cases нормализуют каждую server-provided hidden identity к её visible base без требования, чтобы trait-derived hidden identity присутствовала в race choice bits. [Contract: P1-AC02, P1-AC06, P1-AC10, P1-AC26, P2-AC04, P2-AC05; checks: `sv-core`]
- [x] Staged и automatic DNA независимо применяют и валидируют Maia race и Corrupted trait, сохраняют stable highlighted identities и согласованный base-class/race/trait draft, из которого authoritative final handling выводит исходную hidden identity; wrong race или trait останавливает automatic continuation и не подменяет подтверждённый draft. [Contract: P1-AC02, P1-AC04, P1-AC05, P1-AC06, P1-AC10, P1-AC26, P2-AC04; checks: `sv-core`]
- [x] Оба trait-derived класса проходят production interaction в default и alternate compile-time class/race orders; race-derived hidden class, ordinary visible class, manual и random control cases сохраняют ранее проверенную compatibility семантику. [Contract: P1-AC04, P1-AC05, P1-AC06, P1-AC10, P1-AC19, P1-AC26, P1-AC28, P2-AC04, P2-AC05; checks: `sv-build`, `sv-core`]
- [x] Accept, reject, Backspace, Q/Ctrl-Q, disconnect и stale-generation paths не выполняют DNA persistence и не отправляют partial/completed character result; `birth.restore-dna`/`birth.save-dna` не вводятся, а final send остаётся у SV-B-024. [Contract: P1-AC02, P1-AC12, P1-AC22, P1-AC26, P1-AC27, P1-AC29; checks: `sv-core`, `controller-review`]
- [x] Repair и regression остаются по возможности SV-local, используют production interaction/input-router path и не добавляют test-only derivation; обнаруженные unrelated improvements записываются отдельно. [Contract: P1-AC28, P2-AC01, P2-AC02, P2-AC03, P2-AC05, P2-AC06, P2-AC07, P2-AC08, P2-AC09; checks: `controller-review`]

**Deferred check:** `P1-birth-runtime-evidence` / P1-AC30 остаётся deferred у named full-acceptance producer/verifier и не считается пройденным этим implementation repair; evidence собирается только на exact accepted contract digest и HEAD.

## Answer

Production seam: публичный `sv_endpoint_run(SvEndpointOptions)` получает
authoritative server-provided class/race/trait setup и DNA identities. Реальные
SDL `#`/`%` события проходят native birth adapter и production
`sv_birth_command()`. `SvEndpointOutcome` наблюдает owner, legal identities,
stable highlights и authoritative draft; controlled peer после единственного
разрешённого name packet требует EOF, поэтому любой partial/completed birth send
ломает сценарий.

SV birth interaction теперь отличает server-marked hidden `Hell Knight` и
`Corrupted Priest` от race-derived hidden identities. Их DNA identity
нормализуется к visible Paladin или Priest без hidden bit в race choice mask. В
class-before-race flow pending hidden identity ограничивает следующий выбор
server-provided Maia race и затем Corrupted trait; draft коммитит visible base,
Maia и Corrupted. В alternate order уже подтверждённые Maia и Corrupted
проверяются при class DNA. Эта согласованная тройка остаётся authoritative input
для окончательной server-side derivation hidden class. Race-derived hidden
resolution по race bits не менялась.

Wrong race или wrong trait выключают automatic continuation на текущем owner и
не изменяют ранее подтверждённые поля. Default order сохраняет подтверждённый
visible class (и Maia перед wrong-trait reject); alternate order сохраняет
подтверждённую race при wrong-race reject либо race/trait/body перед
wrong-trait class reject. DNA persistence, restore/save capabilities и final
send не добавлялись. Изменены только SV birth interaction, его production-path
test peer и focused runner matrix; legacy/shared production client остался
неизменным.

TDD red → green:

- `SV_BIRTH_CASE=trait-hidden-dna-hell-knight python3 tests/sv_name_editor_submit_checks.py`
  — RED, exit 1: production flow остался до stats и assertion
  `birth_owner == SV_BIRTH_STATS` упал; после trait-derived normalization и
  независимой Maia/Corrupted validation — GREEN, exit 0 в обеих compile
  configurations.
- `SV_BIRTH_CASE=trait-hidden-dna-corrupted-priest python3 tests/sv_name_editor_submit_checks.py`
  — RED, exit 1 на том же production owner assertion; после добавления второй
  legal server identity — GREEN, exit 0 в обеих compile configurations.
- Automatic cases добавлены после staged implementation как coverage уже
  исправленного continuation path; они не объявляются отдельным red-first
  циклом.

Focused verification, 2026-10-08:

- `trait-hidden-dna-hell-knight`, `trait-hidden-dna-corrupted-priest`,
  `auto-dna-trait-hidden-hell-knight`,
  `auto-dna-trait-hidden-corrupted-priest`,
  `auto-dna-trait-hidden-wrong-race` и
  `auto-dna-trait-hidden-wrong-trait` через
  `SV_BIRTH_CASE=<case> python3 tests/sv_name_editor_submit_checks.py` — PASS,
  exit 0 в default и `SV_CLASS_BEFORE_RACE=0` binaries под ASan/UBSan.
- Existing controls `hidden-derived-dna`, `auto-dna-full` и
  `alternate-random` тем же focused runner — PASS, exit 0.
- `git diff --check` — PASS.

Полный orchestrator build/test/review workflow и отдельный `sv-build` не
запускались. `P1-birth-runtime-evidence` / P1-AC30 остаётся deferred и не
объявляется выполненным. Повторяющиеся font-fallback/SDL leaked-thread
diagnostics уже существовали; новых unrelated improvement opportunities не
обнаружено.
