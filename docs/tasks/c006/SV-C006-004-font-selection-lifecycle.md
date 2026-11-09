# SV-C006-004 — независимый выбор fonts, preview/Save и failure lifecycle

Статус: accepted, 2026-11-09; принят пользователем в согласованных границах.
Labels: enhancement
Owned outcomes: `capability.fonts.text-select`, `capability.fonts.fallback`,
`capability.fonts.live-failure` (единственный owner этих трёх IDs).
Зависимости: C002 requested CFG, C003 file/publication owners, font loaders 003;
map-specific integration [001](SV-C006-001-map-resource-integration.md) и
preference stack [002](SV-C006-002-map-preferences-integration.md).
Взаимодействие с 005: graphics candidate preparation, tile fallback и live failure.

## Результат и границы

Независимые text/map font и profile имеют реальные production операции выбора,
preview, Cancel и явного Save. Text font обслуживает HUD, сообщения, списки,
диалоги и документы и не определяет игровые glyph IDs или outgoing charset.
001 остаётся единственным owner map-select: здесь общий selection/lifecycle,
там map-specific mapping reload, применение к карте и server reporting.
002 — supporting integration с 0 IDs, а не второй owner этих outcomes.

Нормативны [CONTEXT](../../../CONTEXT.md),
[settings policy](../../capabilities/settings-policy.md),
[архитектура](../../sv-architecture.md), ADR
[0002](../../adr/0002-main-thread-state-ownership.md),
[0003](../../adr/0003-explicit-module-connections.md),
[0004](../../adr/0004-revision-based-ui-views.md),
[0005](../../adr/0005-session-scoped-work.md),
[0006](../../adr/0006-errors-and-overload.md).
Существующие решения о приёмке C002/C003/C004 сохраняются; найденные при
интеграции дефекты исправляются в C006 без автоматического переоткрытия этапов.

## Работа

1. Использовать requested поля `SvSettingsConfig` из C002: `text_font`,
   `map_font`, `text_profile`, `map_profile`; full requested references сохранять
   отдельно от фактически загруженных ресурсов/profiles и диагностики.
   Разрешение U/B, чтение и публикация используют C003 owners. Второй CFG parser
   или файловый алгоритм не вводить. Точные public interfaces пересверить при
   реализации: существующий C002 load/parser сам по себе не доказывает Save.
2. Дать минимальный реальный production caller выбора text/map font/profile,
   preview и Cancel в работающем SV. Preview подготавливает и применяет кандидат
   к соответствующему font consumer без disk write. Cancel восстанавливает
   прежний согласованный requested/effective ресурсный набор и представление;
   для map rollback используется путь 001/002 с необходимыми notifications.
   Text-only preview/Save/Cancel не сбрасывает map mappings, identity или wire.
3. Явный Save записывает requested font/profile в собственный S CFG через
   существующую модель C002 и publication C003; при необходимости добавить
   только узкий production seam сохранения. Сохранить unrelated/inert records,
   независимость от legacy CFG, прежний файл при неудачной публикации и явную
   диагностику. Перед Save перечитать destination и объединить только изменённые
   font/resource records; конфликтующее внешнее значение сохранить с диагностикой.
   Failed Save оставляет preview изменения active+dirty; не заявлять успешную
   запись и не откатывать рабочий preview молча. Эта безопасность собственного
   Save обязательна здесь, общий conflict UI C044 её не откладывает.
   Не сохранять effective fallback вместо пользовательского выбора.
   Повторный startup восстанавливает сохранённый request. Закрытие/выход без
   Save, обычная смена ресурса и fallback не создают autosave.
4. Startup: при неудаче выбранного text font/profile применить bundled
   CascadiaMono-Regular.ttf/profile, затем `16x24x.pcf`; для map —
   `16x24x.pcf` с согласованным profile. Ошибку исходного выбора сообщить,
   requested reference сохранить. Все применимые fallback исчерпаны — остановить
   startup с понятной resource error. Успешно загруженный PCF не заменять другим
   размером/семейством/TTF ради вида. Profile failure входит в тот же candidate
   contract; missing и invalid/unreadable overlay не маскировать одним результатом.
5. Live font/profile/tiles selection: validate/prepare до замены. Неудачный
   candidate оставляет прежний рабочий resource/mappings/prepared state и
   effective report. Неудачный draft request остаётся различимым от effective;
   Cancel восстанавливает прежний request, Save сохраняет явно выбранный request.
   005 возвращает результат подготовки tiles и выполняет startup font-only
   fallback с сообщением и сохранением requested tiles/enabled. Этот startup
   fallback не подменяет live-failure правилом потери прежнего рабочего stack.
6. Публиковать согласованный кандидат и версии derived presentation/cache,
   освобождая abandoned resources. Repaint/recreation не перечитывают PRF и не
   повторяют load effects. App resources могут переживать соединение; packets
   и PRF effects принадлежат текущему session generation. Stale completion
   освобождается и не меняет новую сессию. Production generation/ownership и
   отсутствие replay обязательны здесь совместно с 002; actual message/action
   delivery завершают C016/C053 без backward gate.

Общий Settings transaction UI/registry C044/C056 не входит в эту задачу.
Однако минимальный реальный font preview/cancel/private Save входит в C006 и
не откладывается до этих этапов. C006/005 предоставляет реальные production
tile/raw-picture preparation/display, filters, outline и font-only map/message;
C007 завершает полную игровую композицию, FI-02 и coupled graphics PRF flow,
не блокируя приёмку C006. Настройки filters/outline взаимодействуют с этим
requested/effective lifecycle, их единственные outcome owners находятся в 005.

## Минимальная проверка и завершение

Переиспользовать Linux production core/consumer проверки 001/002/003/005.
Показать независимый text selection и реальное использование font, preview →
Cancel без записи, preview → явный private Save → reload request, failed Save
с прежним CFG и active+dirty preview, merge unrelated external edit и сохранение
внешнего значения при конфликте font record; отсутствие autosave. Переиспользовать
минимальный production publication case, без отдельной conflict matrix.
Проверить declared startup fallback text/map
и исчерпание fallback, failed live font/profile/tiles с прежним рабочим stack,
requested/effective distinction и отсутствие map effects при text-only change.
Существующий peer 001/002 подтверждает map-specific bytes; новый общий harness,
полную UI/fault matrix или копию parser suite не строить.

Evidence 007 фиксирует production callers/revision, Linux команды, requested и
effective font/profile, результат preview/Cancel/Save и фактические consumer
результаты. Не заявлять runtime PASS по одному плану или loader snapshot.
Проверки — необходимый минимум core production path;
расширение согласуется заранее по AGENTS.md.
[Linux обязателен](../../agents/platform-validation.md), Windows/MinGW/Wine deferred.

## Результат реализации — 2026-11-09

Production SV реализация и минимальные Linux core проверки выполнены.
[Фактическое evidence и ограничения](../../sv-c006-evidence.md) отделяют
software resource consumer от поздних gameplay flows и непроверенных backends.
Canonical runtime registration и пользовательская приёмка не выполнялись.
