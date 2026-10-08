# 52: Принять trait-derived hidden classes из DNA

**Type:** repair

**Tracker role:** Wayfinder planning/execution history for canonical [SV-B-023](../../../docs/tasks/stage-b/SV-B-023-birth-choices.md); this child does not own Stage B capability IDs.

**What to build:** Staged и automatic DNA принимают legal server-provided Hell Knight и Corrupted Priest, показывают соответствующий visible base class и продолжают через независимые Maia race и Corrupted trait choices. Полный draft сохраняет согласованную комбинацию base class/race/trait, из которой authoritative final handling выводит исходную hidden class, в обеих compile-time class/race order configurations; несовместимая комбинация безопасно останавливает DNA continuation без изменения уже подтверждённого draft и без partial или completed send.

**Blocked by:** 21: Сохранить hidden DNA class до подтверждения race (completed); 19: Продолжить automatic DNA choices через birth flow (completed).

**Status:** ready-for-agent

Labels: bug, ready-for-agent

**Finding ID:** `D-6ea1c7fe7649ba64` (`birth-hidden-dna-trait-derived-class-rejected`), one coherent repair of trait-derived hidden-class normalization and continuation.

**Production seam for TDD:** публичный `sv_endpoint_run(SvEndpointOptions)` получает authoritative server-provided class/race/trait setup и DNA identities; реальные staged/automatic DNA SDL events проходят native birth adapter и публичную production birth-interaction/input-router boundary. `SvEndpointOutcome` и controlled peer наблюдают owner, legal identities, stable highlight, authoritative draft и отсутствие запрещённых outbound bytes. Seam принят P1-AC02, P1-AC04, P1-AC05, P1-AC06, P1-AC10, P1-AC26 и P1-AC28; проверки — `sv-core` и `sv-build`.

- [ ] Red production-path cases для Hell Knight и Corrupted Priest показывают текущий reject на class owner; green cases нормализуют каждую server-provided hidden identity к её visible base без требования, чтобы trait-derived hidden identity присутствовала в race choice bits. [Contract: P1-AC02, P1-AC06, P1-AC10, P1-AC26, P2-AC04, P2-AC05; checks: `sv-core`]
- [ ] Staged и automatic DNA независимо применяют и валидируют Maia race и Corrupted trait, сохраняют stable highlighted identities и согласованный base-class/race/trait draft, из которого authoritative final handling выводит исходную hidden identity; wrong race или trait останавливает automatic continuation и не подменяет подтверждённый draft. [Contract: P1-AC02, P1-AC04, P1-AC05, P1-AC06, P1-AC10, P1-AC26, P2-AC04; checks: `sv-core`]
- [ ] Оба trait-derived класса проходят production interaction в default и alternate compile-time class/race orders; race-derived hidden class, ordinary visible class, manual и random control cases сохраняют ранее проверенную compatibility семантику. [Contract: P1-AC04, P1-AC05, P1-AC06, P1-AC10, P1-AC19, P1-AC26, P1-AC28, P2-AC04, P2-AC05; checks: `sv-build`, `sv-core`]
- [ ] Accept, reject, Backspace, Q/Ctrl-Q, disconnect и stale-generation paths не выполняют DNA persistence и не отправляют partial/completed character result; `birth.restore-dna`/`birth.save-dna` не вводятся, а final send остаётся у SV-B-024. [Contract: P1-AC02, P1-AC12, P1-AC22, P1-AC26, P1-AC27, P1-AC29; checks: `sv-core`, `controller-review`]
- [ ] Repair и regression остаются по возможности SV-local, используют production interaction/input-router path и не добавляют test-only derivation; обнаруженные unrelated improvements записываются отдельно. [Contract: P1-AC28, P2-AC01, P2-AC02, P2-AC03, P2-AC05, P2-AC06, P2-AC07, P2-AC08, P2-AC09; checks: `controller-review`]

**Deferred check:** `P1-birth-runtime-evidence` / P1-AC30 остаётся deferred у named full-acceptance producer/verifier и не считается пройденным этим implementation repair; evidence собирается только на exact accepted contract digest и HEAD.
