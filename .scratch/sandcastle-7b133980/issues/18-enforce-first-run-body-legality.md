# 18: Соблюсти first-run и Archer body legality

**Type:** implementation

**What to build:** Body choice учитывает production first-run eligibility и текущий class: первый персонаж не может напрямую выбрать Fruit Bat, а random для Archer всегда подтверждает Normal; legal non-first-run manual choices продолжают работать по behavior baseline.

**Blocked by:** 05: Выбрать или пропустить body (completed).

**Status:** ready-for-agent

**Assignee:** unassigned

Labels: bug, ready-for-agent

**Finding ID:** `D-2a20d07678497370` (`birth.first-run-body-legality`).

**Production seam for TDD:** публичный `sv_endpoint_run(SvEndpointOptions)` получает first-run eligibility из production character/session handoff вместе с authoritative setup, затем keyboard/mouse/random input проходит native birth adapter и interaction; outcome наблюдает legal body list, highlight и draft. Seam принят P1-AC01, P1-AC07 и P1-AC28; проверка — `sv-core`.

- [ ] First-run eligibility приходит из production SV-B-022/session state, а не из test-only option; при first run Fruit Bat отсутствует среди manual legal choices и не может быть подтверждён direct, mouse, random или DNA input. [Contract: P1-AC01, P1-AC07, P1-AC10, P1-AC28; checks: `sv-core`, `controller-review`]
- [ ] Для Archer random body подтверждает Normal даже когда non-first-run manual Fruit Bat остаётся legal; manual compatible choice сохраняет существующую baseline semantics. [Contract: P1-AC02, P1-AC07, P1-AC10, P2-AC04; checks: `sv-core`]
- [ ] Production-path scenarios покрывают first-run manual/random/DNA reject, non-first-run manual Fruit Bat и Archer random в обеих compile configurations с stable highlight при reject. [Contract: P1-AC07, P1-AC10, P1-AC11, P1-AC19, P1-AC26, P1-AC28; checks: `sv-build`, `sv-core`]
- [ ] Body legality transitions и rejects не отправляют partial/completed character result; DNA persistence и final send остаются вне ticket. [Contract: P1-AC02, P1-AC27, P1-AC29; checks: `sv-core`, `controller-review`]
- [ ] Реализация и production-path tests остаются по возможности внутри SV и не меняют legacy/shared birth rules. [Contract: P2-AC01, P2-AC04, P2-AC05, P2-AC09; checks: `controller-review`]

**Deferred check:** `P1-birth-runtime-evidence` / P1-AC30 остаётся deferred и не считается пройденным этим implementation ticket; evidence собирается только на full acceptance для exact accepted contract digest и HEAD.
