# 69: Декодировать hidden classes старых серверов

**Type:** repair

**Tracker role:** Wayfinder planning/execution history for canonical [SV-B-023](../../../docs/tasks/stage-b/SV-B-023-birth-choices.md); this child does not own Stage B capability IDs.

**What to build:** При подключении к server version не новее 4.7.0.2 SV интерпретирует legacy class layout так же, как behavior baseline: первые 13 server identities остаются публичными class choices, а последующие Death Knight, Hell Knight и Corrupted Priest сохраняются только как derived hidden identities с правильными visible base classes. В обеих compile-time class/race order configurations игрок не может выбрать эти implementation identities напрямую, но legal race/trait derivation продолжает получать правильную class identity без partial или completed send.

**Blocked by:** 52: Принять trait-derived hidden classes из DNA (completed); 58: Сохранить identities видимого class list (completed).

**Status:** resolved

**Assignee:** codex/sandcastle-7b133980-wave-46-1

Labels: bug, ready-for-agent

**Finding ID:** `D-8641402207f42d29` (`sv-birth-legacy-hidden-class-decoding`), one coherent repair of legacy-server class metadata decoding and production birth-choice legality.

**Production seam for TDD:** публичный `sv_endpoint_run(SvEndpointOptions)` подключается к controlled peer, получает negotiated old server version и authoritative unprefixed legacy class setup, после чего реальные keyboard/mouse/random/DNA SDL events проходят contact decoding, native birth adapter и публичную production birth-interaction/input-router boundary. `SvEndpointOutcome` и controlled peer наблюдают owner, visible/legal identities, stable highlight, authoritative draft и отсутствие запрещённых outbound bytes. Seam принят P1-AC02, P1-AC06, P1-AC10 и P1-AC28; проверки — `sv-core` и `sv-build`.

- [x] Red production-path old-version setup воспроизводит появление unprefixed class identities после первых 13 в visible/legal choices; green version-aware SV decoding сохраняет server identity/order, помечает trailing legacy identities hidden и восстанавливает их canonical visible base-class mapping, не меняя modern prefixed layout. [Contract: P1-AC02, P1-AC06, P1-AC10, P2-AC04, P2-AC05; checks: `sv-core`]
- [x] В default class-before-race и alternate race-before-class configurations class surface содержит ровно первые 13 public identities в исходном server order; keyboard letters, mouse rows, navigation и random не делают Death Knight, Hell Knight или Corrupted Priest доступными для прямого выбора и сохраняют stable highlighted public identity при отклонённом вводе. [Contract: P1-AC02, P1-AC06, P1-AC10, P1-AC11, P1-AC19, P2-AC04, P2-AC05; checks: `sv-build`, `sv-core`]
- [x] Legal visible base-class плюс authoritative race/trait combination сохраняют baseline derivation Death Knight, Hell Knight или Corrupted Priest, включая staged/automatic DNA identity, тогда как несовместимая комбинация остаётся отклонённой; ordinary public-class и modern encoded-hidden-class controls остаются green. [Contract: P1-AC02, P1-AC04, P1-AC05, P1-AC06, P1-AC10, P1-AC19, P1-AC26, P2-AC04, P2-AC05; checks: `sv-build`, `sv-core`]
- [x] Confirm/reject, Backspace, Q/Ctrl-Q, disconnect и stale-generation paths старого server setup используют production interaction/input-router state и не отправляют partial/completed character result; DNA persistence и final send остаются вне SV-B-023. [Contract: P1-AC02, P1-AC12, P1-AC15, P1-AC22, P1-AC26, P1-AC27, P1-AC28, P1-AC29; checks: `sv-core`, `controller-review`]
- [x] Repair и regressions остаются по возможности SV-local, не меняют legacy client/server behavior и не добавляют test-only class decoder; обнаруженные unrelated improvement opportunities записываются отдельно. [Contract: P2-AC01, P2-AC02, P2-AC03, P2-AC04, P2-AC05, P2-AC06, P2-AC07, P2-AC08, P2-AC09; checks: `controller-review`]

**Deferred check:** `P1-birth-runtime-evidence` / P1-AC30 остаётся deferred у named full-acceptance producer/verifier и не считается пройденным этим implementation repair; evidence собирается только на exact accepted contract digest `690a04cefb7fd0e1615dce46d1a538e1dcbd856232af1a1373be93f987095c82` и HEAD.

## Answer

Production seam: публичный `sv_endpoint_run(SvEndpointOptions)` подключается к
managed peer версии `4.7.0.2.0.0`, получает authoritative setup с 16
unprefixed server class identities и обрабатывает реальные SDL keyboard, mouse,
random, DNA, Backspace, quit и stale events через production contact decoder,
native birth adapter и input router. `SvEndpointOutcome` наблюдает identities,
owner, stable highlight и draft; peer после единственного разрешённого name
packet требует EOF, поэтому любой partial/completed birth send ломает сценарий.

SV-local contact decoder теперь применяет legacy fixed layout только для server
version не новее `4.7.0.2.0.0`: identities 13–15 остаются в исходном server
порядке, но получают hidden metadata и canonical base classes Paladin (6),
Paladin (6), Priest (2). Для identities 14–15 decoder также сохраняет явную
trait-derived metadata, поэтому legacy wire title `Priest` не подменяется и не
используется как semantic discriminator. Более новые encoded titles продолжают
использовать server-provided prefix без изменения. Birth interaction поэтому
публикует и принимает напрямую только identities 0–12, но staged/automatic DNA и legal
race/trait combinations продолжают выводить Death Knight, Hell Knight и
Corrupted Priest через production draft.

TDD evidence через согласованный production seam:

- RED: `SV_BIRTH_CASE=legacy-hidden-class-direct python3 -B tests/sv_name_editor_submit_checks.py`
  завершился exit 1 на `birth_legal_class_count == 13`: decoder публиковал все
  16 unprefixed identities как прямые choices.
- GREEN: та же команда завершилась exit 0 в default и
  `SV_CLASS_BEFORE_RACE=0` binaries; visible/legal order равен `[0..12]`, а
  hidden-letter input после keyboard/mouse navigation сохраняет public identity
  0 и незакоммиченный draft.
- Второй RED:
  `SV_BIRTH_CASE=auto-dna-legacy-hidden-corrupted-priest python3 -B tests/sv_name_editor_submit_checks.py`
  с authoritative identity 15 и wire title `Priest` завершился exit 1 на
  ожидаемом owner `SV_BIRTH_READY`: прежняя name-based проверка не распознала
  trait-derived identity. После decoder-owned `trait_derived` metadata та же
  команда — GREEN, exit 0 в обеих compile configurations.
- Следующие cases добавлены как production-path regression coverage уже
  исправленного decoder path, без заявления отдельных red-first циклов:
  `legacy-hidden-class-random`, `legacy-hidden-class-back`,
  `legacy-hidden-class-stale`, `legacy-hidden-class-disconnect`,
  `legacy-hidden-dna-death-knight` и `legacy-hidden-dna-hell-knight`.

Focused verification, 2026-10-08:

- Все восемь `legacy-hidden-*` cases через
  `SV_BIRTH_CASE=<case> python3 -B tests/sv_name_editor_submit_checks.py` — PASS
  в обеих compile configurations под ASan/UBSan. Они покрывают direct reject,
  navigation/mouse, random, confirm, Backspace, Q/Ctrl-Q, stale generation,
  disconnect и три derived identities; peer не получил запрещённых birth bytes.
- Modern controls `trait-hidden-dna-hell-knight`, `hidden-derived-dna` и
  ordinary `class-navigate` тем же focused runner — PASS.
- `make -s -C src -f makefile.sv tomenet-sv` — PASS.
- `make -s -C src -f makefile.sv SV_CLASS_BEFORE_RACE=0 tomenet-sv` — PASS.
- `git diff --check` — PASS.

Полный `sv-core` и controller review не запускались по указанию orchestrator.
`P1-birth-runtime-evidence` / P1-AC30 остаётся deferred и не объявляется
выполненным. Legacy/shared production code не менялся. Известные предупреждение
common `atol` и SDL leaked-thread diagnostics уже записаны в
`docs/sv-improvements.md`; новых unrelated improvement opportunities не
обнаружено.
