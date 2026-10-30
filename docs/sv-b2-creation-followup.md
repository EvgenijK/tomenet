# Создание персонажа: имя, экран и ответ сервера

2026-10-30. Follow-up B2-006, приёмка пользователем открыта.

## Ошибка после Next

`src/server/nserver.c` Receive_login отправляет SUCCESS=0 для существующего
персонажа и E_NEED_INFO=3 для нового. `src/client/c-init.c` по E_NEED_INFO
запускает get_char_info. SV WAIT_STATUS принимал только 0 и ошибочно отклонял 3.
Теперь этот штатный ответ переводит SV к birth, сохраняя остальные отказы.
При исчезнувшей строке overview серверный запрос birth сохраняет выбранное имя.

Минимальный существующий core birth test использует реальный E_NEED_INFO вместо
0; negative status fixture использует E_PASSWORD=5. RED на production baseline,
GREEN после исправления. Controlled endpoint с синтетическими credentials прошёл
имя → E_NEED_INFO → birth → точный baseline PKT_PLAY (16266 bytes) → server ACK →
live-session-handoff. VERIFY password проверен на синтетическом значении.
Реальные аккаунты и серверные записи персонажей не менялись.

`GetAccount: New account has no password` в данном пути — побочное сообщение
проверки коллизии нового имени с именами аккаунтов. `check_account` вызывает
GetAccount для настоящего аккаунта, затем для имени персонажа, с NULL password,
в режиме lookup. Для отсутствующего аккаунта с именем персонажа GetAccount
печатает это сообщение. Это не доказательство потери пароля или отказа birth.
Legacy/server не менялись.

## Случайные имена

Проверены таблицы в `src/client/c-birth.c:create_random_name` и
`src/client/sv/input/login-interaction.c:random_name`: 65 начальных, 34 средних
и 70 конечных слогов полностью совпадают, включая порядок и дубликаты.
Оба варианта выбирают по одному слогу из каждой таблицы и соединяют их.
Полной идентичности RNG нет: legacy вызывает rand_int/Rand_div, SV использует
локальный xorshift32 и остаток от деления. Последовательности имён при одинаковом
seed не совпадают. В текущем исправлении генератор не изменялся.

## Оформление

Экран выбора имени приведён к reference: тёплая полоса шапки, subtitle/ornament,
декоративная сводка, отдельные строки выбора и шесть карточек характеристик,
поле имени/Random/Continue/Back и hotkey footer. Значения, ещё не выбранные и
недоступные на name step, не выдумываются: карточки характеристик показывают --.
Нормативным источником являются reference HTML/CSS и предоставленный screenshot;
игровая семантика определяется production birth.


## Проверки и evidence

- Linux build `make -f makefile.sv tomenet-sv -j4` — PASS.
- `python3 tests/sv_login_checks.py` — PASS, core protocol/input/pregame.
- `python3 tests/sv_endpoint_checks.py` — PASS, production SDL scene/native input.
- [Ревью](evidence/sv-b2-creation-followup/review.md) — без открытых замечаний;
  исправлены intermediate-height overflow summary, shared intro width mismatch
  и footer hint truncation при 150%. При нехватке высоты карточки вписываются
  в доступную область, сохраняя все шесть значений.
- Production draw: [до](evidence/sv-b2-creation-followup/before-wide.png),
  [после 2560×1600 / scale 4/3](evidence/sv-b2-creation-followup/after-wide.png),
  [1024×768](evidence/sv-b2-creation-followup/after-min.png),
  [1440×900 с длинным именем](evidence/sv-b2-creation-followup/after-mid-draft.png),
  [1920×768 с длинным именем](evidence/sv-b2-creation-followup/after-wide-short-draft.png),
  [scale 1.5](evidence/sv-b2-creation-followup/after-fractional.png).
  Это controlled renderer snapshots с корректным overview + name-entry view.
  Ошибочные ранние scratch captures pending-состояния заменены и evidence
  не являются. Фон заморожен и logo reduced motion; это не изменение анимации.

Birth geometry сохранена: reclaim compact footer перенесён в существующий
name shell helper, а прежнее идентичное прибавление в birth layout удалено.
Input и renderer используют общую геометрию. Отдельное поведение SUCCESS=0 при
ручном вводе существующего имени подтверждено и записано в sv-improvements;
исправление E_NEED_INFO не заявляется как исправление всех вариантов имени.

Финальный controlled endpoint повторён после visual freeze: [полный production прогон](evidence/sv-b2-creation-followup/creation-final.log) — PASS, live-session-handoff Carol. Клиент пересобран и запущен.
