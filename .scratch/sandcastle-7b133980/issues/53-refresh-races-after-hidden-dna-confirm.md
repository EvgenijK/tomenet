# 53: Обновить race list после hidden DNA confirm

**Type:** repair

**Tracker role:** Wayfinder planning/execution history for canonical [SV-B-023](../../../docs/tasks/stage-b/SV-B-023-birth-choices.md); this child does not own Stage B capability IDs.

**What to build:** После подтверждения class-before-race hidden DNA class временное сужение race list снимается вместе с pending hidden-class constraint. При последующем Backspace из trait, body или stats race owner показывает актуальный полный legal server-provided list, сохраняет текущую highlighted identity, когда она остаётся legal, и использует один и тот же обновлённый list для keyboard, mouse, random и confirm без partial или completed send.

**Blocked by:** 21: Сохранить hidden DNA class до подтверждения race (completed); 27: Получить human disposition для stable hidden DNA race highlight (completed).

**Status:** ready-for-agent

Labels: bug, ready-for-agent

**Finding ID:** `D-217a025736cd4ff5` (`birth-hidden-dna-race-list-stale-after-confirm`), one coherent repair of race-list invalidation and every affected race re-entry path.

**Production seam for TDD:** публичный `sv_endpoint_run(SvEndpointOptions)` получает authoritative class/race lists и hidden DNA identity; реальные DNA, confirm, Backspace, keyboard и mouse SDL events проходят native birth adapter и публичную production birth-interaction/input-router boundary. `SvEndpointOutcome` и controlled peer наблюдают owner, legal race identities, stable highlight, draft и отсутствие запрещённых outbound bytes. Seam принят P1-AC04, P1-AC10, P1-AC11, P1-AC13, P1-AC14, P1-AC16, P1-AC17 и P1-AC28; проверки — `sv-core` и `sv-build`.

- [ ] Red production-path regression подтверждает hidden DNA class в class-before-race flow, затем возвращается к race из каждого достижимого downstream owner — trait, body и stats, включая skipped trait/body variants; green outcome пересобирает legal races после снятия pending hidden-class constraint и больше не оставляет cached narrowed subset. [Contract: P1-AC02, P1-AC04, P1-AC10, P1-AC13, P1-AC14, P1-AC16, P1-AC17, P1-AC19, P1-AC26, P2-AC04, P2-AC05; checks: `sv-build`, `sv-core`]
- [ ] Race identity, подтверждённая до Backspace, остаётся highlighted, если она legal в обновлённом list; если она больше не legal по authoritative setup, production flow выбирает согласованный legal fallback без stale identity и не меняет ранее подтверждённые sex/class values. [Contract: P1-AC02, P1-AC04, P1-AC10, P1-AC13, P1-AC14, P1-AC16, P1-AC17, P2-AC04; checks: `sv-core`]
- [ ] Keyboard previous/next, mouse selection, random и confirm используют один обновлённый server-provided race list; newly legal races достижимы, illegal identities отклоняются с неизменными owner/highlight/draft, а alternate compile order и compatible hidden-DNA controls остаются green. [Contract: P1-AC04, P1-AC10, P1-AC11, P1-AC19, P1-AC26, P1-AC28, P2-AC04, P2-AC05; checks: `sv-build`, `sv-core`]
- [ ] Backspace/re-entry, Q/Ctrl-Q, disconnect и stale-generation input не выполняют DNA persistence и не отправляют partial/completed character result; final send остаётся у SV-B-024. [Contract: P1-AC02, P1-AC12, P1-AC13, P1-AC14, P1-AC16, P1-AC17, P1-AC22, P1-AC27, P1-AC29; checks: `sv-core`, `controller-review`]
- [ ] Repair и regression остаются по возможности SV-local, проходят только через production interaction/input-router path и не добавляют отдельную test-only race-list implementation; обнаруженные unrelated improvements записываются отдельно. [Contract: P1-AC28, P2-AC01, P2-AC02, P2-AC03, P2-AC05, P2-AC06, P2-AC07, P2-AC08, P2-AC09; checks: `controller-review`]

**Deferred check:** `P1-birth-runtime-evidence` / P1-AC30 остаётся deferred у named full-acceptance producer/verifier и не считается пройденным этим implementation repair; evidence собирается только на exact accepted contract digest и HEAD.
