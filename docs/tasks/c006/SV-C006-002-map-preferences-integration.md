# SV-C006-002 — effective map font preferences и reload C004

Статус: planned, 2026-11-09; реализация и flow evidence pending.
Labels: enhancement
Зависимости: production preference core C004, file/reporting core C003,
[SV-C006-001](SV-C006-001-map-resource-integration.md).
Последующий consumer: graphics overlay C007 по
[SV-C007-002](../c007/SV-C007-002-graphics-preferences-integration.md);
его реализация и проверки не являются prerequisite приёмки C006.

## Решение и границы

Пользователь 2026-11-09 принял и закрыл C004, передав оставшуюся интеграцию и
проверки последующим этапам. Этот обязательный consumer выполняется в C006;
найденные здесь дефекты PRF/font integration исправляются здесь без
автоматического переоткрытия C004. C006 этим документом не принимается.
Canonical allocation, IDs, conditions, obligations и runtime statuses не меняются.
Исходные требования и результаты сохраняются в [source contract C004](../c004/source-contract.md),
[evidence](../../sv-c004-evidence.md) и [приёмке C004](../../sv-stage-c004-acceptance.md).

Задача дополняет C003/FI-01 задачу 001: та отвечает за active map resource и
truthful reporting, эта — за реальное применение preference stack. Font loader,
reporting и parser не дублировать. Нормативны [CONTEXT](../../../CONTEXT.md),
[settings policy](../../capabilities/settings-policy.md),
[архитектура](../../sv-architecture.md) и ADR
[0002](../../adr/0002-main-thread-state-ownership.md),
[0003](../../adr/0003-explicit-module-connections.md),
[0004](../../adr/0004-revision-based-ui-views.md),
[0005](../../adr/0005-session-scoped-work.md),
[0006](../../adr/0006-errors-and-overload.md).

## Production путь

1. Подключить effective map-font owner 001 к `SvPreferenceResourceHook` из
   `src/client/sv/settings/preferences.h:sv_preferences_bootstrap`. Сейчас
   `login/native-run.c:sv_endpoint_run` передаёт NULL: настоящего map consumer
   нет. Hook вызывается после system preferences и до `S/global-sv.opt`;
   character OPT остаётся в `login/flow.c` до первого options packet. Открытый
   `ui/font.c:sv_font_open_selected` обслуживает text font и не заполняет этот hook.
2. По фактически загруженному map font выбрать custom font PRF, иначе system
   font PRF. Exact basename/extension и выбор SV system source/fallback сверить
   с `c-util.c:handle_process_font_file` и действительными bundled files; закрепить
   явно в SV, не менять глобальный `ANGBAND_SYS`. Requested имя не использовать
   вместо effective при font fallback. Для каждого источника сохранить logical
   role, resolved `SvFilePath.owner`/path и диагностику; shared overlay U перед B
   разрешать через production preference/file owners C004/C003. Missing custom,
   invalid/unreadable custom и отсутствующий system source различать, применяя
   объявленную consumer policy без скрытого успеха или общего skip-all-errors.
3. Реализовать SV-local reset/rebuild visual definitions перед новым font PRF:
   R/K/F/U, соответствующие visual modifiers и glyph-indexed subset state должны
   терять определения, отсутствующие в новом наборе. У нынешнего `SvPreferences`
   есть clone и mapping accessors, но отдельного visual reset API нет; одного
   clone активного набора и наложения новых строк недостаточно. Добавить только
   необходимый production seam в preference owner, сохранив unrelated options,
   macros/current A и правильное владение retained records/effects. Новый parser
   или отдельную test-only таблицу поведения не вводить.
4. Тем же `sv_preferences_load` применить font definitions с font offset 0
   и предоставить seam для последующего C007 graphics overlay. В C006
   подготовленный набор должен реально определять numeric glyph IDs, attrs,
   font selection и font-based map cell output;
   UI Unicode text не изменяет игровые IDs. Effective resource, mappings и
   reporting публиковать согласованно по load-before-replace: startup применяет
   объявленный fallback с диагностикой; failed live change сохраняет прежний
   рабочий stack. Обычная PRF загрузка сохраняет свои partial/continuation rules;
   resource owner отдельно решает, какой пригодный candidate можно активировать.
5. Для font-based map выполнить reset → font PRF → client setup/font
   notifications → repaint. Между font PRF и notifications оставить seam:
   graphics-enabled ветвь и её coupled reset/override/reload evidence выполняет
   C007, вставляя graphics PRFs в baseline позицию. C006 её не ожидает.
   Initial/live notifications отправлять через
   существующий protocol/reporting owner C003, с текущим session generation.
   Text-only font change этот цикл не запускает. Repaint/recreation используют
   уже подготовленный набор и не перечитывают PRF сами по себе; реальные reload
   effects возникают один раз на load, сохраняют payload/context и имеют явного
   владельца. C006 доказывает production generation/ownership и отсутствие
   replay; actual message/action delivery выполняют C016/C053 по своим задачам
   и не блокируют приёмку C006. Renderer эти bytes не исполняет и не объявляет
   доставленными. App resource lifetime и session effect lifetime
   сохраняют различие; handoff/retry не повторяет startup/character intents.

Baseline seams: `src/client/c-init.c:initialize_main_pref_files`,
`src/client/c-util.c:handle_process_font_file` (20224),
`src/client/c-files.c:process_pref_file_aux_aux`; production SV seams:
`settings/preferences.[ch]`, `files/files.[ch]`, `protocol/login-identity.[ch]`,
`protocol/login.c`, `login/native-run.c`, `login/flow.c`. Координаты пересверить
при реализации. Legacy служит behavior baseline, а не pixel oracle.

## Минимальная Linux проверка и завершение

Через тот же production map-font consumer и preference API проверить различимые
custom/system/U/B mappings, startup hook до own system/character OPT и реальные
requested/effective источники после fallback. Один сменяемый font fixture должен
показать reset старой записи, новые font glyph/attr definitions и действительный
font-based map output; text-font-only change сохраняет map definitions и wire
identity. Graphics-enabled overrides/subsets и coupled actual flow проверяет
C007 на предоставленном seam, без backward gate для C006.
Через существующий peer подтвердить actual client setup/font bytes, успешный
reload и failure с прежним рабочим stack. Repaint/reinit и смена session не
повторяют load effects; исходные CFG/OPT/PRF bytes не меняются. Не строить новую
полную visual/fault matrix: переиспользовать проверки 001 и existing core/peer,
добавляя только необходимое доказательство этого consumer.

Завершение: реальный font-based map использует выбранный preference stack;
порядок/reset/fallback и server notifications доказаны production callers/bytes;
все применимые failures имеют явный результат. В evidence C006 записать revision,
Linux команды, expected/actual sources, definitions, effects и wire; связать
применимые `obligation.preferences.load.{result,failure-lifecycle}` и
`obligation.preferences.bootstrap.{result,failure-lifecycle,allocation-checks}`
из [exact C004 snapshot](../c004/coverage.json). Это трассировка прежних obligations,
а не повторная приёмка или prerequisite закрытого C004. Graphics-enabled
reset/overrides/reload и actual rendering завершает C007; delivery messages/actions
завершают C016/C053. Эти последующие flows не входят в completion criteria C006:
здесь обязательны настоящий font consumer, его semantic/wire результаты и
production effect generation/ownership без fake execution.

Проверки — минимальный production core по [Linux rule](../../agents/platform-validation.md);
расширение согласуется по AGENTS.md. Windows/MinGW/Wine deferred. План не заявляет
новый runtime PASS и не заменяет полный контракт C006.
