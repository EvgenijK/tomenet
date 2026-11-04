# 17: Объединить terminal transitions pregame

Type: repair
Status: resolved
Assignee: codex/sandcastle-f5fbf91a-wave-12-3
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

## Answer

Public production seams сохранены без расширения API: `sv_pregame_fail` и
`sv_pregame_disconnect` имеют прежние signatures и по-прежнему публикуют
различимые `SV_PREGAME_FAILED` / `SV_PREGAME_DISCONNECTED`. Оба entry point
теперь делегируют одному private `terminal_transition` в
`src/client/sv/session/pregame.c`; helper выполняет generation validation,
terminal guard, публикацию reason и revision transition. Legacy/shared
production sources, public pregame ownership и Stage C scope не менялись.

Production-seam coverage в `tests/sv/login-interaction.c` вызывает оба public
entry point и проверяет invalid pointer/generation, stale generation, пустую и
непустую reason, повторную reason, already-terminal переходы в обоих
направлениях, неизменность первой terminal reason и revision, отдельные
destination phases и отсутствие преждевременного authenticated overview.

TDD/characterization evidence 2026-10-06:

- Согласованные seams: public `sv_pregame_fail`, `sv_pregame_disconnect` и
  полный controlled-peer `sv_endpoint_run(SvEndpointOptions)`.
- До production refactor добавлен public-seam characterization test;
  `python3 -B tests/sv_login_checks.py` — **pass**, exit 0. Это
  behavior-preserving repair уже реализованного публичного контракта, поэтому
  честного behavior red не было и failing test/test-only API не создавались.
- После минимального private-helper refactor та же команда — **pass**, exit 0;
  после уточнения repeated non-empty reason assertion — **pass**, exit 0.
- `python3 -B tests/sv_account_create_checks.py` — **pass**, exit 0: production
  account creation и fresh-generation retry сохраняют server authority и не
  публикуют overview до полного подтверждения.
- `python3 -B tests/sv_account_failure_checks.py` — **pass**, exit 0:
  controlled-peer rejection, malformed/error, disconnect, retry и cancel
  сохраняют terminal parent/reason и отсутствие раннего overview.
- `git diff --check` — **pass**, exit 0.

Полные `make -s -C src -f makefile.sv tomenet-sv` (`sv-build`) и
`bash -s -- core` (`sv-core`) не запускались по handoff: post-assembly gates
выполняет orchestrator после объединения tickets. Наблюдавшиеся в focused suites
SDL diagnostics `Leaked thread` уже записаны в `docs/sv-improvements.md`; scope
этого repair не расширялся.

Явные acceptance deferrals сохранены без claims о выполнении:

| Check | Status | Owner |
|---|---|---|
| `P1-linux-native` | **deferred; not run** — native runtime evidence unavailable at implementation gate | `SV-B-021` |
| `P1-windows-native` | **deferred; not run** — native runtime evidence unavailable at implementation gate | `SV-B-021` |
| `P1-entry-integration` | **deferred; not run** — downstream entry-completion acceptance | `SV-B-025` |
