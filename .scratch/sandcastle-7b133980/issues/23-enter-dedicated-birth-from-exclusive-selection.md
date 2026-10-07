# 23: Войти в dedicated birth через production E selection

**Type:** implementation

**What to build:** Игрок с разрешённым exclusive slot нажимает `E` в production character overview, проходит тот же native name/server-confirmation flow и входит в dedicated IDDC или PvP birth с сохранённым intent; недоступный или first-run exclusive action не начинает creation и не отправляет request.

**Blocked by:** 08: Выбрать mode и завершить draft (completed); 22: Передать authoritative first-run state в birth.

**Status:** ready-for-agent

**Assignee:** unassigned

Labels: bug, ready-for-agent

**Finding ID:** `D-fa58f4def44cd460` (`birth.dedicated-entry-production-handoff`).

**Production seam for TDD:** публичный `sv_endpoint_run(SvEndpointOptions)` получает authoritative overview/server flags и реальные SDL `E`, name-edit и confirm events; production login interaction, pregame handoff and birth interaction carry dedicated intent, while controlled peer and outcome observe requests, owner, legal modes and draft. Seam принят P1-AC01, P1-AC02, P1-AC20 и P1-AC28; проверки — `sv-core` и `controller-review`. Test-only `birth_dedicated` injection не может быть единственным входом coverage.

- [ ] Production overview распознаёт `E` как отдельный exclusive creation action только когда authoritative limits/flags and first-run eligibility разрешают dedicated slot; unavailable action сохраняет overview и не отправляет name/character request. [Contract: P1-AC01, P1-AC02, P1-AC20, P1-AC28, P2-AC04; checks: `sv-core`, `controller-review`]
- [ ] Authorized `E` проходит native name editing, server acceptance/rejection and cancel semantics, а dedicated intent принадлежит interaction/session state и переживает handoff до начала birth без dependency на test-only options. [Contract: P1-AC01, P1-AC02, P1-AC20, P1-AC28; checks: `sv-core`, `controller-review`]
- [ ] После server-confirmed name production birth предлагает соответствующие authoritative dedicated IDDC или dedicated PvP modes и сохраняет post-mode PvP Maia trait-return semantics; ordinary `N` path остаётся ordinary. [Contract: P1-AC02, P1-AC09, P1-AC20, P2-AC04; checks: `sv-core`]
- [ ] Complete caller-to-birth scenarios проходят для IDDC и PvP server variants в обеих compile configurations через реальные `E` events; они не используют direct dedicated injection как sole entry и не добавляют fixed-response/test-only wizard. [Contract: P1-AC19, P1-AC20, P1-AC28, P2-AC05; checks: `sv-build`, `sv-core`]
- [ ] Exclusive selection, rejection, cancel and birth choices не отправляют partial/completed character result; final completed-character send остаётся у SV-B-024. [Contract: P1-AC02, P1-AC20, P1-AC29; checks: `sv-core`, `controller-review`]
- [ ] Реализация минимизирует изменения общих и legacy files и сохраняет protocol/gameplay semantics production B-022→B-023 handoff. [Contract: P2-AC01, P2-AC02, P2-AC03, P2-AC04, P2-AC09; checks: `controller-review`]

**Deferred check:** `P1-birth-runtime-evidence` / P1-AC30 остаётся deferred и не считается пройденным этим implementation ticket; evidence собирается только на full acceptance для exact accepted contract digest и HEAD.
