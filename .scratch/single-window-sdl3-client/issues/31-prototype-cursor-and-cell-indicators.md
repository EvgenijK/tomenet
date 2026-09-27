# Prototype cursor and cell indicators

Type: prototype
Status: resolved
Resolution: out of scope — visual design delegated to HTML UX prototype
Assignee: codex
Blocked by: 03, 21, 23

## Question

Применить [Choose raster references and defect compatibility](23-choose-raster-references-and-defect-compatibility.md#answer): full fitted map использует единую целочисленную final-cell geometry; cursor/indicator positions и clipping совпадают с этой сеткой и hit testing. Дизайн должен явно выбрать единицы толщины/минимальную различимость при scale changes, не вводя pixel-perfect contract и не возвращая whole-map double scaling.

Какой новый visual design использует single-window client для map cursor и связанных cell-bound indicators в keyboard targeting, mouse hover/selection и других различимых состояниях, не копируя X11 XOR или legacy SDL3 white-alpha overlay?

Прототип должен показать normal/bright/dark terrain, font-only и mixed tiles/glyphs, player/monster/item cells, color-vision and low-contrast edge cases, focus/target/hover distinctions, animation или static alternatives и поведение при смене map scale. Сохраняются renderer-policy invariants: indicator привязан к ровно одной logical cell или явно заданной области, не повреждает underlying composition, не накапливает alpha, корректно исчезает/перемещается и остаётся различимым без pixel-perfect contract. Конкретный UX утверждается человеком; implementation и performance budgets не входят в тикет.

## Comments

### Prototype ready for live review — 2026-09-18

[Cursor prototype](../cursor-prototype.html?variant=A) предлагает A: угловые скобки / внутренняя рамка / нижняя черта; B: внешняя рамка / ромб / верхняя черта; C: боковые скобы / двойное подчёркивание / верхние уголки для keyboard cursor / target / hover соответственно. Варианты переключаются нижней панелью и URL `?variant=A|B|C`.

Запуск из корня репозитория: `python3 -m http.server 8767 --bind 127.0.0.1`; путь `/.scratch/single-window-sdl3-client/cursor-prototype.html`. Предусмотрены клетки 8×12–32×48 logical units, dark/normal/bright terrain, glyph/mixed sketches, grayscale и low-contrast assets, overlap, focus loss, removal и update beneath target. Static default и optional thickness pulse — предложения, не принятые решения. Stroke proposal: 1 logical unit с чёрной подложкой и cell clipping; маленькие клетки требуют отдельной оценки различимости.

Браузер открыл прототип без console errors/warnings; визуально просмотрен вариант A; проверены B/C switching, URL reload, 8×12, grayscale и overlap controls. Это bounded prototype check, не native acceptance. Synthetic 36×14 scene и tile-like рисунки не представляют full viewport fitting, настоящие tiles/masks, PCF, SDL pixel rounding или симуляцию конкретных видов color-vision deficiency. Эти ограничения нельзя выдавать за доказанную renderer parity.

На момент подготовки ожидался живой выбор пользователя; итог обсуждения записан ниже.

## Answer

Пользователь отклонил все три варианта: «все варианты выглядят плохо, оставь конкретный внешний вид курсора на html прототип».

Тикет закрыт как вынесенный за пределы этой planning-карты. Конкретный внешний вид курсора и связанных cell indicators определяется в основном HTML UX prototype `/home/svechnik/Projects/github_site/tomenet_interface`. Формы, цвета, толщина, визуальные различия состояний и static/animation treatment из отклонённого эскиза не приняты. Этот ответ не утверждает существующий дизайн HTML-прототипа и не требует его изменения в текущей сессии.

Сохраняются ранее принятые native invariants: привязка к final-cell geometry и hit testing, clipping, сохранение underlying composition, отсутствие накопления alpha/stale output и корректное перемещение/исчезновение. Подтверждённый в HTML дизайн становится UX reference; различимость состояний, контраст и поведение при scale changes проверяются при native acceptance. Выбор конкретного рисунка больше не блокирует завершение planning-карты.

Локальный `cursor-prototype.html` остаётся только отклонённым эскизом, не нормативным UX asset.
