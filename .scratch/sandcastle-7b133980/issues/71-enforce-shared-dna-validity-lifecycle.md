# 71: Сохранить общий lifecycle валидности birth DNA

**Type:** repair

**Tracker role:** Wayfinder planning/execution history for canonical [SV-B-023](../../../docs/tasks/stage-b/SV-B-023-birth-choices.md); this child does not own Stage B capability IDs.

**What to build:** Загруженный birth DNA outcome имеет одну общую валидность на протяжении production interaction. Пока outcome валиден, staged `#` и automatic `%` применяют его legal server-provided компоненты как раньше. Если обязательный компонент отсутствует или illegal для текущего birth owner, весь outcome становится невалидным: ручное продвижение, Backspace и повторный вход в owner не позволяют смешать draft игрока с более поздними DNA-фрагментами, а последующие `#`/`%` отклоняются без изменения owner, highlighted identity или draft.

**Blocked by:** 38: Отклонить новый automatic DNA command в ordered stats (completed).

**Status:** resolved

**Assignee:** codex/sandcastle-7b133980-wave-48-1

Labels: bug, ready-for-agent

**Finding ID:** `D-e6f756264f5a2d20` (`birth.dna-validity-lifecycle`), one coherent repair of the shared loaded-DNA validity lifecycle across every birth owner.

**Production seam for TDD:** публичный `sv_endpoint_run(SvEndpointOptions)` получает один authoritative loaded DNA outcome и server-provided birth setup; реальные SDL `#`, `%`, manual confirm и Backspace events проходят native birth adapter и публичную production birth-interaction/input-router boundary. `SvEndpointOutcome` и controlled peer наблюдают общую DNA availability, active owner, stable highlighted identity, committed/local draft, automatic state и отсутствие запрещённых outbound bytes. Seam принят P1-AC02, P1-AC03–P1-AC10, P1-AC13–P1-AC19, P1-AC26 и P1-AC28; проверки — `sv-core` и `sv-build`.

- [x] Red production-path sequence начинает с outcome, где sex DNA доступен, но ранний обязательный class/race/trait component отсутствует или illegal; первая попытка `#` либо automatic `%` на этом owner отклоняется и атомарно делает общий DNA outcome невалидным, выключая automatic continuation без изменения owner, highlighted identity или draft. [Contract: P1-AC02, P1-AC03, P1-AC04, P1-AC05, P1-AC06, P1-AC10, P1-AC26, P1-AC28, P2-AC04, P2-AC05; checks: `sv-core`]
- [x] После такой invalidation игрок может продолжить вручную и использовать Backspace, но на каждом позднее достигнутом или повторно достигнутом owner staged `#` и новый automatic `%` остаются недоступными и не меняют owner, stable highlighted identity, automatic state или любую часть local/committed draft; ни один сохранённый per-component value не восстанавливает общую валидность в том же interaction. [Contract: P1-AC02, P1-AC04, P1-AC05, P1-AC06, P1-AC07, P1-AC08, P1-AC09, P1-AC10, P1-AC13, P1-AC14, P1-AC15, P1-AC16, P1-AC17, P1-AC18, P1-AC26, P2-AC04, P2-AC05; checks: `sv-core`]
- [x] Production regressions доказывают invalidation-before-manual-advance и invalidation-before-backtrack/re-entry в default class-before-race и alternate race-before-class configurations; controls с полностью legal aggregate сохраняют staged и automatic DNA semantics, включая unavailable-step skipping, ordered stats suspension/resume, dedicated IDDC/PvP и post-mode PvP Maia flow. [Contract: P1-AC02, P1-AC03, P1-AC04, P1-AC05, P1-AC06, P1-AC07, P1-AC08, P1-AC09, P1-AC10, P1-AC19, P1-AC20, P1-AC26, P1-AC28, P2-AC04, P2-AC05; checks: `sv-build`, `sv-core`]
- [x] Начальная общая availability и её необратимая invalidation принадлежат production `SvBirth` interaction и публикуются через существующую production view/outcome boundary; per-owner availability/value остаются данными загруженного outcome, но больше не являются независимыми разрешениями. Отдельный typed DNA draft/API aggregate из ранее записанного improvement остаётся future cleanup и не требуется для этого behavior repair. [Contract: P1-AC02, P1-AC03, P1-AC04, P1-AC05, P1-AC06, P1-AC07, P1-AC08, P1-AC09, P1-AC26, P1-AC28, P2-AC01, P2-AC02, P2-AC03, P2-AC08, P2-AC09; checks: `sv-core`, `controller-review`]
- [x] Quit, disconnect и stale-generation guards продолжают подавлять любые дальнейшие sends; repair не добавляет DNA persistence, `birth.restore-dna`, `birth.save-dna`, partial/completed-character send, fixed-response wizard или test-only behavior implementation, а legacy/shared production behavior остаётся неизменным. [Contract: P1-AC02, P1-AC12, P1-AC22, P1-AC26, P1-AC27, P1-AC28, P1-AC29, P2-AC01, P2-AC02, P2-AC03, P2-AC04, P2-AC05, P2-AC09; checks: `sv-core`, `controller-review`]
- [x] Existing focused production matrix и exact implementation gates остаются green; новые unrelated improvement opportunities только записываются с обязательными полями и проверками, не включаются в repair. [Contract: P1-AC21, P1-AC24, P1-AC25, P1-AC28, P2-AC06, P2-AC07, P2-AC08; checks: `sv-build`, `sv-core`, `controller-review`]

**Deferred check:** `P1-birth-runtime-evidence` / P1-AC30 остаётся deferred у named full-acceptance producer/verifier и не считается пройденным этим implementation repair; scenario-indexed native evidence собирается только на exact accepted contract digest `690a04cefb7fd0e1615dce46d1a538e1dcbd856232af1a1373be93f987095c82` и exact accepted HEAD.

## Comments

- 2026-10-09: Claimed by `codex/sandcastle-7b133980-wave-48-1`. TDD seam is the ticket-approved public `sv_endpoint_run(SvEndpointOptions)` path through authoritative loaded DNA/setup, native SDL input and the production birth interaction/input router.

## Answer

Исправление выполнено внутри SV production birth interaction. `SvBirth` теперь
владеет одной общей availability загруженного DNA outcome: любой отсутствующий
или illegal обязательный component необратимо сбрасывает её и automatic
continuation. Центральный guard отклоняет все последующие staged `#` и automatic
`%` до owner-local обработки, поэтому сохранённый legal component больше не может
смешаться с ручным draft после advance, Backspace или re-entry. Per-component
значения и availability сохранены как загруженные данные. Общая availability
публикуется как `SvEndpointOutcome.birth_dna_available` через существующую
production outcome boundary.

Production seam: публичный `sv_endpoint_run(SvEndpointOptions)`. Controlled peer
передаёт authoritative setup и loaded DNA; реальные SDL `%`, `#`, Enter,
Backspace и Quit проходят native adapter и production interaction/input router.
`SvEndpointOutcome` наблюдает общую availability, owner, stable highlight,
local/committed draft и automatic state. Peer принимает только разрешённый name
packet и требует EOF, поэтому partial/completed-character send провалил бы
scenarios. Test-only behavior path не добавлен; legacy/shared production files
не изменялись.

TDD red → green, 2026-10-09:

- `SV_BIRTH_CASE=auto-dna-shared-manual-advance python3 -B tests/sv_name_editor_submit_checks.py`
  — **RED**, exit 1: после invalid early class/race component более поздний legal
  DNA component продолжал interaction и менял owner/automatic state; после
  общего lifecycle guard — **GREEN**, exit 0 в default и
  `SV_CLASS_BEFORE_RACE=0` configurations.
- `SV_BIRTH_CASE=auto-dna-shared-backtrack-reentry python3 -B tests/sv_name_editor_submit_checks.py`
  — **GREEN**, exit 0 в обеих configurations: после invalidation и Backspace
  повторный `#` и `%` не применяют ранее legal component, не меняют owner,
  highlight или committed/local draft, availability остаётся false.

Focused verification:

- `auto-dna-full`, `auto-dna-ordered-keyboard`, `auto-dna-skip-trait-body`,
  `auto-dna-dedicated-iddc`, `auto-dna-dedicated-pvp-maia`,
  `auto-dna-incompatible-class` и `auto-dna-ordered-mode-missing` через
  `tests/sv_name_editor_submit_checks.py` — **PASS**, exit 0 каждый в default и
  alternate configurations. Покрыты fully legal aggregate, unavailable-step
  skipping, ordered stats suspension/resume, dedicated IDDC/PvP, post-mode PvP
  Maia и existing invalid-DNA controls.
- Повторный `auto-dna-full` после assertion общей valid availability — **PASS**,
  exit 0 в обеих configurations.
- `git diff --check` — **PASS**.

Полные `sv-build`, `sv-core`, controller review и общий build/test/review workflow
не запускались по инструкции: их выполняет orchestrator после assembly. Focused
runner с `-Wall -Wextra -Werror` и ASan/UBSan скомпилировал и выполнил изменённый
production path в обеих compile configurations. Диагностики declared font
fallback и SDL `Leaked thread` совпадают с существующим focused-runner output;
compiler/sanitizer failures в green runs не было. Новых побочных improvement
opportunities не обнаружено. `P1-birth-runtime-evidence` / P1-AC30 остаётся
**deferred; not run** до full acceptance на exact contract digest и HEAD.
