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

**Status:** implemented in Stage A ticket 18; see [reviewed scope and evidence gate](sv-stage-a-evidence.md). Human approval remains ticket 19.

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

**Status:** implemented in Stage A ticket 17; see [runtime checks](sv-runtime-checks.md). Overall acceptance remains pending ticket 18 evidence closure.

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

**Status:** implemented in [Stage A ticket 20](sv-direct3d-startup.md). Production startup completes deferred first-present work before session input. Final full gate 62/62 passed; current human review approved.

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


## Produce host provenance on additional build distributions

**Status:** proposed separately; not part of ticket 18's current-host evidence.

**Problem:** the Stage A host producer uses the local pacman package database and
reviewed Linux search paths; another distribution cannot reuse that declaration.

**Affected code:** `tools/stage_a_provenance.py`, `tools/run_stage_a.py`.

**Proposal:** add an equivalent reviewed package/file closure producer when a
Fedora or actual Windows build environment is provisioned. Keep environment
identity explicit and retain the same content/directory freshness checks.

**Required checks:** compiler/SDK/runtime and Python dependency changes, package
updates, added search inputs, missing roots, and the production checkpoint CLI;
no inferred Windows acceptance or automatic human approval.

## Publish SV executable while a prior instance is running

**Status:** deferred; observed during ticket 20 acceptance, outside its renderer fix.

**Problem:** `shell` copies over `src/tomenet-sv`; a concurrently running manual
review holds that inode open, so Linux reports `Text file busy` and fails the build.

**Affected code:** `src/makefile.sv` shell target; coordination of
`tools/stage_a_review.py` and `tools/run_stage_a.py`.

**Proposal:** consider atomic publication of the linked executable, or explicitly
sequence build/gate before launching a review. Preserve the review's exact binary
identity and avoid silently certifying a replacement build.

**Required checks:** build with an existing SV process running, verify old/new
process executable identities, failure cleanup, isolated platform outputs and
review invalidation when executable contents change.

## Read complete legacy metaserver feed

**Status:** proposed separately; not part of SV-B-001 legacy changes.

**Problem:** `meta_read_and_close()` reads at most one available TCP chunk before
closing the socket. A metaserver response split across TCP reads can reach Lua
as an incomplete XML feed even when it is below the 80192-byte limit.

**Affected code:** `src/client/c-birth.c:meta_read_and_close`,
`lib/scpt/meta.lua:meta_display`.

**Proposal:** read a bounded complete response before parsing while retaining
the legacy timeout and error behavior; keep this change separate from SV.

**Required checks:** split feed at every boundary, full/over-limit response,
timeout and close during transfer, malformed XML, unchanged server ordering
and protocol metadata on Linux and Windows.

## Validate complete legacy setup records before publishing them

**Status:** proposed separately; SV-B-002 keeps its setup parser local to SV.

**Problem:** `Net_setup()` accepts several `Packet_scanf()` results for race,
class and trait records without checking that each complete field arrived.
Fragmented or malformed setup data can leave partly filled legacy arrays.

**Affected code:** `src/client/nclient.c:Net_setup`, `src/common/sockbuf.c:Packet_scanf`.

**Proposal:** stage complete setup records and publish them only after every
field and advertised MOTD byte has been validated, preserving supported
version layouts. Treat malformed fields as a visible connection failure.

**Required checks:** split every setup field, all version layouts, maximum
record and string lengths, malformed fields, chained packets, cleanup after
failure and legacy client regressions on supported platforms.

## Make legacy packet queue writes atomic

**Status:** proposed separately; SV-B-002 uses SV-local serializers for its
control replies and does not modify common or legacy code.

**Problem:** `Packet_printf()` in `src/common/sockbuf.c` can leave part of a
packet in the send buffer when space runs out, including at its last-byte
boundary. Some callers treat failure as a complete rejection.

**Affected code:** `src/common/sockbuf.c:Packet_printf`, legacy send callers
in `src/client/nclient.c`.

**Proposal:** preflight complete packet size or stage bytes before committing
to the queue; define waiting/retry behavior per caller without shifting wire
format or packet order.

**Required checks:** every byte of the queue boundary, string slot limits,
retry after drain, adjacent replies, duplicate prevention and both client
platform builds.

## Keep SV CFG field metadata in one place

**Status:** moved to Stage C with the former SV-B-004 settings scope; proposed
separately and not part of the narrowed Stage B.

**Problem:** adding a CFG field currently requires matching edits to the parse
branch, value formatter and settings Save key list. A missed edit can leave the
field out of dirty tracking or changed-record publication.

**Affected code:** `src/client/sv/profile.c`, `src/client/sv/settings.c`.

**Proposal:** use one SV-owned field descriptor for parsing, formatting and
settings enumeration when a later CFG field requires another coordinated edit.

**Required checks:** existing CFG parse/default/CLI precedence, every field's
round trip, changed-record Save, unknown-record retention, conflict detection
and startup behavior on Linux and Windows.

## Align account draft capacity with contact validation

**Status:** proposed separately; B-005 preserves the existing account editor
capacity while adding the private password path.

**Problem:** the endpoint account draft accepts 79 bytes, but the production
contact parser uses `ACCNAME_LEN` 16 and rejects names over 15 bytes only after
form submission. The form cannot explain the actual contact limit at entry.

**Affected code:** `src/client/sv/input/credentials.c`,
`src/client/sv/input/native-endpoint.c`, `src/client/sv/protocol/contact.c`.

**Proposal:** establish the account field's authoritative byte limit and show
validation in the form before contact; retain baseline account/wire semantics.

**Required checks:** 14/15/16/79-byte drafts, paste and edits at capacity,
protocol 1/2 account fields, retry after rejection and Linux/Windows native
input observations.

## Correct legacy S keymap record field parsing

**Status:** moved to Stage C with the former SV-B-007 preference/macro scope;
proposed separately and not part of the narrowed Stage B.

**Problem:** the legacy `S:<key>:<command>:<direction>` branch reads all three
values from `zz[0]`. A PRF record can therefore map the key to itself and
derive its direction from the key instead of applying the command and direction
fields.

**Affected code:** `src/client/c-files.c:process_pref_file_aux_aux`, `S` case.

**Proposal:** parse `zz[1]` and `zz[2]` for command and direction, then define
whether existing files that relied on the defect need compatibility handling.

**Required checks:** all three independent fields, direction bounds, old files,
normal and roguelike keymaps, explicit and automatic PRF load on both clients.

## Refresh SV Makefile source provenance in capability registry

**Status:** proposed separately; discovered during Sandcastle test setup.

**Problem:** two canonical capability-registry tests reject the current
`src/makefile.sv` because its SHA-256 differs from the historical digest in the
manifest. The Sandcastle runtime checks do not alter these records.

**Affected code:** `src/makefile.sv`, `docs/capabilities/manifest.json`,
`docs/capabilities/native-coverage.json`, `tests/sv_capabilities_checks.py`.

**Proposal:** review the Makefile change against the recorded behavior source,
then update the source revision and digest through the registry's normal
provenance workflow.

**Required checks:** canonical source-root validation, capability-registry
tests, coverage consistency, and a clean source fingerprint from committed
`HEAD`.

## Join or drain the contact resolver before SDL shutdown

**Status:** proposed separately; observed by SV-B-021 focused endpoint checks
and not changed by the account-creation failure implementation.

**Problem:** short endpoint runs repeatedly report an SDL `Leaked thread`
diagnostic after `sv_contact_socket_stop`; the resolver is detached, so endpoint
shutdown has no explicit completion handoff even when the socket owner has
already released the attempt.

**Affected code:** `src/client/sv/protocol/contact-socket.c`,
`src/client/sv/endpoint-run.c`, managed-peer endpoint checks.

**Proposal:** give the resolver a bounded, non-reentrant completion/join path
owned by contact lifecycle shutdown, while retaining cancellation and keeping
DNS/connect work off the main thread.

**Required checks:** cancel during DNS and connect, rapid rejection/exit,
retry with a fresh generation, repeated endpoint startup/shutdown under
ASan/UBSan, and Linux/Windows socket cleanup.

## Remove the obsolete common `atol` declaration

**Status:** proposed separately; observed while building the final SV-B-021
implementation and not changed by the account-creation work.

**Problem:** `src/common/h-system.h` declares `extern long atol();` without a
prototype. Current Clang diagnoses it as deprecated and conflicting with the
standard-library declaration whenever affected common sources are compiled.

**Affected code:** `src/common/h-system.h` and legacy/common translation units
that include it, including `src/common/sockbuf.c`, `z-util.c`, `z-form.c` and
`z-virt.c` in the SV build.

**Proposal:** remove the redundant declaration or replace it through the
project's portability-header policy, without changing numeric parsing behavior.
Keep this legacy/shared cleanup separate from SV-B-021.

**Required checks:** Linux SV and legacy builds with strict Clang warnings,
the supported GCC build, MinGW i686 build, and focused parsing regressions for
the legacy callers that use `atol`.

## Relocate the birth interaction backing implementation

**Status:** proposed separately; ticket 14 exposes the interaction through the
documented `input/birth-interaction.h` boundary without broad source movement.

**Problem:** the unfinished birth interaction is owned by input router /
interactions, but its current SV-local backing implementation is physically in
`src/client/sv/session/birth.[ch]`. The path can misleadingly suggest that birth
draft transitions belong to the session presentation model.

**Affected code:** `src/client/sv/session/birth.[ch]`,
`src/client/sv/input/birth-interaction.h`, `src/makefile.sv`, and focused test
source lists.

**Proposal:** when a current implementation task needs coordinated source
movement, relocate the backing implementation under `input/` while retaining
the existing public interaction boundary and semantic commands. Do not combine
this organizational cleanup with legacy/shared extraction.

**Required checks:** default and `SV_CLASS_BEFORE_RACE=0` SV builds, complete
production endpoint birth matrix, dependency-file/source-provenance behavior,
and an include audit proving that UI and endpoint still use only the public
interaction boundary.

## Gate birth presentation view preparation by revisions

**Status:** proposed separately; this record does not implement the optimization
or extend SV-B-023, and no runtime or build check is claimed as completed.

**Problem:** the production endpoint frame loop rebuilds `SvBirthView` on every
frame. `sv_birth_view_prepare` reinitializes the view and copies the large legal
class, race and trait choice arrays even when birth presentation state is
unchanged. There is no revision-based invalidation gate. This leaves repeated
presentation work on the steady-state frame path contrary to the guidance in
[ADR-0004](adr/0004-revision-based-ui-views.md), which calls for derived UI data
to be refreshed from model revisions instead of traversed and formatted every
frame.

**Affected code:** `src/client/sv/endpoint-run.c:run_contact` (the per-frame
`sv_birth_view_prepare` call),
`src/client/sv/session/birth.c:sv_birth_view_prepare`, and the `SvBirth` /
`SvBirthView` definitions in `src/client/sv/session/birth.h`.

**Proposal:** in separate measured work, add a monotonic birth presentation-model
revision (or an equivalently scoped existing model revision) and retain the
endpoint-owned `SvBirthView` as a derived cache. Rebuild and copy its choice arrays
only when the cache key's generation or model revision changes. The invalidation
key must also cover every presentation input used by the cached result, including
logical geometry, display/UI scale, font/resource revisions and renderer lifetime
where those inputs affect prepared birth data; model revision alone must not hide
such changes. Instrument view preparations and copied bytes before and after the
change: after the initial preparation, an arbitrary run of unchanged frames should
perform zero further rebuilds, while each relevant model or presentation-input
change should cause exactly one rebuild. Choose finer-grained revisions only if
measurements justify their additional invalidation rules.

**Required checks:** production `sv_endpoint_run` observations with instrumented
rebuild/copy counts for idle multi-frame birth views and each model/input
invalidation; maximum-length server-provided class, race and trait lists; owner,
highlight and draft stability across keyboard, mouse, focus and resize events;
font/resource, scale, renderer-lifetime and geometry invalidation; both class/race
compile orders; ASan/UBSan and focused performance measurements demonstrating no
stale view and the expected reduction in preparations and copied bytes.

## Centralize production-path birth scenario descriptors

**Status:** proposed separately as future work; this record does not implement the
harness refactor, change accepted behavior, or extend SV-B-023. No deferred
refactor check or P1-AC30 runtime evidence is claimed as completed.

**Problem:** the SV birth production-path scenario harness repeatedly dispatches
the same raw-string scenario identity. The Python runner enumerates string IDs,
while the C harness independently interprets `argv[3]` / `Peer.mode` when building
peer setup, scripting native SDL input, initializing `SvEndpointOptions`, and
selecting expected `SvEndpointOutcome` and peer assertions. These separate
branches can drift so that one scenario is configured under one identity but
driven or asserted as another.

**Affected code:** `tests/sv_name_editor_submit_checks.py` (scenario selection,
profile setup, and default/alternate compile-order invocation) and
`tests/sv/name-editor-submit.c` (`Peer.mode`, `send_setup`, `send_overview`,
`peer_run`, endpoint options, and outcome/peer assertions).

**Proposal:** in separate future harness work, replace redispatch of raw strings
with a typed scenario identity and descriptor table that is the single source for
each scenario's peer setup, native SDL input script, endpoint options, and expected
outcome. Keep the scenario runner on the existing production `sv_endpoint_run`
path and preserve all accepted protocol and gameplay semantics. Descriptor lookup
must fail for an unknown or duplicate identity, and a scenario must pass one
resolved descriptor through setup, input, options, and assertions so it cannot be
configured under one identity and asserted under another. This organizational
refactor must not add capability scope to SV-B-023 or move production behavior
into a test-only interface.

**Required checks:** descriptor completeness for every scenario selected by the
runner in both compile configurations; uniqueness of every typed scenario
identity and rejection of unknown/duplicate identities; explicit agreement of
peer setup, native input script, endpoint options, and expected assertions from
the same descriptor; and the existing production-path birth regression matrix,
including both class/race compile orders, server-provided lists, automatic/staged
DNA, navigation, backtracking, quit, Guide, interleaving, disconnect, and stale
generation cases. These checks belong to the future refactor and have not been
run or passed by this documentation-only record.

## Group SV birth DNA draft fields in a typed aggregate

**Status:** proposed separately as future SV-local work only. This record does
not perform the refactor, extract shared or legacy code, add DNA persistence,
accept `birth.restore-dna` or `birth.save-dna`, or change any verified SV-B-023
behavior. No future build, runtime, or review check is claimed as passed here.

**Problem:** seven DNA choices cross the endpoint-to-birth boundary as repeated
availability/value pairs: sex, class, race, trait, body, stats, and mode. The
same logical draft is expanded into individual fields in `SvEndpointOptions`,
copied through the production handoff to the long `sv_birth_begin` parameter
list, and stored again as parallel members of `SvBirth`. Keeping each
availability flag separate from its value at all four sites makes field omission,
pair mismatch, or reordered handoff possible when the draft evolves.

**Affected code:** `src/client/sv/endpoint-run.h:SvEndpointOptions` (endpoint
options), `src/client/sv/endpoint-run.c:run_contact` (production handoff),
`src/client/sv/input/birth-interaction.h` (the existing public interaction
boundary), and `src/client/sv/session/birth.h:SvBirth` / `sv_birth_begin` plus
`src/client/sv/session/birth.c:sv_birth_begin` (birth state and initialization).

**Proposal:** in a separate task, introduce a typed SV-local DNA draft aggregate
whose sex, class, race, trait, body, stats, and mode members each retain explicit
availability alongside the current value type. Pass that aggregate
field-for-field from endpoint options through the existing production
`input/birth-interaction.h` boundary into birth initialization and state. Preserve
the current meaning of every availability flag and value, including values paired
with an unavailable flag, the six-stat array contents and copy semantics, and all
random, staged-DNA, and automatic-DNA behavior. Do not use this cleanup to move
ownership into endpoint code, introduce persistence, widen SV-B-023, or create a
shared/legacy abstraction.

**Required checks:** the future refactor must prove field-for-field mapping from
endpoint options through production handoff and initialization into birth state,
including unavailable/value pairing without sentinel substitution or dropped
values. Exercise every one of the seven choice owners (sex, class, race, trait,
body, stats, and mode), both compile-time class/race orders, and the existing
production-path random, staged-DNA, and automatic-DNA regressions. Verify that
protocol bytes, send timing and suppression (including partial/completed-character
sends), owner transitions, legality, highlighting, and value semantics remain
unchanged. Run the applicable `sv-build`, `sv-core`, and `controller-review`
checks for that future change; these checks are required but were not run or
passed by this documentation-only record.

## Extract an SV-local birth random-state step

**Status:** proposed separately as future work; this record does not implement the
helper, change production SV or tests, touch legacy/shared code, alter protocol or
gameplay semantics, or extend the accepted SV-B-023 scope. The required future
checks below have not been run or passed by this documentation-only repair.

**Problem:** the seven owner-specific random command handlers for sex, class,
race, trait, body, stats and mode repeat the same three-operation xorshift state
transition on `SvBirth.random_state`. Keeping identical state evolution inline in
each handler makes a future correction or audit easy to apply inconsistently even
though the owner-specific selection and confirmation rules intentionally differ.

**Affected code:** `src/client/sv/session/birth.c`: the `SV_BIRTH_RANDOM`
branches in `sv_birth_command`, `class_command`, `race_command`, `trait_command`,
`body_command`, `stats_command` and `mode_command`; and the `random_state` field in
`src/client/sv/session/birth.h:SvBirth`.

**Proposal:** in separate future SV-local work, introduce a narrowly scoped
random-step helper that owns only the existing unsigned xorshift state transition
and returns/stores its deterministic next state. Preserve exactly when each
handler advances the state. Keep server-provided legal-list selection and its
owner-specific modulo or sex-bit mapping, compatibility and flag handling,
confirmation, repeated ordered-stat selection, and empty-list or unavailable-step
rejection in the current owner handlers. Do not extract this into legacy/shared
code or turn the opportunity into a broader random-dispatch refactor.

**Required checks:** fixed-seed deterministic next-state vectors for the exact
xorshift transition, including consecutive steps; unchanged owner-specific
modulo/list selection, sex-bit mapping and legality; production-path random
behavior for all seven server-provided choice owners; empty-list and unavailable-
step handling, including Archer body and point-buy/ordered-stats conditions; both
compile-time class/race orders; and the existing production interaction/input-
router regression matrix for navigation, compatibility, flags, confirmation,
backtracking, quit, interleaved events, disconnect and stale-generation send
suppression. These are requirements for the future helper work and are not
claimed as passed by this documentation-only record.

## Consolidate SV native birth owner dispatch

**Status:** proposed separately as future SV-local work only. This record does
not implement the refactor, change production or test behavior, touch
legacy/shared code, or widen the accepted SV-B-023 scope. None of the future
checks below is claimed as run or passed here.

**Problem:** native birth presentation and input repeat owner-specific dispatch
across choice counting, highlighted identity lookup, identity-to-position
mapping, semantic selection, and six near-identical mouse select/double-click
confirm branches. A new owner or owner-specific correction can therefore be
added to one dispatch site but omitted from another even though each path is
describing the same active birth owner.

**Affected code:** `src/client/sv/input/native-birth.c`, specifically
`choice_count`, `highlighted_identity`, `highlighted_position`,
`select_position`, and the class/race/trait/body/stats/mode mouse branches in
`sv_native_birth_event`.

**Proposal:** in a separate task, introduce one narrowly scoped SV-local owner
descriptor/helper boundary for presentation lookup and mouse selection. Keep
owner-specific identities, legal lists, command kinds, double-click confirmation
and stable-highlight semantics explicit. Do not move birth rules into the UI,
share the implementation with legacy code, or combine this cleanup with new
birth capability work.

**Required checks:** exercise single-click selection and double-click
confirmation for class, race, trait, body, stats and mode through the production
`sv_endpoint_run` input path; verify legal-list identities, viewport positions,
stable highlighting, owner transitions, draft state and send suppression in both
class/race compile orders; retain the existing keyboard, Guide, backtracking,
interleaving, disconnect and stale-generation regression matrix; and run the
applicable future `sv-build`, `sv-core` and `controller-review` checks. These
checks belong to the future refactor and remain unrun by this documentation-only
record.

## Decode SV birth semantic identities authoritatively

**Status:** proposed separately as future SV-local work only. This record does
not implement the mapping, change production or test behavior, touch
legacy/shared code, or widen the accepted SV-B-023 scope. None of the future
checks below is claimed as run or passed by this documentation-only repair.

**Problem:** birth compatibility, trait-derived hidden-class handling and the
post-mode PvP Maia rules recognize related server domain identities through a
mixture of presentation and layout primitives. `maia_race_id` compares the
server-provided display title with `"Maia"`; `trait_derived_hidden_class`
compares decoded hidden-class titles with `"Hell Knight"` and
`"Corrupted Priest"`; and the same rules use raw trait IDs `14` (Enlightened)
and `15` (Corrupted) together with raw server feature bit `0x200` in
`server_flags[3]`. Treating these race, class, trait and feature identities as
separate string and integer literals obscures that they form one semantic-
identity mapping and makes protocol/layout changes easy to apply inconsistently.

**Affected code:** `src/client/sv/session/birth.c`, specifically
`maia_race_id`, `trait_derived_hidden_class`, `dna_visible_class`, `legal_trait`
and `confirm_mode`; the decoded setup types in
`src/client/sv/session/character-setup.h`; setup title/hidden-class decoding in
`src/client/sv/protocol/contact.c`; and server-feature decoding in
`src/client/sv/protocol/protocol.c`.

**Proposal:** in a separate task, establish one authoritative SV-local decoded
identity/feature mapping for the Maia race, Hell Knight and Corrupted Priest
hidden classes, Enlightened and Corrupted traits, and the PvP Maia post-mode
feature. Prefer explicit semantic identities attached at the existing decoded
setup/control boundary; if the wire data cannot support that representation
without inventing identity, use one set of named SV-local semantic constants and
mapping helpers instead. In either form, preserve the server-provided display
titles, race/class order and choice-bit layout, hidden-class prefix/base-class
decode, trait IDs and `server_flags[3]` feature-bit layout. Preserve all current
compatibility filtering, both trait-derived hidden-class rules and post-mode PvP
Maia selection/backtracking semantics. Do not use this mapping work to extract a
legacy/shared abstraction or add capability scope to SV-B-023.

**Required checks:** the future change must verify the decoded identity/layout
mapping while retaining the exact server-provided title, race/class order,
choice-bit, hidden-class base, trait-ID and feature-flag layouts; Maia matching
at every server-provided race position; Hell Knight and Corrupted Priest
resolution and compatibility; Enlightened (`14`) and Corrupted (`15`) post-mode
trait identities; and the PvP Maia `server_flags[3] & 0x200` feature. Exercise
both compile-time class/race orders and retain the existing production
interaction/input-router regression matrix for server-provided lists,
compatibility, hidden-class DNA, post-mode selection and backtracking,
navigation, confirmation, quit, interleaved events, disconnect, stale-generation
and send suppression. Run the applicable future `sv-build`, `sv-core` and
`controller-review` checks. All of these checks are requirements for the future
mapping work and are not run or passed by this documentation-only record.

## Move SV birth viewport and row layout ownership into UI

**Status:** proposed separately as future SV-local work only. This record does
not perform the ownership refactor, change production or test behavior, touch
legacy/shared code, or widen the accepted SV-B-023 scope. None of the future
checks below is claimed as run or passed by this documentation-only repair.

**Problem:** the native birth input adapter currently owns presentation state in
`SvBirthViewport` together with the choice-row top, height and text-inset
geometry. It derives the visible range from window height, updates scrolling and
maps mouse coordinates back to choice positions. The endpoint birth renderer
then imports that input header and directly consumes the viewport and row
constants to decide which choices to draw and where. This reverses the intended
UI/input boundary: renderer layout is constrained by state and geometry owned by
the input adapter, while hit testing is only implicitly kept in agreement with
the pixels produced by the UI.

**Affected code:** `src/client/sv/input/native-birth.h` (`SvBirthViewport` and
`SV_BIRTH_CHOICE_ROW_*`), `src/client/sv/input/native-birth.c`
(`sv_birth_viewport_sync`, scrolling, Page navigation and mouse hit testing),
`src/client/sv/input/native-endpoint.h:SvEndpointInput`,
`src/client/sv/endpoint-run.c:run_contact`, and the endpoint birth renderer in
`src/client/sv/ui/endpoint-scene.c:sv_endpoint_render`.

**Proposal:** in a separate task, introduce an SV-local, UI-owned birth
viewport/layout contract. The UI should own row geometry and presentation state,
including the visible range and scroll position, and expose the resulting
read-only layout plus a hit-test operation that resolves UI coordinates to a
stable server-provided choice identity (or no hit). The native input adapter
should consume that contract for mouse selection and request semantic scrolling
for wheel and Page navigation instead of defining renderer geometry. Preserve
the current server-provided identities, highlighted selection, keyboard and
mouse navigation, resize behavior, and every birth owner transition, including
backtracking and skipped owners in either class/race order. Keep birth rules and
the production interaction/input-router path outside the renderer. Do not use
this boundary cleanup to change gameplay or protocol behavior, extract a
legacy/shared abstraction, or add capability scope to SV-B-023.

**Required checks:** for every choice owner, verify renderer and hit-test
geometry agreement; this includes the sex owner's two current targets and the
class, race, trait, body, stats and mode row lists. Exercise clicks above, below
and within the viewport, partially or fully clipped rows, every visible row,
single-click selection and double-click confirmation. Verify stable
server-provided identity and highlighting through keyboard movement, mouse
wheel, Page Up/Page Down, focus and resize interleaving, scrolling, owner
transitions, backtracking and skipped owners. Run both compile-time class/race
orders and retain the existing production `sv_endpoint_run` interaction/input-
router regression path, including disconnect, stale-generation and send-
suppression coverage. Run the applicable future `sv-build`, `sv-core` and
`controller-review` checks. These checks are requirements for the future
ownership refactor and are explicitly not run or passed by this documentation-
only record; `P1-birth-runtime-evidence` remains deferred to its named producer
and verifier at full acceptance.

## Publish birth evidence through a birth-owned snapshot

**Status:** proposed separately as future SV-local work only. This record does
not implement the refactor, change production or test behavior, touch
legacy/shared code, or widen the accepted SV-B-023 scope. None of the future
checks below, including deferred P1-AC30 runtime evidence, is claimed as run or
passed by this documentation-only repair.

**Problem:** the application-level endpoint outcome publishes almost the entire
detailed birth interaction state as a flat diagnostic surface. `publish_outcome`
reads owner, highlights, chosen draft values, legal and visible lists, navigation
and confirmation flags, lifecycle/generation data, and Guide placeholder evidence
directly from the birth backing state and expands them into dozens of
`birth_*` members of `SvEndpointOutcome`. This couples application orchestration
and its public diagnostics type to birth-owned gameplay detail: adding or
reorganizing birth evidence requires coordinated edits outside the responsible
interaction interface even when endpoint lifecycle behavior is unchanged.

**Affected code:** `src/client/sv/endpoint-run.c:publish_outcome` (endpoint
outcome publication), `src/client/sv/endpoint-run.h:SvEndpointOutcome` (the
public outcome type), `src/client/sv/input/birth-interaction.h` (the stable
production interaction boundary), the birth state currently backed by
`src/client/sv/session/birth.[ch]`, and production endpoint scenario assertions
in `tests/sv/name-editor-submit.c`.

**Proposal:** in a separate future task, introduce a birth-owned, copied
snapshot/evidence aggregate and expose its construction through the existing
`input/birth-interaction.h` boundary. Make that aggregate the birth diagnostics
portion consumed by endpoint outcome publication, so application orchestration
does not enumerate or read the backing interaction fields individually. Keep
gameplay details and snapshot derivation behind the responsible birth interface;
do not move transition, legality, selection, Guide, lifecycle, or send rules into
the endpoint. Preserve the public production entry seam
`sv_endpoint_run(SvEndpointOptions)` and every currently observable evidence
meaning, value, default, ordering, copy/lifetime property, and suppression
result. Keep this refactor SV-local and separate from SV-B-023 capability work;
do not use it to extract shared/legacy code or add a test-only behavior path.

**Required checks:** the future refactor must provide a complete, reviewed
field-to-snapshot mapping for every current birth evidence member of
`SvEndpointOutcome`, including zero/default states, bounded arrays and copied
values. Verify generation and lifecycle evidence for entry, active, quit, close,
disconnect and stale-generation paths; every owner, stable highlight, chosen and
in-progress draft, legal/visible list and stats state; and Guide evidence with the
exact `The guide is in development` placeholder, open/close counts, owner mask,
unchanged caller, selection and draft, and no Guide-state or network operation.
Retain explicit peer evidence that advancing, backtracking, cancelling, quitting,
disconnect and stale generations suppress every partial or completed-character
send. Exercise
both compile-time class/race orders and the complete existing production
`sv_endpoint_run` birth scenario matrix through the production interaction state
and native input router. Run the applicable future `sv-build`, `sv-core` and
`controller-review` checks. These requirements belong to the future refactor and
are not run or passed by this documentation-only record; P1-AC30 remains deferred
to its named full-acceptance producer and verifier.

## Centralize production endpoint test build inventory

**Status:** proposed separately as future test-infrastructure work only. This
record does not consolidate the runners, change production, legacy or shared
code, or widen the accepted SV-B-023 scope. None of the future build, runtime or
review checks below is claimed as run or passed by this documentation-only
repair.

**Problem:** seven Python runners independently enumerate the production sources
used to compile their endpoint test binaries. The lists substantially duplicate
the same endpoint, native input, interaction, protocol, session and UI source
inventory. When a production endpoint dependency is added or removed, one runner
can omit the matching source update while the others continue to build, causing
dependency drift and inconsistent production-path coverage.

**Affected code:** the seven current production endpoint runners:
`tests/sv_endpoint_checks.py`, `tests/sv_account_create_checks.py`,
`tests/sv_account_failure_checks.py`, `tests/sv_first_session_checks.py`,
`tests/sv_name_editor_submit_checks.py`,
`tests/sv_name_rejection_recovery_checks.py`, and
`tests/sv_ordinary_create_cancel_checks.py`; specifically their `SOURCES` or
`scene_sources` inventories and compile commands.

**Proposal:** in separate future test-infrastructure work, provide one
authoritative endpoint test build inventory/helper for the ordered common
production sources. Each of the seven runners should consume that inventory and
retain its own scenario harness, orchestration, executable, runner-specific
sources, compiler/linker flags and environment so the runners remain isolated.
The helper must continue to compile and execute the real production
`sv_endpoint_run(SvEndpointOptions)` endpoint, the native birth adapter and the
birth interaction/input-router path. It must not replace any production behavior
with a test-only implementation. Do not use this consolidation to change
production, legacy or shared code, alter protocol/gameplay semantics, add
capability scope to SV-B-023, or absorb runner-specific scenarios into a shared
test flow.

**Required checks:** for the future consolidation, verify that the authoritative
inventory is complete and has deterministic ordering; all seven runners consume
it while preserving their runner-specific harness sources, extra production
sources, compiler/linker flags, sanitizers and environment; and adding or removing
a production endpoint dependency produces the corresponding inventory change and
consistent build outcome in every consumer. Exercise both birth compile orders
where applicable and retain the existing production endpoint regression runs
through the real endpoint, native birth adapter and birth interaction/input-router
path. Run the applicable `sv-build`, `sv-core` and `controller-review` gates for
that future change. These checks are required but were not run or passed by this
documentation-only record; `P1-birth-runtime-evidence` / P1-AC30 remains deferred
to its named full-acceptance producer and verifier.
