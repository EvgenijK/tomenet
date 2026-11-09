# C045 — Input, messages и alert options: preference effects

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

Проверить source-defined применение `rogue_like_commands`, message/topline
preferences и alert options в существующих input/message/alert consumers. Значения
не должны оставаться только parser state: последующие gestures и сообщения
используют текущие настройки, уже доставленные occurrences не воспроизводятся.
Связать input routes с C005/C053, messages с C016, alert delivery с C050 по
применимой семантике; эти связи не меняют allocation и не запускают новый
общий option bus. Не вводить немедленный эффект для ключа, у которого baseline
только меняет поведение следующих событий.

Сверить применимые key branches с `src/client/c-util.c:check_immediate_options`
и [settings policy](../../capabilities/settings-policy.md#opt-registry-and-semantic-preservation).
Не всякая option имеет immediate side effect: документировать source-defined
apply/no-immediate-effect disposition, сохраняя её значение и последующие consumers.

## Минимальные Linux проверки и завершение

В existing production input/message/alert path загрузить небольшой PRF с
применимым X/Y и повторной same-value записью; проверить фактический следующий
gesture/message/alert, baseline context gate и отсутствие повторной доставки
исторических occurrences. Готовность — значения и применимые immediate intents
достигают настоящего группового consumer, unsupported branches явно диагностированы,
поздний отсутствующий consumer не объявлен доказанным по parser PASS.

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
