# B2 — тело, пол и переход от имени

Дата: 2026-10-30
Статус: реализовано; пользовательская проверка открыта.
Labels: bug, ready-for-human

Пользователь сообщил: n/f не подтверждают тело; оформление пола и тела отличается
от макета; при переходе от имени к полу форма кратко исчезает. Повторный пункт
про n/f объединён. Видео: `/home/svechnik/Videos/OBS/2026-10-30_16-34-04.mp4`.

## Исправление

- Native body shortcut вызывает существующий select_and_confirm, подтверждая
  только получивший событие owner. Повторы и недоступный Fruit bat игнорируются.
- До ответа сервера на имя сохраняется name surface с отключённым редактированием
  и сообщением ожидания. До исправления production draw на WAIT_STATUS показывал
  overview вместо формы создания; это видно также в пользовательском видео.
- Sex/body: reference intro15, карточки с radius4, title14/copy13.5, выбранный
  левый маркер, цвета, вертикальное центрирование, высота по переносу copy.
  У body удалена дополнительная detail surface, отсутствующая в reference.
  Источник: внешний UX prototype `/home/svechnik/Projects/github_site/tomenet_interface/login.html`,
  commit `1bb2d5d45b9001bf2a6f2084f78184c29a73fca7`, variant D.

Legacy и общие модули не изменялись.

## Проверки Linux

- `make -C src -f makefile.sv tomenet-sv -j4` — PASS.
- Минимальная production core regression `tests/sv/birth-body-hotkeys.c` —
  clang strict + UBSan PASS. Подключена в существующий endpoint runner.
  До исправления: `body n: owner=5 confirmed=0`, assertion failed.
  После: n/f → STATS, body_confirmed; stats не подтверждены; first-run f ignored.
- Точечный production replay из `.scratch/b2-body-sex/build.py`/`endpoint.c` —
  PASS. Observer `sv_endpoint_draw` до исправления ловит pending name retained=0;
  после — retained=1 на каждом ожидающем кадре. Replay вводит natively paced keys,
  намеренно старые keyboard timestamps, затем sex/class/race/trait/f/stats/mode.
  Финальные 16266 bytes совпали с `tests/sv/fixtures/birth-current.hex`; полный ACK
  переводит в live-session-handoff. Учётные данные тестовые, peer на loopback.
  Для разрешённого Fruit bat профиль содержит `sv/tomenet.cfg`.
- Renderer captures 1440×900 и 1024×768 просмотрены. Synthetic SvBirthView через
  production renderer, software/dummy; не создание персонажа на реальном сервере.
- Полная историческая matrix не запускалась, Windows отложен по правилу платформ.

[Core log](../../evidence/sv-b2-body-sex/core-green.log),
[переход до](../../evidence/sv-b2-body-sex/transition-red.log),
[переход после](../../evidence/sv-b2-body-sex/transition-green.log).

[Пол 1440](../../evidence/sv-b2-body-sex/sex-1440.png),
[пол 1024](../../evidence/sv-b2-body-sex/sex-1024.png),
[тело 1440](../../evidence/sv-b2-body-sex/body-1440.png),
[тело 1024](../../evidence/sv-b2-body-sex/body-1024.png).
