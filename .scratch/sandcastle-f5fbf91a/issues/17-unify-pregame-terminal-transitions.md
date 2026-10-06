# 17: Объединить terminal transitions pregame

Type: repair
Status: open
Assignee: unassigned
Labels: bug, ready-for-agent
Finding IDs: D-49dc8d0dd64aef31

**What to build:** Failure и disconnect продолжают быть разными публичными
terminal outcomes, но используют один private production transition для
generation validation, terminal guard, reason publication и revision update.
Так retry и parent behavior сохраняются без двух реализаций одного state rule.

**Blocked by:** 14: Verify focused production coverage.

**Production seam for TDD:** публичные `sv_pregame_fail` и
`sv_pregame_disconnect`, включая их вызов полным
`sv_endpoint_run(SvEndpointOptions)` controlled-peer flow. Это принятые seams
для terminal generation/parent behavior по P1-7 и production-path testing по
P2-AC6; test-only transition API не добавляется.

- [ ] Один private terminal-transition helper обслуживает оба публичных entry
  points, а их signatures и semantic destinations `failed`/`disconnected`
  остаются различимыми и неизменными. [Contract: P1-7, P2-AC5;
  checks: `P1-account-create`, `sv-core`]
- [ ] Production seam coverage подтверждает одинаковые прежние результаты для
  invalid input, stale generation, already-terminal state, empty/non-empty
  reason и повторной reason: корректный `SvResult`, destination phase, reason
  и revision без раннего overview. [Contract: P1-3, P1-7, P2-AC6;
  checks: `P1-account-create`, `sv-core`]
- [ ] Controlled-peer disconnect/error/retry cases через `sv_endpoint_run`
  сохраняют правильный parent, terminal reason, fresh generation и отсутствие
  overview до server confirmation. [Contract: P1-6, P1-7, P1-10;
  check: `P1-account-create`]
- [ ] Repair остаётся внутри SV, не меняет legacy/shared production sources,
  public pregame ownership или Stage C scope; focused account suites,
  `sv-build` и `sv-core` проходят. [Contract: P1-14, P1-15, P1-16, P2-AC1,
  P2-AC2, P2-AC4, P2-AC11; checks: `P1-account-create`, `sv-build`,
  `sv-core`]
- [ ] Linux native, Windows native и SV-B-025 integration остаются
  **deferred; not run** с исходными owners и без claim о выполнении.
  [Contract: P1-11, P1-12, P1-13; checks: `P1-linux-native`,
  `P1-windows-native`, `P1-entry-integration`]
