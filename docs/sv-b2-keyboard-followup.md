# B2-006: клавиатура и возврат из списка персонажей

2026-10-29. Исправления подготовлены; пользовательская приёмка остаётся открытой.

В SV server list одиночные Up/Down теперь распознаются по scancode, если SDL
передаёт `SDLK_UNKNOWN`. Повтор стрелок также перемещает выбор; повтор Enter,
Tab и Escape не активирует действия. Пользователь сообщил о пропусках именно
одиночных и последовательных нажатий, поэтому удержание не считается объяснением
его исходного сбоя. Macro encoder и legacy/common не изменены.

В форме учётной записи Tab распознаётся также по физическому scancode, а
`SDLK_LEFT_TAB` выполняет обратный переход. Обычный `SDLK_TAB` проходил прежние
проверки. Production core воспроизводит пропуск одиночных стрелок и Tab с
неизвестным логическим кодом до исправления и проходит после. Фактический SDL
trace пользовательского сеанса не получен: связь этих вариантов событий с
наблюдаемой прерывистостью пока не доказана.

Escape из character overview теперь возвращает список серверов в том же окне.
Соединение закрывается, пароль очищается, заимствованные представления и состояние
ввода сбрасываются. Выбранный сервер и фоновые providers сохраняются. Закрытие
окна и SDL_QUIT по-прежнему завершают приложение.

Проверки Linux:

- `python3 tests/sv_endpoint_checks.py` — PASS, включая native/sticky input,
  credentials и SDL scene; UBSan.
- `python3 tests/sv_login_checks.py` — PASS; `python3 tests/sv_contact_checks.py`
  — PASS.
- Минимальные существующие core-проверки: baseline RED → исправление GREEN.
  Логи: `.scratch/sv-b2-keyboard-followup/input-red-green.log`.
- Локальный TCP contact/login/overview production-путь: Esc возвращает LIST,
  window ID сохраняется, peer закрыт, character request не отправлен. Отдельно
  проверены SDL_QUIT и закрытие окна. Диагностические probes остаются в
  `.scratch/character-back/`; новые suites не добавлены.
- Независимое ревью без открытых замечаний:
  `.scratch/sv-b2-keyboard-followup/review.md`.

Общий статус B2-006 не меняется на accepted до пользовательской проверки.

Дополнение: Escape и Cancel в форме точного адреса возвращают `MANUAL → LIST`,
сохраняя список и выбранную строку. Прежний переход `MANUAL → CANCELLED`
закрывал приложение. Host/port и ошибка адреса очищаются; Escape непосредственно
в списке сохраняет прежнюю семантику. Существующие core/native проверки отмены
обновлены: production core RED до правки, GREEN после, UBSan. Полный
`tests/sv_endpoint_checks.py` — PASS. Лог: `.scratch/sv-b2-manual-escape/checks.log`.
