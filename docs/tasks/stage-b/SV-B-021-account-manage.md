# SV-B-021 — Создание аккаунта

Статус: specified; реализация и runtime evidence pending.

## Пользовательский результат

Игрок создаёт новый аккаунт через те же native private fields и production
protocol path; только server response подтверждает создание и открывает overview.

## Зависимости и граница

Зависит от [SV-B-002](SV-B-002-contact.md),
[SV-B-005](SV-B-005-vault.md) и [SV-B-006](SV-B-006-login.md).
Смена пароля и account information перенесены в C.

## Единственная первичная ответственность

<!-- owned-capabilities:start -->
| ID | Полный результат baseline / policy | Canonical sources |
|---|---|---|
| `capability.account.create` | A valid unused account name and password follow server new-account handling; account flags/validation and resulting overview remain authoritative. | `source.baseline.session-credentials`, `source.protocol.session-login`, `source.baseline.session-login`, `source.policy.session` |
<!-- owned-capabilities:end -->

## Production SV проверки

1. New-account request exact bytes/version branches через production serializer;
   поля переиспользуют accepted input/private semantics без второй реализации.
2. Used/invalid name, bad password, account flag rejection, disconnect и retry
   сохраняют правильный parent и не показывают overview до server confirmation.
3. Successful authentication выполняет approved credential save policy; provider
   failure остаётся явным session-only состоянием.

## Definition of Done

Controlled peer покрывает success/rejection/cancel/error и fragmentation;
Linux/Windows native input и provider paths проверены. Результат включается в
[SV-B-025](SV-B-025-entry-complete.md).
