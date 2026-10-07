# 22: Передать authoritative first-run state в birth

**Type:** implementation

**What to build:** Production character-management/session owner передаёт first-run eligibility независимо от roster count, поэтому returning player с пустым roster получает legal Fruit Bat, Hellish и PvP choices, а реальный first startup остаётся ограничен даже при непустом server overview.

**Blocked by:** 18: Соблюсти first-run и Archer body legality (completed).

**Status:** ready-for-agent

**Assignee:** unassigned

Labels: bug, ready-for-agent

**Finding ID:** `D-2a20d07678497370` (`birth.first-run-body-legality`).

**Production seam for TDD:** публичный `sv_endpoint_run(SvEndpointOptions)` вызывается тем же production startup/character-management path, который владеет first-run state; server-confirmed name creation переносит этот state через pregame в production birth interaction, а native input и outcome наблюдают legal body/mode identities и draft. Seam принят P1-AC01, P1-AC02, P1-AC07, P1-AC09 и P1-AC28; проверки — `sv-core` и `controller-review`. Отдельный test-only first-run option не является допустимым seam.

- [ ] First-run eligibility имеет authoritative production owner и передаётся через character-management/session handoff отдельно от количества server-provided characters; birth больше не выводит её из empty/non-empty roster. [Contract: P1-AC01, P1-AC02, P1-AC28, P2-AC04; checks: `sv-core`, `controller-review`]
- [ ] Returning-empty-roster production flow разрешает Fruit Bat, Hellish и PvP согласно baseline first-run gate, сохраняя server/RPG compatibility и stable highlighted identity. [Contract: P1-AC02, P1-AC07, P1-AC09, P1-AC10, P2-AC04; checks: `sv-core`]
- [ ] First-startup-with-existing-roster production flow запрещает те же first-run-ineligible body/mode confirmations независимо от direct, mouse, random или DNA input; illegal choice не меняет owner, highlight или committed draft. [Contract: P1-AC02, P1-AC07, P1-AC09, P1-AC10, P1-AC11, P1-AC26; checks: `sv-core`]
- [ ] Production-path cases покрывают обе независимые комбинации first-run/roster в обеих compile configurations и сохраняют уже проверенные Archer random и ordinary legal-choice semantics без test-only behavior implementation. [Contract: P1-AC07, P1-AC09, P1-AC19, P1-AC28, P2-AC05; checks: `sv-build`, `sv-core`]
- [ ] Передача eligibility и все legal/illegal choices не отправляют partial/completed character result и не принимают DNA persistence или final-send ownership. [Contract: P1-AC02, P1-AC26, P1-AC27, P1-AC29; checks: `sv-core`, `controller-review`]
- [ ] Изменения ограничены необходимым production handoff и SV birth behavior; legacy/shared refactor не выполняется только ради deduplication. [Contract: P2-AC01, P2-AC02, P2-AC03, P2-AC04, P2-AC09; checks: `controller-review`]

**Deferred check:** `P1-birth-runtime-evidence` / P1-AC30 остаётся deferred и не считается пройденным этим implementation ticket; evidence собирается только на full acceptance для exact accepted contract digest и HEAD.
