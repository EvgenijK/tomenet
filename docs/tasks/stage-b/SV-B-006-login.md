# SV-B-006 — Login и выбор существующего персонажа

Статус: частично реализован для existing-account/existing-character path;
полная runtime acceptance pending.
[Существующее evidence](../../sv-b006-evidence.md) сохраняется только для
актуальных IDs и должно пройти freshness review по суженной границе B.

## Пользовательский результат

Игрок вводит account/password, получает server-confirmed overview, читает MOTD,
выбирает существующего персонажа либо выходит. Ошибка или disconnect возвращает
к правильному startup owner и не создаёт ложную игровую сессию.

## Зависимости и граница

Зависит от [SV-B-001](SV-B-001-endpoint.md),
[SV-B-002](SV-B-002-contact.md) и [SV-B-005](SV-B-005-vault.md).
Выбор персонажа вместе с peer-driven MOTD заканчивается явным состоянием
ожидания live-session handoff; startup profile/FILE/Lua, `session.enter-game` и
первый gameplay screen принадлежат C.

## Единственная первичная ответственность

<!-- owned-capabilities:start -->
| ID | Полный результат baseline / policy | Canonical sources |
|---|---|---|
| `capability.account.enter-name` | Interactive account name has 15 payload bytes and live trimming; CLI/config producers retain their own boundaries and server validation. | `source.baseline.session-credentials`, `source.protocol.session-login`, `source.policy.session` |
| `capability.account.cancel-name` | Escape from account-name entry exits startup; it does not silently authenticate a stored identity. | `source.baseline.session-credentials`, `source.policy.session` |
| `capability.account.enter-password` | Accept private password bytes up to 15 interactive payload bytes; empty input retries, no normalization, history or diagnostic capture. | `source.baseline.session-password`, `source.policy.session` |
| `capability.account.cancel-password` | Escape from password entry returns to account-name selection without a login packet. | `source.baseline.session-password`, `source.policy.session` |
| `capability.account.reject-unencodable-password` | For server_protocol >= 2 reject login containing star before XOR42 creates a NUL; preserve private draft for correction and send no partial packet. | `source.baseline.session-verify`, `source.baseline.session-change-password`, `source.policy.session` |
| `capability.account.authenticate` | Verify credentials and receive account/character overview; success is server-confirmed, never inferred from sending login. | `source.baseline.session-verify`, `source.baseline.session-login`, `source.protocol.session-login`, `source.policy.session` |
| `capability.account.login-rejected` | Wrong password, invalid name or server rejection shows failure and returns to the appropriate retry owner or exits when RETRY_LOGIN is disabled; clear invalid defaults as baseline requires. | `source.baseline.session-login`, `source.protocol.session-login`, `source.policy.session` |
| `capability.character.read-overview` | Render the terminated server character list with names, modes, levels, race/class and available location; preserve server flags, total/dedicated limits and first-run restrictions. | `source.baseline.session-login`, `source.policy.session` |
| `capability.character.select-existing` | Select an owned slot by letter and complete login status handling before starting play; command-line/default character selection must resolve identically. | `source.baseline.session-login`, `source.policy.session` |
| `capability.character.quit-overview` | Q or Ctrl-Q exits the overview through the baseline connection cleanup without selecting a character. | `source.baseline.session-login`, `source.policy.session` |
| `capability.session.read-motd` | Present received MOTD; acknowledge via a nonzero key after the baseline wait, returning to the startup/play-handshake owner before Net_start; gameplay input is not yet enabled. | `source.baseline.session-motd`, `source.baseline.session-connect`, `source.policy.session` |
| `capability.session.disconnect` | Invalidate session requests, macros and stale input; present reason and take baseline reconnect/exit branch without dispatching pending gameplay or fabricating replies. | `source.baseline.session-quit`, `source.baseline.session-connect`, `source.policy.session` |
<!-- owned-capabilities:end -->

## Production SV проверки

1. Exact field bounds/trim/empty/cancel/star rejection and outgoing login bytes
   for supported versions; private draft survives recoverable rejection.
2. Overview termination, row data, server flags, limits and first-run branches;
   selection by keyboard/mouse/default resolves one stable slot identity.
3. MOTD wait accepts only permitted acknowledgement and continues to the same
   startup owner; gameplay input remains disabled.
4. Wrong password/name, retry disabled, quit and disconnect at every step clear
   generation-scoped pending work and show the exact reason once.

## Definition of Done

The controlled peer drives the production decoder/model/UI/serializer path with
split/chained input and exact expected/actual bytes. Actual SDL physical input,
focus/resize and Linux/Windows evidence are present. Full existing-character
flow is checked by [SV-B-020](SV-B-020-first-session.md).
