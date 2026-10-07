# 07: Выбрать stats

**Type:** implementation

**What to build:** После последнего class/race/body owner игрок завершает baseline stats choice через production interaction: legal server-compatible distribution/order, random/DNA choice, stable highlight, keyboard/mouse confirmation и Backspace к реальному предыдущему owner без сетевого результата.

**Blocked by:** 06: Сохранить alternate class/race compile order.

**Status:** ready-for-agent

Labels: enhancement, ready-for-agent

**Production seam for TDD:** stats flow вызывается через публичный `sv_endpoint_run(SvEndpointOptions)` и production birth interaction/input router в обеих compile configurations. Seam принят P1-AC08, P1-AC17, P1-AC19 и P1-AC28; проверки используют `sv-core`.

- [ ] Stats surface предоставляет legal server-compatible choices для текущего draft, отклоняет illegal/over-budget/stale confirmation и сохраняет stable highlighted identity или allocation. [Contract: P1-AC08, P1-AC10; checks: `sv-core`]
- [ ] Keyboard/mouse navigation, confirm, Q/Ctrl-Q, random и доступный DNA choice используют production router и сохраняют baseline semantics. [Contract: P1-AC08, P1-AC10, P1-AC11, P1-AC12, P1-AC26, P1-AC28; checks: `sv-core`]
- [ ] Backspace возвращает body или ближайший реально доступный predecessor в активном compile order; Backspace и confirmation не отправляют partial/completed result. [Contract: P1-AC02, P1-AC17, P1-AC19, P1-AC29; checks: `sv-core`]
- [ ] Stats behavior и tests остаются production SV-local и сохраняют protocol/gameplay baseline. [Contract: P2-AC01, P2-AC04, P2-AC05; checks: `controller-review`]
