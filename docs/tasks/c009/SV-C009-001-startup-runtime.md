# SV-C009-001 — Lua startup runtime и reload в SV

Статус: **accepted** — Linux core, 2026-11-10, по решению пользователя.
Labels: enhancement
Owned IDs: `capability.lua.reload` (единственный canonical owner).
Зависимости: production C003 resource overlay; существующий vendored Lua engine.

## Работа

1. Сверить [source contract](source-contract.md), CONTEXT и ADR-0001–0006.
   До кодирования зафиксировать closure реальных startup scripts/bindings:
   init → flags → c-init → finish schools/spells, включая вложенные pern_dofile.
   Определить явные SV данные и ownership каждого обязательного callback.
2. Подключить существующий Lua engine к SV build с изолированными объектами.
   Адаптер и state размещать в SV; не подключать весь legacy client ради bindings,
   не изменять общие Lua sources только для удобства SV. Сохранять I/O compile
   policy baseline: `NO_CLIENT_IOLIB` исключает I/O/system library, но base library
   сохраняет native `dofile`/`dostring` с исходной семантикой. Не перехватывать их
   user/bundled resolver и не удалять; не придумывать whitelist или server/admin gate.
3. Реализовать init/open/reopen/cleanup: ранние xml/meta/audio/guide/activations,
   реальные server feature flags до c-init.lua, schools/spells finalization.
   Освободить прежние C-owned names/arrays и state при reload/закрытии. Не
   использовать константные таблицы или no-op callbacks вместо startup поведения.
4. Startup loader и вложенные `pern_dofile` читают scripts через C003 shared
   overlay → bundled fallback только при отсутствии файла. Native Lua `dofile`
   использует переданный путь как baseline, без этого resolver. Ошибку исполнения overlay не заменять bundled success.
   Сохранять пути/provenance и возвращать понятный результат missing/invalid/I/O
   failure; done flag не равен successful load. Ошибка не содержит credentials.
5. Предоставить явные runtime/reload и typed metadata interfaces задаче 002.
   Main thread владеет изменяемыми данными; длительная работа не должна лишать
   сеть/UI обслуживания. Session-derived данные и shared script files имеют
   разные lifetimes; не применять server flags старого соединения к новому.
6. Не добавлять explicit Lua prompt/`//`, macro actions, spell UI или новый
   preference Reload action. Engine готовит требуемые данные, поздние consumers
   подключают свои действия самостоятельно.

## Минимальная проверка и завершение

Через production owner реально загрузить bundled startup closure, выполнить
reload изменённого overlay и наблюдать новое значение настоящего getter.
Missing/invalid script должен дать visible failure; повторный reload/cleanup
не оставляет старые names/state. Включить эти cases в компактную проверку 003,
без отдельного framework. Linux build обязателен, Windows deferred.
Готовность — реальное исполнение required scripts/bindings и проверяемый
результат, а не успешная линковка engine или вызов mock callback.

## Результат реализации — 2026-11-10

Production core реализован агентами по поручению пользователя. Linux build,
единая focused ASan/UBSan core проверка и bounded native early-startup прошли:
[фактическое evidence](../../sv-c009-evidence.md). Runtime исполняет реальные
скрипты; transfer/session consumers C010/C011/C014 остаются в своих этапах.
Canonical statuses и planning snapshot сохранены. Задача принята и завершена
в составе C009: [решение о приёмке](../../sv-stage-c009-acceptance.md).
Поздние integrations обязательны C010/C011/C014 и не блокируют/не переоткрывают
C009 автоматически. Полная native-runtime registration остаётся отдельным workflow.
