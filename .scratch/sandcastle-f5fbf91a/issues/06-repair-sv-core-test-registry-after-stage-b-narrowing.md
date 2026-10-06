# 06: Исправить core registry после сужения Stage B

**What to build:** `sv-core` снова проходит полный актуальный набор проверок до
SV-B-021 account-creation suites и shell smoke на собранной worker revision:
runner больше не вызывает удалённый вместе с перенесённой в Stage C macro
capability script, но сохраняет все существующие production-path проверки
contact, login, first session, account creation/failures и vault. Ремонт не
возвращает Stage C behavior в Stage B и попадает в итоговую ветку только через
worker commit и assembly.

**Blocked by:** 05: Verify focused production coverage.

**Status:** resolved
**Assignee:** codex/sandcastle-f5fbf91a-wave-5-1

Labels: bug, ready-for-agent

**Production seam preserved for verification:** публичный account flow остаётся
`sv_endpoint_run(SvEndpointOptions)` с production serializers/decoders
`sv_contact_*` и `sv_login_*`, authoritative state seams
`sv_pregame_sync_login` / `sv_pregame_fail` / `sv_pregame_disconnect` и vault
lifecycle `sv_vault_store` / `sv_vault_poll` / `sv_vault_cancel`. Это
test-runner repair без изменения production behavior; прохождение этих seams
связано с P1-3—P1-9 и `P2-production-seam-review`.

- [x] На worker revision core registry содержит только существующие актуальные
  check scripts, не вызывает удалённый `sv_macro_checks.py` и не восстанавливает
  macro/options/profile/settings implementation или проверки, перенесённые за
  границу SV-B-021 в Stage C. [Contract: P1-2, P2-1, P2-8; checks: `sv-core`,
  `P1-implementation-review`, `P2-isolation-review`,
  `P2-policy-precedence-review`]
- [x] `sv-core` доходит до и успешно выполняет production-path проверки first
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
- [x] SDL resolver `Leaked thread` diagnostics remain an explicitly separate
  `docs/sv-improvements.md` proposal and are neither silently claimed fixed nor
  used to expand this account-creation repair into contact lifecycle work.
  [Contract: P2-5, P2-6, P2-7; checks: `P2-improvements-review`,
  `P2-isolation-review`]
- [x] Controlled-peer, Linux native, Windows native and SV-B-025 integration
  checks remain recorded as deferred/not run with their original reasons and
  owners; a green local core gate does not claim them. [Contract: P1-10, P1-11,
  P1-12, P1-13; checks: `P1-controlled-peer-runtime`,
  `P1-linux-native-runtime`, `P1-windows-native-runtime`,
  `P1-entry-complete-integration`]

## Answer

Согласованный production seam не менялся: account flow проходит через
`sv_endpoint_run(SvEndpointOptions)`, production `sv_contact_*` / `sv_login_*`,
authoritative `sv_pregame_sync_login` / `sv_pregame_fail` /
`sv_pregame_disconnect` и lifecycle `sv_vault_store` / `sv_vault_poll` /
`sv_vault_cancel`. Production и test sources в этом worker ticket не изменялись.

Минимальный registry repair уже находился в интеграционном предке worker revision
`d824ba90bc363875c8dca08e719971bf4f3712b0`: из core list удалены отсутствующие
Stage C `sv_macro_checks.py`, `sv_options_checks.py`, `sv_profile_checks.py` и
`sv_settings_checks.py`, а production-path `sv_first_session_checks.py` добавлен
перед обеими SV-B-021 suites. Текущий registry также сохраняет contact, endpoint,
login, account create/failure и vault checks; аудит всех перечисленных `.py`
paths не нашёл отсутствующих файлов. Macro/options/profile/settings behavior или
tests не восстанавливались. Этот worker commit фиксирует результат и evidence;
assembly обязана сохранить commit, а повтор полного core gate на assembled HEAD
остаётся за orchestrator и здесь не заявляется как выполненный.

TDD/focused red → green evidence 2026-10-06:

- Red, pre-repair registry:
  `git show d824ba90bc^:.sandcastle/checks.sh | env PATH=/opt/sv-venv/bin:/usr/local/bin:/usr/bin:/bin bash -s -- core`
  — **fail**, exit 2. Runner успешно прошёл account creation и account failure,
  затем вызвал отсутствующий `tests/sv_macro_checks.py` и остановился. Это
  воспроизведение исходной runner-регрессии, а не новый production behavior test.
- Green, текущий worker revision:
  `env PATH=/opt/sv-venv/bin:/usr/local/bin:/usr/bin:/bin bash -s -- core < .sandcastle/checks.sh`
  — **pass**, exit 0. Выполнены все зарегистрированные проверки, включая
  `sv_first_session_checks.py`, `sv_account_create_checks.py`,
  `sv_account_failure_checks.py`, `sv_vault_checks.py`, после чего прошёл
  `sv_shell_smoke.py --backend software`. Account suites компилируют и вызывают
  перечисленные production seams; test-only account implementation не добавлена.

Во время обоих прогонов наблюдался SDL resolver `Leaked thread`. Он не объявлен
исправленным и не использован для расширения scope: отдельное предложение
`Join or drain the contact resolver before SDL shutdown` остаётся в
`docs/sv-improvements.md` со status, problem, affected code, proposal и required
checks. Legacy/shared/contact lifecycle код не менялся.

Deferred acceptance checks не запускались и не считаются пройденными:

| Check | Status | Причина | Owner |
|---|---|---|---|
| `P1-controlled-peer-runtime` | **deferred; not run** | `Requires a controlled-peer runtime environment; no allowed executable controlled-peer runner was supplied for this checkpoint.` | SV-B-021 controlled-peer verification owner |
| `P1-linux-native-runtime` | **deferred; not run** | `Requires the Linux native input and credential-provider environment.` | SV-B-021 Linux native-path verification owner |
| `P1-windows-native-runtime` | **deferred; not run** | `Requires the Windows native input and credential-provider environment.` | SV-B-021 Windows native-path verification owner |
| `P1-entry-complete-integration` | **deferred; not run** | `Downstream integration evidence belongs to SV-B-025 after SV-B-021 acceptance.` | SV-B-025 owner |
