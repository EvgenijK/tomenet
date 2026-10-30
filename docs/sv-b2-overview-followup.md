# B2-006: Character Overview — замечания по оформлению

2026-10-30. Основание — два пользовательских скриншота: макет и текущий клиент.
Нормативный источник оформления — `lib/xtra/sv/login/reference/login.html` и его CSS.

Независимое сравнение выявило расхождения шапки и footer, слишком раннюю колонку
race/class, прямоугольные карточки вместо скруглённых, упрощённые ромбы уровней
и свободных слотов, отсутствие пунктирных рамок, выравнивания mode/location и
ограничения длинного текста. Различия числа персонажей, их имён и серверных
данных не относятся к дефектам оформления.

Дополнительные ограничения:

- Режим `0x0044` должен представляться как IDDC согласно authoritative mode
  flags; протокол и игровые значения не меняются.
- Подсказка S/I/A сортировки из макета не переносится: такой action ещё не
  реализован в SV, а буквы используются для выбора персонажей. Возможность
  отдельно записана в `docs/sv-improvements.md`.
- Back в overview использует существующий возврат к серверам, Q сохраняет выход
  из приложения. Mouse и keyboard activation Back проверены в существующем
  production core-тесте: RED до правки, GREEN после.

Пользовательская приёмка B2-006 остаётся открытой.

## Результат и проверки

Исправлены цвет overview surface, тёплая полоса шапки, title/subtitle/ornament,
положение колонок, rounded selection с градиентом, уровневые ромбы, centered
uppercase mode badges, right-aligned location, пунктирные скруглённые свободные
слоты и их маркеры. Footer использует source-sized Back/Create/Exclusive/Play
и actual count. При узкой панели статистика переносится во вторую строку;
причины disabled actions показываются при hover/focus. Число слотов и доступность
действий остаются серверными данными.

Длинные строки получают ellipsis внутри своих tracks. Вложенный clip пересекается
с clip списка и затем восстанавливается; пустое пересечение и невозможность
уместить ellipsis не вызывают рисования. Внешняя граница списка сохраняет AA
fringe. Геометрия ordinary-name branch сохранена отдельно.

- Linux build и `python3 tests/sv_endpoint_checks.py` — PASS, включая production
  input, credentials, native/sticky и SDL scene; UBSan.
- `python3 tests/sv_login_checks.py` — PASS. Existing resize invariant использует
  актуальное `viewport.rows` после sync, который может изменить scroll offset,
  вместо устаревшего `layout.rows`, вычисленного до sync.
- [Независимое ревью](evidence/sv-b2-overview-followup/source-review.md) — без
  открытых функциональных замечаний; замечания clipping/empty text/mode cases
  исправлены и повторно проверены.
- Production `sv_endpoint_draw` captures: [до](evidence/sv-b2-overview-followup/before-13-scale4-3.png),
  [13 персонажей](evidence/sv-b2-overview-followup/after-13.png),
  [9 персонажей](evidence/sv-b2-overview-followup/after-9.png),
  [1024×768, длинные строки](evidence/sv-b2-overview-followup/after-minimum.png).
  Большие кадры — 2560×1600 с virtual display scale 4/3, software renderer;
  минимальный — scale 1. Это controlled presentation snapshots, не утверждение
  о повторном входе реальной учётной записью. Chronicle отсутствует в snapshot
  model, поэтому его placeholder вне overview не оценивается этими кадрами.

Используется доступный native serif fallback; вес цифр уровня остаётся небольшой
адаптацией шрифта относительно Georgia 700 в HTML. Pixel-perfect совпадение
не заявляется. Legacy/common и сетевой протокол не менялись. Граф обновлён.


## Уточнения по выбранной строке и exclusive creation, 2026-10-30

Удалена отдельная полоса selected row слева. Ромбы уровней и свободных слотов
теперь рисуются geometry с полупрозрачной AA-кромкой; clip учитывает выступающие
вершины. Подсказка exclusive creation измеряется целиком вместе с группой Q,
при недостатке ширины переносится в две строки без сокращения текста.

Причина блокировки exclusive creation: отсутствие локального `sv/tomenet.cfg`
считалось первым запуском даже после серверного подтверждения существующих
персонажей. В SV этот признак теперь снимается при получении непустого overview,
согласованно в input interaction и pregame для последующего birth. Это намеренное
уточнение поведения по запросу пользователя: legacy E guard также жёстко проверяет
`firstrun`, хотя его отображение учитывает `existing_characters`. Legacy/common не
менялись. Серверные ограничения вместимости, dedicated modes и creation support
сохранены. Для пустого аккаунта на новом профиле прежнее ограничение остаётся:
по пустому overview возраст аккаунта определить нельзя.

Минимальная core regression в существующем login-interaction тесте вызывает
production parser/input/pregame, проверяет пустой/непустой аккаунт на новом профиле,
доступные dedicated modes и начало E creation. На baseline — RED, после правки —
GREEN. Linux login/endpoint suites, сборка и независимое ревью прошли.

Evidence: [2560×1600](evidence/sv-b2-overview-defects/after-wide.png),
[1024×768](evidence/sv-b2-overview-defects/after-min.png),
[scale 1.25](evidence/sv-b2-overview-defects/after-125.png),
[ромб до](evidence/sv-b2-overview-defects/before-diamond-detail.png),
[ромб после](evidence/sv-b2-overview-defects/after-diamond-detail.png),
[ревью](evidence/sv-b2-overview-defects/review.md).
Это synthetic production-render captures, не вход реальной учётной записью.
Приёмка B2-006 пользователем остаётся открытой.
