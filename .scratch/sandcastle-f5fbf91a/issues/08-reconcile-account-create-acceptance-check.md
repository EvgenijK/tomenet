# 08: Согласовать account-create acceptance check с production suites

**What to build:** Принятый `P1-account-create` запускает обе существующие
production-path suites одной обязательной проверкой: success/credential-policy
ветви и failure/parent-state ветви. Contract artifact и соответствующий
accepted-state digest согласованы с этой выполняемой командой; отсутствующий
исторически test path больше не является источником acceptance.

**Blocked by:** 07: Verify focused production coverage.

**Status:** ready-for-agent

Labels: bug, ready-for-agent

**Production seam preserved for verification:** это repair acceptance metadata,
а не изменение поведения. Реальные suites продолжают вызывать публичный
`sv_endpoint_run(SvEndpointOptions)`, production `sv_contact_*` / `sv_login_*`,
authoritative `sv_pregame_sync_login` / `sv_pregame_fail` /
`sv_pregame_disconnect` и credential lifecycle `sv_vault_store` /
`sv_vault_poll` / `sv_vault_cancel`. Эти seams приняты для P1-1–P1-10 и
production-path check `P1-account-create`.

- [ ] В worker branch команда `P1-account-create` заменена на последовательный
  запуск `python3 -B tests/sv_account_create_checks.py` и
  `python3 -B tests/sv_account_failure_checks.py`; обе части обязательны, и
  failure любой части делает check красным. [Contract: P1-1, P1-2, P1-3,
  P1-4, P1-5, P1-6, P1-7, P1-8, P1-9, P1-10, P1-15; check:
  `P1-account-create`]
- [ ] Acceptance contract, его digest и принятый Sandcastle state ссылаются на
  один и тот же runnable repository command после штатного reconciliation;
  worker commit проходит assembly, а completed workers 01–07 и их evidence не
  переписываются и не теряются. [Contract: P2-AC6, P2-AC12; checks:
  `P1-account-create`, `sv-core`]
- [ ] Repair не добавляет dummy/pass-through wrapper, не создаёт test-only
  account behavior и не ослабляет, не пропускает и не переводит
  `P1-account-create` в deferred; обе существующие suites по-прежнему
  компилируют и вызывают production SV sources. [Contract: P1-3, P1-4, P1-5,
  P1-6, P1-7, P1-8, P1-9, P1-10, P2-AC5, P2-AC6; checks:
  `P1-account-create`, `sv-core`]
- [ ] Scope остаётся repair детерминированного contract drift: production,
  legacy/shared behavior, password change и account information не меняются;
  любое обнаруженное отдельное улучшение только записывается по действующей
  improvement policy. [Contract: P1-16, P2-AC1, P2-AC2, P2-AC4, P2-AC7,
  P2-AC8, P2-AC9, P2-AC10, P2-AC11; checks: `sv-build`, `sv-core`]
- [ ] `P1-linux-native`, `P1-windows-native` и `P1-entry-integration` остаются
  **deferred; not run** с исходными причинами и owners; локальный green check не
  заявляет их выполненными. [Contract: P1-11, P1-12, P1-13; checks:
  `P1-linux-native`, `P1-windows-native`, `P1-entry-integration`]
