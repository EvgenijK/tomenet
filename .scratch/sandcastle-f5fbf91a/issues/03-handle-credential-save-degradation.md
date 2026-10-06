# 03: Явно обработать отказ сохранения credentials

**What to build:** После подтверждённого сервером создания аккаунта approved OS provider сохраняет exact credential bytes; если provider unavailable, locked, refused или fails, игрок продолжает подтверждённую session с явным unsaved session-only состоянием без plaintext/legacy fallback. Поздний provider result не меняет новую session.

**Blocked by:** 01: Создать неиспользованный аккаунт через production flow.

**Status:** ready-for-agent

Labels: enhancement, ready-for-agent

**Production seam for TDD:** account flow вызывается через публичный `sv_endpoint_run(SvEndpointOptions)`, а provider lifecycle — через production vault seams `sv_vault_store`, `sv_vault_poll` и `sv_vault_cancel`. Связь с P1-9 проверяется `P1-credential-policy-review`; provider doubles могут возвращать результаты, но не реализуют политику вместо production caller.

- [ ] Ни отправка account request, ни частичный server response не запускают credential store; store начинается ровно после server-confirmed authentication/overview и использует approved identity и exact password bytes. [Contract: P1-4, P1-9; checks: `sv-build`, `sv-core`, `P1-state-review`, `P1-credential-policy-review`]
- [ ] Provider unavailable, locked, refused, invalid или error остаётся видимым unsaved session-only состоянием, не закрывает подтверждённый overview, не сообщает ложный success и не читает/пишет legacy plaintext fallback. [Contract: P1-9, P2-3; checks: `sv-build`, `sv-core`, `P1-credential-policy-review`, `P2-behavior-review`]
- [ ] Cancel, disconnect или retry отменяет принадлежащий generation request; stale completion освобождается и не меняет status, secret или save result новой session. [Contract: P1-8, P1-9, P2-4; checks: `sv-build`, `sv-core`, `P1-state-review`, `P1-credential-policy-review`, `P2-production-seam-review`]
- [ ] Branch tests проходят через production account/vault caller и не сохраняют secret в assertions, logs, diagnostics, screenshots или independent SV metadata. [Contract: P1-3, P1-9, P2-4; checks: `sv-core`, `P1-production-path-review`, `P1-credential-policy-review`, `P2-production-seam-review`]
- [ ] Platform-native Linux и Windows provider checks остаются отдельными deferred acceptance checks до запуска в соответствующей среде; unit/integration coverage не отмечает их как passed. [Contract: P1-11, P1-12; checks: `P1-linux-native-runtime`, `P1-windows-native-runtime`]
- [ ] Изменения находятся в SV и не расширяют задачу до credential change/password change или account information. [Contract: P1-2, P2-1, P2-2; checks: `P1-implementation-review`, `P2-isolation-review`]
