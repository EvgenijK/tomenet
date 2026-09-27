# Govern the cross-repo capability manifest

Type: grilling
Status: resolved
Assignee: codex
Blocked by:

## Question

Где живёт capability manifest, каковы его стабильные идентификаторы и схема для возможностей, окон, действий, хоткеев и состояний, и какой workflow синхронизирует изменения между native-репозиторием и параллельным HTML UX prototype без превращения mock-реализации в источник игрового поведения?

## Answer

Capability manifest — канонический компактный индекс общей продуктовой модели, а не спецификация игрового поведения и не UX-спецификация. Он живёт в native-репозитории как `docs/capabilities/manifest.json`; его структура задаётся соседним `docs/capabilities/manifest.schema.json`. Отдельный общий репозиторий не нужен. Native behavior baseline и protocol sources нормативны для игровой семантики, подтверждённые HTML UX sources — для presentation, а acceptance sources только доказывают соответствие. Manifest связывает эти источники, но не заменяет их.

Capability получает отдельную запись только для самостоятельно проверяемого пользовательского outcome или различимого полного behavior flow. Кнопки, packet-поля и части layout сами по себе capabilities не образуют. Системное окно обозначается только как `window`; логическая пользовательская поверхность называется `surface`. Хоткей моделируется не как capability, а как `binding` — контекстная связь gesture с action.

Manifest содержит нормализованные коллекции `capabilities`, `surfaces`, `actions`, `states`, `bindings`, `inputContexts` и `relations`. Общие поля сущности: `id`, `title`, `description`, `lifecycle`, `sources`; для заменённых сущностей доступен `replacedBy`. Глобально уникальные неизменяемые ID имеют вид `kind.domain.name`, например `capability.inventory.manage`, `surface.inventory`, `action.item.drop`, `state.player.hp` и `context.gameplay.normal`. Изменение отображаемого имени ID не меняет.

Binding содержит `gesture`, `actionId`, `contextId` и необязательные ограничения `keyset` и `platform`. Отношения представлены отдельными записями `{kind, from, to}`; закрытый enum схемы как минимум включает `capability-exposed-on-surface`, `capability-uses-action`, `capability-observes-state`, `surface-presents-state` и `binding-invokes-action`. Это позволяет инвентаризации input loops добавлять contexts без изменения архитектуры manifest.

Каждый source типизирован ролью `behavior`, `protocol`, `ux` или `acceptance` и указывает repository, revision, path и при необходимости anchor. У каждой active capability обязателен хотя бы один behavior source. При конфликте behavior/protocol определяют игровую семантику, UX — только presentation; fixtures и mock-данные HTML никогда не являются behavior source.

Lifecycle имеет значения `proposed`, `active`, `deprecated`, `retired`. ID никогда не переиспользуются и не удаляются из истории manifest; split/merge сохраняет старую запись и перечисляет замену в `replacedBy`. `schemaVersion` меняется при изменении структуры, а версия содержимого определяется SHA-256 самого manifest, без зависимости от Git revision.

Каждый потребитель ведёт отдельный machine-checkable coverage ledger, ссылающийся на manifest IDs и своё evidence. Manifest не содержит статусы реализаций. HTML ledger обязан явно учитывать каждую active capability; отсутствие записи является ошибкой. Для HTML implementation coverage различаются `missing`, `planned`, `prototype-complete`, а UX approval фиксируется независимо как `ux-approved`, чтобы эти два утверждения могли сосуществовать. Ни одно из них не означает protocol/native parity. Unknown, retired и локальные experimental IDs не удовлетворяют shared coverage.

Cross-repo синхронизация локальная и не использует PR, merge, Git, сеть или submodule. Одна команда валидирует канонический native manifest, копирует snapshot в HTML-репозиторий, записывает SHA-256 источника и добавляет новым active capabilities явный `missing` в HTML ledger. После уточнения coverage та же команда выполняет общую проверку. Источник задаётся аргументом либо обнаруживается как соседний native-репозиторий. HTML работает с последним локальным snapshot; если canonical source недоступен, проверка актуальности возвращает `unavailable`, но не успех.

Структурная проверка выполняется JSON Schema; семантическая проверка контролирует уникальность и префиксы ID, типы и существование ссылок, допустимые relation endpoints, обязательный behavior provenance, lifecycle/replacedBy и отсутствие replacement cycles. Consumer-проверка контролирует digest snapshot, полное явное покрытие active capabilities, допустимые claims и запрет выдавать prototype/UX evidence за native parity.
