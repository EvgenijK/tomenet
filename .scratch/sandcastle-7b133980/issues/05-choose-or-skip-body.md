# 05: Выбрать или пропустить body

**Type:** implementation

**What to build:** Игрок выбирает legal body modification, когда она доступна для текущего server setup и draft, либо проходит дальше без фиктивного owner; Backspace возвращает к trait или сразу к race, если trait был недоступен.

**Blocked by:** 04: Выбрать или пропустить trait.

**Status:** ready-for-agent

Labels: enhancement, ready-for-agent

**Production seam for TDD:** body flow вызывается через публичный `sv_endpoint_run(SvEndpointOptions)` и production birth interaction/input routing. Seam принят P1-AC07, P1-AC16, P1-AC19 и P1-AC28; проверки используют `sv-core`.

- [ ] Body step получает legal choices из authoritative birth setup/flags, отклоняет illegal/stale choice и сохраняет stable highlighted identity для keyboard, mouse, random и доступного DNA input. [Contract: P1-AC07, P1-AC10, P1-AC11, P1-AC26; checks: `sv-core`]
- [ ] Confirm и Q/Ctrl-Q идут через production input router, а unavailable body автоматически пропускается без назначения owner. [Contract: P1-AC07, P1-AC11, P1-AC12, P1-AC19, P1-AC28; checks: `sv-core`]
- [ ] Backspace из body возвращает trait owner, если тот существовал, иначе race owner; ни confirm, ни skip, ни Backspace не отправляют partial/completed character result. [Contract: P1-AC02, P1-AC16, P1-AC19, P1-AC29; checks: `sv-core`]
- [ ] Body behavior реализован и проверен на production SV path без test-only choice engine или ненужного shared extraction. [Contract: P2-AC01, P2-AC03, P2-AC04, P2-AC05; checks: `controller-review`]
