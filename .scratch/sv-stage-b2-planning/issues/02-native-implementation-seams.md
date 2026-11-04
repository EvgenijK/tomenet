# B2: production implementation seams

Type: decision
Status: resolved
Assignee: b2_native_plan

## Question

Как перенести подтверждённый login UX prototype в production SV минимальными изменениями, сохранив принятые behavior outcomes B и пригодную границу для C?

## Answer

### Решение и границы

B2 переносит visual/layout обязательства уже существующих outcomes B. Принятие B пользователем 2026-10-28 сохраняется; canonical ownership, включая B54 outcomes, заморожен. Новых capability owners, перераспределения B/C и реализации gameplay C нет. UI передаёт существующие semantic commands в `input/login-interaction.h` и `input/birth-interaction.h`; protocol, server setup, разрешённые choices, version gates, generation, cancel/DNA/random/confirmation и authoritative server result сохраняются. Нельзя переводить JS-правила макета в новую игровую модель.

Нужен небольшой SV-local presentation модуль login flow; универсальный widget framework, DOM/CSS engine, общая event bus и рефакторинг legacy/common не нужны. Его интерфейс: read views + размер/scale + локальное состояние представления → layout/draw; SDL gestures + этот же layout → существующие semantic actions. Layout содержит rectangles/clip/visible ranges/semantic IDs, а не указатели на mutable session state. После geometry/font/model revision он пересчитывается до draw и hit testing; заимствованные views не сохраняются между кадрами. Focus/hover/scroll/show-password принадлежат UI; semantic selection/unfinished input остаются у interactions.

### Проверенные причины разрыва

- `src/client/sv/ui/endpoint-scene.c`: фиксированные строки/координаты и отдельная сине-серая палитра. `line()` заменяет non-ASCII на `?`. Нет shell D, background, logo, карточек, полей и создания в композиции макета.
- `ui/font.[ch]`: документирован Stage A ASCII profile; только fixed-width, один размер 18; это нельзя выдавать за text/font роли макета. Existing API использует и shell A: расширение должно сохранить его поведение или B2 получает локальный text helper.
- `endpoint-run.c::enter_credentials`: renderer видит credentials как одну `contact_status` строку. Для двух полей нужны read-only presentation данные draft/mask/focus/error/vault status; пароль нельзя копировать в retained render cache/evidence. `input/credentials.c` уже владеет lookup/restore/end; не создавать второй vault flow.
- `input/native-endpoint.c`: нет mouse route для server cards/direct connection. `native-login.c`, `native-name.c` содержат hardcoded hit zones, а `native-birth.h` — фиксированные row constants. `ui/login-layout.h` делит лишь несколько чисел; полноценная общая geometry render/hit-test необходима.
- `endpoint-run.c` уже корректирует window-coordinate mouse events в logical units в contact loop, но endpoint/credentials имеют отдельные loops. Нужна одна явная conversion policy для всех B2 screens, включая wheel/drag/resize; не двойное масштабирование.
- Read-only reference `/home/svechnik/Projects/github_site/tomenet_interface/login.html`: body variant D, golden/green translucent panels, hero/logo, side chronicle/realm links; account, character, creation layouts, компактный height <=800 и width 821..1240. Точные утверждённые состояния/ресурсы и источник side content фиксируются соседним решением 01; prototype data не становится server truth.

### Минимальные production files / seams

1. Развить `ui/login-layout.h` в `ui/login-layout.[ch]`: layout startup shell и конкретных surfaces, logical rects и видимые идентичности. Простая структура данных и конкретные functions; без runtime widget tree. `native-*.c` получают рассчитанную geometry, а не импортируют новые координатные константы.
2. Добавить `ui/login-scene.[ch]` (либо переименовать/разделить существующий `endpoint-scene` при сохранении thin caller): facade ресурсов/lifetime/draw + конкретные server/account/character/birth routines. Выделять дополнительные files только если реально упрощает владение параллельными задачами.
3. Небольшие `ui/login-theme.h`, `ui/login-resources.[ch]`: semantic palette, spacing/type roles (body mono (итоговый cascade D: Consolas / Liberation Mono / DejaVu Sans Mono), titles serif, ASCII mono), textures/fonts и доступный fallback. Все type roles берутся из итогового computed CSS, а не промежуточных variants. Animated ASCII logo, torches и background transitions входят явно: таймлайн UI использует elapsed time без SDL_Delay/wait внутри draw, reduced-motion/static state и reset resources имеют явную политику; эти эффекты не зависят от C006. Общие для B2, физически внутри SV; выбранные ресурсы разместить в SV-owned installation path с лицензиями. `src/makefile.sv` — только явные objects/dependencies/install inputs, существующий `makefile.sdl3` не трогать.
4. `ui/font.[ch]` минимально расширить измерением/draw нужных type roles и UTF-8 UI text, сохранив Stage A callers, или сделать B2-local text implementation. Shared geometry должна измерять строки тем же font/size, которым рисует. Нельзя менять wire encoding: UI Unicode/стрелки/символы и byte→Unicode display mapping отделены от `input/text-field.[ch]` и contact/name byte validation. Нет всеобщего Unicode editor/layout engine.
5. `input/credentials.[ch]`, `input/native-{endpoint,login,name,birth}.[ch]`, `endpoint-run.c`: подключить mouse/focus/keyboard actions, read-only field view и общую layout, сохранить semantic APIs и поколения. Нужны caret/selection/scroll у реально редактируемых полей; не заводить дублирующий editor. Show password локален и сбрасывается при выходе; private buffers и vault lifecycle сохраняются.

Файлы `ui/ui.[ch]` и игровой renderer A не требуют рестайлинга в B2. Для C остаются usable primitives text measure/draw, logical-coordinate conversion и rect/clip; C сможет использовать их без зависимости от startup views/vault/birth и без обязательства сейчас построить общую систему widgets. Theme login не становится автоматически theme gameplay.

### Исполнимый DAG (6 implementation tickets)

Нумерация здесь — предложение для итогового stage index, не уже созданные owners.

| Ticket | Задача и ограниченный результат | Зависит от |
|---|---|---|
| B2-001 | Зафиксировать reference D/state inventory/resource provenance и screen mapping; добавить login-local palette/type/resource loading, измерение текста и minimum reusable logical geometry. Сохранить A font API; подтвердить выбранный PNG/SVG decode path в имеющемся SDL до добавления dependency. | Planning 01/02, утверждённый scope |
| B2-002 | Реализовать shell/layout D: hero/animated ASCII logo/torches/background transitions, panels/side areas по принятому inventory, clipping/overflow и width/height variants; единая output/window/logical conversion и concrete layout IDs, rebuild на resize/scale/reset. | B2-001 |
| B2-003 | Server cards/direct connection + account form: renderer, focus/hover/selection, поле host, name/password/show, Enter/Tab/click, errors/connect/retry/vault status. Только существующие endpoint/credential actions; без изменения contact semantics. | B2-002 |
| B2-004 | Character overview/name/MOTD и confirmation/error/temporary states по inventory: cards/detail/badges/footer, keyboard/mouse, name caret/selection; существующие login commands, server truth и отмена. | B2-002; интеграция с B2-003 |
| B2-005 | Creation workspace: summary/progress/choice cards/detail/stats controls/viewport и состояния подтверждения, guide/quit в согласованном B scope. Привязать legal/visible/highlighted IDs и existing random/DNA commands; не переносить JS model/preview assumptions. | B2-002; интеграция с B2-004 |
| B2-006 | Свести production flow и Linux visual/input acceptance: одинаковые reference/native states и viewport, software/OpenGL, resize/scale/focus/teardown и semantic regression scope. Зафиксировать deviations/user decisions и итог B2; C handoff остаётся прежним. | B2-003, B2-004, B2-005 |

После foundation B2-003/004/005 можно вести параллельно в разных surface routines, но один владелец меняет общий `endpoint-run.c`, facade и makefile; adapters и screenshots используют один layout contract. Новый generic framework не является prerequisite.

### Проверки и материальные риски

Обязательная платформа Linux; Windows/MinGW/Wine отложены. Проверки касаются production path. Пользователь согласовал ранее расширение одного Secret Service теста, но это не blanket authorization новых B2 checks. Новые automated visual harness, golden pixel suite, huge matrix и повторный полный B acceptance не планируются как обязательные. На implementation шаге выбрать только необходимый минимум core regressions для изменённой geometry/semantic route из существующих проверок; любое расширение согласовать заранее. Визуальное сопоставление реального native executable с закреплённым reference — отдельное необходимое evidence; exact pixels не oracle, состав/иерархия/layout/type/color/состояния должны соответствовать.

- Набор reference D включает изображения PNG, inline/JS SVG logo, serif headings и несколько CSS слоёв. Нельзя незаметно заменить их plain mono text: сначала определить переносимый ресурс/лицензию/packaging. SV сейчас линкует sdl3+sdl3-ttf+FreeType, не SDL_image; loader/dependency выбирается по реальной поддержке и минимальному scope.
- PCF fallback A не эквивалентен UI fonts D. Нужны определённый bundled font/fallback и проверяемое отсутствие missing glyph; PCF fallback сохранять для существующих A callers, но не объявлять им visual fidelity B2.
- Side chronicle/realm links и creation descriptive data в reference используют snapshots. Не выдумывать live feed/provider: owner/data source и approved empty/static states должны быть явно решены в inventory. Без этого можно визуально совпасть и семантически ввести игрока в заблуждение.
- Существующий минимум окна 1024×768 не равен всем web breakpoints; B2 обязан покрыть поддерживаемый desktop диапазон и компактную высоту. Решение о расширении ниже этого минимума — scope decision, а не скрытая отмена контроля overflow.
- Credentials имеют последовательные stages и buffer editing, поэтому перенос двух одновременно видимых полей — не только draw. Focus editing должен сохранять limits, private-field history, cancel/stale lookup и сохранение только после server confirmation. Нельзя отправлять account заново от простого focus или UI tab.
- UI caches обязаны invalidation на font/scale/resource/model revision, освобождение texture на renderer reset; hover animation не может блокировать сеть/main loop.

### Источники и пределы исследования

Прочитаны AGENTS.md, CONTEXT.md, ADR-0001..0006, docs/sv-architecture.md, issue-tracker/platform-validation и skills graphify/codebase-design. Graphify query выполнен до source search; широкий результат обрезан, поэтому интерфейсы уточнены прямым чтением указанных production files. Исходники и tests не менялись; граф не обновлялся, тесты не запускались. B54 outcomes ownership не менялся; исторический moved ticket B-054 не является owner этого этапа.
