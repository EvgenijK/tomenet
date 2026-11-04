# B2 — оформление атрибутов и удержание ввода

Дата: 2026-10-30
Статус: реализовано; пользовательская проверка открыта.
Labels: bug, ready-for-human

## Запрос

Убрать отмеченный левый маркер карточки тела. Привести point-buy атрибуты
к предоставленному reference screenshot и frozen variant D HTML. При удержании
стрелок либо +/- повторять изменения, в том числе для mouse +/-.

## Причины и исправления

- Маркер тела был добавлен предыдущим follow-up; у body удалён.
- Native adapter отбрасывал все KEY_DOWN repeat. Теперь повтор разрешён только
  для point-buy navigation/adjustment; Enter, переходы выбора и popup confirmation
  не повторяются. Стоимость, доступные очки и пределы контролирует production core.
- Mouse DOWN раньше изменял атрибут только один раз. SV viewport хранит только
  gesture/timing, frame tick через production scene вызывает существующий command:
  задержка 350 ms, далее один шаг каждые 65 ms, без catch-up bursts. Tick выполняется
  после обработки очереди, чтобы queued button-up/focus-loss остановили повтор.
  Release, уход указателя с кнопки, focus loss, resize, смена owner/highlight,
  generation и pending confirmation прекращают удержание. Guide очищает hold.
- Таблица: title с оставшимися очками и reference guidance; реальные class/race
  в заголовках, колонки Base/modifiers/Current/controls/recommendation, крупные
  значения, signed modifier colors, разделитель и selected-row marker, горизонтальные
  границы вместо рамок каждой строки. +/- рисуются и hit-test используют общие
  прямоугольники возле Current. Detail использует serif heading и brown copy.
  На 1024×768 список прокручивается; Current 18/40 показывается полностью.

## Проверки

Linux build `make -C src -f makefile.sv tomenet-sv -j4` — PASS.
Минимальная новая production core regression `tests/sv/birth-stats-repeat.c`
подключена в существующий endpoint runner. Отдельный clang strict + UBSan — PASS.
Проверяет keyboard repeat, timer delay/cadence, mouse +/- и release/focus/generation/
pointer cancellation, запрет повторного подтверждения popup. До исправления
Right+repeat давали 10→11 вместо 12 (assertion failed). После — 10→12.
Прежняя core regression body n/f также PASS. Полная matrix не запускалась.

[Core](../../evidence/sv-b2-stats/core-green.log),
[body n/f](../../evidence/sv-b2-stats/body-green.log).
Production renderer просмотрен на synthetic view в software/dummy, не live server:
[атрибуты wide](../../evidence/sv-b2-stats/stats-wide.png),
[1024×768](../../evidence/sv-b2-stats/stats-min.png),
[scale1.5](../../evidence/sv-b2-stats/stats-fractional.png),
[body без полосы](../../evidence/sv-b2-stats/body-wide.png).
Windows отложен по текущему правилу платформ. Legacy/common не изменялись.
