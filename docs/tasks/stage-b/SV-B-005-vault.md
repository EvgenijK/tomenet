# SV-B-005 — Приватный ввод и системное хранилище credentials

Статус: частично реализован; полная runtime acceptance pending.
[Существующее evidence](../../sv-b005-evidence.md) сохраняется только для
актуальных IDs и должно пройти freshness review по суженной границе B.

## Пользовательский результат

Игрок вводит пароль приватно, может восстановить его из approved OS provider и
после успешной authentication сохранить точные bytes. Ошибка provider оставляет
явный session-only secret без скрытого legacy fallback.

## Зависимости и граница

Зависит от [SV-B-001](SV-B-001-endpoint.md). Account/password identity включает
точное server spelling, effective port и account bytes; DNS/IP canonicalization
или account case fold запрещены. Password change и credential change-write
перенесены в C.

## Единственная первичная ответственность

<!-- owned-capabilities:start -->
| ID | Полный результат baseline / policy | Canonical sources |
|---|---|---|
| `capability.credentials.lookup` | Lookup namespace tomenet-sv/v1 by collision-free length-prefixed raw server spelling/effective port/account tuple; Windows lowercase hex, Linux same identity. No account case fold or DNS/IP identity merge; enforce provider length bounds. | `source.behavior.settings-pass-49a7d2`, `source.acceptance.settings-policy-f2aa80`, `source.policy.session`, `source.allocation-b.s844` |
| `capability.credentials.provider-linux` | Use current-user Secret Service/libsecret binary values on Linux; provider unlock prompt remains provider-owned and startup/input stay responsive. | `source.behavior.settings-pass-49a7d2`, `source.acceptance.settings-policy-f2aa80`, `source.policy.session`, `source.allocation-b.s844` |
| `capability.credentials.provider-windows` | Use current-user generic Credential Manager CRED_TYPE_GENERIC/CRED_PERSIST_LOCAL_MACHINE on Windows10/11 i686; no roaming or own master password. | `source.behavior.settings-pass-49a7d2`, `source.acceptance.settings-policy-f2aa80`, `source.policy.session`, `source.allocation-b.s844` |
| `capability.credentials.private-input` | Passwords retain exact source bytes, field limits and protocol-star rejection; exclude history/logs/clipboard exports/dumps/reversible encodings; temporary archive passwords also private. | `source.behavior.settings-pass-49a7d2`, `source.acceptance.settings-policy-f2aa80`, `source.policy.session`, `source.allocation-b.s844` |
| `capability.account.secret-provider-failure` | Unavailable, locked, refused or failed credential provider leaves an explicit unsaved session-only credential; do not report storage success or fall back to legacy password files. | `source.baseline.session-password`, `source.policy.session` |
| `capability.account.restore-secret` | Look up the exact server spelling/effective port/account byte tuple in Linux Secret Service or Windows Credential Manager; invalid or missing secret returns to private manual entry with no hidden legacy-file fallback. | `source.baseline.session-password`, `source.policy.session` |
| `capability.account.save-secret` | After successful authentication save exact credential bytes in the OS provider; independent SV identity metadata contains no secret or reversible representation. | `source.baseline.session-verify`, `source.policy.session` |
<!-- owned-capabilities:end -->

## Production SV проверки

1. Tuple encoding различает ambiguous lengths, raw host spelling, port и account;
   provider bounds не приводят к truncation/collision.
2. Actual Secret Service и Windows Credential Manager: found/missing/locked/
   refused/unavailable/error, binary byte round-trip и current-user scope.
3. Provider prompt не блокирует UI/network; stale completion после cancel/relog
   освобождается и не меняет новый session state.
4. Secret отсутствует в history, clipboard, logs, diagnostics, screenshots,
   evidence и independent SV metadata. Success save происходит только после
   server-confirmed authentication.

## Definition of Done

Provider adapters и private field — production code. Fakes подают provider
results для branch coverage, но actual platform evidence обязательно. Полный
login caller проверяет [SV-B-006](SV-B-006-login.md), account creation —
[SV-B-021](SV-B-021-account-manage.md).
