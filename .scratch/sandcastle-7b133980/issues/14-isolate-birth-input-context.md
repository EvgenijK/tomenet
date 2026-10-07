# 14: Изолировать production birth input context

**Type:** implementation

**What to build:** Пока birth interaction активна, каждый keyboard/mouse gesture обрабатывается только её input context: известные gestures становятся semantic birth commands, остальные безопасно потребляются, а Escape в point-buy выполняет baseline confirmation. Никакой birth input не подтверждает login/MOTD state и не завершает endpoint в обход birth owner.

**Blocked by:** 13: Verify focused production coverage (completed).

**Status:** ready-for-agent

**Assignee:** unassigned

Labels: bug, ready-for-agent

**Finding ID:** `D-a8ac860d1445a95f` (`birth.production-input-routing-boundary`).

**Production seam for TDD:** публичный `sv_endpoint_run(SvEndpointOptions)` с активным production `SvBirth`; реальные SDL keyboard/mouse events проходят через native birth adapter и semantic input router, а outcome и controlled outbound connection наблюдают owner/draft/lifecycle. Seam принят P1-AC08, P1-AC11 и P1-AC28; проверки — `sv-core` и `controller-review`.

- [ ] При активном birth каждый gesture либо преобразуется в semantic birth command, либо потребляется birth context; illegal/unknown input и Escape не попадают в login/name/MOTD routing и не меняют чужое interaction state. [Contract: P1-AC11, P1-AC28; checks: `sv-core`]
- [ ] Escape в point-buy проходит baseline confirmation flow: учитывает pending confirmation и оставшиеся points, сохраняет корректные owner/highlight/draft и не завершает endpoint как глобальный Escape. [Contract: P1-AC08, P1-AC11, P2-AC04; checks: `sv-core`]
- [ ] Native adapter не меняет interaction state, включая ignored/error counters, до проверки active generation; stale или closed-generation input не имеет state/network effects. [Contract: P1-AC22, P1-AC28, P2-AC04; checks: `sv-core`, `controller-review`]
- [ ] Незавершённый birth interaction принадлежит documented input-router/interaction boundary либо доступен через неё без test-only interface; production tests вызывают тот же route для default и alternate compile orders. [Contract: P1-AC19, P1-AC28, P2-AC01, P2-AC05, P2-AC12; checks: `sv-build`, `sv-core`, `controller-review`]
- [ ] Ни consumed input, ни point-buy confirmation, ни stale input не отправляют partial/completed character result; final send остаётся у SV-B-024. [Contract: P1-AC02, P1-AC29; checks: `sv-core`, `controller-review`]

**Deferred check:** `P1-birth-runtime-evidence` / P1-AC30 остаётся deferred и не считается пройденным этим implementation ticket; evidence собирается только на full acceptance для exact accepted contract digest и HEAD.
