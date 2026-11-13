# Редактирование credentials, возврат к имени и class grid

2026-10-30. Follow-up B2-006; пользовательская приёмка открыта.

## Редактор логина/пароля

Credentials adapter распознавал fixed editing keys только по SDL logical key,
за исключением ранее исправленного Tab. Добавлен локальный fallback при
SDLK_UNKNOWN по scancode: Backspace/Delete, Left/Right, Home/End, Tab, Return/
KP Enter, Escape, Space и Ctrl+A/V/L. Известные logical keys и текстовый ввод
сохраняют прежний путь. Production macro/physical encoder не менялся.
Минимальная regression в существующем credentials core проверяет оба поля,
удаление, перемещение, выделение и Ctrl+A. Baseline RED → current GREEN.

Это воспроизводит пропуск такого SDL события; физическая причина именно
пользовательского первоначального пропуска Backspace остаётся неподтверждённой:
живой trace пользовательского ввода не собирался. Не заявляется доказательство,
что все возможные причины пропуска клавиш устранены.

## Назад с выбора пола

Ранее Back был недоступен в sex availability и игнорировался interaction.
Теперь Back/Backspace возвращают смысловой BACK_TO_NAME. Сервер уже принял имя
и находится в PLAY, поэтому повторный LOGIN в старое соединение не отправляется.
Endpoint завершает birth/socket, начинает новое поколение и аутентификацию
с прежними credentials, затем восстанавливает редактор с сохранённым именем.
Отдельный outcome отличает явный Back от отказа имени: повторного запроса
credentials для Back нет, существующий rejection/retry flow сохранён.

Controlled e2e отдельно для Backspace и кнопки Back подтвердили EOF старого
соединения, два contact, различающиеся generation, сохранённые draft/credentials
и отсутствие повторной формы. Реальные аккаунты не использовались.

## Класс и нижняя панель

Класс использует геометрию production viewport: вертикальный шаг равен числу
колонок, горизонтальный — одной позиции. Границы сохраняются, нелегальные
варианты пропускаются; keypad тоже сбрасывает focus в choices.
Видимы только Back/Continue по макету; Tab фокусирует только эти кнопки,
скрытые hit rectangles обнулены. Random/DNA/Auto/Guide доступны через реальные
контекстные keyboard bindings; подсказки не обещают недоступные действия.

Рекомендации собраны в компактный блок с отдельными цветовыми префиксами.
Карточки, позиции, serif detail heading, inline modifier badges и цвет описания
согласованы с reference. Сводка preview учитывает highlighted class/race
adjustments; изменяется только отображение. Для Warrior 14/8/8/12/12/9,
для Istar 5/13/10/11/8/11 в контролируемом setup.
Длинные названия и descriptions сохраняют перенос и прокрутку; вычисление
максимальной прокрутки использует ту же ширину, что и отрисовка.

## Проверки

- Linux build и `tests/sv_endpoint_checks.py` — PASS.
- `tests/sv_name_rejection_recovery_checks.py` — PASS.
- Existing `sv_name_editor_submit_checks.py` subset stats-point-mouse,
  stats-order-mouse, guide-all-owners в обоих class/race orders — PASS, ASan/UBSan.
- Standalone production core sex Back / class grid RED→GREEN; UBSan.
- [Независимое ревью](evidence/sv-b2-editing-followup/review.md) — без открытых
  замечаний. Исправлен найденный mismatch ширин измерения/отрисовки detail.
- [Класс wide](evidence/sv-b2-editing-followup/after-class-wide.png),
  [minimum](evidence/sv-b2-editing-followup/after-class-min.png),
  [Istar](evidence/sv-b2-editing-followup/after-class-istar.png),
  [последняя строка при scroll max](evidence/sv-b2-editing-followup/after-class-detail-bottom.png).
  Это production renderer с synthetic view, не вход реальной учётной записью.

Граф обновлён. Новая сборка установлена атомарно; ранее открытый процесс
остаётся прежней версией до перезапуска. Legacy/common не менялись.
