# 44: Сохранить регистр native birth bindings

**Type:** repair

**Tracker role:** Wayfinder planning/execution history for canonical [SV-B-023](../../../docs/tasks/stage-b/SV-B-023-birth-choices.md); this child does not own Stage B capability IDs.

**What to build:** Native birth keyboard bindings определяют регистр буквы по SDL keycode вместе с Shift/Caps modifier state. Только uppercase Q или Ctrl-Q завершают creation, только uppercase H выбирает legal Hellish mode, а direct sex/body/stats/mode и server-list letters принимаются только в baseline lowercase форме; поэтому lowercase q остаётся доступной семнадцатой list binding и никогда не подменяется quit.

**Blocked by:** 09: Завершить или откатить creation без send (completed); 14: Изолировать production birth input context (completed); 29: Сделать длинные birth choice lists полностью достижимыми (completed).

**Status:** unclaimed

**Assignee:** —

Labels: bug, ready-for-agent

**Finding ID:** `D-9ff71cd954b73dcd` (`birth.case-sensitive-key-binding-semantics`), one coherent repair of native case decoding and every affected birth binding.

**Production seam for TDD:** публичный `sv_endpoint_run(SvEndpointOptions)` получает server-provided birth lists and mode flags; настоящие SDL keyboard events с keycode и modifier state проходят native birth adapter и публичную production birth-interaction/input-router boundary. `SvEndpointOutcome` и controlled peer наблюдают quit state, owner, highlighted server identity, draft и отсутствие запрещённых outbound bytes. Seam принят P1-AC09, P1-AC10, P1-AC11, P1-AC12 и P1-AC28; проверки — `sv-core` и `sv-build`.

- [ ] Red production SDL-event regressions доказывают текущую коллизию; green regressions подтверждают, что unshifted lowercase q не завершает creation ни у одного birth owner, тогда как uppercase Q и Ctrl-Q проходят baseline quit/retry handling без partial или completed send. Shift/Caps case resolution и Ctrl modifier обрабатываются явно, а не выводятся из SDL letter keycode. [Contract: P1-AC02, P1-AC11, P1-AC12, P1-AC28, P2-AC04, P2-AC05; checks: `sv-core`]
- [ ] В legal long server-provided class, race и trait lists lowercase q выбирает семнадцатую identity со stable highlight и legal confirm, а shifted/caps-uppercase list letters не выбирают lowercase bindings и не меняют owner, highlight или draft. Проверка проходит в обеих compile-time class/race order configurations. [Contract: P1-AC04, P1-AC05, P1-AC06, P1-AC10, P1-AC11, P1-AC19, P2-AC04, P2-AC05; checks: `sv-build`, `sv-core`]
- [ ] Lowercase h не выбирает Hellish, uppercase H выбирает Hellish только когда этот ordinary или dedicated mode legal; остальные direct sex/body/stats/mode letters принимаются только в lowercase baseline form, а их uppercase variants безопасно отклоняются с неизменным owner, highlight и draft. [Contract: P1-AC03, P1-AC07, P1-AC08, P1-AC09, P1-AC10, P1-AC11, P2-AC04; checks: `sv-core`]
- [ ] Keyboard case repair не меняет mouse navigation/confirmation, random/DNA semantics, Backspace graph, exclusive input ownership, disconnect/stale-generation suppression или запрет partial/completed-character send; final send остаётся у SV-B-024. [Contract: P1-AC02, P1-AC10, P1-AC11, P1-AC13, P1-AC14, P1-AC15, P1-AC16, P1-AC17, P1-AC18, P1-AC21, P1-AC22, P1-AC26, P1-AC29; checks: `sv-core`]
- [ ] Repair остаётся по возможности SV-local и проверяется через production native adapter и interaction/input router без fixed-response или test-only key decoder; обнаруженные unrelated improvements только записываются отдельно. [Contract: P1-AC28, P2-AC01, P2-AC02, P2-AC03, P2-AC05, P2-AC06, P2-AC07, P2-AC08, P2-AC09; checks: `controller-review`]

**Deferred check:** `P1-birth-runtime-evidence` / P1-AC30 остаётся deferred и не считается пройденным этим implementation repair; полная native evidence matrix собирается только на exact accepted contract digest и HEAD во время full acceptance.
