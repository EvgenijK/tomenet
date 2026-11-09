# SV-C006-007 — минимальное Linux evidence C006

Статус: accepted, 2026-11-09; принят пользователем в согласованных границах.
Labels: enhancement
Зависимости: SV-C006-001–006.
Owned IDs: нет; сверка 11 outcomes и 34 exact obligations.

## Работа

1. Сверить [полный план](../../sv-stage-c006-spec.md), [source contract](source-contract.md)
   и [snapshot](coverage.json): один owner на ID, все conditions/obligations
   сохранены. Canonical JSON и runtime statuses этим workflow не менять.
2. Выполнить Linux build и необходимый минимум production core/peer checks из
   001–006. Переиспользовать одни ресурсные fixtures и один map reload/identity
   flow для 001/002/004; общие loader/preparation cases использовать в 003/005/006.
   Расширение проверок согласуется по AGENTS; новый harness и полная visual/fault
   matrix не требуются. Windows/MinGW/Wine deferred.
3. Создать `docs/sv-c006-evidence.md`: revision/dirty state, команды, фактические
   production callers, expected/actual role/profile/source identities, glyph IDs,
   metrics/final geometry, filter/outline output, fallback/errors, saved CFG и
   wire bytes. Указать software/accelerated Linux backend и непроверенные ветви.
   Отдельно показать реальные font-based map output и prepared graphics output;
   проверка только loader/parser/enum/serialized snapshot не доказывает consumer.
4. Для всех 34 obligation IDs дать evidence или явный pending/deferred с причиной.
   Подтвердить preview/cancel/Save requested refs без подмены effective, независимость
   text/map roles, replacement failure с прежним stack, source disappearance,
   reset/reload без старых definitions, отсутствие replay на recreation и stale
   send при смене generation. Сохранять canonical backend/platform conditions;
   Windows deferred не равен PASS. Обязательный corpus 006 не заменять одной ASCII
   проверкой; найденные непригодные shipped assets получить явную disposition.
5. Reporting map-select проверить через production C003 serializer/peer с
   source-defined gates/bytes. Existing 78/79/80-byte cases переиспользовать,
   сохраняя full local references. Вернуть FI-01 evidence в C003 и source/effect
   trace в C004 по 001/002; принятые этапы не переоткрываются автоматически.
6. Отдельно перечислить поздние flows: C007 full composition/coupled graphics
   reload/FI-02, C011/C013/C014 session/map flows, C016/C053 actual PRF effects
   delivery, C044/C056 full settings/configuration. Их отсутствие не блокирует
   собственные результаты C006, но и не даёт им PASS. Собственные C006
   tile-fallback/filter/outline результаты должны быть наблюдаемы уже здесь.

## Завершение

Компактный проверяемый отчёт охватывает ровно 11 IDs/34 obligations, отличает
source analysis от runtime и не выдаёт late flows за выполненные. Непроверенные
собственные Linux outcomes остаются препятствием полному завершению C006.
Evidence не означает пользовательскую приёмку или canonical runtime registration.

## Результат реализации — 2026-11-09

Production SV реализация и минимальные Linux core проверки выполнены.
[Фактическое evidence и ограничения](../../sv-c006-evidence.md) отделяют
software resource consumer от поздних gameplay flows и непроверенных backends.
Canonical runtime registration и пользовательская приёмка не выполнялись.
