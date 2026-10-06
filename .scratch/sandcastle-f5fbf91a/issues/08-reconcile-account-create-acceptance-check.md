# 08: Согласовать account-create acceptance check с production suites

**What to build:** Принятый `P1-account-create` запускает обе существующие
production-path suites одной обязательной проверкой: success/credential-policy
ветви и failure/parent-state ветви. Contract artifact и соответствующий
accepted-state digest согласованы с этой выполняемой командой; отсутствующий
исторически test path больше не является источником acceptance.

**Blocked by:** 07: Verify focused production coverage.

**Status:** resolved
**Assignee:** codex/sandcastle-f5fbf91a-wave-7-1

Labels: bug, ready-for-agent

**Production seam preserved for verification:** это repair acceptance metadata,
а не изменение поведения. Реальные suites продолжают вызывать публичный
`sv_endpoint_run(SvEndpointOptions)`, production `sv_contact_*` / `sv_login_*`,
authoritative `sv_pregame_sync_login` / `sv_pregame_fail` /
`sv_pregame_disconnect` и credential lifecycle `sv_vault_store` /
`sv_vault_poll` / `sv_vault_cancel`. Эти seams приняты для P1-1–P1-10 и
production-path check `P1-account-create`.

- [x] В worker branch команда `P1-account-create` заменена на последовательный
  запуск `python3 -B tests/sv_account_create_checks.py` и
  `python3 -B tests/sv_account_failure_checks.py`; обе части обязательны, и
  failure любой части делает check красным. [Contract: P1-1, P1-2, P1-3,
  P1-4, P1-5, P1-6, P1-7, P1-8, P1-9, P1-10, P1-15; check:
  `P1-account-create`]
- [x] Acceptance contract, его digest и принятый Sandcastle state ссылаются на
  один и тот же runnable repository command после штатного reconciliation;
  worker commit проходит assembly, а completed workers 01–07 и их evidence не
  переписываются и не теряются. [Contract: P2-AC6, P2-AC12; checks:
  `P1-account-create`, `sv-core`]
- [x] Repair не добавляет dummy/pass-through wrapper, не создаёт test-only
  account behavior и не ослабляет, не пропускает и не переводит
  `P1-account-create` в deferred; обе существующие suites по-прежнему
  компилируют и вызывают production SV sources. [Contract: P1-3, P1-4, P1-5,
  P1-6, P1-7, P1-8, P1-9, P1-10, P2-AC5, P2-AC6; checks:
  `P1-account-create`, `sv-core`]
- [x] Scope остаётся repair детерминированного contract drift: production,
  legacy/shared behavior, password change и account information не меняются;
  любое обнаруженное отдельное улучшение только записывается по действующей
  improvement policy. [Contract: P1-16, P2-AC1, P2-AC2, P2-AC4, P2-AC7,
  P2-AC8, P2-AC9, P2-AC10, P2-AC11; checks: `sv-build`, `sv-core`]
- [x] `P1-linux-native`, `P1-windows-native` и `P1-entry-integration` остаются
  **deferred; not run** с исходными причинами и owners; локальный green check не
  заявляет их выполненными. [Contract: P1-11, P1-12, P1-13; checks:
  `P1-linux-native`, `P1-windows-native`, `P1-entry-integration`]

## Answer

Согласованный production seam сохранён без изменений: обе suites по-прежнему
компилируют production SV sources и проверяют публичный
`sv_endpoint_run(SvEndpointOptions)`, production `sv_contact_*` / `sv_login_*`,
authoritative `sv_pregame_sync_login` / `sv_pregame_fail` /
`sv_pregame_disconnect` и credential lifecycle `sv_vault_store` /
`sv_vault_poll` / `sv_vault_cancel`. Production, legacy/shared account behavior,
password change и account information не менялись; test-only account behavior
и новый improvement proposal не добавлялись.

`P1-account-create` теперь хранит безопасную последовательность двух прямых
repository-команд:

1. `python3 -B tests/sv_account_create_checks.py`
2. `python3 -B tests/sv_account_failure_checks.py`

Sandcastle валидирует каждый элемент по прежним ограничениям focused command,
исполняет их по порядку и прекращает check при первом failure. Поэтому green
возможен только после успеха обеих suites; shell operators и pass-through
wrapper не используются. Contract drafting/reconciliation schemas и guidance
описывают тот же формат. Tracked contract artifact валидирован против его
authoritative sources; исторический digest
`e706e7e1a96fefbddcf8bd3f0d9ac43f5e9e5d5bc2e9516c66f7ac9bc2b32223`
заменён на `8c107ecd47aeac23a221871fcb80ad5c9f87846ba44b10a1d97a00a0d2bd82c0`.
Этот artifact является входом штатного controller reconciliation после
assembly: accepted state должен принять ровно этот contract/digest; worker не
переписывает ignored controller state. Tickets/workers 01–07 и их evidence не
изменены и не удалены. Проверка нового digest и gates на assembled HEAD остаётся
за зависимым ticket 09.

TDD на публичных controller seams (SV behavior не менялся):

- Red: `node --test tests/sandcastle_core_checks.mjs` — **fail**, exit 1:
  `runAcceptanceCheck` передал command array как одну команду вместо запуска
  обеих suites.
- Green первого slice: та же команда — **pass**, exit 0: sequence runner
  выполнил обе команды и вернул failure второй suite как failure общего check.
- Red второго slice: та же команда — **fail**, exit 1: `validateContract`
  отклонил безопасный command array.
- Green второго slice: та же команда — **pass**, exit 0: непустая безопасная
  последовательность принята, пустая или содержащая недопустимую команду
  отклоняется.

Focused verification 2026-10-06:

- `node --test tests/sandcastle_core_checks.mjs tests/sandcastle_contract_draft.mjs tests/sandcastle_contract_reconcile.mjs tests/sandcastle_contract_prompts.mjs`
  — **pass**, exit 0, 19/19 tests.
- `python3 -B tests/sv_account_create_checks.py && python3 -B tests/sv_account_failure_checks.py`
  — **pass**, exit 0; production success/credential-policy и
  failure/parent-state suites прошли последовательно.
- Contract validation и пересчёт digest через production
  `.sandcastle/acceptance.mjs` — **pass**, digest совпал с artifact.

Наблюдавшийся ранее SDL resolver `Leaked thread` снова присутствовал в focused
SV output; он не заявлен исправленным и остаётся отдельным уже записанным
improvement. Полный `sv-build` / `sv-core` / review workflow не запускался —
assembled gates принадлежат ticket 09 и orchestrator.

Deferred acceptance checks не запускались и не считаются пройденными:

| Check | Status | Причина | Owner |
|---|---|---|---|
| `P1-linux-native` | **deferred; not run** | `Requires Linux native input and credential-provider runtime evidence that is unavailable at the implementation gate.` | `SV-B-021` |
| `P1-windows-native` | **deferred; not run** | `Requires Windows native input and credential-provider runtime evidence that is unavailable at the implementation gate.` | `SV-B-021` |
| `P1-entry-integration` | **deferred; not run** | `This is downstream integration evidence owned by the later SV-B-025 entry-completion task.` | `SV-B-025` |
