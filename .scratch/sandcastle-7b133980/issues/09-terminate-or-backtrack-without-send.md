# 09: Завершить или откатить creation без send

**Type:** implementation

**What to build:** На каждом достижимом birth owner Q и Ctrl-Q завершают creation через baseline quit/retry handling, а Backspace проходит по точному owner graph; ни cancel, ни backtracking, ни готовый draft не отправляют partial или completed character packet.

**Blocked by:** 08: Выбрать mode и завершить draft.

**Status:** ready-for-agent

Labels: enhancement, ready-for-agent

**Production seam for TDD:** полная cancel/backtrack matrix выполняется через публичный `sv_endpoint_run(SvEndpointOptions)` и наблюдаемую outbound production connection. Seam принят P1-AC02, P1-AC12–P1-AC20, P1-AC28 и P1-AC29; проверки используют `sv-core`.

- [ ] Q и Ctrl-Q на sex/race/trait/class/body/stats/mode вызывают baseline quit/retry parent outcome, не подтверждают draft и не отправляют partial или completed result. [Contract: P1-AC02, P1-AC11, P1-AC12; checks: `sv-core`]
- [ ] Backspace matrix покрывает race, trait, class, body, stats и mode в обоих compile orders, включая unavailable-step skipping и post-mode PvP Maia trait return. [Contract: P1-AC13, P1-AC14, P1-AC15, P1-AC16, P1-AC17, P1-AC18, P1-AC19, P1-AC20; checks: `sv-core`]
- [ ] Outbound trace после каждого cancel/backtrack/ready-draft case не содержит partial character data или completed play packet; ticket не принимает ownership final send и не добавляет `birth.restore-dna`/`birth.save-dna`. [Contract: P1-AC02, P1-AC26, P1-AC27, P1-AC29; checks: `sv-core`, `controller-review`]
- [ ] Вся matrix проходит через production interaction state/input router, без fixed-response wizard или test-only behavior implementation. [Contract: P1-AC28, P2-AC04, P2-AC05; checks: `sv-core`, `controller-review`]
