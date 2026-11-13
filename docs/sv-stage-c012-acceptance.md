# C012 — решение о приёмке

Статус: **accepted — Linux status/core**, 2026-11-13.

Пользователь подтвердил: **«подтверждаю этап»** после согласования закрытия
технической части C012 и отсрочки интерфейсных и поздних интеграционных приёмок.
C012 и задачи SV-C012-001–007 завершены и приняты в этих границах.

## Принятый результат

Version-aware status decoding, атомарная session model, раздельные source values
и derived semantics, generation/reset, существующие HP reactions и bounded
body/idle occurrences. Финальная Linux сборка и existing focused production
status runner прошли на окончательном existing-slots freeze3; исходники и binary
стабильны. Study сохраняет исходный byte отдельно от availability, stat table
index — отдельно от packet slot ID.

[Итоговая матрица 27 outcomes / 81 obligations](tasks/c012/final-disposition.md)
и [evidence](sv-c012-evidence.md) содержат проверенные subflows, команды,
исторические failures и ограничения. Приёмка не превращает частичное core
evidence в полный runtime PASS каждого canonical obligation.

В репозитории опубликован [компактный итоговый evidence package](../.scratch/sv-c012/README.md).
Промежуточный архив снимков и журналов остаётся локальным; исторические ссылки
на него могут быть недоступны в чистом checkout.

## Сохранённые отсрочки

Все строки [C012-DA-01–10](tasks/c012/deferred-acceptance.md) остаются открытыми:
live HUD при C014; Character/small/resize при C030; settings/option forms при
C044/C046; Study при C023; bank при C027; warnings/messages/audio при
C050/C016/C055. Полные resistance matrix и export остаются C030/C029.
Новых блоков Character не добавлять.

Sanity damage и martyr сейчас представлены latest payload, без полной очереди
occurrences; поздние consumers должны обеспечить per-input обработку или bounded
retention. Actual socket adoption/close/relog C014 и map C013 не приняты этим
решением. Отложенные проверки не являются обратным gate принятого status/core.
Работы следующих этапов этой приёмкой не запускаются.

Canonical IDs, ownership, allocation, prerequisites, conditions и runtime
statuses сохраняются: их регистрация — отдельный workflow. Windows/MinGW/Wine
отложены по [правилу платформ](agents/platform-validation.md).
