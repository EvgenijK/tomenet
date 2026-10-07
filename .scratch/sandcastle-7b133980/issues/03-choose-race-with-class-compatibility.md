# 03: Выбрать race с class compatibility

**Type:** implementation

**What to build:** В default compile order игрок выбирает legal server-provided race, причём class/race compatibility, random/DNA semantics, stable highlight, keyboard/mouse navigation, confirmation и возврат к class совпадают с behavior baseline и ничего не отправляют как завершённого персонажа.

**Blocked by:** 02: Выбрать класс в default compile order.

**Status:** ready-for-agent

Labels: enhancement, ready-for-agent

**Production seam for TDD:** race flow вызывается через публичный `sv_endpoint_run(SvEndpointOptions)` с server setup, уже используемым production contact/login conversation. Seam принят P1-AC02, P1-AC04, P1-AC13 и P1-AC28; проверки используют `sv-core`.

- [ ] Race surface использует полный server-provided list и stable identities, отфильтровывает или отклоняет illegal/incompatible подтверждение согласно выбранному class и сохраняет прежний draft при отказе. [Contract: P1-AC04, P1-AC10; checks: `sv-core`]
- [ ] Keyboard/mouse navigation, confirm, Q/Ctrl-Q, random и доступный DNA choice проходят через production input router и сохраняют highlighted identity. [Contract: P1-AC04, P1-AC10, P1-AC11, P1-AC12, P1-AC26, P1-AC28; checks: `sv-core`]
- [ ] Legal confirm передаёт ownership следующему доступному birth step; Backspace возвращает class owner в default order и не отправляет partial или completed result. [Contract: P1-AC02, P1-AC13, P1-AC29; checks: `sv-core`]
- [ ] Compatibility и choice semantics портированы без расширения legacy/shared implementation и проверяются через production SV state, а не отдельную test-only модель. [Contract: P2-AC01, P2-AC02, P2-AC04, P2-AC05; checks: `controller-review`]
