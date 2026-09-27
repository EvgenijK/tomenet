# Prototype the server-defined MOTD screen

Type: prototype
Status: resolved
Assignee: codex
Blocked by: 21

## Question

Как выглядит отдельная whole-window MOTD surface нового клиента, показывающая произвольный server-defined setup MOTD без изменения текста/строк/colors/ASCII art и без hardcoded Welcome/settings/link sections?

Следовать [Specify surface layouts and responsive rules](21-specify-surface-layouts-and-responsive-rules.md): no-wrap, proportionally fit complete MOTD при нехватке места, independent text/map roles, minimum1024×768 logical client area, OS DPI contract. Native setup payload23×120-byte strides, exact formatted markers и existing any-key/skip/once-only semantics сохраняются; client renderer не определяет outgoing repertoire. MOTD расположен после character selection/creation, до Net_start/map, согласно `src/client/c-init.c:4434–4476` и `src/client/c-files.c:1915–1941`.

Проверить конкретные макеты при разных window dimensions/text metrics/DPI, коротком/широком/многоцветном/неизвестном content, чтобы full-scene fitting не теряло decoration/alignment/visible glyphs. Показать human side-by-side варианты и получить live UX approval. Source-preserving model и input behaviour не перепроектируются. Assets/source capture ссылки, runtime proof и exact reference fonts/defect policy остаются separate evidence owners.

## Comments

Created по прямому уточнению пользователя: «для этого экрана будет сделан отдельный прототип». Устойчивая копия присланного screenshot: [Server MOTD reference](../motd-reference.png), copied byte-for-byte from `/tmp/codex-clipboard-bLM595.png`. Native source/setup fixture нужен отдельно от raster reference. Execution production client находится вне planning scope.

### Prototype review pending — 2026-09-18

Claimed by codex as the first open, unblocked, unclaimed ticket. Applied wayfinder and prototype/UI. No product decision or human UX approval recorded yet.

[Throwaway MOTD prototype](../motd-prototype.html) compares A (top-left, desired text size), B (centered, desired text size), C (centered, maximum uniform fit). All use the same intact 23-row composition; A/B shrink on overflow. Padding24 is a proposal. No semantic content sections, wrap, scroll, URL actions or gameplay HUD are introduced. [Side-by-side capture](../motd-comparison.png).

Run from repo root: `python3 -m http.server 8875 --bind 127.0.0.1 --directory .scratch/single-window-sdl3-client`, then open `http://127.0.0.1:8875/motd-prototype.html?variant=A` (also B/C). Standalone HTML can also be opened directly. Switch off «Рядом» for a larger single-variant view.

Source check: `src/server/nserver.c:463–483` builds setup MOTD from `lib/text/news.txt` into23×120-byte strides; `src/client/c-files.c:1915–1941` displays23 rows then waits via `inkey()`. Supplied screenshot remains a separate reference. Prototype fixtures are synthetic cell/color arrays, not a captured native payload or byte/marker decoder. The screenshot's server copy is not inferred to be the current local server content.

Browser geometry check:720 cases (3 variants ×3 window sizes ×4 text metrics ×4 simulated DPI scales ×5 fixtures), every composition's computed rectangle remains inside the logical window and retains23 rows. Fixtures include empty, short, multicolored, unfamiliar placement and119-column content. Console:0 errors/warnings. Screenshot visually inspected. URL variant navigation and reload checked separately. This checks prototype mechanics and logical bounds, not native font ink extents, SDL rounding, OS DPI transitions, protocol decoding or keyboard/skip/once-only parity.

Pending live review: placement/fit choice and padding. Native input behavior remains as previously decided. Branch capture and resolution follow approval; production implementation remains out of scope.

### Live UX approval — 2026-09-18

Пользователь выбрал: «вариант C».

## Answer

Принят показанный вариант C: whole-window MOTD центрирует полную серверную композицию и применяет maximum uniform fit в доступную область. Показанный отступ24 logical units сохраняется как параметр выбранного макета; отдельная настройка отступа не вводится.

- Композиция содержит все23 исходные строки, включая пустые, с сохранением исходных column positions, colors, ASCII art и неизвестного содержимого. Ширина определяется полным decoded visible extent;120-byte stride не считается числом видимых колонок. Пустое содержимое даёт пустую сцену без клиентского welcome-текста.
- Fit может увеличивать и уменьшать композицию; исходный text size не ограничивает увеличение. Отношение сторон text-cell geometry сохраняется. Resize/DPI пересчитывают fit и центрирование целиком, без wrap, scroll, cropping или перестановки строк. Map font и map zoom на MOTD не влияют.
- Формула прототипа: `s = min((W - 48) / content_width, (H - 48) / content_height)`, центрирование по обеим осям. Она описывает layout intention; final glyph preparation, rounding и ink bounds следуют принятому raster contract, а не browser canvas scaling.
- Server-defined content не превращается в hardcoded welcome/settings/link sections. До gameplay отсутствует игровой HUD. Порядок после character selection/creation и до Net_start/map, any-key, skip и once-per-login semantics остаются прежними.

Основание: [интерактивный прототип](../motd-prototype.html?variant=C), [сравнение вариантов](../motd-comparison.png) и живой выбор пользователя.720 проверок подтверждают только геометрические bounds синтетических fixtures. Native payload/marker decoding, glyph ink visibility и OS/SDL DPI behavior остаются evidence obligations в [Design parity evidence and acceptance](09-design-parity-evidence-and-acceptance.md), а не доказанными свойствами этого HTML.

Новых продуктовых вопросов этот выбор не выявил. Production implementation остаётся вне planning-карты.

Primary-source capture: branch `prototype/server-defined-motd`, commit `e271c582cbf89d404deda6189881e531d3775770`. Содержит только прототип, сравнительный screenshot и исходный MOTD reference. Рабочая ветка и пользовательский index не менялись.
