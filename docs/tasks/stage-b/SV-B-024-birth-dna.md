# SV-B-024 — Подтверждение создания персонажа

Проверки платформ: [текущее правило](../../agents/platform-validation.md); Windows/MinGW/Wine отложены и не блокируют текущую приёмку.

Статус: accepted — принято пользователем 2026-10-28 вместе с этапом B.
Оставшиеся проверки сняты с текущего gate прямым решением пользователя;
они не объявлены выполненными. [Решение о приёмке](../../sv-stage-b-acceptance.md#решение-пользователя--2026-10-28).

Исторический статус до решения пользователя: production implementation complete, user review passed (2026-10-28);
focused core, controlled-peer и частичные визуальные проверки пройдены,
полная runtime acceptance pending.

## Пользовательский результат

После всех обязательных birth steps клиент ровно один раз отправляет полный
versioned creation/play handshake и ждёт authoritative acknowledgement.

## Зависимости и граница

Зависит от [SV-B-023](SV-B-023-birth-choices.md). Историческое имя файла
сохранено для ссылок; DNA load/save в этом тикете больше нет и относится к C.

Handshake использует production defaults/identity bytes, требуемые полным
baseline outcome. Это не принимает profile/resource workflows, real-server
integration, `session.enter-game` или первый gameplay surface — они остаются C.

## Единственная первичная ответственность

<!-- owned-capabilities:start -->
| ID | Полный результат baseline / policy | Canonical sources |
|---|---|---|
| `capability.birth.complete` | Only after all required creation steps and dedicated mode adjustments submit final identity, stats, options, geometry and graphics/resource handshake and wait for play acknowledgement. | `source.baseline.session-play`, `source.baseline.session-dna`, `source.baseline.session-setup`, `source.policy.session` |
<!-- owned-capabilities:end -->

## Production SV проверки

1. Exact bytes/version gates для identity, stats, options, geometry, graphics,
   font/resource fields и dedicated adjustments через production serializer.
2. До завершения всех legal steps send отсутствует; double confirm, resize,
   focus и interleaved packets не дублируют final request.
3. Acknowledgement/rejection/disconnect/partial/malformed reply возвращают
   правильный owner; stale generation никогда не открывает следующий surface.
4. Guide placeholder obligation выполняется через actual caller, не через Guide
   implementation или test-only callback; после acknowledgement peer-driven
   MOTD ведёт в live-session handoff, не в gameplay surface.

## Definition of Done

Controlled peer видит точные bytes и authoritative acknowledgement. Успех
заканчивается на границе startup screen flow; gameplay presentation не требуется.
Сквозная проверка — [SV-B-025](SV-B-025-entry-complete.md).

## Результат — 2026-10-28

Работа выполнена агентами под оркестрацией без Sandcastle. Существующий
production serializer сверён с baseline `Net_start`: version gates, identity,
stats, 199 option defaults, geometry и text-only graphics/font/resource fields
не потребовали изменений. Финальный запрос использует committed birth draft,
включая dedicated adjustments.

Исправлены два lifecycle-пробела: input router больше не принимает завершение
MOTD и не сообщает completion создаваемого персонажа до play acknowledgement;
при сетевом отказе Guide placeholder закрывается вместе с birth caller,
а закрытый birth не публикует UI view.

Проверки:

- `python3 tests/sv_login_checks.py` — passed. Минимальные дополнения к
  существующим core-тестам вызывают production serializer и interaction:
  exact fixture bytes, single submission, partial acknowledgement,
  rejection/malformed reply и запрет преждевременного MOTD/handoff, включая
  skip-MOTD.
- `python3 tests/sv_birth_complete_checks.py` — passed с controlled TCP peer:
  waiting/partial/ack, точные 16266 bytes и live-session handoff после ack.

Закрытие Guide при disconnect проверено по production-path; отдельный runtime
сценарий этого edge не запускался. Полная version/build/native runtime-матрица
в этой работе не выполнялась; сквозная приёмка остаётся за B-025.

## Review и исправление — 2026-10-28

Независимые агенты проверили требования, стандарты проекта и lifecycle.
По стандартам замечаний нет. По требованиям найден и исправлен P1: baseline
сервер добавляет `PKT_END` после успешного play reply, а startup decoder
отклонял этот байт и не доходил до MOTD/handoff. Теперь `PKT_END` принимается
только после authoritative acknowledgement; до него остаётся malformed input.

Минимальные core-проверки дополнены ack с END и запретом END до ack.
Существующий controlled peer теперь отправляет END вместе с завершением reply.
Повторно прошли `tests/sv_login_checks.py` и
`tests/sv_birth_complete_checks.py`: waiting/partial сохраняют play-pending,
ack с END даёт live-session handoff при точных 16266 bytes handshake.
Исправление независимо просмотрено на lifecycle-регрессии; новых замечаний нет.

Передача следующих gameplay bytes live-session owner остаётся этапом C;
наблюдение записано в [списке улучшений](../../sv-improvements.md#preserve-live-protocol-bytes-at-the-startup-handoff).

## Comments

2026-10-28: пользователь подтвердил: «ок, мой ревью пройден».
Пользовательское review завершено. Визуальный прогон существующего test
executable с production `sv_endpoint_run` и видимыми X11/SDL окнами прошёл
waiting/partial/ack/skip; кадры Guide, MOTD и handoff просмотрены при 1024×768.
Самостоятельный `tomenet-sv`, визуальные rejection/disconnect, закрытие Guide
при disconnect и настоящее изменение размера окна не проверялись.
Подтверждение review не подменяет эти проверки и сквозную приёмку B-025.


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
