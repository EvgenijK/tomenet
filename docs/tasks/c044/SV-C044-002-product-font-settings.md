# SV-C044-002 — продуктовые font/resource settings вместо временного F6 UI

Статус: planned, 2026-11-09.
Labels: enhancement
Тип: обязательная продуктовая интеграция C006 и удаление временного UI; 0 новых IDs.
Зависимости: production resource core C006 и согласованная Settings surface C044.
Связка: [001 — explicit preference load и общий Save owner](SV-C044-001-explicit-preference-load.md).

## Решение и результат

По поручению пользователя 2026-11-09 временный font/resource editor C006 заменяется
продуктовым интерфейсом **в C044**. Удаление временного экрана входит в критерии
завершения этого этапа. C006 предоставляет рабочие resource APIs; его F6 glyph
atlas не является принятым продуктовым UX.

Нормативны [CONTEXT](../../../CONTEXT.md),
[settings policy](../../capabilities/settings-policy.md),
[архитектура](../../sv-architecture.md), подтверждённые решения UX prototype и ADR
[0002](../../adr/0002-main-thread-state-ownership.md),
[0003](../../adr/0003-explicit-module-connections.md),
[0004](../../adr/0004-revision-based-ui-views.md),
[0005](../../adr/0005-session-scoped-work.md),
[0006](../../adr/0006-errors-and-overload.md).

## Работа

1. Найти подтверждённый продуктовый дизайн Settings и font/resource controls в
   UX prototype. Зафиксировать применимые surfaces/actions/bindings и поведение
   preview. Неподтверждённые layout/bindings получить как UX-решение; не объявлять
   текущий F6-экран утверждённым образцом. Продуктовый preview показывает влияние
   выбранных ресурсов на репрезентативные текст и карту, с различимыми requested,
   effective и error states; technical glyph atlas не подменяет этот сценарий.
2. Подключить независимый выбор text/map fonts и profiles, graphics/PCF filters
   и outline к общей Settings surface и её draft. Использовать C006
   `ui/font-selection.[ch]`, `ui/font.[ch]`, `ui/resource-preparation.[ch]`,
   `settings/map-font.[ch]` и existing C002/C003 publication. Сверить актуальные
   interfaces при реализации; второй loader/parser/Save owner не создавать.
   Полную игровую композицию предоставляет C007; resource preview не получает
   отдельной реализации игрового поведения.
3. Сохранить prepare-before-publish, startup fallback и прежний рабочий stack
   при failed live change. Text-only change не меняет map mappings/reporting;
   map change сохраняет установленный reset/PRF/graphics/server-update порядок.
   Preview не пишет на диск. Явный Save сохраняет requested refs в own CFG,
   перечитывает destination, объединяет changed records и сообщает external-wins
   conflicts. Failed Save оставляет active dirty preview.
4. Объединить font/resource draft с общими Save/Cancel/dirty-close C044:
   «Сохранить / Отменить изменения / Вернуться». Cancel восстанавливает opening
   settings/resource stack, не откатывая полученные игровые данные или уже
   исполненные действия. Обычный выход не сохраняет pending edits. Связка 001
   использует тот же Settings lifecycle, сохраняя отдельный смысл PRF load effects.
5. После подключения продуктового пути удалить временные
   `src/client/sv/ui/font-editor.[ch]`, их object из `src/makefile.sv`, includes,
   создание/отрисовку/event routing и специальную F6 ветку в
   `src/client/sv/login/native-run.c`. Сохранить необходимые resource lifecycle
   connections, перенесённые в продуктового owner. Glyph atlas, diagnostic
   controls и отдельный временный экран не оставлять скрытым production fallback.
   Новый shortcut допустим только как подтверждённый binding продуктовой action.
6. Сохранить минимальные core tests C006. Проверку `tests/sv/font-editor.c`
   заменить необходимой проверкой продуктового caller; удалить зависимости от
   удалённого UI. Исторические evidence/кадры C006 сохранить с пометкой, что они
   фиксируют прежний временный consumer, а не текущий продуктовый интерфейс.

## Минимальная проверка и завершение

Через продуктовую Settings action проверить независимый text/map выбор, реальное
применение preview, failed resource replacement с прежним stack и один flow
Save→reload, Cancel и dirty-close. Reuse существующих production core checks
для PRF/wire/fallback/conflicts; новый harness и широкая visual/fault matrix
не нужны. Layout и интеракции сверить с подтверждённым UX prototype.
Только [Linux](../../agents/platform-validation.md); расширение проверок заранее
согласуется по AGENTS.md, Windows/MinGW/Wine deferred.

Готовность: продуктовый font/resource flow работает через C006 core, общий
Settings lifecycle соблюдён; временные F6 UI files, build inputs и native routes
удалены, production доступен через согласованные controls/bindings. Подтвердить
это source/build сверкой и фактическим UI caller evidence. Простое скрытие
shortcut или добавление второго экрана рядом с прежним задачу не завершает.

Canonical fonts ownership остаётся C006; общий Settings outcome allocation —
C044. C056 сохраняет собственные configuration outcomes, но его будущая приёмка
не откладывает удаление временного UI за границы C044. План не заявляет runtime
PASS или приёмку C044 и не переоткрывает C006 автоматически.
