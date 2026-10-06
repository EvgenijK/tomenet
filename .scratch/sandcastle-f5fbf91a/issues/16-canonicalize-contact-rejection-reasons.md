# 16: Канонизировать причины contact rejection

Type: repair
Status: open
Assignee: unassigned
Labels: bug, ready-for-agent
Finding IDs: D-72dcc7c122d0a059

**What to build:** При server contact rejection сохранённая terminal reason и
показанный игроку status получают один и тот же server-shaped базовый текст из
единственного production mapping. Presentation добавляет только прежнюю
retry/exit guidance; rejected account не открывает overview и остаётся у
правильного parent.

**Blocked by:** 14: Verify focused production coverage.

**Production seam for TDD:** публичный `sv_endpoint_run(SvEndpointOptions)` с
managed controlled peer и production contact/pregame path. Это принятый seam
для rejection и parent-state поведения по P1-6, P1-7 и P2-AC6; отдельный
test-only mapper или endpoint не допускается.

- [ ] Один private production mapping определяет базовую reason для каждого
  поддерживаемого contact rejection code и unknown fallback; persisted pregame
  reason использует её без UI guidance, а presentation добавляет только
  прежнюю retry/exit guidance. Existing player-visible и persisted тексты не
  меняются. [Contract: P1-6, P1-7, P2-AC5; check: `P1-account-create`]
- [ ] Production-path rejection coverage через `sv_endpoint_run` подтверждает
  server-shaped invalid-name, bad-password и account/server-flag reasons,
  correct parent и отсутствие overview до server confirmation; обе стороны
  mapping не могут разойтись из-за двух switch tables. [Contract: P1-3, P1-6,
  P1-7, P2-AC6; check: `P1-account-create`]
- [ ] Repair остаётся внутри SV, не меняет public endpoint, contact или
  pregame contracts и не затрагивает legacy/shared production sources либо
  Stage C password/account-information scope. [Contract: P1-16, P2-AC1,
  P2-AC2, P2-AC4, P2-AC11; checks: `sv-build`, `sv-core`]
- [ ] Focused account suites, `sv-build` и `sv-core` проходят; Linux native,
  Windows native и SV-B-025 integration сохраняются как **deferred; not run**
  у прежних owners. [Contract: P1-11, P1-12, P1-13, P1-14, P1-15;
  checks: `P1-account-create`, `sv-build`, `sv-core`, `P1-linux-native`,
  `P1-windows-native`, `P1-entry-integration`]
