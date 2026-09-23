# Возможные улучшения TomeNET SV и существующего кода

Список ведётся отдельно от текущих implementation tickets. Политика изоляции:
[AGENTS.md](../AGENTS.md). Статус «предложено» означает отдельную будущую работу,
а не уже выполненное изменение или автоматическое расширение текущего тикета.

## SV-IMP-001 — Локализовать существующие SV-зависимости от вынесенных helpers

**Статус:** реализовано в тикете
[SV-ARCH-002 — изоляция SV](tasks/SV-ARCH-002-isolate-sv-from-legacy-changes.md),
helpers локализованы в SV, legacy/common восстановлены; 2026-09-22.

**Проблема:** тикеты 02–04 выделили helpers с изменениями вызовов в legacy,
тогда как текущий приоритет — минимальное затрагивание существующего клиента.

**Код:** `src/client/nclient.c`, `src/client/hp-update.h`,
`src/client/message-update.h`, `src/client/key-request.h`,
`src/common/common.c`, `src/common/version-newer.inc`, `src/client/sv/version.c`.

**Предложение:** перенести необходимые реализации в область SV; рассмотреть
восстановление затронутых legacy-вызовов и определения `is_newer_than()` в
`common.c`. Сохранить алгоритмы, выбранные версии пакетов и проверки границ в SV.
Сравнивать конкретные участки с актуальным upstream, сохраняя независимые
upstream-изменения. Перенос не требует создавать ещё один декодер для тестов.

**Проверки:** SV и legacy builds; HP, message и request scenarios, точные ответы,
fragmentation/adjacent packets, malformed fields; sanitizer и native regressions.
Отдельно проверить, какие тесты сейчас используют вынесенные helpers.

## SV-IMP-002 — Безопасные границы строковых полей в legacy protocol

**Статус:** защита реализована только в SV; исправление legacy и общий аудит предложены.

**Проблема:** `Packet_scanf` в `src/common/sockbuf.c` может закончить чтение
переполненного `%s`/`%S`/`%I` усечением строки, оставив остаток поля в потоке.
Следующий разбор может принять этот остаток за новый пакет.

**Текущее состояние:** проверки NUL внутри допустимого slot добавлены в
`src/client/sv/protocol/message-update.h` и `src/client/sv/protocol/key-request.h`. При выполнении
SV-ARCH-002 общие вызовы удалены из `nclient.c`: legacy-защита снята вместе
с восстановлением baseline. Общий scanner и legacy-обработчики не исправлены.

**Предложение:** отдельной задачей проверить вызывающие стороны, согласовать
поведение ошибок и устранить проблему в нужных legacy-путях. При локализации
SV по SV-IMP-001 сохранить в этом списке факт удаления legacy-защиты, если она
будет удалена вместе с общими вызовами.

**Проверки:** длины 0/limit−1/limit/limit+1, все границы фрагментации, следующий
пакет, отсутствие частичного apply, корректный disconnect и совместимость
применимых client/server пользователей scanner.

## SV-IMP-003 — Выделение самостоятельного модуля сравнения версий

**Статус:** предложено только при появлении самостоятельной потребности в общем модуле.

**Проблема:** ранее `is_newer_than()` включался через `version-newer.inc`.
SV-ARCH-002 устранил текстовое включение: теперь есть baseline-определение в
`common.c` и намеренная локальная копия в SV `version.c`. Само дублирование
не является дефектом; общий модуль имеет смысл только при отдельной потребности.

**Предложение:** если общий модуль понадобится независимо от SV, выделить
сравнения версий в обычный небольшой модуль с явными build dependencies.
До этого локальное дублирование для SV допустимо; унификация не обязательна.

**Проверки:** равенство, все компоненты версии, историческое поведение при
`major == 0`, пороги выбора wire layouts и сборки затронутых клиентов/сервера.

## SV-IMP-004 — Явный предел очереди legacy-макросов

**Статус:** предложено отдельно; legacy не изменён в тикете 05.

**Проблема:** `Term_key_push_buf_aux` увеличивает размер очереди удвоением
до размещения action без явного общего предела. Для legacy не определена
пользовательская политика отказа при чрезмерном накоплении ввода. Локальная
ограниченная очередь SV решает свою задачу, но не меняет это поведение legacy.

**Код:** `src/client/z-term.c:Term_key_push_buf_aux`,
`src/client/c-util.c:inkey_aux` и вызов `Term_key_push_buf`.

**Предложение:** отдельной задачей определить разумный предел, безопасную
арифметику размера и явное поведение при отказе без частичного исполнения или
молчаливого удаления macro action. Сохранить baseline-порядок обычных макросов.

**Проверки:** заполненная очередь, action на границе/выше предела, сохранение
порядка при расширении, отказ выделения памяти, отсутствие частичного dispatch
и регрессии normal/hybrid/command macros на существующих клиентах.

## SV-IMP-005 — Подготовка неизменившегося текста SV между кадрами

**Статус:** предложено отдельно; не реализовано в тикете 06.

**Проблема:** `sv_font_draw` создаёт surface и texture при каждом draw даже для
неизменившихся строк shell. Это повторяет работу при постоянном repaint;
измерения тикета 06 сами по себе не требуют менять этот путь.

**Код:** `src/client/sv/ui/font.c:sv_font_draw`, `draw_surface`,
`src/client/sv/ui/ui.c:sv_ui_draw`.

**Предложение:** при подтверждённой измерениями потребности добавить ограниченный
кэш подготовленного текста с явным владением и инвалидированием по font/resource,
output scale, renderer lifetime, text и color. Сохранить подготовку в final output
size и композицию 1:1; не расширять это до архива кадров.

**Проверки:** реальный decode/input-to-submission до/после, TTF/PCF на целевых
масштабах, renderer reset, смена ресурсов, очистка при teardown и отсутствие
роста кэша от истории сообщений.

## SV-IMP-006 — Группировка исходников SV по ответственности

**Статус:** выполнено в [SV-ARCH-003](tasks/SV-ARCH-003-group-sv-sources-by-module.md#результат--2026-09-22); Linux checks passed, MinGW/Wine unverified.

**Проблема:** плоский каталог `src/client/sv` смешивает orchestration, session
model, protocol, input, rendering и diagnostics; при росте клиента труднее
находить владельца поведения.

**Код:** `src/client/sv/*`, `src/makefile.sv`, пути исходников и includes в
`tests/sv*_checks.py`, `tests/sv/` и `src/temporary/sv/`.

**Предложение:** оставить `main.c`, `app.[ch]`, `result.[ch]` в корне SV;
сгруппировать session/model и alerts в `session/`, protocol/version/декодеры
в `protocol/`, router и native input adapter в `input/`, renderer/font/status
presentation/message text в `ui/`, submission timing в `diagnostics/`.
Использовать явные includes от корня SV, например `session/session.h` и
`ui/ui.h`; не добавлять все подкаталоги в include search path. Сохранить текущие
interfaces и игровое поведение. Временный peer и scenarios оставить в их
существующих отдельных каталогах. Перенос не требует изменений legacy.

**Проверки:** чистая Linux-сборка с вложенными object/dependency paths,
headless и native SV regression suites, доступный MinGW smoke, отсутствие
ссылок на прежние пути в активных build/test callers. Исторические отчёты
сохраняют привязку к проверенным версиям; текущую архитектурную справку обновить.

**Результат:** 24 файла перенесены по указанным группам; includes и build/test
callers адаптированы. Чистая и incremental Linux-сборки, все 23 доступных
regression runners и Standards/Spec review прошли. Реализации сохранены;
MinGW/Wine блокируются отсутствующими cross development dependencies.

## SV-IMP-007 — Уточнить устаревшие сводки input inventory

**Статус:** предложено отдельно; в тикете 08 расхождения учтены в reconciliation,
исходные legacy-функции и исторический аудит не изменены.

**Проблема:** строки `input.command.minimap` и `input.command.target` в
`docs/research/single-window-input-loops.md` смешивают правила разных владельцев:
выход из locate описан как выход из overview; hostile target на неизвестной
клавише завершается, а friendly target вообще не открывает modal loop.
Использование сводки без проверки исходника может задать неверный SV-контракт.

**Затронутый код:** `src/client/c-cmd.c`: `cmd_mini_map`, `cmd_locate`,
`cmd_target`, `cmd_target_friendly`, `cmd_look`; потребители input inventory.

**Предложение:** отдельным обновлением исторического аудита разделить владельцев
и указать точные cancel/retry правила, сохранив provenance старого снимка.
Текущие правильные dispositions находятся в
`docs/capabilities/session-reconciliation.md`.

**Проверки:** сверить клавиши и исходящие команды каждого владельца с текущими
исходниками для обоих keysets; при будущей реализации SV проверить production
input→command путь, nested target cancel и восстановление родительского контекста.

## SV-IMP-008 — Уточнить исторический inventory-view input audit

**Статус:** предложено отдельно; тикет 09 фиксирует правильные dispositions,
legacy и исходный исторический аудит не изменены.

**Проблема:** строка `input.command.inventory-view` в
`docs/research/single-window-input-loops.md` описывает одно действие с выходом и
examine по букве. Текущие владельцы продолжают цикл после многих дочерних команд;
uppercase выполняет прямой paste через `Send_paste_msg` и завершает просмотр.
В equipment takeoff вызывается буквальной `t`, независимо от gameplay keyset.

**Затронутый код:** `src/client/c-cmd.c`: `cmd_inven`, `cmd_equip`, `cmd_subinven`;
потребители input inventory и будущие SV bindings этих поверхностей.

**Предложение:** отдельно уточнить историческую сводку по каждому владельцу,
разделив продолжение цикла, возврат дочернего действия, paste и закрытие списка.
Контракты текущего переноса сохранены в `docs/capabilities/item-reconciliation.md`.

**Проверки:** сверить все switch branches и прямой uppercase/newest paste,
normal/roguelike и `ENABLE_SUBINVEN`; проверить production input→command путь,
возврат в shopping/final-review и очистку bag redirection без дублирования send.

## SV-IMP-009 — Уточнить сводки information/DM inventory

**Статус:** предложено отдельно; тикет 10 сохраняет фактический baseline,
legacy и исторические инвентаризации не изменены.

**Проблема:** input inventory ошибочно описывает 2/8 как переключение страниц
character sheet, h как export, а local lore как серверный special-file запрос.
Сводка DM выхода объединяет Escape и Ctrl-Q, хотя в `cmd_master` Ctrl-Q лишь
выходит из switch, а цикл завершается при Escape. Аналогично устроены `cmd_purchase_house` и
`cmd_house_chown`. Player-пункты 2/4/6 означают acquirement/static/delete,
а отмена editor всё равно отправляет префикс команды. Успешная player-команда
закрывает весь DM menu. Эти различия могут потеряться при переносе в SV.

**Затронутый код:** `src/client/c-cmd.c`: `cmd_character`, `artifact_lore`,
`monster_lore`, `cmd_master`, `cmd_master_aux_player`;
`docs/research/single-window-input-loops.md` и его потребители.

**Предложение:** отдельно уточнить исторические строки по владельцам и вынести
вопрос об изменении Ctrl-Q в самостоятельное решение. Корректные dispositions
текущей задачи — в `docs/capabilities/information-reconciliation.md`.

**Проверки:** сверить все ветки page/topic/export/local-lore/DM exit с исходниками;
проверить production SV input→command путь, отмену дочерних prompts и возврат
в фактического родителя. Исправление legacy-поведения не входит в эту запись.

## SV-IMP-010 — Согласовать legacy CFG и пути личных файлов

**Статус:** предложено отдельно; тикет 11 фиксирует approved SV dispositions,
legacy loaders/writers не изменены.

**Проблема:** CFG reader распознаёт префикс `sound`, исключая `soundpackFolder`,
но не `soundpackSubset`; subset может ошибочно менять audio enable. Кроме того,
SDL3 load истории/закладок использует `os_temp_path`, а save — пользовательский
каталог, поэтому чтение и запись могут обращаться к разным файлам.

**Затронутый код:** `src/client/client.c:read_mangrc`,
`src/client/c-init.c` (load/save chat history и Guide bookmarks).

**Предложение:** отдельно исправить полное сопоставление CFG token и симметрию
legacy путей, заранее определив совместимость со старыми файлами. SV использует
собственный утверждённый CFG/history owner и не требует исправлять legacy.

**Проверки:** sound/soundpackFolder/soundpackSubset во всех порядках;
read→save→reload обоих subset keys; TMPDIR и SDL3 user root в разных каталогах;
пустая/непустая история, relog и ошибки записи без потери старого файла.

## Incremental extraction of baseline inventory rows

**Status:** proposed separately; ticket 12 uses content-addressed audits and fails
completeness when their source bytes change.

**Problem:** current Markdown inventories contain historical line numbers, labels
and subtotals. Content identity reliably detects drift, but a reviewer must locate
new packet/version/input/field obligations manually. For example, the physical SDL3
owner is now `react_keypress`, and the historical `Receive_keypress` label suggests
input injection although its production body is a payload-free stub.

**Affected code:** `tools/reconcile_capabilities.py`,
`docs/capabilities/inventories/`, client packet registration/decoders and input
owners in `src/client/nclient.c`, `c-util.c`, `main-sdl3.c`.

**Proposal:** add focused source extractors that produce reviewable candidate row
changes for registration, wire-format and conditional input sites. Keep semantic
outcome mapping and exclusions explicitly reviewed; do not auto-certify a caller
from a shared handler name or silently accept a refreshed hash.

**Required checks:** additions/removals under compile/version guards, same-name
changed bodies, dead/commented code, response-only paths and deliberate NULL
registrations; generated candidates must exercise the production validator and
must not alter legacy/SV runtime behavior.

## Capture reviewed native dependency inventories from build outputs

**Status:** proposed separately; ticket 13 validates supplied directory inventories.

**Problem:** fingerprints detect drift within a declared scope, but cannot prove
that a producer included every transitive SDK, build-script or resource-loader
input. Reviewers currently establish that closure manually.

**Affected code:** `tools/native_evidence.py`, `src/makefile.sv`,
`src/client/sv/ui/font.c`, native scenario evidence producers.

**Proposal:** generate candidate dependency inventories from compiler `.d` files,
link metadata and explicit resource/fixture roots, retaining reviewed directory
boundaries to catch newly introduced inputs. Keep unknown-impact snapshots broad;
do not treat automatically collected paths as semantic acceptance.

**Required checks:** header additions outside existing `.d` entries, changed SDKs
and build switches, resource discovery changes, untracked files, missing roots,
and unchanged Git HEAD; validate all generated artifacts through the production CLI.

## Record runtime fallback absence before promoting Stage A observations

**Status:** proposed separately; ticket 15 reports observations with pending claims.

**Problem:** native scenarios currently print `fallback_routes=0` as a literal.
The current binary has no terminal adapter, but that diagnostic is not the
completed runtime counter required by the evidence contract. Promoting its log
as accepted evidence would overstate the measured guarantee.

**Affected code:** `src/client/sv/main.c`, `tests/sv/scenarios/*-scenario.c`,
`tools/native_evidence.py`, `docs/capabilities/native-evidence.json`.

**Proposal:** establish a production-owned scenario lifecycle/check record and
instrument any future fallback transfer before entering the adapter. Export only
bounded route metadata; pair it with reviewed dependency inventories (above).
Keep existing observations pending until these evidence obligations are met.

**Required checks:** completed native scenario with no entries, incomplete check,
entry during child interaction/cancellation, session reset, registered and unknown
routes, and exact executable/report/dependency fingerprints through the production
evidence checker. No prompt or account data in runtime metadata.

## Profile Wine Direct3D first prompt submission

**Status:** proposed separately; ticket 16 retains the failed timing gate.

**Problem:** on the ticket 16 i686 Wine environment, the first prompt submission
through SDL's actual `direct3d` renderer repeatedly took 30–42 ms against the
20 ms urgent budget. Software passed. Other Direct3D production scenarios ran;
this is a measured latency failure, not renderer unavailability.

**Affected code:** `src/client/sv/ui/font.c`, `src/client/sv/ui/ui.c`,
`src/client/sv/diagnostics/timing.c`, `tests/sv/scenarios/timing-scenario.c`;
Wine/SDL/graphics driver interaction may also contribute.

**Proposal:** profile the first changed prompt frame under Wine Direct3D, separate
font/texture creation, draw submission and driver work, then optimize the measured
cause within SV. Do not raise the budget, hide cold work in the scenario or treat
Wine as actual Windows certification.

**Required checks:** retain unmodified urgent/interactive budgets and the delayed
negative control; repeat fresh-prefix TTF/PCF and software/accelerated scenarios,
verify exact replies and event preservation, then measure on actual Windows 10/11.
See `docs/sv-mingw.md` for reproducible environment and archived observations.
