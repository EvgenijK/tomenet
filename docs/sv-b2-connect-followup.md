# B2-006 — Show/Hide и ошибка подключения

2026-10-29. Исправлено, code review и Linux проверки прошли. Пользовательская
приёмка B2 ожидается. Baseline этой итерации —
`/tmp/sv-b2-connect-followup-baseline.tar`, отдельно от накопленного Git diff.

## Show/Hide

Удалён отдельный прямоугольный focus border. При mouse toggle и клавиатурном
фокусе меняется цвет текста, без рамки вокруг Show/Hide. Toggle, keyboard routing,
private password draw, очистка и запрет capture при reveal сохранены.

Production executable / OpenGL с synthetic SDL событиями: выбрать сервер,
ввести синтетические данные, Show → Hide → Show, сохранить masked кадр, Back,
Quit. До исправления контрольный пиксель рамки `(201,170,77)` — RED; после
обычный фон `(4,6,5)` — GREEN. OS Guide boundary из переиспользованного диагностического
сценария перехвачен, реальные вкладки не открываются.
[До](evidence/sv-b2-connect-followup/show-before.png),
[после](evidence/sv-b2-connect-followup/show-final.png).

## Почему не подключался клиент

Реальный setup официального сервера содержит 32 trait slots, включая пустые
зарезервированные названия (13 и 16–31). SV decoder запрещал любую пустую строку,
из-за чего валидный ответ отвергался с `SV_DECODE_ERROR` ещё до account-login.

Теперь пустое название разрешено только в trait slot; индекс и choice mask
сохраняются. Race/class titles по-прежнему непустые, ограничения размеров и
проверки частичного пакета сохранены. Legacy/common не менялись.

Сохранённый настоящий ответ EU воспроизвёл ошибку. После исправления production
parser принимает его блоками 1, 2, 4, …, 4096 байт. Минимальная existing core
регрессия содержит пустые slots до и после именованного trait и проверяет IDs/masks.

## Понятная ошибка

Вместо общего `Server setup failed` карточка указывает host:port и различает
обрыв до получения настройки, неразбираемый ответ и превышение поддержанного
размера. Socket API отдаёт ограниченный код причины, без копирования credentials
или сырых пакетов в UI. Причина сохраняется значением после закрытия socket
provider; первоначальный UI probe обнаружил, что без этого EOF на следующих
кадрах заменялся текстом decode. Исправление проверено повторным TCP capture.
Retry теперь находится внутри карточки с отступом; hitboxes сохранены.
[Карточка EOF](evidence/sv-b2-connect-followup/setup-closed.png).

## Проверки

- Build и `python3 tests/sv_contact_checks.py` — PASS, ASan/UBSan.
- `python3 tests/sv_endpoint_checks.py` — PASS после окончательных правок.
- `python3 tests/sv_contact_live_checks.py` — PASS: controlled TCP, split packets,
  версии, setup/login/character handoff, malformed/EOF/connect/DNS/timeout.
  Existing fixture успешного peer теперь ждёт EOF клиента вместо фиксированных
  3.5/1.5 секунд, которые ошибочно вызывали разрыв при более дорогом рендере.
  Ожидания setup messages механически обновлены, новых сценариев/suites нет.
- Actual socketpair — EOF → `SV_CLOSED`, malformed → `SV_DECODE_ERROR`, strict
  `-Werror`/UBSan PASS. Исправлен выбор фазы для overflow без FAILED state.
- Реальные EU, NA, APAC, Arcade и kurzel.dev — `SV_SOCKET_READY`: 18 races,
  16 classes, 32 traits, 2760 MOTD bytes. Версии 4.9.4 / 4.9.3.
  Проверка останавливается до account-login: пароли пользователя не использованы,
  аккаунты не созданы; полный авторизованный вход реальным аккаунтом не заявляется.
- [Независимое ревью](evidence/sv-b2-connect-followup/review.md) без открытых
  замечаний. Source whitespace check PASS; Graphify обновлён. Временные probes
  находятся в `.scratch/`; production debug instrumentation нет.

[Binary fingerprint](evidence/sv-b2-connect-followup/binary.sha256).
Inbound preauth captures содержат публичные server setup/MOTD, без отправляемых
паролей. Synthetic input/captures используют virtual DPI 125% и offscreen OpenGL;
физические клики и physical display timing не заявляются.

Отдельно записано улучшение startup stderr: ambient SDL_GetError иногда даёт
старое/пустое сообщение после уже корректного semantic contact log. Оно не
подменяет исправленную UI причину; proposal остаётся в `docs/sv-improvements.md`.
