# Choose raster references and defect compatibility

Type: grilling
Status: resolved
Assignee: codex
Blocked by: 13, 30, 33, 34

## Question

Для visual roles, explicit TTF legacy glyph correspondence, corrected PCF origin/default/bounds и font-switch lifecycle применить [Define encoding and glyph identity](22-define-encoding-and-glyph-identity.md) и [all bundled font metadata](../research/encoding-font-assets.md). Выбрать конкретные font/profile assets и complete baseline glyph corpus, различая metadata, bitmap drawings и proven Unicode correspondence. Shipped profile с replacements вместо поддержанных glyphs не доказывает parity; проверять font-only и mixed tiles/glyphs, ASCII weather, terrain/actor masks и map-font switches отдельно от text-only changes.

Какие reference assets/environments и observable invariants задают renderer parity нового SDL3-клиента при существующих различиях X11 и SDL3? Определить строгие semantic comparisons и допустимые raster deviations для PCF/TTF, cursor, outline/alpha, masks, scaling filters, palette и timed effects. Решить, какие baseline outputs требуют точного сохранения, какие должны сохранять информацию/поведение при иной композиции, и как обнаруженные legacy defects получают явный disposition и regression evidence. Не объявлять blanket pixel identity между backend-ами и не вводить допуски без обоснования.

Encoding-specific mapping и его defects решаются в [Define encoding and glyph identity](22-define-encoding-and-glyph-identity.md). Численные performance budgets и устройство harness обсуждаются в существующих latency/acceptance тикетах. Здесь выбирается проверяемая политика совместимости, а не реализация renderer или golden tests.

Источник: [Specify renderer parity](13-specify-renderer-parity.md) и [Renderer parity inventory](../research/renderer-parity.md).

## Comments

### Canonical raster oracle — confirmed by user

Пользователь выбрал вариант A: новый SDL3 pipeline является canonical raster pipeline. X11 и legacy SDL3 остаются нормативными источниками semantic identity и behavior, но не равноправными pixel oracles. Точная pixel equality требуется только для закреплённых воспроизводимых asset/backend/scale/dependency environments; между target platforms и legacy backends строго сохраняются identity, geometry, color roles, layer ordering и lifecycle, а допустимые raster-only различия задаются отдельно.

### Pixel-perfect contract removed — confirmed by user

Пользователь уточнил предыдущее решение: pixel-perfect полностью исключается и не требуется даже для закреплённых environments. Normative acceptance проверяет exact semantic identity, geometry, color roles, layer ordering, visibility, lifecycle и structural raster properties: glyph presence, footprint, clipping, legibility, transparency и отсутствие bleed/stale output. Raster captures допустимы как diagnostic/review evidence, но не как pixel-equality gates. Reference bundle означает конечный набор coverage scenarios для renderer paths, не набор профилей с обещанием неизменных pixels.

### Defects, cursor, TTF, scaling and custom assets — partial confirmation

- Q3 принят: legacy defect исправляется, если он небезопасен, теряет поддерживаемую информацию, неверно индексирует ресурс, оставляет stale output или ложно сообщает успех. Сохранение quirks требует доказанной зависимости и regression evidence.
- Q4: legacy SDL3 alpha cursor и X11 XOR cursor не выбираются как канонический дизайн; finished client получит новый cursor design. Его конкретные visual и state variants вынесены в [Prototype cursor and cell indicators](31-prototype-cursor-and-cell-indicators.md).
- Q7: finished client должен иметь как минимум один TTF; пользователь отверг формулировку про специально выбранный «лицензируемый TTF». Требуется уточнить, поставляется ли font/profile вместе с клиентом или выбирается из внешней среды.
- Q8: локальная filter policy не принята. Сначала требуется исследовать целостную модель UI/asset scaling, применимые standards и их использование в этом проекте. Создан research ticket [Research UI and asset scaling standards](30-research-ui-and-asset-scaling-standards.md).
- Q9: возможность использовать собственные fonts и tilesets существующего клиента обязательна и сохраняется. Validation, correspondence, scaling и failure contract ещё предстоит согласовать после scaling research.

### Scaling research completed

[Research UI and asset scaling standards](30-research-ui-and-asset-scaling-standards.md) разрешён с primary-source отчётом [UI and asset scaling standards](../research/ui-and-asset-scaling-standards.md). Исследование разделило SDL window coordinates, output pixels, OS display scale и независимые user UI/map scales; сравнило output-pixel, role-target и fixed-framebuffer composition; fractional grid policies; TTF/PCF/tile/mask/raw-picture scaling; custom-asset lifecycle и monitor transitions. Оно не выбрало product policy и не ввело pixel-perfect acceptance.

### Asset coverage and bundled TTF — confirmed by user

- Q6 принят: все bundled assets проходят discovery/load/metadata/range/default/failure validation, а representative assets проходят глубокие semantic/lifecycle/composition scenarios без pixel equality. Representative corpus включает ordinary `9x15.pcf`, полный byte-ID path `9x15tg`, mixed `16x24tg` + `16x24sv`, PCF origin fixtures `12x24`/`8x16` и partial-range `16x22`; проверяются numeric IDs, PRF mappings, graphics overrides, masks/subsets, weather/raw pictures, missing/default и font switching.
- Q7 принят: finished client поставляет как минимум один TTF, чтобы штатный TTF path не зависел от внешнего файла. Точный asset/profile не выбирается здесь; candidate facts вынесены в [Inventory bundled TTF candidates](32-inventory-bundled-ttf-candidates.md), а default/resource ownership остаётся у persistence ticket.

### Coordinate and composition model — confirmed by user

Q8.1 принят: layout и hit testing задаются в floating-point logical UI units; SDL window coordinates преобразуются через актуальные output-pixel size и display scale. UI scale и map scale независимы. Финальная композиция строится для текущих output pixels; role-specific render targets допустимы как внутренняя техника, но один fixed framebuffer, растягиваемый вместе со всем UI, не является моделью клиента. Monitor/display-scale changes вызывают relayout/rerasterization без изменения semantic session state.

### Map visual stack — confirmed by user

Q5 принят: каждая map cell сохраняет независимые terrain и foreground identities/attrs до rendering. Canonical composition следует semantic stack `background/color role → terrain → foreground game visual → effects/weather → indicators/cursor`; удаление или перемещение foreground открывает актуальный terrain, не старый raster snapshot. X11 bitwise masks и legacy SDL3 alpha/outline являются implementation evidence, но не нормативными pixels нового pipeline.

### Map composition canvas and fit zoom — confirmed by user

Смысл fit и zoom сохраняется; описанная ниже промежуточная raster-canvas technique заменена разделом «Prepared assets and 1:1 composition».

Q8.2 принят в варианте 3. Выбранный map font/profile задаёт внутреннюю uniform cell geometry; tiles приводятся к той же cell geometry; полный `66×22` или `66×44` map visual stack собирается в отдельный composition canvas. Canvas uniform-scale вписывается в allocated map area в обе стороны с сохранением aspect ratio, без crop/scroll, и центрируется. Maximum fit заполняет одну размерность области. Независимый user map fit zoom лежит в диапазоне до 100% maximum fit и может только уменьшить композицию; resize/monitor changes пересчитывают maximum fit, сохраняя выбранный zoom и semantic viewport. Конкретные range/steps/default/storage принадлежат persistence ticket.

### Single-pass proposal — clarification requested

История обсуждения; итоговое решение о подготовке assets записано в разделе «Prepared assets and 1:1 composition».

Пользователь поддержал направление single-pass `source asset → final cell`, но не принял упрощённую tile/mask модель. Решение обязано сохранить фактический current-client preparation/composition process, а не рисовать готовый tile и затем произвольные masks поверх него. Также требуется сначала оценить готовые SDL3 scaling/sampling algorithms вместо автоматического переноса current custom resamplers. Создан [Research SDL3 tile composition and scaling APIs](33-research-sdl3-tile-composition-and-scaling-apis.md); Q8.3 остаётся открытым.

### SDL3 composition/scaling research completed

[Research SDL3 tile composition and scaling APIs](33-research-sdl3-tile-composition-and-scaling-apis.md) разрешён с отчётом [SDL3 tile composition and scaling APIs](../research/sdl3-tile-composition-and-scaling-apis.md). Он подтверждает single-scaling pipeline без semantic упрощения: encoded mask keys декодируются source-space в authored-color/base, background-hole, foreground-coverage и optional outline roles; live attrs, independent subtilesets и legacy single/two-mask ordering применяются при нескольких ordered draws непосредственно в final cell. `SDL_Renderer` предоставляет Nearest/Linear/PixelArt (PixelArt с SDL3.4; release pin3.4.10), float destinations, tint, alpha blend и final-size targets; `SDL_GPU` добавляет nearest/linear min/mag и mipmaps. First-party Lanczos отсутствует. На момент исследования Q8.3 policy ещё не была принята; последующее решение записано ниже.

### Prepared assets and 1:1 composition — confirmed 2026-09-18

Пользователь ответил «такой вариант принимаю» на пересмотренный Q8.3a. Канонический renderer path использует assets, подготовленные под конечный размер клетки, и последующую композицию 1:1.

1. Source assets сохраняются в исходном разрешении. Mask keys/profile/subset semantics декодируются до фильтрации; authored base, recolorable foreground coverage, background holes и outline roles сохраняют порядок original single/two-mask composition из research report. Это не наложение необработанных mask colors поверх готового tile.
2. Из allocated map area, maximum fit и user fit zoom вычисляется общий конечный целочисленный размер клетки в output pixels. Все клетки используют одну геометрию; небольшой остаток площади превращается в центрирующие поля, zoom/resize визуально меняются ступенями. Normal/big viewport, glyph identity и semantic selection сохраняются. Точная rounding policy с учётом aspect ratio ещё требует формулировки; произвольное независимое растягивание X/Y не утверждено.
3. После стабилизации resize/zoom исходные decoded layers однократно масштабируются штатными средствами SDL3 непосредственно до final cell size и кешируются. TTF rasterize выполняется сразу под конечные метрики. Промежуточный font-sized raster с последующим постоянным scaling всей карты заменён этим решением.
4. В обычном кадре prepared layers компонуют карту 1:1, с текущими palette attrs и правильными terrain/foreground/subset/outline roles. Если используется map render target, он имеет final output size и также копируется 1:1.
5. Raster-cache generation учитывает asset/profile generation (включая mask definitions), subset, final size, filter и outline policy. Палитра применяется как runtime tint к coverage; её изменение не требует повторного масштабирования нейтральных слоёв. Новая готовая generation атомарно заменяет старую; resource budgets и конкретная организация кеша остаются acceptance/implementation decisions.
6. Во время live resize разрешено временно масштабировать предыдущие prepared assets аппаратным renderer, затем после короткого debounce подготовить новую generation и вернуться к 1:1. Это явное ограниченное исключение из steady-state single-scaling. Предыдущая generation означает assets, а не замороженный screenshot: текущие game updates, palette/effects и hit testing продолжают работать. Debounce откладывает дорогую подготовку, но не network/model updates или обязательный repaint; действуют budgets из [Classify display urgency and latency budgets](20-classify-display-urgency-and-latency-budgets.md).
7. Primary path может использовать аппаратный `SDL_Renderer`; raw `SDL_GPU` не требуется этим решением. Реальный backend и fallback отображаются в диагностике. Поддерживаемые software paths и их gates, точная debounce величина и ресурсные пределы ещё не выбраны. Преимущество по скорости не объявляется измеренным без runtime acceptance.

На момент принятия Q8.3a набор filters, судьба legacy Lanczos, PixelArt fallback и sampling coverage/outline оставались открытыми; ответы Q8.3b/c записаны ниже. Generated-outline geometry требует отдельного уточнения. Общего pixel-perfect контракта нет.

### SDL filters and mask sampling — confirmed 2026-09-18

Пользователь ответил «принимаю оба» на Q8.3b и Q8.3c.

- Q8.3b: новый клиент использует готовые SDL3 `Nearest / Linear / PixelArt`; самописный legacy Lanczos не переносится и не является обязательной capability finished client. Если PixelArt недоступен на выбранном renderer/preparation path, применяется явный Nearest fallback, отражённый в settings/diagnostics. Запрошенный и фактический режим различаются; выдавать замещающий режим за PixelArt нельзя. Concrete defaults/settings registry и преобразование imported legacy Lanczos остаются у [Specify persistence ownership and UI configuration schema](25-specify-persistence-ownership-and-ui-configuration-schema.md).
- Q8.3c: исходные mask keys декодируются до любого non-nearest sampling. Decoded foreground/recolor coverage и authored/generated outline coverage масштабируются через Nearest; обычные authored-color части tile используют выбранный SDL filter. Сглаживание границ coverage этим решением не вводится. Общая final-cell geometry, original single/two-mask composition order, live palette tint и изоляция соседних tiles сохраняются.
- Эти filters применяются на стадии подготовки final-size assets из Q8.3a, обычный вывод остаётся 1:1. Решение не требует поддержки PixelArt любым CPU surface scaler: используется реально поддерживаемый штатный SDL path либо согласованный явный Nearest fallback.

Остаются отдельными вопросами PCF bitmap sampling, filter для raw pictures, generated-outline geometry, целочисленное округление aspect ratio, UI/accessibility scaling и concrete defect dispositions/timed-effect policy. Принятие двух filter-вопросов не закрывает весь тикет.

### Cell rounding, raw pictures and generated outlines — confirmed 2026-09-18

Пользователь принял Q8.4, Q8.6 и Q8.7; Q8.5 (PCF) оставлен для исследования.

- Q8.4: допустимо небольшое отклонение aspect ratio, возникающее при округлении идеального fitted размера клетки до целых output pixels. Все клетки, слои и hit testing используют одинаковую итоговую геометрию; сетка полностью помещается и центрируется. Сохранение точного рационального aspect ratio ценой крупных ступеней размера не требуется.
- Q8.6: raw pictures масштабируются выбранным filter графики непосредственно из исходного source rectangle под конечный размер и затем выводятся 1:1. Их многоклеточные anchors/extents и special-canvas transform сохраняются; normal tile-mask decoding не навязывается этому отдельному пути.
- Q8.7: authored outline из tileset масштабируется Nearest. Generated outline рассчитывается при подготовке конечного размера с учётом настроенной толщины и индивидуальных tile bounds, сохраняя текущую семантику генерации. Готовый generated outline не проходит дополнительный steady-state scaling.
- Q8.5: обязательный Nearest для PCF пока не принят. Возможности SDL3, SDL_ttf/FreeType и применимые bitmap-font practices исследуются в [Research PCF loading and scaling options](34-research-pcf-loading-and-scaling-options.md).

### PCF capabilities researched — Q8.5 remains open

[PCF loading and scaling options](../research/pcf-loading-and-scaling-options.md) подтверждает загрузку шести representative PCF через SDL_ttf/FreeType, native bitmap size независимо от requested point size и сохранение encoded ID lookup в исследованной версии. SDL_ttf public glyph API описан как Unicode: использование для arbitrary custom PCF maps требует явного adapter/regression contract; direct FreeType предоставляет управление charmap и metadata. Выбор loader пока не сделан.

Decoded PCF coverage можно масштабировать Nearest, Linear либо реально поддерживаемым renderer PixelArt; принятый Nearest для tile recolor/outline coverage не запрещает отдельную font sampling policy. Для PCF применима уже принятая схема source glyph → final-size cache → 1:1. CPU `SDL_ScaleSurface` не доказывает PixelArt support: исследованный SDL surface path заменяет этот режим на Nearest. Предложение отдельного PCF filter с Nearest как возможным default остаётся предложением, не принятым решением. Самовольная замена выбранного PCF другим размером/семейством/TTF не предполагается.

### Independent PCF sampling — confirmed 2026-09-18

Пользователь ответил «принимаю» на уточнённый Q8.5. Этот раздел заменяет pending status PCF sampling выше; loader choice этим ответом не утверждён.

- PCF имеет отдельный от tiles filter: Nearest / Linear / PixelArt. Default — Nearest; Nearest-only restriction не вводится.
- PixelArt используется только при фактической поддержке выбранным preparation/rendering path; иначе применяется согласованный явный Nearest fallback. Requested и effective mode различаются и отображаются в settings/diagnostics, включая CPU surface preparation.
- Исходные glyph bitmaps сохраняются; при изменении конечного размера/filter готовится final-size cache, обычный вывод — 1:1. Сглаженная PCF coverage разрешена выбранным filter и не меняет Nearest policy для tile recolor/outline coverage. Identity, metrics, клеточная geometry и защита от соседнего glyph bleed сохраняются.
- Выбранный PCF не заменяется автоматически другим размером/семейством/TTF. Default/filter registry, storage и import относятся к persistence ticket; PCF default Nearest уже согласован здесь и не является открытым default choice.

Тикет остаётся claimed: UI/accessibility scaling, конкретный PCF loader contract, custom-asset failure details и timed-effect/defect dispositions ещё не согласованы полностью.

### Next decision round — proposed, not confirmed

- Q8.5a — PCF decoder: предложен FreeType напрямую для explicit encoded-ID/charmap/metrics/default handling вместо переноса самописного parser; SDL_ttf сохраняется для TTF. Filter policy не меняется. Пользователь пока не выбрал decoder.
- Q8.8 — user UI scale: предложено увеличивать UI text вместе с соответствующими controls/spacing и пересчитывать layout, не растягивать screenshot. Карта сохраняет независимый zoom и полный viewport, но доступная map area может измениться. Диапазон и поведение при нехватке места зависят от этого решения и будут уточняться затем.
- Q8.9 — accessibility text scale: предложен единый внутриклиентский UI control на Linux/Windows поверх обязательного OS display-scale handling; автоматическое чтение отдельной системной настройки text size не делать обязательным. Это не отказ от DPI и не утверждение equivalence системным accessibility settings.
- Q9.1 — custom-resource failure: предложено до активации проверять новый resource/profile/cache; неудачная замена сохраняет предыдущий рабочий набор и сообщает причину. Если на старте рабочего набора нет, предложен явный выбор исправленного либо bundled ресурса, без молчаливой подмены. Missing glyph policy из encoding ticket сохраняется отдельно; одиночная отсутствующая glyph не приравнивается к structural load failure.
- Q10 — timed effects: предложено сохранять виды эффектов, server parameters, palette/color roles, порядок start/stop/restore, baseline timing rules и пользовательские toggles. Случайное расположение частиц/мерцание не обязаны повторять legacy frames; время/RNG фиксируются для проверяемых scenarios, не вводя pixel equality. Конкретные defect dispositions уточняются после выбора этой границы.

### PCF loader, UI scaling and resource failures — confirmed 2026-09-18

Пользователь принял Q8.5a, Q8.8, Q8.9 и Q9.1 с явной поправкой startup behavior; Q10 запросил пояснить и пока не принял.

- Q8.5a: PCF декодируется через FreeType напрямую, с явной обработкой encoded IDs, charmap, metrics и default glyph. Самописный legacy PCF parser не переносится; SDL_ttf остаётся для TTF. Принятые glyph identity/fallback contracts и PCF sampling не меняются.
- Q8.8: user UI scale увеличивает текст вместе с соответствующими controls, rows и spacing; layout пересчитывается, готовый UI screenshot не растягивается. Map zoom остаётся независимым. Увеличенные панели могут уменьшить доступную map area, в которую вписывается прежний полный viewport. Диапазон UI scale и правила при нехватке места ещё требуют уточнения.
- Q8.9: собственный единый регулятор UI scale применяется на Linux/Windows поверх обязательного OS display-scale/DPI handling. Автоматическое чтение отдельной системной настройки text size не является обязательной capability; DPI handling этим не отключается.
- Q9.1, live change: до активации проверяются resource/profile и необходимая подготовка; неудачная смена сохраняет предыдущий рабочий набор и сообщает причину. Отдельные missing glyphs обрабатываются по уже принятому glyph fallback, а не автоматически считаются полной ошибкой загрузки.
- Q9.1, startup correction: при ошибке загрузки выбранных ресурсов на запуске показывается ошибка и автоматически применяется fallback; диалог выбора другого ресурса не предлагается. Это явное исключение из запрета автоматической замены выбранного font/resource, а не разрешение подменять успешно загруженный PCF ради внешнего вида или масштаба. Конкретный fallback resource/profile, цепочка при его недоступности и сохранение requested/effective выбора относятся к дальнейшему persistence/failure contract и здесь не выбраны.

### Q10 clarification — awaiting user answer

«После исчезновения эффекта показывать актуальную карту» означает убрать временный слой и показать состояние клетки по последним уже полученным и применённым игровым данным. Например, частица снега закрыла монстра; пока она была видима, клиент получил его перемещение. После удаления частицы в прежней клетке должны остаться текущий terrain/другие актуальные слои, а не сохранённая до снега картинка монстра. Если клетка не менялась, её прежний вид корректен. Это не запрос дополнительной карты с сервера, не раскрытие неизвестных клиенту данных и не требование полного repaint всей карты каждый кадр. Уточнение согласуется с принятым map visual stack; вся Q10 timed-effect policy остаётся открытой.

### Timed effects and current-state restoration — confirmed 2026-09-18

Пользователь ответил «10 - ок» после пояснения. Q10 принят: сохраняются виды эффектов, server parameters, palette/color roles, baseline timing rules, start/stop/restore ordering и пользовательские toggles. Точное совпадение случайных частиц и кадров мерцания с legacy не требуется; controlled time/RNG используется для проверяемых scenarios, а не pixel-equality gates.

После удаления временного слоя композиция использует последние полученные и применённые данные клетки, включая terrain, foreground и другие ещё активные слои. Изменения под эффектом не откатываются сохранённым raster snapshot. Это не требует дополнительного server request, не раскрывает неизвестное состояние и не навязывает full-map repaint на каждом кадре. Clarification выше больше не pending.

### Remaining scope and visual quirk — awaiting user answer

UI overflow rules уже заданы [Specify surface layouts and responsive rules](21-specify-surface-layouts-and-responsive-rules.md): primary/form text не auto-shrink, соответствующие content regions scroll, mandatory controls visible; отдельно согласованные panel/canvas fit exceptions сохраняются. Q8.8 не заменяет эти правила растягиванием готового framebuffer. Конкретные supported UI scale range/steps/default и проверка сочетаний с minimum window/font/profile остаются у persistence/acceptance owners; новые численные пределы здесь не принимаются.

Q11: [Renderer parity inventory](../research/renderer-parity.md#4-palettes-animations-и-lighting) фиксирует повтор рисунка searchlight в нижней половине big map. Предлагается не сохранять этот повтор как обязательную совместимость: применять эффект к полной сетке карты без специального дублирования её половин, сохраняя смысл, цвета и timing effects. Это предложение о конкретном legacy quirk, а не автоматически одобренное исправление; ожидается ответ пользователя. Остальные выявленные unsafe/lossy/stale/misreported defects подчиняются принятому Q3 и encoding/failure contracts, а точные regression fixtures принадлежат acceptance.

### Searchlight quirk disposition — confirmed 2026-09-18

После пояснения пользователь ответил «ок» и принял Q11. Сам эффект TERM_SRCLITE сохраняется; legacy workaround, повторяющий рисунок в нижней половине big map, не переносится. Эффект рассчитывается для полной map geometry с сохранением смысла, цветов и временного поведения, без специального дублирования половин. Это не удаление lighting capability и не изменение обычного torch lighting.

Уточняющее evidence: [client colour animation](../../../src/client/z-term.c:1008) задаёт два вращающихся red/orange/yellow луча на white base. [Feature185](../../../lib/game/f_info.txt:1300) и [special mirror-arena generation](../../../src/server/generate.c:8508) подтверждают реальное применение к особым permanent walls, включая pillars. Пометка testing в resource comment не означает отсутствия gameplay usage.

### Final shared-understanding confirmation — pending

Все заданные вопросы этого раунда приняты. Перед резолюцией требуется финальное подтверждение целостного контракта: semantic/behavior compatibility без pixel-perfect; исходные слои → final-size assets/cache → 1:1 composition с ограниченным live-resize exception; полный fitted viewport и независимый UI scale; SDL filters и отдельный PCF filter/FreeType loader; custom assets и startup/live failure distinction; сохранение effects с current-state restoration и согласованным searchlight correction.

Остающиеся отдельные владельцы: [cursor prototype](31-prototype-cursor-and-cell-indicators.md) — visual design/indicator strokes; [persistence schema](25-specify-persistence-ownership-and-ui-configuration-schema.md) — concrete bundled TTF/profile, UI/zoom ranges/steps/defaults кроме уже принятого PCF Nearest, fallback resources/chain и storage/import; [acceptance](09-design-parity-evidence-and-acceptance.md) — full corpus/evidence, supported backend/software paths, legibility/resource limits, debounce и измерения. Они не считаются согласованными этим тикетом и не препятствуют фиксации его собственной policy. Implementation не выполнялась; status остаётся claimed до подтверждения итогового понимания.

## Answer

Уточнение пользователя 2026-09-19 в [Specify persistence ownership and UI configuration schema](25-specify-persistence-ownership-and-ui-configuration-schema.md#comments): карта всегда использует 100% maximum fit, пользовательский map fit zoom отменён. Все нижеследующие упоминания регулируемого zoom и его хранения заменены этим решением; остальные geometry/raster invariants сохраняются.

Пользователь подтвердил итоговое общее понимание и закрытие тикета ответом «верно» 2026-09-18. Ниже — каноническая резолюция. Противоречащие ей ранние предложения и pending statuses в Comments являются историей обсуждения, не действующим контрактом. Реализация клиента и полная runtime acceptance не выполнялись.

### Compatibility и дефекты

Новый SDL3 pipeline является canonical raster pipeline. Legacy SDL3/X11 нормативны для semantic identity и behavior, но не для точного совпадения pixels. Pixel-perfect полностью исключён, включая закреплённые environments. Проверяются glyph/tile identity, geometry, color roles, layer order, visibility, lifecycle, presence/footprint/clipping/legibility/transparency и отсутствие bleed/stale output. Captures используются для диагностики и visual review, не pixel-equality gates.

Unsafe/lossy/misindexed/stale/misreported legacy defects исправляются; сохранение quirks требует доказанной зависимости и regression evidence. Encoding defects и glyph fallback следуют [Define encoding and glyph identity](22-define-encoding-and-glyph-identity.md), а не воспроизводят ошибочные PCF origin/default/bounds или TTF raw-byte decoding. Курсор получает новый дизайн через [Prototype cursor and cell indicators](31-prototype-cursor-and-cell-indicators.md), не обязан повторять X11 XOR или SDL3 alpha overlay.

### Координаты, UI и карта

- Layout/hit testing используют floating-point logical UI units; актуальные window/output-pixel sizes и OS display scale задают преобразование. DPI/monitor changes пересчитывают layout/raster без утраты session/interaction state. Fixed whole-window framebuffer с постоянным растягиванием не является моделью клиента.
- User UI scale увеличивает text/controls/rows/spacing с relayout независимо от map fit zoom. Обязательный OS DPI handling сохраняется. Отдельная системная accessibility text-size setting не обязана автоматически считываться; собственный UI control одинаков по смыслу на Linux/Windows.
- Уже согласованные [surface layout rules](21-specify-surface-layouts-and-responsive-rules.md) сохраняются: primary/form text не auto-shrink, соответствующий content scrolls; отдельно принятые panels/canvases fit. Увеличение UI может уменьшать map area, но не менять selected viewport или map-font definitions.
- Map font/profile задаёт базовые пропорции клетки. Полная сетка 66×22 либо 66×44 вписывается и центрируется без crop/scroll; fit может увеличивать или уменьшать исходные assets. User map fit zoom до 100% maximum fit только уменьшает карту относительно максимального вписывания, не меняя server dimensions.
- Final cell size — общий для всей сетки целочисленный размер в output pixels. Небольшое отклонение aspect ratio от округления допустимо; точный рациональный ratio ценой крупных ступеней не требуется. Остаток площади даёт центрирующие поля. Все слои и hit testing используют ту же geometry.

### Подготовка assets и композиция

1. Source assets сохраняются в исходном разрешении. Encoded mask keys декодируются до фильтрации; authored base, recolor coverage, background holes, outline roles и независимые subsets сохраняют реальную семантику [legacy single/two-mask composition](../research/sdl3-tile-composition-and-scaling-apis.md). Это не произвольное наложение необработанных masks поверх готового tile.
2. Decoded layers готовятся непосредственно под final cell size и кешируются. TTF rasterize выполняется сразу под конечные метрики. Промежуточный font-sized raster с последующим постоянным scaling всей карты не применяется.
3. Steady-state layers рисуются 1:1. Optional map target имеет конечный output size и также копируется 1:1. Terrain и foreground сохраняют независимые IDs/attrs; semantic stack: background/color role → terrain → foreground game visual → effects/weather → indicators/cursor. Palette применяется runtime tint, без пересборки нейтральных layers при смене цвета.
4. Cache identity учитывает source/profile generation, mask definitions, subset, final size, filter и outline policy. Готовая generation заменяется атомарно; конкретная cache organization и бюджеты здесь не выбраны.
5. При live resize допустимо временное аппаратное масштабирование предыдущих prepared assets; после короткого debounce готовится новая generation и возвращается вывод 1:1. Это ограниченное исключение, не постоянный double scaling. Game/model updates, palette/effects, hit testing и обязательные repaint не замораживаются и соблюдают [latency budgets](20-classify-display-urgency-and-latency-budgets.md).
6. Аппаратный SDL_Renderer допустим; raw SDL_GPU не обязателен. Backend/fallback виден в диагностике. Быстродействие не считается доказанным наличием cache; supported software paths, debounce и resource limits требуют acceptance решения.

### Filters и fonts

| Asset role | Принятая policy |
| --- | --- |
| Authored-color tile regions | Готовые SDL Nearest / Linear / PixelArt |
| Decoded tile recolor coverage | Nearest; encoded mask colors нельзя интерполировать до decoding |
| Authored outline | Nearest |
| Generated outline | Рассчитывается при подготовке final size с настроенной толщиной и индивидуальными tile bounds; затем без дополнительного steady-state scaling |
| Raw pictures | Выбранный graphics filter: исходный rectangle → final size → 1:1; anchors/extents и special-canvas transform сохраняются, обычный tile-mask pipeline не навязывается |
| PCF | FreeType напрямую для encoded-ID/charmap/metrics/default handling; отдельный от tiles filter Nearest / Linear / PixelArt, default Nearest; source glyph → final-size cache → 1:1 |
| TTF | SDL_ttf, rasterize под конечные метрики; как минимум один TTF поставляется вместе с клиентом |

Legacy custom Lanczos не переносится и не является обязательной capability. Если PixelArt реально недоступен на renderer/preparation path, применяется явный Nearest fallback; requested и effective различаются в settings/diagnostics. Само наличие enum или успешный SDL_ScaleSurface call не доказывают PixelArt support: проверенный CPU surface path заменяет его на Nearest. См. [PCF loading and scaling options](../research/pcf-loading-and-scaling-options.md).

Сглаженная PCF coverage разрешена его выбранным filter и не меняет Nearest policy для tile recolor/outline. Успешно загруженный PCF не заменяется другим размером/семейством/TTF ради вида или масштаба. Glyph identities, metrics, общий cell transform и изоляция соседних glyphs/tiles сохраняются. Конкретный bundled TTF/profile выбирается отдельно на основе [candidate research](../research/bundled-ttf-candidates.md).

### Пользовательские assets и failures

Собственные fonts и tilesets обязательны, включая совместимые glyph/profile/mask/subset associations и mixed tile/glyph rendering. До активации проверяется загрузка и необходимая подготовка. Ошибка live change оставляет предыдущий рабочий набор и сообщает причину; missing glyph отдельно использует уже согласованный font-default/visible one-cell fallback, не приравнивается автоматически к отказу всего ресурса.

При startup load failure показывается ошибка и автоматически применяется fallback без resource-picker dialog. Это явное исключение из запрета автоматической замены выбранного ресурса. Конкретный резервный набор, последующие отказы и persistence requested/effective selection остаются у settings owner, а не считаются молча выбранными.

### Эффекты

Для UI recreation transient special-store animation применять последующее исключение из [Define bounded working retention and optional capture](26-define-bounded-working-retention-and-optional-capture.md#answer); остальные effect lifecycle/timing правила ниже сохраняются.

Сохраняются виды эффектов, server parameters, palette/color roles, baseline timing rules, start/stop/restore и toggles. Совпадение случайных частиц и кадров мерцания с legacy не требуется. После исчезновения временного слоя показывается клетка по последним уже полученным и применённым данным, включая изменения под эффектом и другие активные слои; старый raster snapshot не откатывает эти изменения. Дополнительный server request и full-map repaint на каждом кадре не обязательны.

TERM_SRCLITE сохраняется как цветовая анимация специальных стен/столбов; его расчёт охватывает полную map geometry без legacy повторения рисунка в нижней половине big map. Смысл, цвета и timing сохраняются. Это не изменение обычного torch lighting.

### Coverage и владельцы дальнейших решений

Все bundled assets проходят basic discovery/load/metadata/range/default/failure validation. Глубокие semantic/lifecycle/composition scenarios используют representative corpus: ordinary 9x15.pcf, full byte-ID 9x15tg, mixed 16x24tg + 16x24sv, origin fixtures 12x24/8x16 и partial-range 16x22. Проверяются numeric IDs, PRF/graphics overrides, masks/subsets, ASCII weather/raw pictures, missing/default, font switching и отдельно text-only changes. Shipped profiles обязаны покрыть применимый полный baseline glyph corpus; replacements не доказывают parity. Controlled time/RNG делает effect scenarios проверяемыми без pixel-perfect contract.

- [Prototype cursor and cell indicators](31-prototype-cursor-and-cell-indicators.md): конкретный cursor/indicator design, strokes и contrast/scale variants.
- [Specify persistence ownership and UI configuration schema](25-specify-persistence-ownership-and-ui-configuration-schema.md): конкретный bundled TTF/profile, UI/zoom ranges/steps/defaults, graphics default, resource fallback chain, storage/import и conversion legacy Lanczos без порчи общих legacy settings. PCF default Nearest уже принят здесь.
- [Design parity evidence and acceptance](09-design-parity-evidence-and-acceptance.md): exact environments/full corpus, supported accelerated/software paths, memory/render/legibility limits, debounce, lifecycle/failure scenarios и реальные измерения на обеих платформах.

Исследования и bounded Linux PCF smoke — evidence отдельных фактов, не доказательство finished-client parity, Windows dependency closure или performance. Закрытие этого decision ticket не закрывает перечисленные tickets и не разрешает реализацию в рамках planning-карты.
