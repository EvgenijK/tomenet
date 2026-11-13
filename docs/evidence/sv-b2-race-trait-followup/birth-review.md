# Независимое review: birth letter shortcuts и race/trait UI

2026-10-30. Baseline: `.scratch/birth-hotkeys/before-native.c`, `.scratch/sv-b2-race-trait-style/{scene-before.c,layout-before.h}`. Reviewer production source/tests не изменял.

## Functional review

Actionable замечаний по native-birth delta нет.

- `select_and_confirm` сначала отправляет обычную production select-команду, подтверждает только результат HANDLED и только при сохранении исходного owner. Недопустимый/нелегальный выбор не подтверждается; переход с пропуском последующих шагов не получает второго Confirm.
- Нижний регистр m/f для пола, visible class letters, server race IDs и позиции legal trait list используют исходную привязку и model legality. Stats/body/mode semantics не изменены.
- Repeat, generation/timestamp, modifier/layout guards остаются на прежнем пути. Shift/Caps/Control не превращаются в обычный letter shortcut; Shift+Q/Ctrl+Q quit, uppercase H mode и reserved symbols сохраняются. SDL text-input не вызывает повторного подтверждения.
- Race/trait стрелки используют текущие layout columns, сохраняют owner и только подсвечивают выбор. Нелегальные race cells пропускаются; края не выходят за count; keypad снимает footer focus.
- Повторно выполнены production scratch core modes `letters`, `grid`, `guards`: PASS. Красные исходные логи показывают неподтверждённый f и вертикальный шаг race 1 вместо columns.

## Visual review

Просмотрены race wide 2560×1600, trait fractional 1536×1152 при scale1.5, trait last-page minimum 1024×768 и race detail at maximum scroll. Wide race — три колонки, запрещённые варианты приглушены. Trait intro переносится до начала карточек. Последняя страница и конец detail доступны, подложки/скругления/карточки не перекрывают footer; layout и hit testing используют общий helper.

## Исправленное замечание

P2 / documented contract: первоначальный footer loop уменьшал font size с 12 до 9 вопреки `docs/tasks/stage-b2/reference.md:71`. Автор удалил shrink, оставил фиксированные 12px и добавил общий `SvBirthLayout.footer`: 70 logical units при width≤1240, 46 на wide. В compact caption находится в отдельной верхней строке, Back/Continue сохраняют прежние общие input/paint rectangles снизу. Summary и detail резервируют дополнительную высоту footer; native viewport получает тот же layout. Повторно просмотрены обновлённые `after-trait-fractional.png`, `after-trait-last-min.png`, `after-trait-detail-bottom.png`: наложений нет, конец текста доступен. P2 закрыт, открытых замечаний visual delta нет.

Финальные просмотренные source SHA-256:
- `endpoint-scene.c`: `9f22a403c15d9b6bc97f267889b06db64231aeb9006c2901b71cb4b701edbaf4`.
- `login-layout.h`: `baa42f9310a399e276889dfdc87013ee42c82e491c3fd9988ea9dbe49fce1aa0`.

## Отдельный пользовательский сбой

Initial credentials non-text input расследуется отдельно root/chrono. Предыдущий UNKNOWN-key fallback и его синтетические red→green проверки не доказывают исправление физического пользовательского сценария. Данный review его исправленным не объявляет.
