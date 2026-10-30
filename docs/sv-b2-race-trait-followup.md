# Хоткеи, race/trait grid и clock guard текстовых полей

2026-10-30. Follow-up B2-006. Пользовательская приёмка всего B2 остаётся открытой.

## Подтверждённая причина первоначального сбоя редактирования

Проверка в настоящем Wayland-сеансе пользователя различила два пути:
открытие credentials мышью работает, открытие серверного входа клавишей Enter
приводит к игнорированию Backspace/стрелок/Tab. Раннее уточнение «не работает»
для мыши пользователь исправил на «работает»; далее проверен именно Enter.

В [trace до исправления](evidence/sv-b2-race-trait-followup/physical-before.log)
новые KEY_DOWN имеют корректные logical/scancode, repeat=0, активное поле/фокус,
но timestamp предшествует scene entry. Старый caller фильтрует их до вызова
редактора; свежий TEXT_INPUT проходит. Предыдущее расширение UNKNOWN key fallback
этот реальный случай не устраняло. Привязка причины к конкретному SDL/compositor
не доказана и не требуется для исправления ошибочного SV clock guard.

Вместо сравнения OS keyboard clock с SDL tick/session time введён маленький
SV-local pregame queue barrier. При передаче владения вводом SDL Pump обновляет
состояние, затем удаляются pending KEY_DOWN/UP/TEXT_INPUT/EDITING. Свежие события
после барьера принимаются независимо от native timestamp. Repeat, pointer clock
и generation identity/active guards сохранены. Барьеры стоят на list/manual,
credentials/contact, name start/cancel/submit, birth entry/owner transition,
Guide open/close. Каждый кадр очередь не очищается; USER/close/render/network
события сохраняются. Legacy/common/gameplay native-input не менялись.

После исправления пользователь повторил тот же Enter-сценарий и подтвердил:
«Да, работает сразу». [Trace после](evidence/sv-b2-race-trait-followup/physical-after.log)
показывает handler_before/after и переход Tab focus0→1 даже при before_begin=1.
Диагностика записывала только фиксированные нетекстовые клавиши, timestamps,
repeat/modifiers и focus/stage/булеву валидность cursor/anchor; текст, буквы,
пароли, их длины и позиции не записывались. Диагностическое окно закрыто,
запущен обычный обновлённый клиент без logging wrapper.

## Создание персонажа

Legal буквенные хоткеи sex/class/race/trait выполняют выбор и ровно одно
подтверждение текущего owner, затем переходят дальше. Стрелки только изменяют
highlight. Сохранены reserved Q/Ctrl+Q, mode H/h, layout/modifier/repeat guards,
проверка legality и перевод trait letter position в реальный server trait ID.
Class/race/trait используют фактические columns общего viewport: вертикальный
шаг columns, горизонтальный 1, с проверкой границ и пропуском нелегальных ячеек.

Race показан без лишнего generic intro, с округлёнными карточками pitch50.
Trait на широком экране имеет 3 колонки, reference intro и serif/brown detail.
Footer объясняет немедленный выбор буквой. Фиксированный font12 сохранён:
compact footer70 в две строки, wide46. Общая геометрия учитывает footer при
summary/detail/viewport sizing. Реальные IDs, наличие и значения выбираются
production setup, а не копируются из reference.

## Проверки

- Linux build через `tests/sv_endpoint_checks.py` — PASS; endpoint/native/sticky,
  credentials, SDL scene; UBSan. Минимальная новая regression в существующем
  native-endpoint core проверяет older-clock fresh keys и queue barrier.
- `tests/sv_login_checks.py`, `tests/sv_name_rejection_recovery_checks.py` — PASS.
- В двух существующих event fixtures добавлена пауза после открытия редактора:
  future-owner Escape/текст теперь подаются после перехода, а не заранее в одной
  очереди с открывающей клавишей. Набор сценариев не расширялся.
- Standalone strict production native core letters/grid/guards — PASS, UBSan.
- Controlled production name→birth→PLAY→ACK с timestamp1 для свежих клавиш,
  queued OLD/Enter на границе name — PASS; exact16266 legacy golden bytes.
  [Лог](evidence/sv-b2-race-trait-followup/name-birth-green.log).
- [Boundary review](evidence/sv-b2-race-trait-followup/clock-review.md),
  [birth/visual review](evidence/sv-b2-race-trait-followup/birth-review.md) — без
  открытых замечаний. После этих отчётов root отдельно записал успешную физическую
  проверку пользователя; промежуточная запись pending в report историческая.
- [Race wide](evidence/sv-b2-race-trait-followup/after-race-wide.png),
  [race minimum](evidence/sv-b2-race-trait-followup/after-race-min.png),
  [trait wide](evidence/sv-b2-race-trait-followup/after-trait-wide.png),
  [trait scale1.5](evidence/sv-b2-race-trait-followup/after-trait-fractional.png),
  [последний trait minimum](evidence/sv-b2-race-trait-followup/after-trait-last-min.png).
  Renderer captures используют synthetic view; это не вход/создание на реальном
  сервере. Runtime feedback использовал Test без настоящего пароля.

Старые full-birth fixtures, заранее ставящие команды всех owners одним batch,
целиком не перерабатывались и full matrix не запускалась. Текущий paced golden
replay проверяет production path, не альтернативную реализацию поведения.
