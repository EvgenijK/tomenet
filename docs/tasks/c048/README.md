# C048 — Item handling и session options: preference effects

Статус: planned, 2026-11-09. Компактное обязательное дополнение к этапу,
не полный план его canonical outcomes и не statement runtime acceptance.

## Production seam и работа

`settings/options.c:sv_settings_option_apply_record` задаёт original slot identities
и порядок direct/alias assignments. `settings/preferences.c:apply_option` сохраняет
каждое применённое assignment как `SV_PREF_OPTION`, включая прежнее bool value.
`sv_preferences_effect` даёт slot/value/in_game/generation/source, а не новый
format parser. Consumer принимает occurrences через явную assembly
[C011](../c011/SV-C011-001-preference-adoption.md)/
[C014](../c014/SV-C014-002-preference-lifecycle.md). Не терять startup intents
при character clone и не очищать весь batch до обработки message/action recipients.

Связать применимые item/session option occurrences с действующими owners,
сохранив сетевые slot identities и gates. `show_newest` baseline вызывает
`redraw_newest` и для повторной записи того же значения: реальный marker
показывается/очищается по текущему item state. Сохранить значения для последующих
item selection, pickup/destroy/inscription/session событий; parser не вызывает
новую item operation лишь из-за assignment, если baseline этого не делает.
Settings preview не откатывает уже произошедшее игровое действие.

Сверить применимые key branches с `src/client/c-util.c:check_immediate_options`
и [settings policy](../../capabilities/settings-policy.md#opt-registry-and-semantic-preservation).
Не всякая option имеет immediate side effect: документировать source-defined
apply/no-immediate-effect disposition, сохраняя её значение и последующие consumers.

## Минимальные Linux проверки и завершение

В existing production item/session path применить небольшой PRF с относящейся
X/Y записью и повторным `show_newest`, проверить actual marker/следующий decision,
точные slots и in-game/generation semantics. Готовность — реальный item/session
consumer использует options и выполняет применимый immediate refresh, без
повторных игровых действий от redraw. Не расширять это в новую inventory/action
matrix; отсутствующий поздний flow проверяется и исправляется в своём owner.

Готовность этого этапа определяется actual production responsibility и доступными
consumers. Если конкретный recipient/caller ещё относится позднему этапу, его
реальный combined flow обязателен при реализации того владельца и фиксируется
как integration obligation, без нового обратного prerequisite и без helper PASS
за отсутствующую функцию. Не ждать поздний full runtime для независимой проверки
уже действующего consumer.

C004 принят и завершён пользователем 2026-11-09. Эта интеграция и необходимые
исправления обязательны здесь; поздний результат не переоткрывает C004
автоматически. Три C004 IDs, conditions и девять obligations остаются в
[canonical snapshot](../c004/coverage.json), allocation/statuses не меняются.
Evidence хранится у текущего этапа, обратная ссылка в C004 optional для
трассировки и не gate его приёмки. Проверки — минимальный Linux core через
production путь; расширение согласовать по [AGENTS](../../../AGENTS.md),
Windows/MinGW/Wine deferred. Новые suites/harness/full plans не вводятся.
