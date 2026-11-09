# C046 — HUD, lighting и visibility options: preference effects

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

Подключить применимые HUD/lighting preferences к действующим semantic views.
Baseline `check_immediate_options` для `mp_huge_bar/sn_huge_bar/hp_huge_bar/st_huge_bar`,
`solid_bars/huge_bars_gfx` имеет context/value gates, clear/reset/redraw поведение
даже при прежнем option value; server-gated sanity-bar message сохраняется.
Сохранить осмысленное SV действие expanded indicators и number/bar preferences
по settings-policy, не переносить Term topology. Font/lighting mappings используют
C006/C007 resource owners; `font_map_solid_walls` не становится parser-only no-op.

Сверить применимые key branches с `src/client/c-util.c:check_immediate_options`
и [settings policy](../../capabilities/settings-policy.md#opt-registry-and-semantic-preservation).
Не всякая option имеет immediate side effect: документировать source-defined
apply/no-immediate-effect disposition, сохраняя её значение и последующие consumers.

## Минимальные Linux проверки и завершение

В реальном production HUD path применить минимальный относящийся X/Y case и
повторить same-value assignment, наблюдая baseline-required reset/redraw/model
результат при применимом контексте. Проверить enabled/disabled state и applicable
server gate через existing interfaces, без новой raster matrix. Готовность —
реальный consumer принимает occurrence и сохраняет gates/visible information,
а не только bool/option packet; недоступный renderer branch остаётся явным
late integration evidence у его owner.

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
