# Modern TomeNET Client

Контекст описывает язык нового нативного интерфейса TomeNET, который сохраняет возможности существующих клиентов, но представляет их через собственную оконную модель.

## Language

**Single-window client**:
Нативный клиент TomeNET, использующий ровно одно системное окно и собственное логическое разделение интерфейса на области, вкладки и оверлеи. Он не воспроизводит модель отдельных или виртуальных legacy-терминалов.
_Avoid_: Single-term client, multi-window client

**Legacy client**:
Существующие X11- и SDL3-клиенты, чьи доступные пользователю возможности и игровое поведение образуют базу совместимости нового клиента.
_Avoid_: Old UI

**Behavior baseline**:
Совокупность возможностей SDL3-клиента и применимых возможностей X11-клиента, исключая только разделение интерфейса на системные терминальные окна и связанную с ними конфигурацию.
_Avoid_: SDL3 parity

**UX prototype**:
Параллельно развиваемый HTML-интерфейс, нормативный для подтверждённых решений о внешнем виде и компоновке, но не для полноты функций или игрового поведения.
_Avoid_: Web client, behavior baseline

**Capability manifest**:
Канонический компактный индекс пользовательских возможностей и связанных с ними surfaces, actions, bindings и состояний. Он связывает нормативную игровую семантику из behavior baseline с подтверждёнными UX-решениями, но не заменяет ни один из этих источников.
_Avoid_: Feature list, behavior specification, UX specification

**Capability**:
Самостоятельно проверяемый пользовательский outcome или различимый behavior flow. Элементы управления, packet-поля, состояния и части layout сами по себе не являются capabilities.
_Avoid_: Button, packet handler, screen fragment

**Surface**:
Логическая пользовательская поверхность single-window client независимо от того, представлена она панелью, вкладкой, диалогом, оверлеем или режимом. Слово window обозначает только единственный системный `SDL_Window`.
_Avoid_: Window, terminal, subwindow

**Action**:
Семантическое намерение пользователя, которое может быть доступно через разные элементы управления и способы ввода.
_Avoid_: Button callback, keypress

**Binding**:
Контекстная связь между жестом ввода и action; один action может иметь разные bindings в разных input contexts.
_Avoid_: Hotkey, capability

**Game interaction**:
Незавершённый сценарий выполнения action или ответа на запрос сервера, включающий необходимые выборы, подтверждения и отмену. Его смысл и текущий шаг не зависят от способа отображения или ввода.
_Avoid_: Dialog, blocking input loop

**Input context**:
Семантический режим ввода, в котором gesture разрешается в action. Он не зависит от конкретного элемента управления или способа размещения surface.
_Avoid_: Focused widget, keymap

**Input router**:
Модуль single-window client, связывающий ввод UI с переиспользуемой семантикой команд и пользовательских макросов. Он владеет текущим input context, смысловым focus и незавершённым вводом взаимодействия.
_Avoid_: Widget callback, renderer, keymap

**Coverage ledger**:
Machine-checkable заявление конкретного потребителя о том, какие manifest IDs он реализует и каким evidence это подтверждено. Coverage ledger не определяет игровую семантику и не изменяет capability manifest.
_Avoid_: Capability manifest, behavior specification

**Presentation state**:
Семантическое клиентское представление уже полученных игровых данных, из которого single-window client строит интерфейс без чтения legacy-терминального буфера.
_Avoid_: Terminal state, screen scrape

**Session presentation model**:
Глубокий модуль между переиспользуемым ядром и новым UI, владеющий всем session-derived presentation state и его lifecycle. Он принимает version-aware decoded changes от ядра; UI и renderer не читают legacy globals, wire packets или `Term`.
_Avoid_: UI state store, packet model, terminal adapter

**Terminal fallback**:
Временное представление ещё не перенесённого legacy-экрана внутри единственного системного окна. Оно допустимо только во время разработки и не является частью завершённого клиента.
_Avoid_: Virtual terminal

**Compatible protocol extension**:
Необязательное дополнение протокола, при отсутствии которого новый клиент сохраняет корректное базовое поведение, а существующие клиенты продолжают работать без изменений.
_Avoid_: Required server change

**Client profile**:
Набор настроек и личных данных single-window client с независимыми от legacy client основной конфигурацией и игровыми опциями. Файлы пользовательских макросов и ресурсы используются совместно; выбор ресурсов и компоновка относятся к собственным настройкам клиента.
_Avoid_: UI configuration, legacy layout

**Shared SDL3 resources**:
Ресурсы общего пользовательского SDL3-каталога, используемые обоими клиентами для чтения и полноценного изменения; installation `lib/` также общий при размещении binaries в одном каталоге. Включают аудиопаки, шрифты, tilesets, настройки паков и ресурсные caches; изменения этих ресурсов общие для обоих клиентов.
_Avoid_: Read-only legacy assets, independent resource copy

**Legacy import**:
Явный перенос совместимых настроек и личных данных из выбранного legacy-источника в client profile. Перенос из внешнего источника оставляет источник неизменным; уже общие SDL3 данные используются напрямую.
_Avoid_: Required copy of shared SDL3 data, ordinary preference loading

**Preference loading**:
Штатная загрузка preference-файлов single-window client с сохранением предусмотренных behavior baseline действий при загрузке и применения настроек. Она отличается от разового legacy import, который переносит данные без исполнения действий.
_Avoid_: Legacy import, data-only migration


**Text font**:
Выбранный шрифт текстовых surfaces single-window client: HUD, сообщений, списков, диалогов и документов. Его выбор независим от map font и не определяет кодировку отправляемого игрового текста.
_Avoid_: Main Term font, outgoing charset

**Map font**:
Выбранный шрифт font-based представлений terrain, monsters, items, player и клеточных эффектов карты; остаётся применимым при включённых tiles. Его масштаб задаёт общую клеточную геометрию карты независимо от text font.
_Avoid_: Text font, tileset, Unicode encoding

**Map viewport**:
Прямоугольная сетка игровых клеток, которую клиент получает от сервера для отображения карты. Число её строк и столбцов отличается от размера области интерфейса для карты и от размера отображаемой клетки.
_Avoid_: Window size, map zoom, map display area

**Map fit**:
Максимальное вписывание полной карты в доступную область с сохранением пропорций клеток и центрированием. Масштаб определяется доступным местом, не является пользовательской настройкой и не меняет map viewport.
_Avoid_: Map fit zoom, server viewport resize, map crop, font point size

**Map visual stack**:
Семантические visual layers одной map cell: background/color role, terrain identity, foreground game visual и последующие effects/indicators. Terrain и foreground сохраняются раздельно до rendering, поэтому перемещение или удаление foreground открывает актуальный terrain, а не восстановленный raster snapshot.
_Avoid_: Flattened cell bitmap, actor-only layer, legacy mask pixels

**Canonical raster pipeline**:
Нормативная визуальная композиция single-window client. Legacy backends определяют обязательную семантику и поведение, но не являются pixel oracles; неизменны identity, geometry, color roles, layer ordering, visibility и lifecycle. Pixel-perfect compatibility и pixel-perfect acceptance не входят в контракт.
_Avoid_: Behavior baseline, pixel oracle, golden-frame contract

**Logical UI unit**:
Независимая от физических пикселей единица layout и hit testing single-window client. Текущий OS/display scale отображает logical UI units в output pixels; user UI scale и map scale изменяют разные роли поверх этой координатной системы. Готовый интерфейс не является фиксированным framebuffer, растянутым до размера окна.
_Avoid_: Screen pixel, fixed framebuffer coordinate, map cell

**UI scale**:
Пользовательский масштаб текста интерфейса вместе с соответствующими элементами управления, строками и отступами. Может менять доступное карте место и тем самым map fit, но не число клеток map viewport.
_Avoid_: OS display scale, text-only scale, map fit zoom

**Small layout**:
Выбираемая игроком компоновка single-window client с normal map, скрытой правой панелью, визуально вписанной по высоте левой панелью и короткой общей лентой сообщений над картой.
_Avoid_: Automatic responsive layout, message history

**Live message feed**:
Постоянное краткое представление поступающих сообщений игры в текущей компоновке. Оно отличается от полного просмотра истории через Messages primary.
_Avoid_: Message history, independent primary surface
