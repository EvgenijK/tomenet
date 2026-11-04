# B2-006 — корректировка по сравнению макета и клиента

2026-10-29. Исправления реализованы; независимое ревью и Linux проверки прошли.
Пользовательская приёмка B2 ожидается. Изменения этой итерации изолированы в SV
и его ресурсах. Сравнение велось с исходным состоянием из
`/tmp/sv-b2-design-followup-baseline.tar`, а не со всем накопленным Git diff.

## Результат по замечаниям

| Замечание | Исправление |
|---|---|
| Отличаются иконки | Сохранены исходные SVG paths/viewBox из frozen `login.html`; исправлены CSS размеры 28/27/26, fill/stroke и окончания линий. Дверь использует исходную SVG-графику. |
| Нет live истории | Один background worker запускает обновление девяти HTTPS источников каждые 5000 ms. Долгий запрос завершается перед следующим циклом, без наложения. |
| Цвет подложки | Оттенок панели откалиброван по присланному скриншоту: native sample был `(5,8,6)`, стал `(12,15,12)`, reference `(12,16,13)`. Восстановлены общий градиент затемнения и золотистая радиальная подсветка. |
| Нет скруглений панели | Основная панель получила радиус 3 logical units. |
| Неплавные края кнопок | Скругления карточек и кнопок двери рисуются геометрией с alpha fringe шириной в один output pixel для software/OpenGL. |
| SVG внутри кругов отличаются | Убраны лишний stroke у заполненных Discord/GitHub и прежнее увеличение до 30; круги остаются строго круглыми по предыдущему требованию. |
| Ссылки не работают, нет hover | Круг и подпись вызывают `SDL_OpenURL` с адресом из макета. Hover меняет цвет значка, кольца, фона и подписи, сдвигает ссылку на 2 logical units. Работа доступна также на account/character surfaces; modal блокирует фоновые ссылки. |
| Links unavailable | Текст удалён. Ошибка системного открытия URL записывается в лог. |
| Подсказки хоткеев | Восстановлена строка `↑ ↓ select · a–e select directly · Enter connect · press Q for direct connection`, текущая последняя буква следует числу серверов. Q в строке сохраняет mouse/keyboard действие Direct. |

Успешная загрузка истории заменяет записи конкретного источника; ошибка сохраняет
его последние данные. Latest 100 пересобирается из ограниченных per-source caches,
поэтому повторные загрузки не накапливают дубликаты. TLS, body/time limits,
main-owner snapshot и неблокирующая отмена сохранены.

## Ревью и проверка

[Независимое ревью](evidence/sv-b2-design-followup/review.md) нашло ошибку Q при
125%: FreeType менял ширину подсказок и переносил их, а hitbox оставался на прежней
строке. Исправлено общей логической геометрией отрисовки и input; растровая ширина
только этой строки нормализуется к общим advances, renderer scale восстанавливается.
Открытых замечаний к коду в текущем diff нет.

- `make -C src -f makefile.sv tomenet-sv -j4` — PASS.
- `python3 tests/sv_endpoint_checks.py` — PASS после окончательного исправления.
  Минимальная регрессия в existing native core test проверяет все 6 URL, клики
  по кругу и подписи, hover, блокировку modal, обычные и sticky keys. OS URL
  launcher перехвачен на границе; реальные вкладки браузера тест не открывает.
- `SDL_VIDEODRIVER=dummy python3 tests/sv_shell_smoke.py --backend software` — PASS.
- Scratch probe вызывает production worker/parser: RED — один batch вместо трёх;
  GREEN — 0/5/10 s, замена без дублирования, сохранение при отказе. Публичный
  start/poll/stop подтверждает повторные циклы и отмену 0 ms.
- Реальная HTTPS проверка: все 9 источников готовы, 100 записей; начала повторных
  циклов на 5023/10049/15060 ms, данные остаются видимыми при загрузке, stop 0 ms.
  См. [сетевой лог](evidence/sv-b2-design-followup/chronicle-live-network.log).
- Software render 2560×1600/1024×768 и OpenGL offscreen 1440×900 проверены визуально.
  Штатный драйвер desktop также успешно стартовал и сохранил кадр.
- Дополнительные production captures с инъекцией SDL mouse events: hover ссылки,
  клик Q на software, клик Q при виртуальном DPI 125% на OpenGL, output 1280×960 /
  logical 1024×768. Последний кадр действительно показывает Direct connection.
  Это изолированная проверка geometry/input, а не физического дисплея.
- Manifest ресурсов: 57 SHA256 записей, ни одного несовпадения; исходные 19 PNG
  фонов не изменены. Graphify обновлён: 18260 nodes / 47773 edges.

Реальный X11/XTest replay в этой итерации не завершился: test window на текущем
Xwayland зависал до первого кадра; `import` и XGetImage также не могли снять окно.
Эти попытки не засчитаны как PASS. Зависший собственный probe остановлен; вместо
него использованы штатный desktop driver, offscreen GPU и production SDL events.
Скрипты и временная инъекция остаются только в `.scratch/`; production debug-код
не добавлялся. Новые test suites/frameworks не вводились.

## Визуальное evidence и границы

[Присланный макет](evidence/sv-b2-design-followup/user-mockup.png),
[присланный клиент до исправления](evidence/sv-b2-design-followup/user-client-before.png),
[после исправления 2560×1600](evidence/sv-b2-design-followup/after-final.png),
[1024×768](evidence/sv-b2-design-followup/after-small.png),
[OpenGL hover](evidence/sv-b2-design-followup/opengl-hover.png),
[Q при 125%](evidence/sv-b2-design-followup/opengl-q-125.png).

Captured fixture data, шрифтовый raster и DPI могут отличаться от присланного
browser screenshot. Dummy/offscreen captures без сетевого доступа показывают
unavailable историю; успешная live загрузка доказана отдельным сетевым логом.
Ресурсы и код не подставляют mock telemetry вместо данных сервера.
Browser panel backdrop-filter blur(10px) отдельно не воспроизведён; цвет подложки
скорректирован по screenshot. CSS glow/drop-shadow SVG wrappers также не являются
pixel-perfect копией; геометрия и размеры исходных значков сохранены.

Клиент повторно запущен в штатном полноэкранном режиме после проверок.
[Кадр пользовательского запуска с live данными](evidence/sv-b2-design-followup/user-launch.png),
output 2560×1601; повторные источники в текущем запуске также отвечают ready.

[Fingerprint executable](evidence/sv-b2-design-followup/binary.sha256).

## Уточнение: обрезанная граница карточек серверов

2026-10-29, после следующего скриншота пользователя. Исправлен горизонтальный
clip списка: координаты округляются наружу, с запасом в один output pixel
для сглаженной рамки. Ранее отдельное приведение x/width к int срезало правую
линию при дробном DPI. Геометрия карточек и input не менялись.

Production render при virtual scale 4/3, output 2560×1600: baseline проверка
правой прямой линии FAIL (RGB sum 41 — подложка), исправленный software PASS
(89), OpenGL PASS (147). Проверены 82 последовательных пикселя линии. Первоначальная
проба брала соседний столбец вне raster stroke; итоговая проверка использует
фактический столбец рамки x1824. Сборка PASS, независимое ревью без замечаний;
новых постоянных тестов нет. [До](evidence/sv-b2-border/before.png),
[после software](evidence/sv-b2-border/after.png),
[после OpenGL](evidence/sv-b2-border/after-opengl.png),
[новый fingerprint](evidence/sv-b2-border/binary.sha256).

## Уточнение: полоса выделения и серверные метаданные

2026-10-29. Удалён отдельный прямоугольный маркер выделения слева (3 logical
units). Золотая рамка и фон выбранной карточки сохранены. Production capture
до/после при virtual scale 4/3 подтвердил удаление лишней заливки: контрольный
пиксель `(221,189,78)` заменён обычным selected background `(38,34,15)`.
[До](evidence/sv-b2-server-details/selected-before.png),
[после](evidence/sv-b2-server-details/selected-after.png).

Причина отсутствия версий/онлайна: парсер метасервера извлекал только атрибуты
host/port/protocol и пропускал дочерние game/version/notes/player. Теперь эти
поля проходят через SvServer в UI. Известные 0 игроков отличаются от неизвестного
онлайна; player names ограничены по размеру, полное число подсчитывается отдельно.
XML entities декодируются; каждый набор metadata ограничен своим server element.
Публикация и обновление остаются в supplementary provider, без legacy изменений.

Один worker обновляет feed и ping с целевым интервалом 5 секунд; медленный цикл
заканчивается перед следующим, без наложения. Ошибка оставляет последние данные,
выбор сохраняется по host+port. Версия и протокол — разные поля; protocol
подключения не менялся. Static file fixtures не подменяют неизвестный онлайн нулём.

Проверки: минимальная existing core регрессия metadata RED→GREEN; сборка и полный
`python3 tests/sv_endpoint_checks.py` PASS, strict `-Werror` и UBSan. Реальный provider
за 12 секунд опубликовал 18 snapshots, сохранил selected index 2, отмена 0 ms.
Независимое [ревью](evidence/sv-b2-server-details/review.md) без замечаний.
На момент проверки официальный feed дал EU 4.9.4 / 2 игрока, Arcade 4.9.3 / 0,
NA 4.9.4 / 1, APAC 4.9.4 / 0, kurzel 4.9.3 / 0; это реальные изменяемые данные.
[Feed](evidence/sv-b2-server-details/metaserver.xml),
[provider log](evidence/sv-b2-server-details/meta-live.log).

Production frames с реальной сетью:
[software 2560×1600](evidence/sv-b2-server-details/live-software.png),
[OpenGL 1280×960 / logical 1024×768 при виртуальном DPI 125%](evidence/sv-b2-server-details/live-opengl-small.png).
Видны фактические версии, число и имена игроков. Использована временная SDL
инъекция mouse selection из `.scratch/`, без production debug changes и без
заявления о физических кликах. [Новый binary fingerprint](evidence/sv-b2-server-details/binary.sha256).
Пользовательская приёмка B2 ожидается.
