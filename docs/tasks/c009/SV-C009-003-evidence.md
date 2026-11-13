# SV-C009-003 — минимальное Linux evidence

Статус: **accepted** — Linux core, 2026-11-10, по решению пользователя.
Labels: enhancement
Owned IDs: 0; сверка одного outcome и трёх obligations.
Зависимости: SV-C009-001, SV-C009-002. Поведение при ошибке согласовано 2026-11-10.

## Работа

1. Сверить exact [coverage.json](coverage.json) с canonical rows/hashes: один
   owner, prerequisite resource-overlay, три obligations, без изменения статусов.
2. Выполнить Linux production build и одну компактную core проверку production
   runtime/completion API (расширить существующий подход проекта, не вводить
   новый harness). Required cases: реальный bundled startup closure; overlay
   reload с изменением getter/Guide metadata; missing/invalid script с явной
   ошибкой; ожидание последнего completion и отсутствие повторов; audio warning
   перед Guide metadata без live remap; sticky flags после accepted INIT →
   disconnect до END → last END новой session; stale generation/cleanup; reload после
   failed last close с разными transfer/reload results. Совместить связанные cases,
   не создавать по suite на каждый пункт. Применимые compile guards описать.
3. Подготовить `docs/sv-c009-evidence.md`: revision/dirty state, команды,
   expected/actual, реально исполнявшиеся scripts/bindings и source provenance,
   ошибки и непроверенные ветви. Не выдавать static audit, literal audio reader
   или callback spy за исполнение Lua.
4. Отдельно дать disposition `.result`, `.failure-lifecycle`, `.allocation-checks`.
   Прямой API вызов доказывает core; FILE_END/publication/ACK consumer остаётся
   C010, actual preparation/server flags/handoff — C011, teardown — C014.
   Сослаться на конкретные будущие owners в README; missing flows не получают PASS.
5. Зафиксировать исполнение согласованного reload после failed last close и
   сохранённый sticky lifetime. Не заявлять пользовательскую приёмку, не повышать canonical implementation/evidence status
   автоматически; core-only acceptance требует фактического решения пользователя.

Только Linux; Windows/MinGW/Wine deferred. По AGENTS добавляется необходимый
минимум core tests через production-путь; расширение набора согласуется заранее.
Протокольная matrix, новый peer, visual sweep, stress/soak и real-account runs
этим планом не вводятся. Планирование само по себе не требует запуска runtime tests.

## Результат реализации — 2026-11-10

Production core реализован агентами по поручению пользователя. Linux build,
единая focused ASan/UBSan core проверка и bounded native early-startup прошли:
[фактическое evidence](../../sv-c009-evidence.md). Runtime исполняет реальные
скрипты; transfer/session consumers C010/C011/C014 остаются в своих этапах.
Canonical statuses и planning snapshot сохранены. Задача принята и завершена
в составе C009: [решение о приёмке](../../sv-stage-c009-acceptance.md).
Поздние integrations обязательны C010/C011/C014 и не блокируют/не переоткрывают
C009 автоматически. Полная native-runtime registration остаётся отдельным workflow.
