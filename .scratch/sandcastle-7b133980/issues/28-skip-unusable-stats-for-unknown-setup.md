# 28: Пропустить unusable stats для unknown creation setup

**Type:** repair

**What to build:** Игрок с authoritative `creation_flags > 1` не попадает на пустую stats surface: после последнего доступного выбора production birth flow безопасно сохраняет baseline draft и переходит к mode. Backspace из mode возвращает к фактически предшествующему доступному owner в обеих compile-time class/race order configurations, а повторный проход снова пропускает недоступные body/stats без partial или completed send.

**Blocked by:** 08: Выбрать mode и завершить draft (completed); 09: Завершить или вернуться без send (completed).

**Status:** unclaimed

**Assignee:** —

Labels: bug, ready-for-agent

**Finding ID:** `D-498640166ca1cc34` (`birth.unknown-creation-flags-stats-deadlock`).

**Production seam for TDD:** публичный `sv_endpoint_run(SvEndpointOptions)` получает authoritative decoded `creation_flags` и server-provided choices; реальные keyboard/mouse SDL events проходят native birth adapter и production interaction/input router, а controlled peer наблюдает owner/draft transitions и отсутствие запрещённых outbound bytes. Seam принят P1-AC02, P1-AC08, P1-AC19 и P1-AC28; проверки — `sv-core` и `sv-build`.

- [ ] Red regression через production seam воспроизводит `creation_flags > 1` с нулём legal stats и доказывает, что прежний flow застревает на stats; green outcome никогда не назначает owner недоступной stats surface и достигает mode с неизменённым baseline draft. [Contract: P1-AC02, P1-AC08, P1-AC19, P1-AC28, P2-AC04, P2-AC05; checks: `sv-core`]
- [ ] Обычный и alternate compile-time class/race orders проходят unknown setup до mode: недоступные body и stats пропускаются, а доступные sex/race/trait/class choices сохраняют server-provided legality, highlighted identity и committed selections. [Contract: P1-AC02, P1-AC03, P1-AC04, P1-AC05, P1-AC06, P1-AC08, P1-AC10, P1-AC19; checks: `sv-build`, `sv-core`]
- [ ] Backspace из mode возвращает к последнему доступному owner (trait в class-before-race order, class в race-before-class order); повторное продвижение снова достигает mode без пустой stats owner и без потери draft. [Contract: P1-AC15, P1-AC16, P1-AC17, P1-AC18, P1-AC19; checks: `sv-core`]
- [ ] Confirm, random, Backspace, Q/Ctrl-Q, disconnect и stale generation на этом setup не отправляют partial/completed character result; accepted mode только завершает draft для будущего SV-B-024 send. [Contract: P1-AC02, P1-AC09, P1-AC12, P1-AC18, P1-AC22, P1-AC29; checks: `sv-core`]
- [ ] Repair остаётся по возможности SV-local и не меняет legacy/shared gameplay semantics; discovered out-of-scope improvements записываются отдельно. [Contract: P2-AC01, P2-AC02, P2-AC03, P2-AC04, P2-AC06, P2-AC07, P2-AC08, P2-AC09; checks: `controller-review`]

**Deferred check:** `P1-birth-runtime-evidence` / P1-AC30 остаётся deferred и не считается пройденным этим implementation repair; evidence собирается только на full acceptance для exact accepted contract digest и HEAD.
