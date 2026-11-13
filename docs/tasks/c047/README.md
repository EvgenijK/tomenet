# C047 — Map identity, animation, movement и targeting options: preference effects

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

Подключить options к настоящим map/movement/targeting consumers и source-defined
immediate behavior. `no_weather` прекращает текущие visuals/sounds при исходном
gate; выключение `highlight_player` скрывает cursor; прочие key effects/animation
следуют baseline и современному presentation owner, без чтения Term. Alias
`basic_players`/`consistent_players` производит сначала `basic_players_col=false`,
затем значение `basic_players_symb`; потребитель не сортирует и не склеивает
эти occurrences по slot. Сопоставление map consumers — C006/C007, actual target/
movement — соответствующие stages; данные их новых возможностей не принимаются
по успешной загрузке options.

Сверить применимые key branches с `src/client/c-util.c:check_immediate_options`
и [settings policy](../../capabilities/settings-policy.md#opt-registry-and-semantic-preservation).
Не всякая option имеет immediate side effect: документировать source-defined
apply/no-immediate-effect disposition, сохраняя её значение и последующие consumers.

## Минимальные Linux проверки и завершение

Через существующий production path проверить применимый immediate map/target
case и ordered one-to-many alias: реальные получатели видят col=false до symb,
в том числе если col уже false. Проверить source-defined context gate, актуальное
поколение и отсутствие повторов на redraw. Готовность — actual semantic effect
и source-defined ordering; ни bool table, ни terminal snapshot не заменяют
production map/target outcome. Ограничиться нужным core case, без полной
animation/weather/movement matrix.

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
