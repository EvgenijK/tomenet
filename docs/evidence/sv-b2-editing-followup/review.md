# Независимое review: editing / birth navigation / class UI

2026-10-30. Проверялись текущие изменения относительно `credentials-before.c`, `.scratch/birth-navigation/before-*.c` и `.scratch/sv-b2-class-style/{scene-before.c,layout-before.h}`. Старые изменения dirty working tree в область review не включались.

## Результат

Открытых actionable замечаний после корректировки нет.

- `credentials.c`: fallback действует только при `SDLK_UNKNOWN`, распознаёт фиксированные editor keys и Ctrl+A/V/L; обычная раскладка, отдельный SDL text-input, noncached private password rendering и wipe сохраняются. Новые core assertions вызывают production interaction для обоих полей. Это синтетический red→green для отсутствующего key mapping; доказательством причины пользовательского физического сбоя он не является.
- SEX Back: отдельный результат инициирует disconnect старого pregame, normal teardown закрывает socket, birth и login, затем создаётся новое generation. В новый contact передаётся сохранённый draft и тип ordinary/exclusive; пароль остаётся в прежнем приватном буфере до общего wipe. Повторный LOGIN на старом сокете не отправляется. В production recovery evidence старый peer получает EOF, затем второе соединение сохраняет draft, исправляет имя и отвергает stale input; поколения различаются. Keyboard и mouse interactive logs завершаются PASS.
- Grid: вертикальный шаг соответствует актуальному `viewport.columns`; верхняя/нижняя граница не wrap, нелегальные клетки пропускаются, keypad снимает footer focus. Tab проходит только видимые Back/Continue, удалённые кнопки имеют пустые hitboxes. Повторно выполнены scratch production core `back` и `grid`: PASS.
- Class presentation: просмотрены actual production-render captures wide 2560×1600 (logical 1920×1200), minimum 1024×768, fractional 1536×1152 при scale1.5, Istar и длинное название. Guidance не перекрывает карточки, footer не перекрывает content; minimum показывает одну строку двух колонок с прокруткой. Istar preview — 5/13/10/11/8/11 — соответствует модификаторам только в presentation. Длинный detail title переносится, attribute badges остаются отдельными, class card сохраняет существующее clipping длинных подписей.

## Исправленное замечание

P2: detail height считалась для ширины `detail.w-28`, а текст рисовался в `detail.w-36`. На minimum Warrior/Rogue получалось 9 расчётных строк при 10 нарисованных, что ограничивало максимальную прокрутку и обрезало последнюю строку. Автор исправил расчёт body/title на ту же ширину `-36`; текущий source использует одинаковую геометрию измерения и отрисовки. Запрошен дополнительный capture при максимальной прокрутке, если доступен; исходная причина устранена структурно.

## Evidence

- `.scratch/sv-b2-editing-followup/credentials-red.log`, `endpoint-checks.log`.
- `.scratch/birth-navigation/back-green.log`, `grid-green.log`, `recovery-green.log`, `interactive-keyboard.log`, `interactive-mouse.log`.
- `.scratch/sv-b2-class-style/manifest.json`, `after-class-{wide,min,fractional}.png`, `after-class-{istar,long}.png`.

Новые suites и production instrumentation в рамках review не добавлялись. Проверки Linux; user acceptance не объявляется.
