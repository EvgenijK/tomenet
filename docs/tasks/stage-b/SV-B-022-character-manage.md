# SV-B-022 — Новый slot и имя персонажа

Проверки платформ: [текущее правило](../../agents/platform-validation.md); Windows/MinGW/Wine отложены и не блокируют текущую приёмку.

Статус: accepted — принято пользователем 2026-10-28 вместе с этапом B.
Оставшиеся проверки сняты с текущего gate прямым решением пользователя;
они не объявлены выполненными. [Решение о приёмке](../../sv-stage-b-acceptance.md#решение-пользователя--2026-10-28).

Исторический статус до решения пользователя: partially implemented; ordinary/name/rejection и exclusive entry
через B-023 реализованы, completed-character transition и native/caller
acceptance ожидают SV-B-024.

## Пользовательский результат

Из server overview игрок выбирает обычный или разрешённый exclusive slot,
вводит имя, исправляет server rejection либо отменяет и возвращается к overview.

## Зависимости и граница

Зависит от [SV-B-006](SV-B-006-login.md). Swap/insert/append и двухшаговая
отмена reorder перенесены в C; они не нужны минимальному creation flow.

Выбор ordinary slot, native name editor/submission, cancel и recovery после
server rejection реализованы и проверены через production endpoint.
Exclusive `E`, eligibility/denial и IDDC/PvP propagation в B-023 реализованы и
проверены через production endpoint. Точный completed-character transition и
его native/caller acceptance остаются pending у
[SV-B-024](SV-B-024-birth-dna.md).

## Единственная первичная ответственность

<!-- owned-capabilities:start -->
| ID | Полный результат baseline / policy | Canonical sources |
|---|---|---|
| `capability.character.create-ordinary` | N chooses an available ordinary slot; name acceptance starts birth only when server permits creation. | `source.baseline.session-login`, `source.policy.session` |
| `capability.character.create-exclusive` | E selects dedicated creation when exclusive_ok and not first run; carry dedicated IDDC/PvP mode into completed birth. | `source.baseline.session-login`, `source.baseline.session-dna`, `source.policy.session` |
| `capability.character.name` | Use 15-byte live-trim/plus-prefix editor; empty entry generates a random suggestion and remains in editor, nonempty accepts, plus prefix requests reincarnation. Server Trim_name and ownership checks remain authoritative. | `source.baseline.session-login`, `source.protocol.session-login`, `source.policy.session` |
| `capability.character.cancel-name` | Escape returns to character overview with no character-login submission. | `source.baseline.session-login`, `source.policy.session` |
| `capability.character.name-rejected` | Server rejects forbidden/duplicate/unowned character name with retry-login or exit policy; never expose a partially entered live session. | `source.baseline.session-login`, `source.protocol.session-login`, `source.policy.session` |
<!-- owned-capabilities:end -->

## Production SV проверки

1. Ordinary/exclusive availability следует server flags/limits/first-run;
   unavailable action не отправляет request.
2. Name editor: byte limit, live trim, empty random suggestion, plus prefix,
   mouse/keyboard/input method, cancel и сохранение draft после rejection.
3. Duplicate/forbidden/unowned name, retry policy and disconnect сохраняют
   overview identity and session generation; birth не начинается преждевременно.

## Definition of Done

Actual caller повторяет SV-B-001 input primitives и SV-B-002 protocol path;
exact bytes и screen transitions зафиксированы. Следующий owner —
[SV-B-023](SV-B-023-birth-choices.md).

Текущее частичное состояние:

- done: ordinary availability/create, cancel без character request, 15-byte
  name editor, random suggestion, reincarnation prefix, exact name submission;
- done: forbidden/duplicate/unowned rejection, retry/exit/disconnect policy и
  generation-scoped draft recovery;
- done: exclusive `E`, eligibility/denial и IDDC/PvP propagation в B-023;
- pending у SV-B-024: final completed-character transition, native/caller
  runtime evidence и финальная acceptance всего DoD.

Полный Definition of Done не закрыт.


2026-10-28: интеграционные проверки M2 и исправления input lifecycle возвращены
owner в [отчёте B-025](SV-B-025-entry-complete.md#реализация-и-проверки--2026-10-28).
Это дополнительное Linux evidence; pending Linux native acceptance
и capability ownership не изменены. Windows build/runtime validation отложена
пользователем и не блокирует текущую приёмку.


## Linux protocol/core freshness — 2026-10-28

Current production-path проверки и карта применимости всех 54 B IDs описаны в
[Linux protocol/core evidence](../../sv-stage-b-linux-protocol-evidence.md).
Они дополняют историческое evidence; full native/provider и cumulative acceptance
не выводятся из dummy/software или shared-primitive PASS. Canonical denominator
составляет 54 IDs / 162 obligations; Windows-only provider остаётся deferred.
Для B-002 исправлен endpoint pong telemetry в production login/pregame.
Новая regression-проверка не добавлялась по прямому отказу пользователя; existing
checks проверяют отсутствие обнаруженных регрессий, а не отдельно новую ветвь.
