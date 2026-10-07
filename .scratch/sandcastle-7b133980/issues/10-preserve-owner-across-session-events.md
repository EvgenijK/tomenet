# 10: Сохранить owner при session events

**Type:** implementation

**What to build:** Во время любого birth choice сеть продолжает обслуживаться, а focus и resize обновляют только presentation; interleaving не меняет owner, selection или draft. Disconnect и stale generation немедленно закрывают interaction и запрещают любые последующие sends.

**Blocked by:** 08: Выбрать mode и завершить draft.

**Status:** ready-for-agent

Labels: enhancement, ready-for-agent

**Production seam for TDD:** event sequences подаются через публичный `sv_endpoint_run(SvEndpointOptions)` и его production network/SDL loop, с outbound connection как наблюдаемой границей. Seam принят P1-AC21, P1-AC22 и P1-AC28; проверки используют `sv-core`.

- [ ] Interleaved legal network traffic на каждом birth owner обслуживается без блокировки UI и не меняет owner, highlighted identity или draft. [Contract: P1-AC21, P1-AC28; checks: `sv-core`]
- [ ] Focus lost/gained и resize меняют только native presentation/layout; после события тот же production owner и selection продолжают принимать keyboard/mouse input. [Contract: P1-AC11, P1-AC21; checks: `sv-core`]
- [ ] Disconnect завершает текущую birth interaction, а input и queued result после закрытия не могут отправить bytes или изменить новую generation. [Contract: P1-AC22; checks: `sv-core`]
- [ ] Любое действие со stale generation отклоняется до сериализации; outbound trace остаётся пустым для stale confirm, Backspace, quit и Guide-adjacent input. [Contract: P1-AC22, P1-AC29; checks: `sv-core`]
- [ ] Lifecycle behavior соответствует main-thread/session-generation ADR и проверяется через production SV path без legacy/shared expansion. [Contract: P2-AC01, P2-AC04, P2-AC05, P2-AC12; checks: `controller-review`]
