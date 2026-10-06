# 03: Явно обработать отказ сохранения credentials

**What to build:** После подтверждённого сервером создания аккаунта approved OS provider сохраняет exact credential bytes; если provider unavailable, locked, refused или fails, игрок продолжает подтверждённую session с явным unsaved session-only состоянием без plaintext/legacy fallback. Поздний provider result не меняет новую session.

**Blocked by:** 01: Создать неиспользованный аккаунт через production flow.

**Status:** resolved

**Assignee:** codex/sandcastle-f5fbf91a-wave-2-2

Labels: enhancement, ready-for-agent

**Production seam for TDD:** account flow вызывается через публичный `sv_endpoint_run(SvEndpointOptions)`, а provider lifecycle — через production vault seams `sv_vault_store`, `sv_vault_poll` и `sv_vault_cancel`. Связь с P1-9 проверяется `P1-credential-policy-review`; provider doubles могут возвращать результаты, но не реализуют политику вместо production caller.

- [x] Ни отправка account request, ни частичный server response не запускают credential store; store начинается ровно после server-confirmed authentication/overview и использует approved identity и exact password bytes. [Contract: P1-4, P1-9; checks: `sv-build`, `sv-core`, `P1-state-review`, `P1-credential-policy-review`]
- [x] Provider unavailable, locked, refused, invalid или error остаётся видимым unsaved session-only состоянием, не закрывает подтверждённый overview, не сообщает ложный success и не читает/пишет legacy plaintext fallback. [Contract: P1-9, P2-3; checks: `sv-build`, `sv-core`, `P1-credential-policy-review`, `P2-behavior-review`]
- [x] Cancel, disconnect или retry отменяет принадлежащий generation request; stale completion освобождается и не меняет status, secret или save result новой session. [Contract: P1-8, P1-9, P2-4; checks: `sv-build`, `sv-core`, `P1-state-review`, `P1-credential-policy-review`, `P2-production-seam-review`]
- [x] Branch tests проходят через production account/vault caller и не сохраняют secret в assertions, logs, diagnostics, screenshots или independent SV metadata. [Contract: P1-3, P1-9, P2-4; checks: `sv-core`, `P1-production-path-review`, `P1-credential-policy-review`, `P2-production-seam-review`]
- [x] Platform-native Linux и Windows provider checks остаются отдельными deferred acceptance checks до запуска в соответствующей среде; unit/integration coverage не отмечает их как passed. [Contract: P1-11, P1-12; checks: `P1-linux-native-runtime`, `P1-windows-native-runtime`]
- [x] Изменения находятся в SV и не расширяют задачу до credential change/password change или account information. [Contract: P1-2, P2-1, P2-2; checks: `P1-implementation-review`, `P2-isolation-review`]

## Answer

Использованы согласованные production seams: полный account flow через
`sv_endpoint_run(SvEndpointOptions)` и provider lifecycle через
`sv_vault_store`, `sv_vault_poll`, `sv_vault_cancel`. До работы прочитаны
корневой `CONTEXT.md`, `docs/sv-architecture.md`, ADR-0002, ADR-0003, ADR-0005,
ADR-0006 и правила local Markdown. Реализация ограничена SV account/vault
caller; legacy/shared код, password change и account information не изменялись.

`SvEndpointOutcome` теперь публикует non-secret `SvCredentialSaveState`.
Successful provider completion даёт `SAVED`; unavailable, locked, refused,
invalid, error либо невозможность начать request дают явный `SESSION_ONLY`,
сохраняя server-confirmed overview. Player-facing status также прямо сообщает
session-only состояние. Незавершённый store отменяется до финальной публикации;
retry создаёт новый generation, а stale completion старого request освобождается
и не меняет save result новой session. Plaintext/legacy fallback не добавлен.

TDD seam и red → green, команда во всех slices:
`python3 -B tests/sv_account_create_checks.py`.

- Public save outcome: red — compile error из-за отсутствующих
  `SvEndpointOutcome.credential_save` / `SV_CREDENTIAL_SAVE_SAVED`; green —
  successful provider result опубликован как `SAVED`.
- Provider degradation: red — unavailable result оставался `PENDING`; green —
  confirmed overview сохранён, outcome/status стали `SESSION_ONLY` без false
  success.
- Failure classes: red — отсутствовали `SV_VAULT_LOCKED`,
  `SV_VAULT_REFUSED`, `SV_VAULT_ERROR`; green — production caller одинаково
  безопасно обрабатывает unavailable/locked/refused/invalid/error.
- Pending cancellation: red — отменённый pending store публиковался как
  `PENDING`; green — request отменяется и публикуется `SESSION_ONLY`.
- Retry/stale-generation coverage прошёл при первом запуске после добавления и
  не заявлен как red→green: managed peer разрывает первую confirmed session,
  retry проходит второй production flow, double освобождает late completion, а
  outcome содержит только результат второго generation.

Focused verification:

- `python3 -B tests/sv_account_create_checks.py` — passed под ASan/UBSan для
  saved, unavailable, locked, refused, invalid, error, pending-cancel и
  disconnect/retry/stale-result scenarios; partial response не запускает store,
  identity/password сверяются внутри provider boundary без вывода secret.
- `python3 -B tests/sv_vault_checks.py` — passed для production vault identity,
  unavailable и изолированного integration provider path.
- `python3 -B tests/sv_first_session_checks.py` — passed для соседнего
  production startup caller, включая disconnect/retry.
- `git diff --check` — passed.

Полный build/core/review workflow не запускался: его выполняет orchestrator.
`P1-linux-native-runtime` и `P1-windows-native-runtime` остаются deferred до
соответствующих native environments; выполненные unit/integration checks не
считаются их acceptance. `P1-controlled-peer-runtime` также не отмечен passed:
managed test peer не заменяет внешний checkpoint runner. Возможностей улучшения
legacy/shared/SV вне текущего scope не обнаружено; `docs/sv-improvements.md` не
изменялся.
