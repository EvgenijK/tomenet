# 58: Сохранить identities видимого class list

**Type:** repair

**Tracker role:** Wayfinder planning/execution history for canonical [SV-B-023](../../../docs/tasks/stage-b/SV-B-023-birth-choices.md); this child does not own Stage B capability IDs.

**What to build:** В race-before-class flow class surface сохраняет полный server-visible class list и показывает несовместимые с выбранной race identities disabled вместо удаления. Keyboard letters, mouse rows и viewport positions продолжают обозначать стабильные visible identities: прямой выбор incompatible class отклоняется, тогда как navigation и random обходят только legal entries, а legal confirm продвигает тот же authoritative draft без partial/completed send.

**Blocked by:** 06: Сохранить alternate class/race compile order (completed); 29: Сделать длинные birth choice lists полностью достижимыми (completed).

**Status:** ready-for-agent

Labels: bug, ready-for-agent

**Finding ID:** `D-9dd37219e6bb4866` (`birth.class-list-illegal-choice-identity`), one coherent repair of visible class presentation, stable input mapping and compatibility rejection.

**Production seam for TDD:** публичный `sv_endpoint_run(SvEndpointOptions)` получает authoritative server-provided class/race setup, включая incompatible visible class перед compatible class; реальные keyboard, mouse, paging/scroll, random, confirm и Backspace SDL events проходят native birth adapter и публичную production birth-interaction/input-router boundary, а production scene читает тот же class view. `SvEndpointOutcome` и controlled peer наблюдают visible/legal identities, owner, stable highlight, draft и отсутствие запрещённых outbound bytes. Seam принят P1-AC02, P1-AC06, P1-AC10, P1-AC11, P1-AC19, P1-AC21 и P1-AC28; проверки — `sv-core` и `sv-build`.

- [ ] Red alternate-order production case помещает incompatible visible class перед compatible class и воспроизводит сжатие list/letter identities; green surface сохраняет server-visible order, помечает incompatibility disabled и не перенумеровывает последующие keyboard, mouse или viewport positions. [Contract: P1-AC02, P1-AC06, P1-AC10, P1-AC11, P1-AC19, P2-AC04, P2-AC05; checks: `sv-build`, `sv-core`]
- [ ] Baseline letter и mouse click по disabled row разрешаются в ту же visible server identity и отклоняются без изменения owner, highlighted legal identity или draft; double-click не подтверждает incompatible class, а letter/mouse по следующей compatible identity выбирают именно её. [Contract: P1-AC02, P1-AC06, P1-AC10, P1-AC11, P1-AC19, P2-AC04, P2-AC05; checks: `sv-core`]
- [ ] Previous/next, paging/scroll и random перемещаются только между legal classes, удерживают stable highlighted identity видимой и используют общий bounded viewport с renderer/hit testing; focus, resize и interleaved network events не меняют owner или selection. [Contract: P1-AC06, P1-AC10, P1-AC11, P1-AC21, P1-AC28, P2-AC04, P2-AC05; checks: `sv-core`, `controller-review`]
- [ ] Legal confirm в alternate order коммитит выбранную compatible class и продвигает flow; default class-before-race order, DNA semantics, Backspace, Q/Ctrl-Q, disconnect и stale-generation controls остаются green и не отправляют partial/completed character result, а final send остаётся у SV-B-024. [Contract: P1-AC02, P1-AC06, P1-AC10, P1-AC12, P1-AC15, P1-AC19, P1-AC22, P1-AC26, P1-AC29; checks: `sv-build`, `sv-core`]
- [ ] Repair и regressions остаются по возможности SV-local, сохраняют interaction ownership вне renderer, используют production input-router path и не добавляют fixed-response либо test-only class mapping; обнаруженные unrelated improvements записываются отдельно. [Contract: P1-AC28, P2-AC01, P2-AC02, P2-AC03, P2-AC05, P2-AC06, P2-AC07, P2-AC08, P2-AC09; checks: `controller-review`]

**Deferred check:** `P1-birth-runtime-evidence` / P1-AC30 остаётся deferred у named full-acceptance producer/verifier и не считается пройденным этим implementation repair; evidence собирается только на exact accepted contract digest `690a04cefb7fd0e1615dce46d1a538e1dcbd856232af1a1373be93f987095c82` и HEAD.
