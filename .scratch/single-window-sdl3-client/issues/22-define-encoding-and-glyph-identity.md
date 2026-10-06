# Define encoding and glyph identity

Type: grilling
Status: resolved
Assignee: codex
Blocked by: 13

## Question

Учитывать input/clipboard facts из [Platform deltas and packaging: статический аудит](../research/platform-deltas-and-packaging.md): текущий SDL3 key-down путь не реализует полноценные text commit/preedit/IME events; SDL clipboard API также не доказывает Unicode end-to-end. Разделить baseline command/macro byte semantics, text entry/clipboard conversion и renderer glyph identity; точные input routes согласуются в [Preserve input and macro semantics](04-preserve-input-and-macro-semantics.md).

Как finished single-window client сохраняет legacy unsigned-byte glyph identity и lossless formatted bytes, преобразуя их для Unicode UI и PCF/TTF rendering? Определить контракты отдельно для wire/formatted text, локального UI text и glyph/tile IDs; отображение control/high bytes и solid-wall символов, invalid или неполного text, unknown/truncated color markers, длину и координаты formatted content, readable XHTML projection. Не предполагать, что `char32_t` означает Unicode или что все исторические bytes относятся к одной кодировке.

Разобрать обнаруженное расхождение: X11/PCF и Term text ориентированы на bytes, но SDL3 TTF передаёт исходную строку в UTF-8 API. Какие результаты являются обязательной совместимостью, а какие исправлением ошибочного decoding/mapping, и какое evidence доказывает сохранность игровых данных и пользовательских ресурсов? Не выбирать C API или реализовывать conversion. Общие raster tolerances и остальные legacy defects относятся к [Choose raster references and defect compatibility](23-choose-raster-references-and-defect-compatibility.md).

Источник: [Specify renderer parity](13-specify-renderer-parity.md) и [Renderer parity inventory](../research/renderer-parity.md), разделы encoding, PCF/TTF и XHTML.


## Comments

### Round 1 — confirmed by user

Пользователь ответил «все принимаю» на Q1–Q3.

- Q1: разделены lossless wire/legacy formatted bytes с markers, собственный Unicode UI text и исходные игровые glyph/tile IDs, не считающиеся автоматически Unicode. Display projection отдельно от исходных данных; ID 200 сохраняет identity независимо от Unicode/bitmap representation.
- Q2: server documents и special-store canvas сохраняют исходную logical grid: один legacy byte-symbol занимает одну клетку; supported control markers имеют собственную семантику. Unicode conversion не сдвигает server coordinates. Собственный UI использует обычную text layout.
- Q3: accidental TTF UTF-8 decoding legacy bytes исправляется с сохранением исходных bytes, IDs и user font mappings; искажённый результат не является compatibility oracle. Конкретные encoding tables и missing-glyph fallback ещё не согласованы; единой кодировки всех legacy sources не предполагается.

Тикет остаётся claimed. Следующий frontier требует source facts о mappings/font encoding/input conversion, затем выбора encoding/fallback и malformed content/input contracts.


### Source facts for Round 2

Bounded primary-source investigation: default SDL3 font 9x15 (src/config.h:870), lib/xtra/font/9x15.pcf declares ISO8859-1 in PCF properties; custom TG/BDF font assets do not universally declare Unicode correspondence. Это evidence конкретного font repertoire, не всей wire text encoding.

PCF table loader reads origin/default at src/client/main-sdl3.c:5632–5662, while lookup :5905 directly indexes byte; default_char is assigned as glyph index without encoded-character lookup. Nonzero origin and encoded-default cases require defect disposition. TTF path :854 has no explicit legacy-byte conversion.

SDL copy/paste src/client/c-util.c:2337–2378/:2506–2557 has no declared charset conversion; signed-char control filtering may discard high bytes, byte truncation may split UTF-8. These are defects/limitations, not agreed Unicode behavior. KEY_DOWN route does not implement full text commit/preedit handling.

Term_putstr src/client/z-term.c:3350–3397 consumes 0xff introducer; unknown following code is visible in current color, doubled 0xff prints '{', '-', '.', '%' have explicit semantics. Lone trailing 0xff ordinarily emits no glyph, but NULL pointer subtraction is undefined behavior; malformed tails need deterministic handling. Copy stripping currently differs from display parsing for unknown pairs.

Product defaults/tables, missing glyph, malformed input/export and text entry policies remain unapproved.


### Round 2 — user requested factual clarification

Пользователь не принял Q4–Q7: спросил, проверен ли только один font, на что влияет encoding и влияет ли на server interaction. Предложенный ISO-8859-1 default не утверждён. Первоначальная выборка включала standard 9x15 PCF metadata и несколько custom assets, не полный font inventory; запущены отдельные factual audits всего bundled font набора и server text/glyph routes.

Уточнение границ: font glyph lookup влияет на рисунок исходного ID; received-text display conversion не должна менять wire bytes; outbound Unicode-to-byte conversion меняет server-visible payload и byte length и поэтому требует field-specific protocol/validation evidence. Выбор font не должен неявно переключать encoding chat/commands/credentials. Утверждённые Q1–Q3 сохраняются; downstream defaults/input decisions ждут фактов и пользователя.


### Expanded clarification evidence

- [Bundled font encoding metadata audit](../research/encoding-font-assets.md): все 100 bundled assets (50 PCF, 50 FON) и все 32 дополнительные BDF; 24 PCF объявляют ISO8859/1, 26 не объявляют charset. Metadata не доказывает фактическое Unicode correspondence custom drawings и не задаёт wire text encoding. Найдены реальные nonzero/partial PCF ranges, требующие корректного lookup.
- [Encoding and the server contract](../research/encoding-server-contract.md): wire strings передаются как NUL-terminated bytes без charset negotiation; capacities byte-based. Outbound conversion может менять chat/slash arguments, names/password verification и request lengths. Account-name alphabet ограничен отдельно; password и character-name contracts не следует приравнивать к нему.
- Числовые glyph redefinitions действительно отправляются server setup: менять исходный glyph ID при Unicode conversion нельзя; drawing для сохранённого ID является local representation. Font selection не определяет автоматически outgoing text encoding.

Q4 ISO-8859-1 default остаётся предложением и не утверждён. Q5–Q7 также пока не приняты. Исследования — evidence для живого обсуждения, не новая policy или runtime parity claim.


### Revised Q4/Q7 scope — confirmed by user

Пользователь ответил «да, рекомендованный ответ»: для outgoing text сохраняется существующий byte contract без расширения repertoire; Unicode используется для собственного UI и display projection. Возможное расширение ввода — отдельное решение только при доказанной совместимости конкретных полей. Универсальный ISO-8859-1 default снят, не утверждён. Font selection не переключает outgoing charset, glyph IDs не меняются из-за Unicode conversion.

Этот ответ заменяет прежнее предложение общего default и не подтверждает прежние Q5/Q6 или полноценный Unicode editor для всех server-bound полей. Остаточный frontier: per-source display mapping/fallback, PCF/default lookup, malformed markers/export, unsupported text-input/clipboard conversion и byte-length boundaries без изменения существующего wire repertoire.


### Remaining Round — Q5 comparison requested

Пользователь спросил, как Q5 реализован в текущем client и как recommendation с ним соотносится. Q5–Q7 не утверждены. Сопоставление с SDL3 должно отделить существующий direct byte PCF glyph lookup и solid/tile special paths от исправления PCF origin/default bugs и нового explicit TTF mapping/bitmap fallback; такой гибридный fallback сейчас не является реализованной baseline capability.


### User steering — visual model must preserve baseline

Пользователь подчеркнул критичность fonts/text, font-based map/monsters/items и tiles для визуала: finished client должен работать как минимум как оригинальный, желательно с независимым выбором font для text surfaces и map glyphs при одновременно включённых tiles. Это требует рассмотреть visual roles/configuration и mixed glyph/tile rendering целиком; Q5 не сводится к исправлению PCF lookup. Автоматический hybrid fallback и generic encoding default не утверждены.

Применяется codebase-design; factual investigation существующего font/map/tile composition в SDL3/X11 создаёт отдельный linked report. Text/map font role split рассматривается как явное пользовательское направление. Детальные scopes/fallback/cell metrics и их связи с layouts/persistence требуют live решения. Numeric resource budgets и raster oracle остаются в существующих tickets/fog; implementation не разрешена этой planning картой.


### Visual-model evidence and proposed decision frontier

[Font, map glyph and tiles contracts](../research/font-map-and-tiles-contract.md) подтверждает per-Term independence, mixed byte glyphs и tiles через higher_pict, font-sized tile geometry, weather/cursor/raw-picture зависимости и main-font visual preference lifecycle. В main Term font карты и surrounding text общий; separate semantic text/map roles в одном SDL_Window — новое согласуемое UX решение.

Важное уточнение предыдущего тезиса identity: Unicode display conversion не меняет полученный game ID; explicit map-font/profile switch может штатно загрузить другие F/R/K/U visual definitions, затем graphics overrides и уведомить server через существующий setup/Send_font route. Эти действия не являются сменой encoding chat/commands/passwords. Новый UI сохраняет этот core-owned contract, не запускает его при изменении text-only font.

Предлагаемый frontier для живого согласования: два независимых font roles (text surfaces и map glyphs), существующий mixed font/tiles path в единой map grid, map-font-dependent cell/tile geometry как baseline default и визуальные definitions/font-switch ordering. Missing-glyph и malformed/input/export вопросы ещё не решены; код не создаётся.


### Visual-role round — confirmed by user

Пользователь ответил «принимаю рекомендованные варианты» на Q8–Q10.

- Q8: text font (HUD/messages/lists/dialogs/documents), map font (terrain/monsters/items/player/font-based cell effects) и tileset/graphics mode — independent semantic settings в одном SDL_Window. Text-font changes не меняют map cell metrics или game visual mappings. Server documents/special-store content сохраняют собственные logical coordinates; own UI text layout отдельно.
- Q9: baseline map grid определяется map font и его масштабом; tiles масштабируются в ту же клетку. Glyphs/tiles/cursor/weather имеют согласованную геометрию. Text UI scale независим от map scale. Graphics enabled не выключает font paths; glyph/tile representation определяется visual definitions.
- Q10: font-specific visual definition lifecycle привязан к map font: font preferences, затем graphics overrides и baseline server setup/font notifications. Text-only font change не запускает этот цикл. Unicode display conversion сохраняет received IDs; configured map-font switch может штатно менять visual definitions.

Остаточные вопросы: glyph lookup/TTF mappings/missing fallback, malformed markers/copy/XHTML, unsupported input и byte lengths. Role defaults/file registry относятся к persistence ticket, DPI/layout и viewport negotiations — к layout ticket, raster oracles — к raster ticket. Итоговое общее понимание тикета ещё не подтверждено.


### Remaining visual/input round — partial confirmation

Пользователь ответил «11, 12 - ок, 13 - подробнее».

- Q11: отдельные PCF/TTF для text/map font; PCF encoded ID lookup с корректными origin/default. TTF Unicode own UI напрямую; legacy byte symbols через explicit visual-profile mapping, printable ASCII → ASCII, прочие IDs требуют заданного соответствия. Missing glyph: корректный font default, затем видимый one-cell replacement с original ID retained. Custom bitmap fonts сохраняются PCF path; automatic TTF→bitmap fallback не вводится. Shipped profiles обязаны пройти full baseline glyph corpus; replacement не доказывает parity.
- Q12: supported marker semantics; unknown 0xff+code оставляет code в текущем цвете, trailing lone marker safely emits no glyph, raw bytes сохраняются. Copy parsed projection; readable XHTML baseline substitutions и '~' при отсутствии доказанного correspondence; native screenshot actual composed visual.
- Q13: пользователь запросил подробности; decision pending. Требуется объяснить text commit vs physical gesture/macro bytes, field-specific admissibility/conversion, clipboard path, draft-preserving encoding errors отдельно от утверждённой chat-expansion truncation.


### Q13 expanded contract — confirmed by user

Пользователь ответил «принимаю рекомендованный» на подробное описание Q13. Physical game gestures и legacy macro bytes сохраняют context/queue semantics, не декодируются как UTF-8. Active text editor получает text commits отдельно от navigation/Enter/Esc actions без двойного исполнения gesture и вставки. До передачи ядру применяются точное field-specific Unicode↔byte correspondence, allowed bytes, byte capacities и штатные escaping/substitutions. Font settings не выбирают outgoing charset.

Непредставимый символ не отправляется как guessed UTF-8, не удаляется молча; ошибка объясняется, draft сохраняется. Это не blanket ASCII restriction: разрешённые исходным field contract другие bytes поддерживаются при заданном точном преобразовании. Credentials не нормализуются/перекодируются по font; imported bytes сохраняются точно. Clipboard использует ту же representability/byte-limit проверку с сохранением baseline paste escaping/newline handling. Approved safe chat-expansion truncation/empty input не пересматриваются; encoding error отдельно.

## Answer

Пользователь подтвердил итоговое общее понимание и закрытие тикета 2026-09-15 ответом «принимаю». Ниже каноническая согласованная резолюция: Q1–Q3, revised outgoing-text contract, Q8–Q10 и Q11–Q13. Прежний universal ISO-8859-1 default и automatic TTF→bitmap fallback не приняты. Implementation и runtime acceptance не выполнялись.

### Evidence и обязательная база

Визуал finished client должен сохранять как минимум информацию/поведение оригинальных SDL3 и применимых X11 paths: font-only map, mixed font/tiles, layered terrain/actor composition, custom glyphs, palette/animated attrs, cursor/weather/raw pictures, font reload и font-specific visual preferences. Ни metadata font, ни удачный ASCII screenshot не доказывают этой полноты.

Source reports: [Renderer parity inventory](../research/renderer-parity.md), [all bundled font assets](../research/encoding-font-assets.md), [server text/glyph contract](../research/encoding-server-contract.md), [font/map/tiles lifecycle](../research/font-map-and-tiles-contract.md). Аудит всех 100 bundled assets и 32 BDF подтверждает отсутствие единого declared charset. Default 9x15 ISO8859-1 — repertoire конкретного font, не universal network text contract. Runtime/raster acceptance не выполнялась.

### Три разных представления

| Данные | Нормативный контракт |
|---|---|
| Wire/legacy formatted text | Исходные bytes/markers и lossless ownership сохраняются; display projection отдельно |
| Собственные UI labels/text | Unicode representation; не устанавливает Unicode repertoire server-bound fields |
| Game glyph/tile IDs | Numeric identity, attrs, layers и coordinates сохраняются независимо от Unicode scalar representation |

Unsigned byte-symbol legacy text занимает одну logical cell. Supported formatting markers имеют отдельные semantics/visible length; Unicode projection не меняет server document/store coordinates. Byte capacities wire/input files отдельно от cells и Unicode text length. Char32 storage/transfer width не означает Unicode text protocol.

Successful complete decode предшествует model apply; partial packets остаются transport-owned без partial model mutation. Lossless unknown formatted bytes не считаются invalid UTF-8 по умолчанию и не выбрасываются. Unicode decoding применяется только к явно объявленным Unicode sources/profile correspondence, без charset угадывания. Исходный content удерживается на owning snapshot/event lifetime согласно state/retention contracts.

### Independent visual roles

| Setting | Scope | Что не затрагивается изменением |
|---|---|---|
| Text font и text UI scale | HUD/messages/lists/dialogs/documents | Map grid, map-font visual definitions и outgoing text encoding |
| Map font и map scale | Terrain/monsters/items/player/font cell effects | Text-font selection и outgoing text encoding |
| Tileset / graphics mode | Tile representations по visual mappings | Не отключает byte-glyph font paths |

Все roles живут внутри одного SDL_Window; per-Term/virtual-Term модель не переносится. Existing PCF и TTF modes сохраняются независимо для text и map roles. Server documents/special-store canvases имеют свои logical grids, отдельные от карты; own UI допускает обычную text layout.

Map cell geometry задаётся map font и его масштабом как baseline. Tiles масштабируются в ту же клетку. Glyphs/tiles/cursor/weather используют согласованную geometry; raw pictures сохраняют свои anchors/extents. UI text scale отдельно от map scale. Monitor DPI/viewport/resize и concrete surface layout задаются layout ticket; отдельный font-independent tile zoom этим решением не введён.

Graphics enabled сохраняет mixed rendering: representation определяется visual definitions, не правилом «все monsters/items обязательно tiles». Byte glyphs и ASCII weather остаются font paths. Foreground/background IDs/attrs/subsets и clear/keep sentinels не теряются; original attrs и timing не сводятся к однажды вычисленным RGB.

### Font switches и server-visible visual definitions

Explicit map-font switch сохраняет baseline lifecycle: проверка загрузки font; font-specific preference layer; затем graphics overrides; штатные core-owned client setup/font notifications и repaint. Glyph definitions могут намеренно изменяться этим настроечным действием. Font load failure не заменяет успешно работающий font ошибочным ресурсом по догадке; source-backed load-before-replace behavior сохраняется.

Text-only font changes не запускают map visual definitions reload. Unicode display conversion не изменяет received game IDs. Это различает legitimate visual preference changes, передаваемые server setup, и запрещённое implicit remapping из-за text conversion. Shared PRF/resource formats/loading/owners следуют migration/persistence policy.

### Glyph lookup и fallback

- PCF glyph выбирается по encoded ID с range/bounds check и корректным origin offset. Default encoded character разрешается через encoding table, а не принимается за bitmap index. Случаи bundled 12x24/8x16 с origin 1 и 16x22 с max127 входят в обязательный corpus.
- TTF получает Unicode own UI text напрямую; legacy byte-symbols сначала проходят explicit visual-profile ID→Unicode correspondence. Printable ASCII соответствует ASCII; другие IDs требуют заданного mapping. Profile не объявляет charset network text и не меняет numeric game IDs.
- Missing glyph использует корректный font default, затем видимый one-cell replacement с сохранением исходного ID. Shipped profiles должны покрыть полный применимый baseline glyph corpus: массовая замена missing symbols не доказывает parity.
- Custom bitmap glyph fonts сохраняются PCF path. Automatic TTF→bitmap fallback не является обязательным новым механизмом этого решения.
- Solid-wall ID рисуется filled cell; tile IDs проходят graphics lookup и соответствующую композицию. Control/high bytes не считаются автоматически Unicode controls: interpretation задаётся formatted/glyph context.

Accidental TTF UTF-8 decoding raw legacy byte strings исправляется; его искажения не являются oracle. PCF origin/default/bounds defects исправляются с отдельными regression fixtures. Остальные raster/defect dispositions принадлежат raster ticket.

### Formatting, copy и readable XHTML

Supported markers сохраняют baseline transitions/reset first/swap previous/neutral и doubled-0xff literal '{'. Unknown 0xff+code скрывает introducer и оставляет code в current color. Lone trailing 0xff безопасно завершает parsing без glyph; NULL pointer arithmetic/overread не сохраняются. Raw representation не теряет markers. Cell clipping/visible count и byte length различаются и проверяются отдельно.

Copy textual content строится из той же parsed text projection с explicit correspondence, не удаляя неизвестный visible code произвольным strip-pair helper. Readable XHTML сохраняет baseline reverse mappings и glyph substitutions (#/$/.) и '~' при отсутствии доказанного text correspondence, с корректным escaping. Это intentionally readable projection, не lossless archive или raster screenshot. Native screenshot показывает final composed visual; precise screenshot lifecycle/gates обсуждаются в acceptance.

### Input и clipboard contract

Game gestures и runtime user macro definitions сохраняют exact byte/context semantics; не проходят Unicode decoding. Text commits active editor отдельно от gesture actions исключают двойную вставку/command execution. Navigation, Enter/Esc, request replies и cancel sentinels следуют input-router baseline.

Outgoing text сохраняет существующий field-specific byte contract без repertoire extension. Точный correspondence и allowed bytes/limits задаются по field, не по выбранному font. Нет universal ISO-8859-1/UTF-8 network default и blanket ASCII policy. Before core handoff проверяются representability, окончательная byte length и штатные preprocessing/escaping rules. Unknown correspondence не угадывается.

Непредставимый текст вызывает понятную encoding error с draft retained; не превращается в guessed UTF-8 и не удаляется молча. Password normalization/case conversion/font-based transcoding не применяются, imported credential bytes сохраняются точно. Mechanism хранения credentials отдельно.

Clipboard использует ту же field representability/byte-limit проверку с предусмотренными baseline newline/paste escaping rules. Signed-char high-byte dropping и partial UTF-8 slicing не считаются корректным conversion contract. Empty accepted input и approved safe chat-expansion truncation остаются: encoding errors не вводят blanket reject/reopen всех overflow submits. Field-specific editor/wire capacity mismatches требуют atomic evidence/disposition, а не расширения server format по размеру editor.

### Проверяемая матрица и остающиеся владельцы решений

| Проверка | Обязательный outcome |
|---|---|
| Text-only font/scale switch | Map font/grid/tiles/mappings и outgoing bytes сохраняются |
| Map-font switch при graphics off/on/dual-mask | Font preferences → graphics overrides → baseline notifications; UI text choice независим |
| Font-only / tiles с byte glyphs / ASCII weather | Correct glyph/tile identity, attrs и актуальная cell composition |
| PCF origins/partial tables/missing/default | Correct lookup без OOB; поддержанные glyphs не заменяются ошибочным default |
| TTF profile ASCII/high/control/custom IDs | Explicit correspondence, ID retained; full corpus coverage, не raw UTF-8 guessing |
| DPI/font reload с tiles/cursor/weather/raw pictures | Согласованная geometry, repaint без stale cached output и без утраты logical state |
| Marker valid/unknown/doubled/trailing и clipping | Утверждённые visible glyphs/colors/cell positions; raw bytes retained |
| Copy/XHTML/native screenshot | Parsed text consistency/readable substitutions отдельно от final raster capture |
| Text commit/macros/paste/account/password/request boundaries | Exact permitted bytes и replies; errors preserve draft; baseline cancel/empty/truncation сохранены |

Atomic source/field contract inventory требуется в [Enumerate source text and server-field byte contracts](27-enumerate-source-text-and-server-field-byte-contracts.md); результаты не устанавливают новый repertoire. Это sharpened research question, не реализация. Profile/resource/settings registry и defaults/ownership — [Specify persistence ownership and UI configuration schema](25-specify-persistence-ownership-and-ui-configuration-schema.md); конкретные reference fonts/maps/raster compatibility — [Choose raster references and defect compatibility](23-choose-raster-references-and-defect-compatibility.md); DPI/geometry/layout — [Specify surface layouts and responsive rules](21-specify-surface-layouts-and-responsive-rules.md); harness/configuration/platform/runtime evidence — [Design parity evidence and acceptance](09-design-parity-evidence-and-acceptance.md).

Memory/render resource budgets и terminal fallback removal fog без изменений. Implementation, C conversion signatures, generated mapping tables, renderer build и release не выполнялись. Маршрут карты остаётся planning.


### Final confirmation — 2026-09-15

Пользователь принял итоговую резолюцию и закрытие тикета. Status изменён на resolved; context pointer добавлен в Decisions so far карты. Linked layouts/raster/persistence/acceptance и новый source/field research ticket сохраняют собственные ещё не разрешённые вопросы.

### Cross-ticket scaling amendment

[Choose raster references and defect compatibility](23-choose-raster-references-and-defect-compatibility.md) уточняет display scale: map font/profile по-прежнему задаёт внутреннюю glyph/cell geometry и tiles используют ту же клетку, но полный map composition canvas затем получает maximum uniform fit в allocated map area и отдельный user zoom до 100% этого fit. Это не меняет glyph identity, font-specific visual definitions или independent text-font role.

В принятом там решении «Prepared assets and 1:1 composition» canvas обозначает логическую композицию: исходные assets подготавливаются сразу под конечную клетку, TTF rasterize выполняется в конечных метриках, обычный вывод идёт 1:1. Промежуточный raster в размере map font с постоянным повторным scaling отменён; прежний encoding и visual-preference lifecycle сохраняется.
