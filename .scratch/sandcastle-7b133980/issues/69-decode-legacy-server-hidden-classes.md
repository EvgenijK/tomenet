# 69: Декодировать hidden classes старых серверов

**Type:** repair

**Tracker role:** Wayfinder planning/execution history for canonical [SV-B-023](../../../docs/tasks/stage-b/SV-B-023-birth-choices.md); this child does not own Stage B capability IDs.

**What to build:** При подключении к server version не новее 4.7.0.2 SV интерпретирует legacy class layout так же, как behavior baseline: первые 13 server identities остаются публичными class choices, а последующие Death Knight, Hell Knight и Corrupted Priest сохраняются только как derived hidden identities с правильными visible base classes. В обеих compile-time class/race order configurations игрок не может выбрать эти implementation identities напрямую, но legal race/trait derivation продолжает получать правильную class identity без partial или completed send.

**Blocked by:** 52: Принять trait-derived hidden classes из DNA (completed); 58: Сохранить identities видимого class list (completed).

**Status:** ready-for-agent

Labels: bug, ready-for-agent

**Finding ID:** `D-8641402207f42d29` (`sv-birth-legacy-hidden-class-decoding`), one coherent repair of legacy-server class metadata decoding and production birth-choice legality.

**Production seam for TDD:** публичный `sv_endpoint_run(SvEndpointOptions)` подключается к controlled peer, получает negotiated old server version и authoritative unprefixed legacy class setup, после чего реальные keyboard/mouse/random/DNA SDL events проходят contact decoding, native birth adapter и публичную production birth-interaction/input-router boundary. `SvEndpointOutcome` и controlled peer наблюдают owner, visible/legal identities, stable highlight, authoritative draft и отсутствие запрещённых outbound bytes. Seam принят P1-AC02, P1-AC06, P1-AC10 и P1-AC28; проверки — `sv-core` и `sv-build`.

- [ ] Red production-path old-version setup воспроизводит появление unprefixed class identities после первых 13 в visible/legal choices; green version-aware SV decoding сохраняет server identity/order, помечает trailing legacy identities hidden и восстанавливает их canonical visible base-class mapping, не меняя modern prefixed layout. [Contract: P1-AC02, P1-AC06, P1-AC10, P2-AC04, P2-AC05; checks: `sv-core`]
- [ ] В default class-before-race и alternate race-before-class configurations class surface содержит ровно первые 13 public identities в исходном server order; keyboard letters, mouse rows, navigation и random не делают Death Knight, Hell Knight или Corrupted Priest доступными для прямого выбора и сохраняют stable highlighted public identity при отклонённом вводе. [Contract: P1-AC02, P1-AC06, P1-AC10, P1-AC11, P1-AC19, P2-AC04, P2-AC05; checks: `sv-build`, `sv-core`]
- [ ] Legal visible base-class плюс authoritative race/trait combination сохраняют baseline derivation Death Knight, Hell Knight или Corrupted Priest, включая staged/automatic DNA identity, тогда как несовместимая комбинация остаётся отклонённой; ordinary public-class и modern encoded-hidden-class controls остаются green. [Contract: P1-AC02, P1-AC04, P1-AC05, P1-AC06, P1-AC10, P1-AC19, P1-AC26, P2-AC04, P2-AC05; checks: `sv-build`, `sv-core`]
- [ ] Confirm/reject, Backspace, Q/Ctrl-Q, disconnect и stale-generation paths старого server setup используют production interaction/input-router state и не отправляют partial/completed character result; DNA persistence и final send остаются вне SV-B-023. [Contract: P1-AC02, P1-AC12, P1-AC15, P1-AC22, P1-AC26, P1-AC27, P1-AC28, P1-AC29; checks: `sv-core`, `controller-review`]
- [ ] Repair и regressions остаются по возможности SV-local, не меняют legacy client/server behavior и не добавляют test-only class decoder; обнаруженные unrelated improvement opportunities записываются отдельно. [Contract: P2-AC01, P2-AC02, P2-AC03, P2-AC04, P2-AC05, P2-AC06, P2-AC07, P2-AC08, P2-AC09; checks: `controller-review`]

**Deferred check:** `P1-birth-runtime-evidence` / P1-AC30 остаётся deferred у named full-acceptance producer/verifier и не считается пройденным этим implementation repair; evidence собирается только на exact accepted contract digest `690a04cefb7fd0e1615dce46d1a538e1dcbd856232af1a1373be93f987095c82` и HEAD.
