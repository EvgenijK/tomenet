# SV-C011-003 — Lua startup: реальные features и preparation/handoff

Статус: **accepted** — Linux preparation/core, 2026-11-11, по решению пользователя.
Labels: enhancement
Owned IDs: `capability.session.transfer-startup-files`,
`capability.session.startup-file-failure` (6 obligations); новых IDs нет.
Также обязательная интеграция принятого C009.
Зависимости: production C009 runtime/reload, C010 transfer result contract,
preparation/handoff C011-001/002; actual live adoption/teardown выполняет C014.

## Основа и production seam

C009 и SV-C009-001–003 завершены и приняты пользователем 2026-11-10 в текущих
границах core. Эта работа обязательна C011, не блокирует и не переоткрывает C009
автоматически. [C009](../c009/README.md), [evidence](../../sv-c009-evidence.md),
[source contract](../c009/source-contract.md),
[API runtime](../../../src/client/sv/lua/runtime.h) и
[API completion](../../../src/client/sv/lua/reload.h).

После принятого C010 `transfer/startup.c` уже получает настоящие decoded flags,
строит `SvLuaFeatures`, однократно вызывает begin; каждый END синхронно
потребляет completion, summary сохраняет ранние failures. `flow.c` копирует begin
и summary в outcome. Использовать `SvLoginTransfer`,
`sv_login_lua_begin_result` и `sv_login_transfer_summary` без второго begin,
reload или FILE decoder. Ранний init result пока локален native runner.

Оставшийся gap: copied outcome не сохраняет runtime. `close_transfer_attempt`
уничтожает attachment, а `transfer/startup.c` вызывает `sv_lua_reload_close`,
очищая runtime/Guide. Preparation payload должен быть готов до этого teardown;
successful preparation сохраняет передаваемый owner, cancel/failure закрывает его.

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
2. Принять результат существующего C010 пути: реальные version-aware flags →
   `SvLuaFeatures` (`rpg/arcade/fun/party/test`, raw temporary mask) →
   `sv_lua_reload_begin(owner, generation, features)` в baseline startup порядке
   до зависимых Lua consumers. Не вызывать begin второй раз из preparation. Отсутствующие данные не подменять нулевыми
   fixture/default flags; legacy default применим только там, где его задаёт
   конкретная protocol/version branch. Сохранить provenance/version/generation.
3. Подключить к preparation результаты [C010](../c010/README.md): каждый END
   уже синхронно потребляется C010 через `sv_lua_reload_take` до следующего END, включая
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

## Актуализация readiness — 2026-11-11

Пункты о flags/begin/END выше означают подключение уже работающего C010 пути
к C011 decision/payload, а не повторную реализацию. Передавать early init result,
version/feature provenance, begin, transfer summary, текущую пригодность runtime
и применимую Guide validity. История ошибок и текущая readiness различаются:
успешный последующий reload может восстановить runtime, сохранив initial failure;
он не стирает file failure. Optional degradation не равно закрытию приложения,
зависимый обязательный consumer не получает stale state как успешную подготовку.

Проверить startup completion до readiness: PLAY_ACKNOWLEDGED недостаточно,
нужен завершённый startup stream с END. `sv_login_start_play` сейчас допускает
только creating-character branch. Реализовать actual existing-character
preparation caller/PLAY branch; existing и committed birth должны использовать
общий production serializer и достигать одной preparation boundary. Сохранить
baseline порядок reports/options/PLAY и актуальные effective resources.
Описания интерфейса без достижимого production caller недостаточно. Это не
принимает `session.enter-game`: adoption и полноценный live loop остаются C014.

Readiness должна явно учитывать: startup stream завершён; нет active transfers
или незавершённой обязательной подготовки; текущий runtime пригоден для нужных
зависимых consumers; обязательные output/ACK bytes отправлены либо удержаны
явным owner с offset/order. Факт успешного queue не означает доставки peer.
История failures передаётся отдельно; готовность транспорта к передаче owner
не подменяет успешность обновления файлов или полной identity-dependent подготовки.

Необходимые failure/readiness cases объединить с production check 001/006.
Успешный C010 native flow подтверждает flags/begin/aggregate, но не lifetime
нового C011 payload. Snapshot сохраняет все шесть canonical obligations.

В уже предусмотренном combined failure case проверить initial open failure →
исправленный файл/успешный reload: текущий runtime становится usable, initial
failure остаётся видимой. Failed publication → successful reload сохраняет
transfer ERR/failed-update result. Нельзя принимать по одному summary counter
ни вечную неготовность восстановленного runtime, ни успех ошибочного обновления.

## Comments

2026-11-11 — реализация и проверка по запросу пользователя. Existing и committed-birth используют общий PLAY serializer. Preparation принимает actual C010 flags/results, раздельную readiness/history и удерживает Lua owner. Post-END FILE обслуживает тот же C010 registry; future async CHECK/INIT продолжаются public receive_file/pump. Нет придуманного CHECK completion timer. Failure/repair/cancel/retry проверены.

[Финальное evidence, команды и ограничения](../../sv-c011-evidence.md).
Пользовательская приёмка не заявлена; Windows проверки отложены.


2026-11-11 — follow-up по запросу пользователя закрыть оставшиеся проверки самого C011. Дополнительный isolated actual-server startup подтвердил PLAY/reply/END, сохранение текущего runtime и complete payload. Необходимый preceding prefix удерживается без поздних consumers; FILE продолжает прежний C010 owner. Полный live lifecycle не переносился в C011.

Результаты и границы — [обновлённое evidence](../../sv-c011-evidence.md). Работы следующих этапов не выполнялись.


2026-11-11 — пользователь явно принял C011 и задачи 001–006 в проверенных границах Linux preparation/core. Задача завершена; [решение о приёмке](../../sv-stage-c011-acceptance.md). Исторические записи выше отражают состояние до приёмки; поздние obligations и canonical runtime statuses не повышаются автоматически.
