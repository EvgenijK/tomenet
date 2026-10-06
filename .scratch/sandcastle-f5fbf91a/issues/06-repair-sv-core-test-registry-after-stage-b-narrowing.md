# 06: Исправить core registry после сужения Stage B

**What to build:** `sv-core` снова проходит полный актуальный набор проверок до
SV-B-021 account-creation suites и shell smoke на собранной worker revision:
runner больше не вызывает удалённый вместе с перенесённой в Stage C macro
capability script, но сохраняет все существующие production-path проверки
contact, login, first session, account creation/failures и vault. Ремонт не
возвращает Stage C behavior в Stage B и попадает в итоговую ветку только через
worker commit и assembly.

**Blocked by:** 05: Verify focused production coverage.

**Status:** ready-for-agent

Labels: bug, ready-for-agent

**Production seam preserved for verification:** публичный account flow остаётся
`sv_endpoint_run(SvEndpointOptions)` с production serializers/decoders
`sv_contact_*` и `sv_login_*`, authoritative state seams
`sv_pregame_sync_login` / `sv_pregame_fail` / `sv_pregame_disconnect` и vault
lifecycle `sv_vault_store` / `sv_vault_poll` / `sv_vault_cancel`. Это
test-runner repair без изменения production behavior; прохождение этих seams
связано с P1-3—P1-9 и `P2-production-seam-review`.

- [ ] На worker revision core registry содержит только существующие актуальные
  check scripts, не вызывает удалённый `sv_macro_checks.py` и не восстанавливает
  macro/options/profile/settings implementation или проверки, перенесённые за
  границу SV-B-021 в Stage C. [Contract: P1-2, P2-1, P2-8; checks: `sv-core`,
  `P1-implementation-review`, `P2-isolation-review`,
  `P2-policy-precedence-review`]
- [ ] `sv-core` доходит до и успешно выполняет production-path проверки first
  session, account creation, account failures и vault, а затем shell smoke;
  исправление не скрывает отсутствующий актуальный script и не заменяет
  production seam test-only behavior. [Contract: P1-1, P1-3, P1-4, P1-5,
  P1-6, P1-7, P1-8, P1-9, P2-3, P2-4; checks: `sv-core`,
  `P1-production-path-review`, `P1-protocol-review`, `P1-state-review`,
  `P1-credential-policy-review`, `P2-behavior-review`,
  `P2-production-seam-review`]
- [ ] Worker records the focused failing command before the registry repair and
  the passing `sv-core` result after it; assembly preserves the worker commit
  and the same full core gate passes at assembled HEAD. [Contract: P2-3, P2-4,
  P2-9; checks: `sv-core`, `P2-production-seam-review`, `P2-workflow-review`]
- [ ] SDL resolver `Leaked thread` diagnostics remain an explicitly separate
  `docs/sv-improvements.md` proposal and are neither silently claimed fixed nor
  used to expand this account-creation repair into contact lifecycle work.
  [Contract: P2-5, P2-6, P2-7; checks: `P2-improvements-review`,
  `P2-isolation-review`]
- [ ] Controlled-peer, Linux native, Windows native and SV-B-025 integration
  checks remain recorded as deferred/not run with their original reasons and
  owners; a green local core gate does not claim them. [Contract: P1-10, P1-11,
  P1-12, P1-13; checks: `P1-controlled-peer-runtime`,
  `P1-linux-native-runtime`, `P1-windows-native-runtime`,
  `P1-entry-complete-integration`]

