# 58: Сохранить identities видимого class list

**Type:** repair

**Tracker role:** Wayfinder planning/execution history for canonical [SV-B-023](../../../docs/tasks/stage-b/SV-B-023-birth-choices.md); this child does not own Stage B capability IDs.

**What to build:** В race-before-class flow class surface сохраняет полный server-visible class list и показывает несовместимые с выбранной race identities disabled вместо удаления. Keyboard letters, mouse rows и viewport positions продолжают обозначать стабильные visible identities: прямой выбор incompatible class отклоняется, тогда как navigation и random обходят только legal entries, а legal confirm продвигает тот же authoritative draft без partial/completed send.

**Blocked by:** 06: Сохранить alternate class/race compile order (completed); 29: Сделать длинные birth choice lists полностью достижимыми (completed).

**Status:** resolved

**Assignee:** codex/sandcastle-7b133980-wave-38-2

Labels: bug, ready-for-agent

**Finding ID:** `D-9dd37219e6bb4866` (`birth.class-list-illegal-choice-identity`), one coherent repair of visible class presentation, stable input mapping and compatibility rejection.

**Production seam for TDD:** публичный `sv_endpoint_run(SvEndpointOptions)` получает authoritative server-provided class/race setup, включая incompatible visible class перед compatible class; реальные keyboard, mouse, paging/scroll, random, confirm и Backspace SDL events проходят native birth adapter и публичную production birth-interaction/input-router boundary, а production scene читает тот же class view. `SvEndpointOutcome` и controlled peer наблюдают visible/legal identities, owner, stable highlight, draft и отсутствие запрещённых outbound bytes. Seam принят P1-AC02, P1-AC06, P1-AC10, P1-AC11, P1-AC19, P1-AC21 и P1-AC28; проверки — `sv-core` и `sv-build`.

- [x] Red alternate-order production case помещает incompatible visible class перед compatible class и воспроизводит сжатие list/letter identities; green surface сохраняет server-visible order, помечает incompatibility disabled и не перенумеровывает последующие keyboard, mouse или viewport positions. [Contract: P1-AC02, P1-AC06, P1-AC10, P1-AC11, P1-AC19, P2-AC04, P2-AC05; checks: `sv-build`, `sv-core`]
- [x] Baseline letter и mouse click по disabled row разрешаются в ту же visible server identity и отклоняются без изменения owner, highlighted legal identity или draft; double-click не подтверждает incompatible class, а letter/mouse по следующей compatible identity выбирают именно её. [Contract: P1-AC02, P1-AC06, P1-AC10, P1-AC11, P1-AC19, P2-AC04, P2-AC05; checks: `sv-core`]
- [x] Previous/next, paging/scroll и random перемещаются только между legal classes, удерживают stable highlighted identity видимой и используют общий bounded viewport с renderer/hit testing; focus, resize и interleaved network events не меняют owner или selection. [Contract: P1-AC06, P1-AC10, P1-AC11, P1-AC21, P1-AC28, P2-AC04, P2-AC05; checks: `sv-core`, `controller-review`]
- [x] Legal confirm в alternate order коммитит выбранную compatible class и продвигает flow; default class-before-race order, DNA semantics, Backspace, Q/Ctrl-Q, disconnect и stale-generation controls остаются green и не отправляют partial/completed character result, а final send остаётся у SV-B-024. [Contract: P1-AC02, P1-AC06, P1-AC10, P1-AC12, P1-AC15, P1-AC19, P1-AC22, P1-AC26, P1-AC29; checks: `sv-build`, `sv-core`]
- [x] Repair и regressions остаются по возможности SV-local, сохраняют interaction ownership вне renderer, используют production input-router path и не добавляют fixed-response либо test-only class mapping; обнаруженные unrelated improvements записываются отдельно. [Contract: P1-AC28, P2-AC01, P2-AC02, P2-AC03, P2-AC05, P2-AC06, P2-AC07, P2-AC08, P2-AC09; checks: `controller-review`]

**Deferred check:** `P1-birth-runtime-evidence` / P1-AC30 остаётся deferred у named full-acceptance producer/verifier и не считается пройденным этим implementation repair; evidence собирается только на exact accepted contract digest `690a04cefb7fd0e1615dce46d1a538e1dcbd856232af1a1373be93f987095c82` и HEAD.

## Answer

Production seam: публичный `sv_endpoint_run(SvEndpointOptions)` получает
authoritative setup от managed peer. Реальные SDL keyboard, mouse, wheel,
PageUp/PageDown, focus и resize events проходят `sv_native_birth_event()` через
production endpoint/input-router path; production scene и hit testing читают
один `SvBirthView` и один bounded viewport. `SvEndpointOutcome` теперь отдельно
наблюдает server-visible и legal class identities. Отдельного test-only choice
engine или class mapping нет.

В alternate race-before-class flow interaction сохраняет все non-hidden class
identities в server order в `visible_classes`, а совместимое подмножество — в
`legal_classes`. Renderer показывает несовместимые строки disabled. Letter,
mouse row и viewport position разрешаются через visible list, поэтому disabled
identity не сжимает последующие позиции; semantic select повторно проверяет
compatibility и отклоняет disabled row без смены owner, legal highlight или
draft. Previous/next и random по-прежнему используют legal list, а paging ищет
legal target поверх visible viewport. Default class-before-race list сохраняет
прежнюю семантику.

TDD evidence через согласованный production seam:

- RED letter slice:
  `SV_BIRTH_CASE=alternate-visible-class-letter python3 -B tests/sv_name_editor_submit_checks.py`
  завершился с exit 1 на `outcome.birth_illegal_ignored`: compressed mapping
  ошибочно разрешал `A` сразу в compatible identity 2.
- GREEN letter slice: та же команда завершилась с exit 0; outcome наблюдает
  visible `[0, 2, 3]`, legal `[2]`, owner class, highlight 2 и неизменённый
  uncommitted draft после отклонения identity 0.
- RED mouse slice:
  `SV_BIRTH_CASE=alternate-visible-class-mouse python3 -B tests/sv_name_editor_submit_checks.py`
  завершился с exit 1 на `birth_illegal_ignored && birth_mouse_navigated`:
  double-click первой disabled row преждевременно подтверждал compressed legal
  row.
- GREEN mouse slice: та же команда завершилась с exit 0; disabled double-click
  отклонён, следующая visible compatible row подтверждает exact identity 2 и
  переводит того же authoritative draft в stats.
- GREEN viewport/interleave slice:
  `SV_BIRTH_CASE=alternate-visible-class-viewport python3 -B tests/sv_name_editor_submit_checks.py`
  завершился с exit 0 для 20 visible identities и legal `[0, 15]`:
  PageUp/PageDown обходят disabled rows, wheel/mouse сохраняют stable highlight,
  focus/resize и production ping не меняют owner/selection.

Focused verification:

- `SV_BIRTH_CASE=long-class-viewport python3 -B tests/sv_name_editor_submit_checks.py` — PASS в обеих compile configurations.
- Alternate `navigate`, `random`, legal confirm/order, Backspace и incompatible DNA focused cases — PASS.
- `session-interleave`, `session-stale-input` и `session-disconnect` focused cases — PASS в применимых compile configurations; managed peer не получил partial/completed birth bytes.
- `make -s -C src -f makefile.sv tomenet-sv` — PASS.
- `make -s -C src -f makefile.sv SV_CLASS_BEFORE_RACE=0 tomenet-sv` — PASS.
- `git diff --check` — PASS.

Полный `sv-core` и controller review не запускались по указанию orchestrator.
`P1-birth-runtime-evidence` / P1-AC30 остаётся deferred и здесь не объявлен
пройденным. Legacy/shared gameplay code не менялся. Build warning про obsolete
common `atol` declaration и SDL `Leaked thread` diagnostics уже записаны в
`docs/sv-improvements.md`; новых unrelated improvement opportunities не
обнаружено.
