# 19: Продолжить automatic DNA choices через birth flow

**Type:** implementation

**What to build:** `%` включает baseline automatic reincarnation: production birth interaction последовательно применяет legal DNA identity на каждом доступном owner, пропускает только действительно unavailable steps и безопасно останавливается на missing/incompatible DNA, оставляя игроку согласованный ручной выбор без persistence.

**Blocked by:** 14: Изолировать production birth input context.

**Status:** ready-for-agent

**Assignee:** unassigned

Labels: bug, ready-for-agent

**Finding ID:** `D-36ccf3e367566848` (`birth.dna-auto-reincarnation-semantics`).

**Production seam for TDD:** публичный `sv_endpoint_run(SvEndpointOptions)` получает доступные DNA identities и реальный `%` SDL key event; native birth adapter создаёт semantic automatic-DNA command, а production `SvBirth`/input router ведёт его через owners. Outcome и socket boundary наблюдают auto state, owner/highlights/draft и отсутствие send. Seam принят P1-AC02, P1-AC03–P1-AC10 и P1-AC28; проверка — `sv-core`.

- [ ] `%` распознаётся только в active birth input context как semantic automatic-DNA command и включает `auto_reincarnation`; команда проходит generation/active validation до любой state mutation. [Contract: P1-AC10, P1-AC22, P1-AC28, P2-AC04; checks: `sv-core`, `controller-review`]
- [ ] Automatic mode последовательно применяет legal DNA для каждого доступного sex/class/race/trait/body/stats/mode owner, сохраняя те же compatibility, stable identity и confirmation semantics, что одиночный `#`. [Contract: P1-AC03, P1-AC04, P1-AC05, P1-AC06, P1-AC07, P1-AC08, P1-AC09, P1-AC10, P1-AC26; checks: `sv-core`]
- [ ] Missing, stale или incompatible DNA безопасно выключает automatic continuation на текущем owner, не подтверждает illegal value и оставляет согласованные highlight/draft для ручного keyboard/mouse продолжения. [Contract: P1-AC02, P1-AC10, P1-AC11, P2-AC04; checks: `sv-core`]
- [ ] Production-path scenarios проверяют полный `%` flow, остановку и ручное продолжение в обеих compile configurations, включая unavailable trait/body и dedicated IDDC/PvP branches; никакая ветвь не использует fixed-response/test-only wizard. [Contract: P1-AC19, P1-AC20, P1-AC28, P2-AC05; checks: `sv-build`, `sv-core`]
- [ ] Automatic DNA меняет только текущий local draft: `birth.restore-dna`/`birth.save-dna`, DNA persistence, partial/completed send и ownership SV-B-024 не добавляются. [Contract: P1-AC02, P1-AC26, P1-AC27, P1-AC29; checks: `sv-core`, `controller-review`]
- [ ] Реализация остаётся SV-local и сохраняет behavior baseline без расширения shared/legacy client. [Contract: P2-AC01, P2-AC02, P2-AC04, P2-AC09; checks: `controller-review`]

**Deferred check:** `P1-birth-runtime-evidence` / P1-AC30 остаётся deferred и не считается пройденным этим implementation ticket; evidence собирается только на full acceptance для exact accepted contract digest и HEAD.
