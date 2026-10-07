# 02: Выбрать класс в default compile order

**Type:** implementation

**What to build:** В production build с `CLASS_BEFORE_RACE` игрок после sex выбирает legal server-provided class, сохраняет устойчивую подсветку при keyboard/mouse/random/DNA input, подтверждает выбор для перехода к race либо возвращается Backspace к sex без сетевого результата.

**Blocked by:** 01: Войти в birth и выбрать пол.

**Status:** ready-for-agent

Labels: enhancement, ready-for-agent

**Production seam for TDD:** class flow проходит через публичный `sv_endpoint_run(SvEndpointOptions)` и его production birth interaction/input routing. Seam принят критериями P1-AC02, P1-AC06, P1-AC15 и P1-AC28; проверки используют `sv-core` и `sv-build`.

- [ ] Default compile order назначает class owner сразу после sex и показывает только legal server-provided class identities; hidden/illegal/incompatible или stale identity нельзя подтвердить. [Contract: P1-AC06, P1-AC10, P1-AC19; checks: `sv-build`, `sv-core`]
- [ ] Keyboard и mouse navigation, confirm, Q/Ctrl-Q, random и доступный DNA choice используют одну production selection; подсветка остаётся привязанной к identity, а не к случайному индексу. [Contract: P1-AC06, P1-AC10, P1-AC11, P1-AC12, P1-AC26, P1-AC28; checks: `sv-core`]
- [ ] Legal confirm сохраняет class draft и передаёт ownership race step; Backspace возвращает sex owner и не создаёт partial result или completed play packet. [Contract: P1-AC02, P1-AC15, P1-AC29; checks: `sv-core`]
- [ ] Class compatibility следует authoritative setup и behavior baseline, оставаясь в SV-local production code и production-path tests. [Contract: P1-AC06, P2-AC01, P2-AC04, P2-AC05; checks: `sv-core`, `controller-review`]
