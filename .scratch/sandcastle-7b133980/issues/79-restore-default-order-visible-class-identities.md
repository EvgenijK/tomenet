# 79: Восстановить identities class list в default order

**Type:** repair

**Tracker role:** Wayfinder planning/execution history for canonical [SV-B-023](../../../docs/tasks/stage-b/SV-B-023-birth-choices.md); this child does not own Stage B capability IDs.

**What to build:** В default class-before-race flow игрок видит каждый non-hidden server-provided class в исходном server order, включая пока несовместимые identities. Несовместимая строка остаётся disabled и не сдвигает letters, mouse rows или viewport positions последующих classes; direct selection отклоняется без изменения owner, highlighted legal identity или draft, тогда как navigation и random продолжают выбирать только legal identities и legal confirm продвигает birth flow без partial/completed send.

**Blocked by:** 58: Сохранить identities видимого class list (completed).

**Status:** resolved

**Assignee:** codex/sandcastle-7b133980-wave-56-1

Labels: bug, ready-for-agent

**Finding ID:** `D-9dd37219e6bb4866` (`birth.class-list-illegal-choice-identity`), reopened default-order portion of the same coherent visible-class presentation and input-mapping repair; do not create a second unrelated implementation of this defect.

**Production seam for TDD:** публичный `sv_endpoint_run(SvEndpointOptions)` получает authoritative server-provided class setup с incompatible visible class перед compatible class. Реальные keyboard, mouse, navigation, random и confirm SDL events проходят native birth adapter и публичную production birth-interaction/input-router boundary, а production scene читает тот же class view. `SvEndpointOutcome` и controlled peer наблюдают visible/legal identities, owner, stable highlight, authoritative draft и отсутствие запрещённых outbound bytes. Seam принят P1-AC02, P1-AC06, P1-AC10, P1-AC11, P1-AC19 и P1-AC28; проверки — `sv-core` и `sv-build`.

- [x] Red default-order production case с incompatible non-hidden class перед compatible class воспроизводит compressed class list; green case сохраняет все non-hidden identities в точном server order, держит compatible subset отдельно и показывает incompatible row disabled без перенумерации последующих rows, letters или viewport positions. [Contract: P1-AC02, P1-AC06, P1-AC10, P1-AC11, P1-AC19, P2-AC04, P2-AC05; checks: `sv-build`, `sv-core`]
- [x] Direct keyboard letter и mouse click/double-click разрешаются через stable visible identity: incompatible class отклоняется без изменения owner, highlighted legal identity или draft, а последующая compatible identity выбирается и подтверждается именно под своим server ID. [Contract: P1-AC02, P1-AC06, P1-AC10, P1-AC11, P1-AC19, P2-AC04, P2-AC05; checks: `sv-core`]
- [x] Previous/next, paging/scroll и random в default order обходят только legal classes, сохраняют highlight на legal server identity и согласуют renderer, hit testing и bounded viewport; interleaved focus, resize и network events не меняют owner или selection. [Contract: P1-AC06, P1-AC10, P1-AC11, P1-AC21, P1-AC28, P2-AC04, P2-AC05; checks: `sv-core`]
- [x] Default и alternate compile orders сохраняют hidden-class exclusion, legal confirm, DNA/backtracking/quit semantics и suppression partial/completed birth sends; final completed-character send остаётся у SV-B-024, а DNA persistence — вне этого repair. [Contract: P1-AC02, P1-AC06, P1-AC12, P1-AC15, P1-AC19, P1-AC22, P1-AC26, P1-AC27, P1-AC29, P2-AC04; checks: `sv-build`, `sv-core`]
- [x] Repair и regressions остаются по возможности SV-local, используют production interaction state/input router и не добавляют fixed-response либо test-only class mapping; unrelated legacy/shared/SV organization improvements при обнаружении записываются отдельно, не расширяя этот ticket. [Contract: P1-AC28, P2-AC01, P2-AC02, P2-AC03, P2-AC05, P2-AC06, P2-AC07, P2-AC08, P2-AC09; checks: `controller-review`]

**Deferred check:** `P1-birth-runtime-evidence` / P1-AC30 остаётся deferred у named full-acceptance producer/verifier и не считается пройденным этим implementation repair; evidence собирается только на exact accepted contract digest `690a04cefb7fd0e1615dce46d1a538e1dcbd856232af1a1373be93f987095c82` и HEAD.

## Answer

Production seam: публичный `sv_endpoint_run(SvEndpointOptions)` получает от
controlled peer authoritative setup, в котором non-hidden class 0 несовместим,
а следующая visible identity 2 legal. Настоящие SDL keyboard, mouse, wheel,
PageUp/PageDown, random, focus и resize events проходят native adapter и public
birth interaction/input-router boundary. Production scene и hit testing читают
тот же `SvBirthView`; `SvEndpointOutcome` наблюдает visible/legal identities,
owner, highlight и draft. Peer после разрешённого name packet ожидает EOF, так
что partial или completed birth bytes провалили бы сценарии.

Исправление осталось в SV: `prepare_classes()` теперь всегда записывает в
`visible_classes` все non-hidden identities в исходном server order, затем
независимо строит compatible `legal_classes`. Existing semantic selection
повторно проверяет legality, navigation/random продолжают использовать legal
subset, а renderer помечает visible identity, отсутствующую в legal subset,
disabled. Legacy/shared production code не менялся, test-only mapping или
choice engine не добавлялись.

TDD evidence через согласованный production seam:

- RED: `SV_BIRTH_CASE=default-visible-class-letter python3 -B
  tests/sv_name_editor_submit_checks.py` завершился exit 1 на
  `outcome.birth_visible_class_count == 3`: default flow возвращал compressed
  legal-only list.
- GREEN: та же команда завершилась exit 0 после минимального изменения;
  outcome сохранил visible `[0, 2, 3]`, legal `[2]`, owner class, highlight 2,
  uncommitted draft и rejected identity 0.
- Дополнительные GREEN slices `default-visible-class-keyboard-confirm`,
  `default-visible-class-mouse`, `default-visible-class-viewport` и
  `default-visible-class-random` прошли через тот же endpoint. Они проверили
  exact server identity 2 для letter/mouse confirm, disabled double-click,
  legal-only previous/next и random, двунаправленный paging, wheel/hit testing,
  bounded 20-row viewport и стабильность при focus/resize/network interleave.

Focused verification:

- Новые пять default-order cases, `long-class-viewport` и
  `class-random-empty` — PASS; runner компилировал production sources с ASan,
  UBSan и `-Werror`, а применимые cases выполнял в обеих compile configurations.
- Alternate visible letter/mouse/viewport/random и incompatible DNA cases,
  default/alternate class Backspace/DNA, legacy hidden-class exclusion и hidden
  Death Knight DNA — PASS. Legacy hidden case теперь дополнительно наблюдает
  полный non-hidden `[0..12]` list отдельно от legal subset.
- `session-interleave`, `session-stale-input`, `session-disconnect` — PASS;
  запрещённых outbound birth bytes не было.
- `make -s -C src -f makefile.sv tomenet-sv` — PASS.
- `make -s -C src -f makefile.sv SV_CLASS_BEFORE_RACE=0 tomenet-sv` — PASS.
- `git diff --check` — PASS перед завершением ticket.

Полный `sv-core` и controller review не запускались по указанию orchestrator.
`P1-birth-runtime-evidence` / P1-AC30 остаётся deferred и здесь не объявлен
пройденным. Повторившиеся build warning про общий obsolete `atol` declaration и
SDL `Leaked thread` diagnostic уже известны из предыдущего record 58; новых
unrelated improvement opportunities не обнаружено.
