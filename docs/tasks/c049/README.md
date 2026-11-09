# C049 — Audio и client-integration options: preference effects

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

Подключить occurrences к уже работающим audio/client integration consumers.
Baseline `shuffle_music`/`play_all` refresh вызывается и при same-value assignment,
но с исходными `music_next == -1` и in-game gates; не вызывать reload из renderer.
`clone_to_stdout/clone_to_file` влияют на последующие message occurrences через
существующие C003 message sinks и C016 assembly; не воспроизводить историю и
не вводить CFG/OPT autosave. Остальные audio/client keys сохраняют свои реальные
потребители и baseline availability, не превращаются в общую команду reinitialize.
C049 реализует/проверяет доступные сейчас consumers. Отсутствующий полный live
audio flow завершается при реализации C055 с этим же option occurrence contract;
это не новый prerequisite C049→полный C055 и не повод строить C055 внутри C049.

Сверить применимые key branches с `src/client/c-util.c:check_immediate_options`
и [settings policy](../../capabilities/settings-policy.md#opt-registry-and-semantic-preservation).
Не всякая option имеет immediate side effect: документировать source-defined
apply/no-immediate-effect disposition, сохраняя её значение и последующие consumers.

## Минимальные Linux проверки и завершение

Через текущий production audio/sink interface проверить относящееся X/Y и
повторное same-value assignment, фактический refresh или source-defined отказ
по gate. Для clone toggles наблюдать последующую реальную occurrence/bytes,
без двойной записи. Готовность текущей связки — действующие consumers подключены,
requested/effective и available/unavailable различены; недостающий live audio
recipient имеет конкретный pending C055 integration, без helper-only PASS и
без задержки независимых current-consumer работ. Новую audio matrix не запускать.

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
