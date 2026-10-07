# 08: Выбрать mode и завершить draft

**Type:** implementation

**What to build:** Игрок выбирает legal mode для server flags и текущего draft, включая dedicated IDDC/PvP paths; после mode обычный draft становится готовым для будущего SV-B-024 send, а PvP Maia при обязательном post-mode trait возвращается к trait owner и затем снова к mode без преждевременной отправки.

**Blocked by:** 07: Выбрать stats.

**Status:** ready-for-agent

Labels: enhancement, ready-for-agent

**Production seam for TDD:** mode и dedicated branches проходят через публичный `sv_endpoint_run(SvEndpointOptions)` с production server setup/flags и input router. Seam принят P1-AC09, P1-AC18, P1-AC20, P1-AC28 и P1-AC29; проверки используют `sv-core`.

- [ ] Mode surface предлагает только legal server-provided/flag-authorized identities, сохраняет stable highlight и корректно обрабатывает illegal, keyboard, mouse, random и доступный DNA choice. [Contract: P1-AC09, P1-AC10, P1-AC11, P1-AC26; checks: `sv-core`]
- [ ] Dedicated IDDC и PvP birth contexts сохраняют baseline mode semantics; недоступные dedicated choices нельзя подтвердить. [Contract: P1-AC09, P1-AC20; checks: `sv-core`]
- [ ] Backspace из mode возвращает stats owner без отправки; PvP Maia, требующая trait после mode, получает trait owner и после legal trait снова возвращается к mode-ready outcome с сохранённым draft. [Contract: P1-AC18, P1-AC20; checks: `sv-core`]
- [ ] Обычный legal mode confirm формирует только session-scoped ready draft: partial result и PKT_PLAY/completed-character send отсутствуют, поскольку send остаётся у SV-B-024. [Contract: P1-AC02, P1-AC20, P1-AC29; checks: `sv-core`, `controller-review`]
- [ ] Q/Ctrl-Q и все mode branches используют production interaction/input router; implementation и tests остаются SV-local. [Contract: P1-AC11, P1-AC12, P1-AC28, P2-AC01, P2-AC04, P2-AC05; checks: `sv-core`, `controller-review`]
