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

## Restore the ordinary Hellish case-sensitive fixture setup

**Status:** resolved by the exact-HEAD recovery for ticket 73.

**Problem:** the production endpoint scenario
`case-sensitive-mode-ordinary-hellish` fails both focused and unfiltered
`sv-core`: after its real shifted/caps-lock `H` input sequence,
`SvEndpointOutcome.birth_highlighted_mode` is not
`SV_BIRTH_MODE_HELLISH`. This blocks completion of the exact-HEAD unfiltered
matrix independently of the repaired aggregate-DNA fixtures.

**Affected code:** `tests/sv/name-editor-submit.c` case-sensitive mode event
setup and assertions, `tests/sv_name_editor_submit_checks.py` profile/startup
fixture setup, and the production path reached through
`sv_endpoint_run(SvEndpointOptions)` and the native birth input adapter.

**Proposal:** in a separate repair, reconcile the scenario's authoritative
first-start/profile assumptions with the current mode legality rules, then keep
the shifted/caps-lock binding assertions at the public production seam. Do not
change production mode legality merely to satisfy stale test setup.

**Required checks:** reproduce the focused failure before repair; verify shifted
and caps-lock ordinary Hellish selection plus lowercase/uppercase rejection
controls through `sv_endpoint_run`; run both compile orders and the complete
unfiltered `sv-core` with `SV_BIRTH_CASE` unset; confirm no protocol/gameplay,
legacy/shared, sanitizer, compiler, or assertion regression.

**Resolution:** authoritative profile setup already made the scenario a
returning-player flow and exposed six legal ordinary modes. The native adapter
instead lost uppercase Latin layout identities after SDL translation. For the
mode owner, normalize translated ASCII `A..Z` to the lowercase binding identity
while continuing to derive semantic case from Shift/Caps modifiers; other owners
retain their uppercase rejection semantics. Focused ordinary/dedicated Hellish
and rejection controls pass in both compile orders; the exact resolved HEAD
passes the full unfiltered `sv-core`.

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

## Give the shared login name-entry state a neutral name

**Status:** proposed separately as future production-state cleanup only. This
record does not rename the state, change ordinary or exclusive/dedicated
creation, alter production behavior, touch legacy/shared gameplay code or widen
the accepted SV-B-023 scope. None of the future checks below is claimed as run
or passed by this documentation-only repair.

**Problem:** `SvLoginInteraction.ordinary_name_active` now gates the shared name
editor for both ordinary creation started by
`SV_LOGIN_COMMAND_CREATE_ORDINARY` and exclusive/dedicated creation started by
`SV_LOGIN_COMMAND_CREATE_EXCLUSIVE`. Its ordinary-only name no longer describes
the state it represents and can mislead later maintenance of the common flow.

**Affected code:** the state declaration in
`src/client/sv/input/login-interaction.h`; both activation paths and the generic
insert, key, cancel and submit consumers in
`src/client/sv/input/login-interaction.c`; the generic native name-event router
in `src/client/sv/input/native-name.c`; the shared `SvOrdinaryNameEvidence`
type and `ordinary_name_entry` login-view field in
`src/client/sv/endpoint-run.c`; and the corresponding shared view consumer in
`src/client/sv/endpoint-scene.c`.

**Proposal:** in separate future production cleanup, rename
`ordinary_name_active` to a scope-neutral state name such as
`name_entry_active`, rename `SvOrdinaryNameEvidence` and
`ordinary_name_entry` to corresponding scope-neutral evidence/view names, and
migrate every production reference together. Preserve the existing
`dedicated_creation` intent and dedicated mode flags, the ordinary and
exclusive/dedicated activation and exit semantics, and the current handoff
through the production birth interaction and input router. Do not use the rename
to change protocol/gameplay behavior, introduce a test-only interface, modify
legacy/shared gameplay code or add capability scope to SV-B-023.

**Required checks:** for the future rename, verify a complete production
reference migration; ordinary and exclusive/dedicated name-entry activation,
editing, submit, cancel and exit; the accepted-name handoff into the existing
birth interaction/input router; quit behavior; and suppression of sends after
disconnect or stale-generation detection. Run the applicable `sv-build`,
`sv-core` and `controller-review` gates. These future rename checks are required
but were not run or passed by this documentation-only record;
`P1-birth-runtime-evidence` / P1-AC30 remains deferred to its named
full-acceptance producer and verifier.

## Keep controlled-peer workers outside endpoint SDL teardown

**Status:** proposed separately; existing fixtures unchanged by handshake slice 01.

**Problem:** selected SV-B-023 endpoint checks print `Leaked thread` because their
controlled-peer `SDL_Thread` is joined after `sv_endpoint_run` has already called
`SDL_Quit`. This obscures resource diagnostics even when protocol assertions pass.

**Affected code:** `tests/sv/name-editor-submit.c` worker creation/join around
`sv_endpoint_run`; inspect equivalent worker lifetimes in the other endpoint
fixtures before changing them.

**Proposal:** in a separate test-organization task, give peer workers a lifetime
independent of the endpoint-owned SDL lifecycle, or join them before its teardown
through an agreed production boundary. Slice 01 uses a pthread peer; no new
production callback or legacy/shared change is needed.

**Required checks:** rerun affected native endpoint cases under sanitizers;
verify peer completion, bounded timeout, socket cleanup, retry and repeated
endpoint invocation, and absence of SDL thread diagnostics. Preserve production
endpoint evidence and do not replace it with protocol-only tests.

## Correct the compatibility-table version comment

**Status:** proposed separately; no legacy/shared change in handshake slice 02.

**Problem:** the compatibility comment in `src/common/defines.h` describes
`MAX_F_IDX_COMPAT` as applying through 4.9.2.1.0.2, but the canonical baseline
`Net_start` actually switches to `MAX_F_IDX` strictly after 4.9.1.2.0.2. Using
that comment as a protocol source would select the wrong table size.

**Affected code:** `src/common/defines.h` compatibility comment near
`MAX_F_IDX_COMPAT`; `src/client/nclient.c:Net_start` feature-table gate.

**Proposal:** correct the comment in a separate documentation cleanup after
checking client/server gates. Keep the executable baseline semantics intact;
this ticket implements the canonical `Net_start` condition in SV.

**Required checks:** compare canonical client/server feature-table gates and
rerun the controlled-peer below/at/above 4.9.1.2.0.2 examples. Applicable build,
core and controller review remain required for any future behavior change;
none is claimed passed by this improvement record.

## Bound legacy play acknowledgement reads

**Status:** proposed separately; no legacy/shared change in reply slice 05.

**Problem:** baseline `Net_start` performs a blocking readability wait and a
single read, then inspects `rbuf.ptr[0]` and the peer-quit string before proving
that a complete packet is present. Split replies are not retained by a startup
owner and may be reported as failures. This is a legacy robustness opportunity,
not a reason to change the established play result or retry policy.

**Affected code:** `src/client/nclient.c:Net_start`, `Receive_reply`, and the
legacy socket-buffer read/advance contract.

**Proposal:** separately investigate bounded incremental reply assembly and
length-checked quit messages for legacy startup, preserving exact packet order,
server reasons, fatal rejection policy and trailing packet dispatch. SV already
uses its own incremental production decoder; no consolidation is proposed.

**Required checks:** independently derived success/rejection/quit/wrong-type
fixtures; splits at every reply boundary; truncated EOF and unterminated quit;
ack with trailing packets; timeout/exit policy; applicable legacy builds and
real peer checks. None of these future legacy checks is claimed passed here.

## Preserve live protocol bytes at the startup handoff

**Status:** proposed for stage C; outside SV-B-024 startup scope.

**Problem:** after the successful play reply and `PKT_END`, the server sends
version and gameplay updates. TCP can combine these with the reply in one read;
the current startup login decoder rejects unsupported live packets. Accepting
the baseline reply terminator in B-024 does not implement live packet handoff.

**Affected code:** `src/server/nserver.c` play acknowledgement and subsequent
`Send_version`; `src/client/sv/protocol/login.c:sv_login_receive`;
`src/client/sv/endpoint-run.c:pump_login` and the future live-session owner.

**Proposal:** transfer the unconsumed opaque input tail and the transport to
the live-session owner exactly once, preserving packet order and session
generation. Keep gameplay decoding outside the startup interaction.

**Required checks:** coalesced and split ack/END/live packets, partial live
packets, exactly-once delivery, disconnect and stale-generation teardown through
the production startup-to-live path. These checks were not run for B-024.


## Verify startup mouse coordinates across display scale and pixel density

**Status:** SV coordinate mapping fixed and verified with controlled X11 scale
1.5 on 2026-10-28; actual display migration/pixel-density != 1 remain pending.
See [current native evidence](sv-stage-b-linux-native-evidence.md).

**Problem:** startup rendering uses display scale while native event coordinates
are window coordinates. The B-025 native run established correspondence only
at scale 1; a display-scale/pixel-density ratio other than 1 remains unverified.

**Affected code:** `src/client/sv/endpoint-run.c` window sizing and native event
routing; `src/client/sv/input/native-login.c`, `native-name.c`, `native-birth.c`;
`src/client/sv/ui/endpoint-scene.c`.

**Proposal:** verify the existing coordinate contract on actual scaled displays
and, if necessary, convert at the SV native event boundary consistently for
startup surfaces. Keep wire and interaction semantics unchanged.

**Required checks:** native mouse selection/editing on each startup surface at
ratio 1 and non-1; resize, display migration and minimum geometry; Linux and
Windows when returned to validation scope. The B-025 scale-1 run remains
historical; the current controlled X11 non-1 run verifies mouse creation and
exact wire output, without claiming physical HiDPI/display migration.


## Reduce repeated source lists in SV production-path check builds

**Status:** proposed, outside B2-001 scope; no build-framework refactor performed.

**Problem:** twelve existing endpoint consumer scripts repeat production source
lists and pkg-config modules. Adding one SV-owned shell translation unit requires
mechanical link wiring in all twelve, with a risk of missing an existing consumer.

**Affected code:** `tests/sv_endpoint_checks.py` and endpoint-scene consumers in
`tests/sv_*checks.py`; `src/makefile.sv`.

**Proposal:** separately evaluate a small shared declaration of production link
inputs, preserving independent scenario selection and production-path execution.
This concerns test build wiring, not merging SV and legacy implementations.

**Required checks:** all affected existing Linux consumers still build the same
production sources, sanitizer flags and compile-order branches; no added suite
expansion without user agreement.

## Report the actual endpoint failure instead of a stale SDL error

**Status:** proposed, outside B2-002 scope; diagnostic behavior unchanged.

**Problem:** a controlled OpenGL run with `--frames` intentionally exits after
a refused connection, but the final generic startup diagnostic can print an
unrelated earlier SDL error (`Parameter 'texture' is invalid`). This obscures
the actual contact failure and can be mistaken for a rendering failure.

**Affected code:** `src/client/sv/endpoint-run.c:run_contact` frame-limited
failure exit and `sv_endpoint_run` final `SDL_GetError()` diagnostic;
`src/client/sv/ui/login-shell.c` optional texture cleanup.

**Proposal:** separately preserve the owning operation's error when returning
failure and avoid presenting ambient SDL error state as its cause. Preserve
the existing frame-limit and network retry/exit semantics.

**Required checks:** production refused-contact runs with and without
`--frames`, an actual rendering failure, and successful startup; verify the
reported cause and exit policy independently. B2-002's same-input run without
`--frames` retains the error surface; a diagnostics fix is not claimed here.

## Investigate incomplete startup renderer captures

**Status:** proposed, outside B2-003 implementation; verify completeness of
retained captures individually.

**Problem:** B2-003 replay produced incomplete name/edit/error images through
the existing SDL capture hook and some actual X11 snapshots, while subsequent
window captures showed the complete form. The cause has not been established;
capture timing and persistent rendering faults must be distinguished before
drawing conclusions from these images.

**Affected code:** `src/client/sv/ui/endpoint-scene.c:sv_endpoint_draw`,
`SV_B2_CAPTURE` / `SV_B2_CAPTURE_FRAME`, and evidence capture scripts.

**Proposal:** separately reproduce and identify the renderer-readback or file
capture discrepancy before relying on this hook for these states. Keep the
production interaction and wire behavior unchanged.

**Required checks:** compare stable SDL readback and actual window captures of
the same completed overview/name/edit/error frames on Linux software/OpenGL;
record binary identity and distinguish capture failures from rendering failures.
Any expanded verification requires the user's agreement.

## Correct the web prototype Chronicle updater source URLs

**Status:** proposed, outside the B2-006 native corrections.

**Problem:** the web prototype updater requests `legends-na.php` and
`legends-apac.php`, which returned HTTP 403 during the 2026-10-29 native
Chronicle implementation. Its all-or-nothing update therefore fails. The
official `legends.php?server=...` endpoint returned HTTP 200 with the regional
heading; the native implementation uses that endpoint.

**Affected code:** the separate, read-only web repository's
`tools/update-login-news-snapshot.mjs`; its generated
`js/data/login-news-snapshot.js` provenance.

**Proposal:** separately update that web updater to use the canonical endpoint
and verify the regional headings and generated provenance. The external web
repository was not modified for B2-006.

**Required checks:** fetch the nine official source/category combinations,
verify their regional headings and parsed events, then inspect the existing
prototype Chronicle rendering. No native scope expansion is implied.

## Keep birth footer command mappings consistent

**Status:** proposed, outside B2-004 implementation; non-blocking review note.

**Problem:** mouse and keyboard footer handling repeat the control-to-command
mapping and Guide position. Future changes could update one input path only.

**Affected code:** `src/client/sv/input/native-birth.c:sv_native_birth_event`.

**Proposal:** if these controls change again, consider one local named mapping
for both input paths. This does not call for sharing SV and legacy code.

**Required checks:** the affected existing production-path mouse and keyboard
scenarios, including Guide return and unused-points modal focus; preserve native
command availability and session-generation checks. No extra suite is implied.

## Preserve the semantic startup failure in stderr

**Status:** proposed, separate from the B2 contact/setup fix.

**Problem:** after a handled connection failure the final generic startup logger
prints `SDL_GetError()`, which can contain an unrelated earlier nonfatal SDL
message (observed `Parameter 'texture' is invalid`) or be empty. The preceding
semantic contact log and the UI correctly report the actual cause.

**Affected code:** `src/client/sv/endpoint-run.c`, run_contact cleanup and the
final `SV endpoint startup failed` logger.

**Proposal:** carry a bounded semantic failure summary to the startup owner,
keeping renderer/resource failures distinct from handled protocol/transport
errors; avoid using the ambient SDL error as a universal status channel.

**Required checks:** existing controlled-peer malformed/EOF startup failures
with a deliberately stale nonfatal SDL error, plus a real renderer startup
failure. No additional suite or legacy/common change is implied.

## Character overview reorder bindings

**Status:** proposed; outside the current overview paint/layout correction.

**Problem:** the prototype advertises S/I/A reordering, while the SV overview
currently interprets letters as character selection. Copying that hint would
advertise an unavailable action and could select a character instead.

**Affected code:** `src/client/sv/input/native-login.c`,
`src/client/sv/input/login-interaction.c`, prototype character caption;
behavior baseline in `src/client/nclient.c:reorder_characters`.

**Proposal:** specify and port reorder behavior into SV before exposing its
bindings, preserving character identity and the actual server selection order.
Do not add the hint as part of a visual-only correction.

**Required checks:** minimal production-path checks for each supported order,
selection identity, cancellation and reconnect behavior, with scope agreed
before expanding the existing checks. Legacy/common changes are not implied.


### Авторитетный SUCCESS при ручном вводе существующего имени

- Проблема: `sv_login_create_character` заранее ставит `creating_character`;
  если пользователь ввёл имя своего уже существующего персонажа и сервер
  вернул SUCCESS=0, SV сохраняет birth-ветку. Baseline по SUCCESS продолжает
  существующим персонажем; E_NEED_INFO=3 означает создание нового.
- Код: `src/client/sv/protocol/login.c` WAIT_STATUS, `sv_login_selected_character`;
  исторические peer fixtures в `tests/sv/birth-complete.c` и смежных birth tests
  также возвращают 0 для нового имени, скрывая различие.
- Предложение: отдельно согласовать authoritative 0/3 переходы и механически
  исправить исторические controlled peers на E_NEED_INFO для новых имён.
- Статус: предложено, вне текущего исправления нового имени со статусом 3.
  Подтверждено production API probe: `.scratch/creation-next/existing-name-red.c`,
  результат `.scratch/creation-next/existing-name-red.log`.
- Проверки: минимальный core для вручную введённого существующего имени,
  новые/существующие/stale overview selection, согласованный прогон существующих
  birth/name suites после обновления fake server статусов; реальные аккаунты
  для этих проверок не требуются.


### Явный контракт случайной генерации имён SV

- Проблема: словари и сборка трёх слогов совпадают с legacy, но алгоритм PRNG
  отличается; требование «тот же генератор» может пониматься как совпадение
  словаря либо как точное воспроизведение выбора при заданном seed.
- Код: `src/client/c-birth.c:create_random_name`, `src/common/z-rand.c:Rand_div`,
  `src/client/sv/input/login-interaction.c:random_name`.
- Предложение: отдельно определить необходимый уровень совместимости RNG;
  при необходимости перенести локально SV требуемый baseline PRNG, сохранив
  изоляцию и не подключая общие зависимости только ради переиспользования.
- Статус: предложено, текущая задача генератор не изменяет. Таблицы 65/34/70
  сравнены по содержимому, порядку и дубликатам; совпадение полное.
- Проверки: только при утверждении изменения — минимальный core production-путь
  с фиксированным seed и сравнением выбранного baseline; ограничения имени
  и подтверждение Random/Enter должны сохраняться.


### Проверить keyboard clock guard вне pregame

- Проблема: в pregame подтверждено, что сравнение native keyboard timestamp
  со временем создания owner отбрасывает свежие клавиши. Аналогичный guard
  присутствует в игровом/synthetic input adapter, но его физический эффект в
  gameplay в этой задаче не проверялся.
- Код: `src/client/sv/input/native-input.c`; pregame исправлен отдельно в
  `input/pregame-events.h`, endpoint/run и native birth/endpoint.
- Предложение: отдельный audit gameplay input ownership и queue boundaries;
  не переносить flush/отмену без проверки pending game interaction/macros.
- Статус: предложено, вне текущего pregame follow-up.
- Проверки: минимальный production gameplay input с fresh old-clock events,
  queued retired-owner commands, macros, held/repeat keys, close/callback events.

### Упорядочить ввод в исторических full-birth fixtures

- Проблема: часть existing fixtures заранее помещает клавиши всех owners в одну
  SDL очередь, обходя intended input boundaries. Исправленный pregame barrier
  обязан удалять такую будущую очередь при передаче owner.
- Код: `tests/sv/name-editor-submit.c` и смежные birth event producers.
- Предложение: отдельно нормализовать readiness/paced event delivery, не
  дублируя interaction behavior в test model.
- Статус: предложено; в текущем follow-up исправлены только две необходимые
  паузы existing scene/name-rejection fixtures, core и paced golden проверены.
- Проверки: согласованный relevant subset existing suites; новая matrix не нужна.

Дополнение 2026-10-30 к историческим full-birth fixtures: кроме заранее queued
owners, отдельные `push_to_mode` в `tests/sv/name-editor-submit.c` всё ещё подают
Enter после букв sex/class/race/body, хотя native letters уже подтверждают owner.
Из-за этого isolated endpoint replay на старом producer не достигает intended
play boundary. Предложение и статус выше сохраняются: отдельно адаптировать
readiness/paced producers к production input, не расширяя текущую matrix.
Необходимые проверки — согласованный subset и literal wire bytes. В текущем
mode-final fix переиспользован уже рабочий paced replay; массовая адаптация
fixtures не выполнена и не включена в scope.

### Fail-fast при unresolved dependency в legacy Linux packaging

- Проблема: recursive `ldd` parser извлекает только resolved `=> /...` paths;
  `not found` не становится ошибкой, поэтому копирование зависимостей может
  закончиться без полного runtime closure.
- Код: `.github/docker/fedora41/release-common.sh:copy_linux_deps_recursive`
  (первый и recursive `ldd`); аналогичная копия в `fedora43`.
- Предложение: отдельной legacy packaging задачей явно отклонять unresolved
  dependencies и сохранять диагностику. C001 решает эту проверку локально в SV
  packaging, без обязательного изменения общих release helpers.
- Статус: предложено, выявлено при планировании C001 2026-10-31; вне реализации
  текущего этапа. Изоляция SV сама по себе не требует объединения helpers.
- Проверки: при согласовании изменения — минимальный missing-library negative
  case и существующая Linux package сборка; без новой platform matrix.

### Уточнить чуждые package-launch mappings в reconciliation

- Проблема: пять inventory rows связывают admin actions, audio/Windows overrides
  и item/spell name lookup с `capability.platform.package-launch`. Эти связи
  могут ошибочно расширить границы раннего C001 до поздних этапов.
- Код: `docs/capabilities/reconciliation.json`, rows 281, 681, 976, 1252, 1805;
  source-backed разбор —
  [C001 sweep](../.scratch/sv-c001-planning/issues/01-scope-and-ownership.md).
- Предложение: в общей semantic reconciliation работе уточнить настоящих owners
  и source classification; не делать автоматическую замену IDs. Для C001
  обязательна только мотивированная applicability disposition этих связей либо
  необходимая точечная metadata correction до acceptance, как указано в spec.
- Статус: обнаружено 2026-10-31; общий cleanup предложен отдельно от C001.
  Canonical JSON при планировании не изменялся.
- Проверки: source-backed ownership review затронутых rows и существующая
  registry/reconciliation validation при изменении metadata. Общий sweep и
  остальные этапы не считаются завершёнными локальным C001 планом.


### Уточнить исторический манифест B2-004

- Проблема: сумма `capture-reference.cjs` в историческом манифесте B2-004
  уже не совпадала с файлом до переноса login-эталона из `lib/`.
- Код: `docs/evidence/sv-b2-004/SHA256.json`, `capture-reference.cjs`.
- Предложение: отдельно выяснить версию скрипта, которой получено evidence,
  и явно различить исторический capture и текущую команду воспроизведения.
- Статус: обнаружено при переносе артефактов; историческая сумма сохранена.
- Проверки: сопоставить Git-историю скрипта и сохранённые результаты;
  не считать обновление путей повторной приёмкой.


### Разделить production CLI и диагностический launcher SV

- Проблема: текущий executable сохраняет общий CLI для `--endpoint`,
  синтетической оболочки и проверочных сценариев; физически вынесенные
  diagnostic objects всё ещё входят в его линковку.
- Код: `src/client/sv/main.c`, `src/makefile.sv`,
  `tests/sv/diagnostics/launcher.c`.
- Предложение: отдельно определить production entry и отдельную сборку
  проверочного launcher, сохранив нужные команды существующих runners.
- Статус: исходники обвязки вынесены в текущей задаче; разделение executable
  предложено отдельно, существующий CLI сохранён.
- Проверки: Linux production launch и согласованный минимум существующих
  native/core сценариев через production API.

### Уточнить границу результатов endpoint и накопителей evidence

- Проблема: `SvEndpointOutcome` и `publish_outcome` совмещают результат
  production запуска с большим числом полей о действиях приёмочных сценариев.
  Часть данных, например submitted/name recovery, влияет на реальный UI,
  поэтому механический перенос всех этих структур в тесты меняет поведение.
- Код: `src/client/sv/endpoint-run.[ch]`, `SvOrdinaryNameEvidence`,
  `SvBirthPlaceholderEvidence`, `publish_outcome`.
- Предложение: выделить семантический результат запуска и отдельно определить
  наблюдение за переходами для диагностических накопителей, без копии игровой
  логики и без расширения общей архитектуры ради тестов.
- Статус: обнаружено при аудите диагностики; отдельная адаптация этой смешанной
  границы не включена в перенос измерителей, capture и synthetic harness.
- Проверки: согласованный минимум существующих endpoint/name/birth core
  сценариев; сохранение submitted-layout, recovery и generation lifecycle.


### Заменить явно выделенные заглушки SV реальными providers

- Проблема: play login пока использует фиксированный профиль опций и ресурсов;
  Guide и завершение startup заменены временными текстами ожидания.
- Код: `src/temporary/sv/stubs/play-profile.h`, `pregame-ui.h`;
  потребители — `protocol/login.c`, `endpoint-run.c`, `ui/endpoint-scene.c`.
- Предложение: в задачах соответствующих возможностей подключить реальные
  опции/ресурсы, Guide и live-session handoff, удаляя соответствующую заглушку.
- Статус: заглушки выделены и явно обозначены; их замена не входит в текущий
  перенос и не объявлена выполненной. Текущее поведение сохранено.
- Проверки: минимальные production-path проверки PKT_PLAY с реальным профилем,
  Guide и handoff в рамках реализации соответствующей возможности;
  сохранять версионные границы и протокольные bytes.

## Архитектура нового клиента: модуль входа и локальность сценариев

**Статус:** реализовано 2026-11-04; адресные Linux checks и двухосное
code-review завершены. [Результаты и ограничения](tasks/login-architecture/SV-LOGIN-ARCH-005-integration.md).
Область — только новый клиент SV. Архитектура legacy,
сервера и общих модулей не меняется. Это направление развития, не новая
обязательная структура всех будущих возможностей.

Проработка тремя параллельными агентами оформлена в
[план и пять тикетов](tasks/login-architecture/README.md), затем реализована по
поручению пользователя. Пользователь принял правки 2026-11-04.

**Проблема:** один сценарий входа распределён между `session/`, `input/`, `ui/`
и корневым `endpoint-run.c`. Последний связывает socket/login, переходы фаз,
редактирование credentials, layout/hit testing, rendering и накопление evidence.
`SvEndpointInput` одновременно содержит input state и заимствованные данные
для отображения. `input/birth-interaction.h` лишь включает `session/birth.h`,
поэтому имя публичной границы пока не скрывает backing state. Сам размер файлов
не является основанием для разбиения; основание — знания и обязанности callers.

**Затронутый код:** `src/client/sv/endpoint-run.[ch]`,
`session/{pregame,birth,login-view}.[ch]`, `session/character-setup.h`,
`input/{endpoint,metaserver,chronicle,credentials,login-interaction,native-endpoint,native-login,native-name,native-birth}.[ch]`,
`input/birth-interaction.h`, `ui/{endpoint-scene,login-shell}.[ch]`,
`ui/login-layout.h`, `protocol/{contact-socket,contact,login}.[ch]`.
Пути в этой строке, кроме первого, относительны `src/client/sv/`.

**Предложение:**

1. Сосредоточить сценарий от выбора сервера до передачи готового соединения
   игровому runtime в `src/client/sv/login/`. Разместить рядом подмодули выбора
   сервера, account/character selection, character creation и отображения MOTD.
   Внутри сохранить раздельные правила, native input и presentation; физическое
   соседство не объединяет их ответственность. Общие text editing, font/render
   utilities, vault и wire protocol остаются самостоятельными модулями.
2. Сделать flow глубоким модулем: он владеет переходами и lifetime конкретной
   попытки входа, черновиками и отменой/повтором; наружу предоставляет смысловые
   commands, read-only view и результат перехода к игре либо выхода/ошибки.
   Начать с переноса существующего orchestration, не строить вторую state machine
   поверх сохранённых прежних правил. Протокольные состояния и UI focus имеют
   собственный смысл и не должны механически сливаться в одну enum.
3. Уменьшить обязанности native runner: SDL events, время, вызовы flow и rendering.
   Смысловые решения и восстановление имени уходят в flow; hit testing остаётся
   в native input. Renderer получает отдельное представление, а не общий
   `SvEndpointInput` как контейнер всей сцены.
4. Скрыть backing state character creation за настоящим интерфейсом interaction,
   сохранив production implementation. Разделять birth на внутренние подмодули
   только там, где это упрощает работу с правилами, а не по числу строк.
5. До подключения реального live runtime явно определить однократную передачу
   владения соединением, generation, server setup, выбранным персонажем и
   оставшимися прочитанными bytes. Сохранить значимый порядок пакетов и текущие
   проверки generation. Сегодня live handoff ещё представлен заглушкой;
   предложение не объявляет его реализованным.

Разделение production CLI и диагностического launcher, семантического результата
и evidence уже записано выше: продолжить эти предложения, не создавать
параллельную систему наблюдения. Сохранить существующее разделение session model,
protocol, interactions и UI по ADR-0001/0003/0005; потребуется актуализировать
описание физического размещения в `docs/sv-architecture.md` при реализации.
Не вводить общую event bus, plugin framework или новые абстракции I/O без
практической необходимости.

**Порядок:** сначала согласовать интерфейс flow и владельцев состояния;
затем минимальный перенос файлов с адаптацией includes/build callers;
после этого переносить orchestration и отделять view/input, сохраняя один
production-путь. Реальный live handoff и разделение CLI выполнять отдельными
задачами соответствующих возможностей.

**Необходимые проверки при реализации:** Linux build; согласованный минимум
существующих production-path core проверок выбора/создания персонажа, back/cancel,
retry/name recovery и generation. Для изменения handoff — сохранность leftover
bytes, порядок и однократная передача владения. Новые проверки только при
необходимости для конкретного core изменения; расширение набора заранее
согласовать с пользователем. Тесты расположения файлов и копия поведения для
тестов не нужны. При подготовке первоначального предложения код не менялся,
сборки и тесты не запускались. Последующая реализация, проверки и сравнение
известного неисправного historical fixture с исходным HEAD записаны в 005;
full suites не объявляются пройденными.

## SDL3 legacy: audioBuffer не достигает device open (C002 audit, 2026-11-05)

**Проблема:** main CFG `audioBuffer` описывает размер буфера/latency, но
`open_mixer` в `src/client/snd-sdl3.c` делает `(void)chunksize`; clamp и передача
из `open_audio` не меняют sample frames устройства. Успешный open нельзя считать
подтверждением применения выбранного буфера.

**Затронутый код:** `src/client/snd-sdl3.c`: `open_mixer`, `open_audio`; main CFG
reader/writer `audioBuffer` в `src/client/client.c`.

**Предложение:** отдельной legacy задачей использовать поддержанный SDL3 hint
`SDL_HINT_AUDIO_DEVICE_SAMPLE_FRAMES` до open, сохранять внешний hint priority,
и читать actual frames через mixer device ID + `SDL_GetAudioDeviceFormat`.
Различать requested/effective при отказе provider выполнить request. В C002
эта адаптация локально реализована для SV; объединение реализаций не требуется.

**Статус:** proposed для legacy, вне реализации C002.

**Необходимые проверки:** минимальный Linux production open с boundary/default
requested frames и queried effective frames; negotiated mismatch; приоритет и
восстановление внешнего hint; no-device не разрывает session. CFG не переписывать
на effective value. Расширение проверок заранее согласовать по AGENTS.md.

## SV: birth-complete scripted burst теряется на input boundary (C002 comparison, 2026-11-05)

**Проблема:** существующий `tests/sv_birth_complete_checks.py` не достигает
финального `PKT_PLAY` в первом `waiting` case. Изолированное дерево исходного
HEAD `dc14c4ad5d5a9aa202051de929cbc9b601ddadb7` с неизменёнными production и test
assertions воспроизводит тот же `poll(&final_ready, 1, 800) == 1` failure,
что и рабочее дерево C002. Raw comparison —
[baseline log](../.scratch/sv-c002/evidence/birth-baseline.log);
[current run report](../.scratch/sv-c002/evidence/birth-complete.log).
Это существующий дефект сценария, а не доказательство регрессии C002;
полный production birth handshake этим сценарием пока не подтверждён.

**Затронутый код:** `tests/sv/birth-complete.c`: `complete_peer`;
`tests/sv/name-editor-submit.c`: `push_to_mode`/`push_to_stats`;
`src/client/sv/login/creation/native-birth.c`: `apply` вызывает
`sv_pregame_input_boundary()` при смене owner. Этот production boundary
намеренно очищает ожидающие input events, поэтому заранее поставленная в очередь
последовательность жестов для нескольких owners может быть сброшена после
первого перехода.

**Предложение:** отдельно адаптировать существующие scripted gestures к
фактической готовности и переходам birth owner: подавать жесты последовательно,
сохраняя проверки отсутствия преждевременного `PKT_PLAY`, literal wire oracle и
ожидания authoritative reply. Не менять production boundary ради старого burst
и не создавать отдельную реализацию поведения для теста.

**Статус:** resolved 2026-11-07 отдельным test-only follow-up. В
`tests/sv/birth-complete.c` локальный wrapper настоящего `sv_endpoint_draw`
наблюдает borrowed view в main thread и подаёт один gesture по owner;
stats ждут highlight/count, финальный MODE Enter разрешает peer под mutex.
Production input boundary и shared `name-editor-submit.c` не изменены.
Shortcut letters сами подтверждают выбор, поэтому лишние Enter удалены.
Также source-backed исправлена устаревшая exit assertion: normal
`SDL_EVENT_QUIT` возвращает `0`, успех handshake по-прежнему различает phase.
Существующий Linux ASan+UBSan recipe прошёл waiting/partial/ack с literal
16266-byte handshake и всеми packet/outcome assertions; см.
[raw follow-up log](../.scratch/sv-c002/evidence/birth-fixed-2026-11-07/raw-04.log).
Исторические failures сохранены. Version matrix и соседние suites не расширялись.

**Проверки:** пользователь разрешил минимальное изменение существующей
Linux `sv_birth_complete_checks.py`; выполнены те же `waiting`/`partial`/`ack` cases
через production flow, первые character option bytes, однократный `PKT_PLAY`,
порядок authoritative replies и сохранение input boundaries. Не расширять matrix
или набор suite без согласования по AGENTS.md.

## Безопасная граница encoded PRF records в legacy

**Статус:** proposed separately, 2026-11-09; не входит в реализацию legacy в C004.

**Проблема:** `process_pref_file_aux_aux` без диагностики ставит NUL в byte1023
перед dispatch. У `A/P/H/C/!/?` это может обрезать escape-последовательность;
legacy `text_to_ascii` читает следующие bytes без проверки полноты escape.
Смысловой префикс файла и фактически исполненная строка отличаются незаметно.

**Затронутый код:** `src/client/c-files.c:1199`, `text_to_ascii` в том же файле.

**Предложение:** отдельно спроектировать безопасный decode и явную диагностику
границы, сохранив совместимость исполняемого префикса. Не увеличивать молча
допустимую последовательность команд. C004 выполняет SV-local bounded decode и
диагностику, но не меняет существующие клиенты.

**Необходимые проверки:** encoded boundary до/на/после byte1023, complete/incomplete
hex/octal/control escapes, точные фактически поставленные в очередь bytes,
includes до ограничения, отсутствие source rewrite.

## Компактное хранение provenance preferences SV

**Статус:** proposed separately, 2026-11-09; оптимизация не входит в C004.

**Проблема:** preference records, mappings, macros и effects содержат копию
`SvFilePath` с 4096-byte path. Поставляемый font PRF может содержать 1272 active
mapping records; хранение source path в record и effective mapping многократно
повторяет одно имя, увеличивая стоимость fresh character clone. Текущий core
ограничен 64MiB и не должен молча терять записи.

**Затронутый код:** `src/client/sv/settings/preferences.[ch]`, retained records и clone.

**Предложение:** при работе с реальными resource consumers C006/C007 оценить
стоимость и при необходимости хранить общую immutable source identity внутри
preference owner. Не объединять SV/legacy ради этой оптимизации и не менять
публичную provenance semantics.

**Необходимые проверки:** supplied font/graphics corpus, peak memory и clone cost,
source identity после U/B fallback, корректность borrowed views/lifetime и явное
поведение при memory bound. Намеренное SV-local дублирование legacy не является
причиной этой записи.

## Define safe legacy quantity arithmetic at numeric limits

**Status:** proposed separately, discovered during C005 planning, 2026-11-09;
legacy changes are outside C005 implementation scope.

**Problem:** `c_get_quantity` parses with `atoi` and computes `i1 * mul + i2`
in signed 32-bit arithmetic before clamping. Large ten-character inputs with
k/M/G can overflow; undefined C behavior is not a stable compatibility result.

**Affected code:** `src/client/c-util.c:c_get_quantity`; quantity callers in
`c-cmd.c`, `c-store.c`, `nclient.c` and SDL audio browsers.

**Proposal:** define bounded parsing and overflow disposition explicitly, then
consider a narrow legacy fix. Preserve defined baseline defaults, all shortcuts,
factor/fraction semantics and each caller's zero handling. C005 should use safe
SV-local arithmetic without claiming overflow artifacts as baseline semantics;
it does not authorize this legacy change or a wider numeric-parser refactor.

**Required checks:** representable values around s32 boundaries, k/M/G products,
fractional tails, max clamp, negative predef, Escape/empty/all and actual caller
zero interpretation. Current required platform is Linux; Windows deferred.

## Retain native input events when live dispatch is backpressured

**Status:** proposed separately, discovered during C005 integration, 2026-11-09.
Actual live input orchestration belongs to C014/C053; no generic queue is added
by C005.

**Problem:** the native adapter historically returns only whether an SDL event
was handled, and diagnostic event loops ignore app dispatch backpressure. C005
now exposes `SvNativeInput.last_result` and app atomic retry semantics, but a
future live event owner must retain denied input rather than consume it silently.

**Affected code:** `src/client/sv/input/native-input.[ch]`, `app.c:gameplay_key`,
`tests/sv/scenarios/*-scenario.c` input loops and future C014 native live owner.

**Proposal:** make the live owner inspect `last_result`, retain/retry the denied
event with its resolved bytes and generation, and drain semantic commands before
accepting later input. Preserve bounded queues, order and exactly-once delivery;
keep physical resolution stable across retry. Diagnostic loops are not an
implementation of the future live command executor.

**Required checks:** output/command backpressure, focus or binding change during
retry, stale generation, close/reconnect, and no dropped or duplicate bytes on
Linux. C005 checks its own production app retry result and prefix rollback;
these combined live-owner checks remain with the future caller.

## Уточнить неверные N records в shipped options.prf

**Проблема:** native bootstrap C006 показывает ошибки на строках
`N:ascii_weather` и `N:no2mask_weather`: baseline PRF parser не принимает N как
option assignment. Реализация C006 сохраняет source bytes и существующую
диагностику; намерение этих строк не подменяется догадкой X/Y.

**Код:** `lib/user/options.prf:36`, `lib/user/options.prf:37`,
`src/client/c-files.c:process_pref_file_aux_aux`,
`src/client/sv/settings/preferences.c:apply_record`.

**Предложение:** отдельно установить задуманные значения относительно baseline
defaults и заменить неверные записи корректными X/Y после решения об изменении
shipped defaults. Это не часть реализации C006.

**Статус:** proposed; source и поведение не изменены.

**Проверки:** минимальный bootstrap через production legacy/SV PRF paths,
expected значения обоих options, отсутствие новых parsing diagnostics и
сохранение иных records/порядка загрузки. Дополнительный набор согласуется по AGENTS.


## Legacy weather: частичная публикация cloud packet до полного чтения

**Проблема:** при анализе C007 обнаружено, что `Receive_weather` меняет cloud/
weather globals во время чтения, хотя неполный packet затем откатывает позицию
буфера. Возврат указателя сам по себе не откатывает уже изменённое состояние.

**Затронутый код:** `src/client/nclient.c`, `Receive_weather` (около строки 6482).

**Предложение:** отдельно рассмотреть decode во временный candidate и commit
после полного packet для legacy consumers. C007 сохраняет atomicity в собственной
SV-local реализации; исправление legacy в его scope не входит.

**Статус:** recorded, 2026-11-09; legacy изменение не реализовано.

**Необходимые проверки:** при отдельном исправлении — split packet внутри cloud
loop не меняет state до completion, complete/chained packets сохраняют порядок,
weather start/stop не повторяет одноразовые эффекты.


## Legacy Lua: sticky признаки обновления Guide/audio

**Проблема:** `updated_audio` и `updated_guide` устанавливаются при успешном INIT
по substring имени и не сбрасываются. После первой такой передачи warning/
metadata rebuild повторяются при последующих last END, даже без нового обновления.

**Затронутый код:** `src/client/nclient.c:Receive_file`, static flags около
строки 489, INIT около 518–524, END около 557–589.

**Предложение:** отдельно решить, должны ли признаки принадлежать batch или
процессу, и согласовать изменение behavior baseline. План C009 пока сохраняет
фактический sticky lifetime; исправление не входит в текущую задачу.

**Статус:** recorded, 2026-11-10; изменение поведения не согласовано и не реализовано.

**Необходимые проверки:** два последовательных batches (первый Guide/audio,
второй без них), failed INIT/END, reconnect; точное число warnings/rebuild и
отсутствие незаявленной audio backend reinitialization. Только применимая платформа
по текущему правилу; расширение набора согласовать.

## Legacy Lua: done flags не отражают ошибку загрузки startup scripts

**Проблема:** init_lua/open_lua игнорируют failure pern_dofile и ставят done flags.
exec_lua/string_exec_lua возвращают 0/empty и при ошибках, поэтому такой результат
нельзя надёжно отличить от корректных пустых данных.

**Затронутый код:** `src/client/c-script.c:init_lua`, `open_lua`, `pern_dofile`,
`exec_lua`, `string_exec_lua` (около строк 348–546).

**Предложение:** отдельно в legacy ввести явный результат script loading и
передавать его владельцу startup, сохранив видимую диагностику. C009 требует
явный error contract только в SV; legacy refactor этим планом не разрешён.

**Статус:** recorded, 2026-11-10; legacy не изменён.

**Необходимые проверки:** missing/invalid script и корректный zero/empty getter,
ошибка вложенного script, reopen после ошибки; отсутствие ложного startup success
и сохранение допустимых пустых результатов. Набор для отдельного исправления
согласовать по AGENTS.

## Legacy transfer: границы DATA и восстановление после ошибки записи

**Проблема:** DATA length копируется в фиксированный buffer 1024 без проверки;
unknown ID возвращает управление до consumption payload. Ошибка дисковой записи
возвращается как 0 из Receive_file после уже совершённых effects, как и ожидание
продолжения пакета, но без ACK/ERR. Это затрудняет корректное framing/recovery.

**Затронутый код:** `src/common/files.c:local_file_write`,
`src/client/nclient.c:Receive_file`, `Receive_file_data`.

**Предложение:** отдельной задачей исправить legacy length validation и разбор
неизвестного ID, разделить incomplete/error result и согласовать wire recovery.
В C010 небезопасные buffer accesses не копируются; legacy/common не меняются,
новый ERR/retry protocol не вводится скрыто.

**Статус:** recorded, 2026-11-10; legacy исправление и изменение wire recovery
не реализованы, не включены в план C010.

**Необходимые проверки:** при отдельном исправлении — граничные длины DATA,
unknown/zero ID с payload и следующим пакетом, split/chained packets,
write failure без повторной записи и ложного ACK. Расширение проверок согласовать.

## Legacy transfer: освобождение record при rejected INIT

**Проблема:** `local_file_init` выделяет и включает transfer record в `fdata`
до проверки filename, затем при leading `/` или `..` возвращает отказ без
`remove_ft`. Оставшийся record влияет и на `get_xfers_num`/last-END reload.

**Затронутый код:** `src/common/files.c:getfile`, `local_file_init`, `get_xfers_num`.

**Предложение:** отдельно перенести validation до allocation либо освобождать
record на каждом отказе. В SV C010 обеспечить собственный корректный cleanup,
не копируя утечку; legacy исправление не включать в перенос.

**Статус:** recorded, 2026-11-10; legacy не изменён.

**Необходимые проверки:** rejected filenames не меняют active-transfer count,
handles/temp files; следующий valid transfer завершается и достигает last-END
reload. Для отдельной задачи согласовать необходимый минимум проверок.

## Native checks: ручные списки production sources в старых runners

**Проблема:** `tests/sv_first_session_checks.py` содержит собственный SOURCES list,
который не отражает новые зависимости native startup C006–C010. Независимые списки
легко расходятся с production `makefile.sv`; успешная старая проверка не является
доказательством актуального link closure.

**Затронутый код:** `tests/sv_first_session_checks.py`, `src/makefile.sv`;
другие native runners с ручными source lists требуют отдельной сверки.

**Предложение:** по мере самостоятельной необходимости переводить выбранные
native checks на production object closure с их существующим test main.
Не создавать общий новый harness и не запускать широкую миграцию только ради
устранения дублирования.

**Статус:** recorded, 2026-11-10. Focused `--case c010-transfer` в SV-C010-006
использует конкретный make target с production objects; старые modes в этой
задаче не запускались и не получают PASS от нового case.

**Необходимые проверки:** для каждого отдельно перенесённого runner — точный
link closure/flags и только его согласованные production scenarios на Linux;
сохранять различие ordinary и sanitizer objects. Расширение набора согласовать.


## Legacy startup: момент отправки character options до PLAY

**Проблема:** pre-initialization в `c-init.c` вызывает `Send_options`, который
только добавляет bytes в `wbuf`. `Net_start` очищает эту очередь перед PLAY,
но промежуточные birth/MOTD input loops могут вызвать `Net_flush`. Поэтому
комментарий о ранней доставке options не задаёт однозначной wire-границы.
Pre-play server dispatcher не обрабатывает standalone `PKT_OPTIONS`.
В actual C011 проверке SV безусловная ранняя отправка блокировала PLAY;
SV-local вызов убран, effective options остаются внутри PLAY.
Наличие аналогичного пользовательского дефекта legacy отдельно не доказано.

**Затронутый код:** `src/client/c-init.c` (pre-initialize/Send_options),
`src/client/nclient.c:Send_options`, `Net_start`, `Net_flush`,
`src/client/c-util.c` (input loops), `src/server/nserver.c:Receive_play`.

**Предложение:** отдельно проследить legacy queue/flush в existing-character,
interactive birth и MOTD paths; затем согласовать явный момент отправки,
не меняя протокол и обязательный effective options snapshot в PLAY.

**Статус:** recorded, 2026-11-11; legacy/common/server не изменены.
Отдельное улучшение не включено в C011.

**Необходимые проверки:** при отдельной работе — actual server capture
первого pre-play packet и embedded PLAY options для быстрого входа, ожидания
в birth/MOTD и keepalive; сравнение censor option при initial notes.
Необходимый минимум проверок согласовать отдельно.


## SV MOTD: устаревшая ошибка SDL после успешного draw

**Проблема:** после передачи новой texture в MOTD cache локальный pointer
обнуляется, затем передаётся в `SDL_DestroyTexture`. Успешный production draw
оставляет `SDL_GetError() = Parameter 'texture' is invalid`. Это затрудняет
диагностику последующего общего startup failure: сообщение само по себе не
доказывает ошибку отрисовки. В C011 probe остановка была вызвана отсутствием
MOTD acknowledgement и лимитом кадров; renderer regression не подтверждён.

**Затронутый код:** `src/client/sv/login/ui/login-shell.c:sv_login_shell_motd`,
`src/client/sv/login/native-run.c` (общий startup failure diagnostic).

**Предложение:** отдельно убрать вызов destroy для NULL и сохранять диагностику
реального отказа в месте возникновения, отличая её от завершения по frame limit.

**Статус:** recorded, 2026-11-11; исправление не включено в C011.

**Необходимые проверки:** успешный MOTD draw/cache reuse не создаёт эту ошибку;
реальная ошибка и frame-limit exit различаются; минимальный production probe
использует прежние ресурсы без новой visual matrix. Проверки согласовать отдельно.
