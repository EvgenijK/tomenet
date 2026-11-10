# SV-C011-003 — Lua startup: реальные features и preparation/handoff

Статус: planned, 2026-11-10.
Labels: enhancement, ready-for-agent
Owned IDs: 0 новых IDs; обязательная интеграция принятого C009.
Зависимости: production C009 runtime/reload, C010 transfer result contract,
preparation/handoff C011-001/002; actual live adoption/teardown выполняет C014.

## Основа и production seam

C009 и SV-C009-001–003 завершены и приняты пользователем 2026-11-10 в текущих
границах core. Эта работа обязательна C011, не блокирует и не переоткрывает C009
автоматически. [C009](../c009/README.md), [evidence](../../sv-c009-evidence.md),
[source contract](../c009/source-contract.md),
[API runtime](../../../src/client/sv/lua/runtime.h) и
[API completion](../../../src/client/sv/lua/reload.h).

`login/native-run.c` уже владеет application `SvLuaReload` и вызывает
`sv_lua_reload_init` для ранних xml/meta/audio/guide/activations. Реальные server
features, session open/c-init и их handoff там пока отсутствуют. `login/flow.c`
сохраняет server flags в outcome; источник и version/build branches сверить с
`nclient.c` (flags перед `open_lua`) и `c-script.c:set_server_features`.

Нормативны [CONTEXT](../../../CONTEXT.md), ADR
[0002](../../adr/0002-main-thread-state-ownership.md),
[0003](../../adr/0003-explicit-module-connections.md),
[0005](../../adr/0005-session-scoped-work.md),
[0006](../../adr/0006-errors-and-overload.md) и
[правило платформ](../../agents/platform-validation.md).

## Работа

1. Передать существующий application Lua owner в actual preparation через явный
   borrowed/owned contract, сохранив user/bundled roots, diagnostics и process-sticky
   признаки accepted INIT. Не создавать второй interpreter или parser. Ранний
   `init` не означает успешный session `open`; штатное пересоздание state внутри
   `sv_lua_reload_begin` допустимо и не сбрасывает application flags.
2. Из реально полученных version-aware server flags собрать `SvLuaFeatures`
   (`rpg/arcade/fun/party/test`, raw temporary mask) и вызвать production
   `sv_lua_reload_begin(owner, generation, features)` в baseline startup порядке
   до зависимых Lua consumers. Отсутствующие данные не подменять нулевыми
   fixture/default flags; legacy default применим только там, где его задаёт
   конкретная protocol/version branch. Сохранить provenance/version/generation.
3. Подключить к preparation результаты [C010](../c010/README.md): каждый END
   синхронно потребляется через `sv_lua_reload_take` до следующего END, включая
   chained packets. C010 сохраняет результат каждого transfer и runtime/Guide
   failure в передаваемом startup aggregate; latest-result slot coordinator не
   является историей. Успех следующего END не стирает прежнюю ошибку.
4. Различать early init, session open, transfer result, reload result и применимый
   Guide metadata result. `reload_attempted`, done flag, отсутствие transfers или
   успешный Lua reload не означают успешное обновление файла и готовность startup.
   По решению пользователя last END вызывает попытку reload даже после failed
   close; ERR остаётся C010 независимо от результата чтения. Не считать
   `guide_status == NOT_READY` ошибкой там, где rebuild не требовался; при требуемом
   rebuild не выдавать старые/пустые metadata за успешно обновлённые.
5. Actual session owner определяет безопасное продолжение и readiness по этим
   результатам, baseline и ADR-0006; сообщение optional failure не равно закрытию
   приложения. Перед dependent play нельзя молча принять stale scripts за успешный
   startup. Видимая диагностика не раскрывает credentials. Подготовить явный
   payload с generation, server features, Lua owner/lifetime, aggregate results
   и готовностью для C014 до teardown login attempt. Borrowed Lua pointers не
   переживают reload/close; передавать owner либо owned copies, не dangling views.
6. Согласовать этот payload с C011-001/002 без повторного исполнения preferences,
   controls, Lua open/reload при inspection/redraw/handoff. C014 принимает owner,
   отменяет старое generation и освобождает session данные; shared files и sticky
   flags сохраняют application lifetime. C010 владеет decoder/publication/ACK;
   C011 не создаёт второй FILE handler. Если поздний gameplay callback/consumer
   ещё отсутствует, явно закрепить его за соответствующим этапом при полном
   планировании C011; не регистрировать успешный no-op ради readiness. Spell actions,
   macro runtime C053, local execution C058 и настоящий Guide остаются их owners.

## Минимальные Linux проверки и завершение

В существующей production preparation проверке дойти от полученных server flags
до реального Lua owner и handoff API: наблюдать effective globals/TEMP masks,
успех open и один applicable missing/invalid script failure с явным readiness
result. Проверить сохранение разных transfer/reload результатов и ранней ошибки
при последующем успешном completion, а также generation/lifetime payload при
retry/cancel без повторного эффекта от inspection. Использовать существующий
managed peer и core подход; direct fabricated features call подтверждает только
C009 core, не этот preparation flow. Не создавать отдельную suite/симулятор.

Готовность этой связки: actual preparation использует настоящие flags, возвращает
различимые результаты и передаёт корректный ownership contract C014. Actual
socket-to-live adoption/close проверяется C014; его отсутствие не выдаётся за PASS
и не является обратным prerequisite preparation части C011. Canonical allocation,
conditions, prerequisites и pending registry statuses не менять. Evidence своего
этапа связать с `obligation.lua.reload.result`, `.failure-lifecycle`,
`.allocation-checks` для трассировки; backlink не gate принятого C009.

Только необходимый минимум Linux production checks по [AGENTS](../../../AGENTS.md);
расширение согласовать заранее. Windows/MinGW/Wine deferred. Реализация и приёмка
этой связки не заявлены; подготовка задачи не требует запуска runtime tests.
