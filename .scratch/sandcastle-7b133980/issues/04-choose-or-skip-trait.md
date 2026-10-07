# 04: Выбрать или пропустить trait

**Type:** implementation

**What to build:** После race игрок выбирает legal compatible trait из server setup, а когда trait unavailable или единственный dummy choice не требует выбора, production interaction сразу назначает следующего реального owner; Backspace никогда не останавливается на пропущенном шаге.

**Blocked by:** 03: Выбрать race с class compatibility.

**Status:** ready-for-agent

Labels: enhancement, ready-for-agent

**Production seam for TDD:** trait flow проходит через публичный `sv_endpoint_run(SvEndpointOptions)` и authoritative setup, декодированный production contact path. Seam принят P1-AC05, P1-AC14, P1-AC19 и P1-AC28; проверки используют `sv-core`.

- [ ] Доступный trait step показывает legal server-provided identities и сохраняет stable highlight при illegal, keyboard, mouse, random и доступном DNA input; compatibility учитывает выбранные race/class. [Contract: P1-AC05, P1-AC10, P1-AC11, P1-AC26; checks: `sv-core`]
- [ ] Confirm и Q/Ctrl-Q проходят через production interaction/input router, не отправляют partial/completed result и сохраняют baseline choice semantics. [Contract: P1-AC02, P1-AC05, P1-AC11, P1-AC12, P1-AC28, P1-AC29; checks: `sv-core`]
- [ ] Backspace из доступного trait возвращает race owner; unavailable/dummy trait автоматически пропускается в обоих направлениях и никогда не получает ownership. [Contract: P1-AC14, P1-AC19; checks: `sv-core`]
- [ ] Trait rules остаются SV-local, а tests вызывают production interaction state с реальным server setup. [Contract: P2-AC01, P2-AC04, P2-AC05; checks: `controller-review`]
