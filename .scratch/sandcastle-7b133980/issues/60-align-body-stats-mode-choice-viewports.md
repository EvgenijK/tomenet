# 60: Синхронизировать viewport выбора body, stats и mode

**Type:** repair

**Tracker role:** Wayfinder planning/execution history for canonical [SV-B-023](../../../docs/tasks/stage-b/SV-B-023-birth-choices.md); this child does not own Stage B capability IDs.

**What to build:** После уменьшения native surface игрок видит тот же bounded slice server-provided body, stats или mode choices, который использует production hit testing. Wheel и keyboard navigation делают поздние identities видимыми; click выбирает identity отображённой строки, double-click подтверждает её, а resize и interleaved events сохраняют stable highlight без partial/completed character send.

**Blocked by:** 29: Сделать длинные birth choice lists полностью достижимыми (completed).

**Status:** resolved

**Assignee:** codex/sandcastle-7b133980-wave-40-1

Labels: bug, ready-for-agent

**Finding ID:** `D-1032de8ff6bb0776` (`birth.choice-list-viewport-reachability`), reopened exact-HEAD recurrence after the earlier class/race/trait viewport repair; this ticket is the single coherent repair for body, stats and mode presentation/hit-testing alignment.

**Production seam for TDD:** публичный `sv_endpoint_run(SvEndpointOptions)` получает authoritative server-provided body, stats и mode choices; реальные resize, wheel, keyboard, click и double-click SDL events проходят native birth adapter и public production birth-interaction/input-router boundary, а production scene и hit testing читают один bounded viewport. `SvEndpointOutcome` и controlled peer наблюдают owner, highlighted identity, committed draft и отсутствие запрещённых outbound bytes. Seam принят P1-AC02, P1-AC07, P1-AC08, P1-AC09, P1-AC10, P1-AC11 и P1-AC28; проверки — `sv-core` и `sv-build`.

- [x] Для каждого owner body, stats и mode red production-path case после resize до малой высоты, wheel scroll и click по первой отображённой строке воспроизводит несовпадение identity; green surface рисует только slice `first..first+rows`, использует row geometry относительно `first` и выбирает именно показанную server identity. [Contract: P1-AC02, P1-AC07, P1-AC08, P1-AC09, P1-AC10, P1-AC11, P2-AC04, P2-AC05; checks: `sv-core`]
- [x] Double-click по отображённой legal строке каждого из трёх owners подтверждает ту же identity и продвигает production interaction; clipped или неотображённая строка не становится mouse target и не меняет owner, highlight или draft. [Contract: P1-AC02, P1-AC07, P1-AC08, P1-AC09, P1-AC10, P1-AC11, P1-AC28, P2-AC04, P2-AC05; checks: `sv-core`]
- [x] Keyboard navigation и repeated resize/wheel сохраняют stable highlighted identity видимой; interleaved network, focus и resize events не меняют owner, selection или draft, а server-provided ordering и legal/illegal/random/DNA semantics остаются прежними. [Contract: P1-AC07, P1-AC08, P1-AC09, P1-AC10, P1-AC11, P1-AC21, P1-AC26, P2-AC04, P2-AC05; checks: `sv-core`, `controller-review`]
- [x] Existing class/race/trait viewport scenarios, both compile-time class/race orders, Backspace, Q/Ctrl-Q, disconnect и stale-generation controls остаются green и не отправляют partial/completed character result; final completed-character send остаётся у SV-B-024. [Contract: P1-AC02, P1-AC04, P1-AC05, P1-AC06, P1-AC12, P1-AC16, P1-AC17, P1-AC18, P1-AC19, P1-AC22, P1-AC29; checks: `sv-build`, `sv-core`]
- [x] Repair и regressions остаются по возможности SV-local, сохраняют interaction ownership вне renderer, используют только production input-router path и не добавляют test-only choice rendering или hit testing; обнаруженные unrelated improvements записываются отдельно. [Contract: P1-AC28, P2-AC01, P2-AC02, P2-AC03, P2-AC05, P2-AC06, P2-AC07, P2-AC08, P2-AC09; checks: `controller-review`]

**Deferred check:** `P1-birth-runtime-evidence` / P1-AC30 остаётся deferred у named full-acceptance producer/verifier и не считается пройденным этим implementation repair; evidence собирается только на exact accepted contract digest `690a04cefb7fd0e1615dce46d1a538e1dcbd856232af1a1373be93f987095c82` и HEAD.

## Answer

Production seam: публичный `sv_endpoint_run(SvEndpointOptions)` получает
authoritative server setup от controlled peer. SDL resize, focus, wheel,
PageDown, click и double-click events проходят native birth adapter и public
production birth-interaction/input-router boundary. `SvEndpointOutcome` наблюдает
owner, stable highlighted identity и committed draft; controlled peer проверяет
отсутствие любых partial/completed birth bytes. Отдельных test-only rendering,
hit testing или choice engine нет.

Body, ordered stats и mode renderer теперь читают тот же `SvBirthViewport`, что
и native hit testing: каждый surface рисует только bounded slice
`first..first+rows`, а координата строки вычисляется относительно `first` через
общие `SV_BIRTH_CHOICE_*` constants. PageUp/PageDown production navigation
расширена на body, stats и mode и разрешает viewport position обратно в exact
server-provided identity. Interaction ownership и legality checks остаются в
birth interaction/input router.

TDD evidence через согласованный production seam, отдельными vertical slices:

- RED body:
  `SV_BIRTH_CASE=body-viewport python3 -B tests/sv_name_editor_submit_checks.py`
  завершился с exit 1 на ожидаемой `SV_BIRTH_BODY_FRUIT_BAT`: PageDown не
  обрабатывался для body и double-click подтверждал первую identity.
- GREEN body: та же команда завершилась с exit 0 в обеих compile configurations;
  первая отображённая строка подтвердила `Fruit bat`, clipped row была
  проигнорирована, interleaved ping/focus/resize сохранили draft.
- RED stats:
  `SV_BIRTH_CASE=stats-viewport python3 -B tests/sv_name_editor_submit_checks.py`
  завершился с exit 1 на `birth_stats[0] == 1`: подтверждалась stat identity 0.
- GREEN stats: та же команда завершилась с exit 0 в обеих compile configurations;
  double-click подтвердил displayed stat identity 1, оставив ordered stats owner
  с ожидаемыми пятью legal identities.
- RED mode:
  `SV_BIRTH_CASE=mode-viewport python3 -B tests/sv_name_editor_submit_checks.py`
  завершился с exit 1 на `SV_BIRTH_MODE_NO_GHOST`: подтверждался `Normal`.
- GREEN mode: та же команда завершилась с exit 0 в обеих compile configurations;
  double-click подтвердил displayed `No Ghost` и продвинул owner в ready.

Во всех трёх green cases wheel прошёл вниз и обратно, repeated small resize и
focus events прошли до и после keyboard navigation, clipped double-click не стал
target, а controlled peer подтвердил единственный обязательный name request и
закрытие без birth result bytes.

Focused verification:

- `SV_BIRTH_CASE=<body-viewport|stats-viewport|mode-viewport> python3 -B tests/sv_name_editor_submit_checks.py` — PASS для всех трёх cases в обеих compile configurations.
- Existing `long-class-viewport`, `long-race-viewport` и `long-trait-viewport` тем же focused runner — PASS в обеих compile configurations.
- Existing `session-interleave`, `session-stale-input` и `session-disconnect` тем же focused runner — PASS; disconnect diagnostics ожидаемы, дальнейших sends нет.
- `make -s -C src -f makefile.sv tomenet-sv` — PASS.
- `git diff --check` — PASS.

Полный `sv-core` и controller review не запускались по указанию orchestrator.
`P1-birth-runtime-evidence` / P1-AC30 остаётся deferred и здесь не объявлен
пройденным. Изменения ограничены SV production renderer/router и его endpoint
tests; legacy/shared gameplay code не менялся. Известные build warning про
common `atol` и SDL `Leaked thread` diagnostics уже записаны в
`docs/sv-improvements.md`; новых unrelated improvement opportunities не найдено.
