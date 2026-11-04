# Сводка на всех шагах создания персонажа

2026-10-30. Follow-up B2-006; пользовательская приёмка открыта.

По уточнению пользователя стартовый preview шести атрибутов показывает 10,
на name step и до подготовки атрибутов birth. Это presentation default:
wire/draft/priority-order/point-buy значения и игровые правила не меняются.
Когда подготовлены point-buy атрибуты, сводка показывает реальные значения
с действовавшими поправками подтверждённых класса и расы.

Name и остальные birth steps используют одну декоративную сводку с отдельными
карточками атрибутов, рамкой и линейками. После Continue оформление сохраняется;
значения выбора берутся из подтверждённого/текущего highlighted production view.
Длинные class/race значения переносятся, independent summary scroll сохраняется.
Числовая колонка вмещает значения вида 18/100 без ellipsis.

Правая/нижняя граница обрезалась собственным clip: SDL_RenderRect достигает
right/bottom endpoint, а clip исключает эту границу. Локальный clip сводки
включает raster fringe и пересекается с внешним clip, затем восстанавливается.
Глобальная отрисовка прямоугольников не менялась.

У выбранных birth cards оставлена одна внешняя рамка; внутренний rectangle и
левая полоса удалены по запросу пользователя. Добавлены контекстные footer
подсказки настоящих native bindings. Все действия Random/DNA/Auto/Guide/Back/
Quit/Continue сохраняются. Для размещения подсказки и существующих кнопок
birth footer занимает две строки; draw и pointer input используют общий layout.


## Проверки

- Linux build — PASS. Действующий старый процесс удерживал executable inode;
  новая сборка установлена атомарной заменой файла без прерывания текущего клиента.
  В уже открытом процессе остаётся прежняя версия до перезапуска.
- `python3 tests/sv_endpoint_checks.py` — PASS.
- Existing production birth subset `SV_BIRTH_CASE=stats-point-mouse,stats-order-mouse,guide-all-owners,birth-context-escape python3 tests/sv_name_editor_submit_checks.py`
  — PASS в обоих существующих class/race orders, ASan/UBSan.
  Новые suites и core tests не добавлялись; изменения presentation-only.
- birth.c/native-birth.c byte-identical исходному baseline этой правки;
  protocol/login.c hash unchanged. Проверены draw/input shared rectangles.
- [Независимое ревью](evidence/sv-b2-summary-followup/review.md) — без открытых
  замечаний. Исправлены обрезание 18/100, внешний clip рамки и подсказки,
  которые ранее обещали недоступные команды.
- Production renderer snapshots: [имя](evidence/sv-b2-summary-followup/after-name-wide.png),
  [граница при scale 1.5](evidence/sv-b2-summary-followup/after-name-fractional.png),
  [выбор пола](evidence/sv-b2-summary-followup/after-sex-wide.png),
  [раса 1024×768](evidence/sv-b2-summary-followup/after-race-min.png),
  [атрибуты 1024×768](evidence/sv-b2-summary-followup/after-stats-min.png),
  [длинные значения и прокрутка](evidence/sv-b2-summary-followup/after-stress-bottom.png).
  Модели синтетические, отрисовка production; это не вход реальным аккаунтом.

Граф обновлён. Приёмка пользователем остаётся открытой.
