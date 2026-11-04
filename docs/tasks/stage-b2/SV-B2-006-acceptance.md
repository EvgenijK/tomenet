# SV-B2-006 — Интеграция и visual/input приёмка B2

Статус: accepted, 2026-10-30. Этап B2 принят пользователем;
[итоговое решение](../../sv-stage-b2-acceptance.md). Все записи ниже об ожидаемой
приёмке сохранены как история до этого решения.

## История реализации и требования

Статус: implemented, corrections ready for user review, 2026-10-29.
[Исправления по замечаниям пользователя](../../sv-b2-006-corrections.md),
независимое code/visual review и применимые Linux проверки выполнены.
[Исходные замечания и reference screenshot](../../sv-stage-b2-acceptance.md#предварительные-замечания-пользователя--2026-10-29)
сохранены; пользовательская приёмка B2 ожидается.
После пользовательского запуска также исправлены и проверены
[blur, двойной клик и возврат Back](../../sv-b2-interactive-bugs.md).
Зависимости: SV-B2-002, SV-B2-003, SV-B2-004, SV-B2-005.
Существующие semantic consumers: Все B2 presentation consumers; ноль новых IDs.
Новых capability IDs: 0; B ownership и protocol semantics сохраняются.

Общий контракт: [B2 spec](../../sv-stage-b2-spec.md),
[reference и screen mapping](reference.md), [DAG](README.md).
Linux обязателен; Windows/MinGW/Wine deferred.

Уточнения пользователя от 2026-10-29 для текущих корректировок:
World Chronicle сразу получает данные из сетевых источников, а не ограничивается
offline snapshot; иконки ссылок сразу выполняются строго круглыми, без
приплюснутой нижней части. Эти требования заменяют прежние допущения reference.

## Дополнительные замечания пользователя — 2026-10-29

При сопоставлении скриншотов макета и клиента требуется: использовать исходные
иконки макета (включая SVG внутри строго круглых ссылок); обновлять World
Chronicle каждые 5 секунд; привести цвет и скругление основной панели к макету;
сделать края кнопок плавными; обеспечить клики по иконкам и тексту ссылок с hover;
убрать `Links unavailable`; привести размеры и положение подсказок хоткеев
к макету. Статус этой корректировки: implemented; [результат, ревью и проверки](../../sv-b2-design-followup.md). Приёмка пользователем ожидается.

[Следующие исправления экрана account](../../sv-b2-account-followup.md): стили формы, факелы, тексты, заголовок; implemented, пользовательская приёмка ожидается.

[Исправления Show/Hide и реального подключения](../../sv-b2-connect-followup.md): implemented, пользовательская приёмка ожидается.
[Клавиатура и возврат по Esc](../../sv-b2-keyboard-followup.md): implemented, пользовательская проверка одиночных нажатий ожидается.
[Character Overview](../../sv-b2-overview-followup.md): implemented, пользовательская приёмка ожидается.

## Пользовательский результат

Один production executable проходит оба startup routes с дизайном по reference;
визуальные отличия и actual input результат явно представлены для принятия B2.

## Работы

- Собрать reference/native side-by-side screen/state evidence по reference.md:
  shell/server/manual/account/overview/name/birth/children/MOTD/wait/error/handoff.
  Same logical viewport и сопоставимые synthetic display data; reference и build
  fingerprints, environment/fonts/backend/resources записаны в Markdown report.
- Проверить композицию/пропорции/palette/type/spacing/art/animation, readable text,
  hover/selected/focus/disabled distinctions и отсутствие overlap/clipping.
- Два коротких production controlled-peer routes: existing character и account/new
  character creation; распределить keyboard/mouse, edit/focus, cancel/back,
  scroll/resize и handoff suppression. Separate preview scene не итоговый executor.
- Linux software и OpenGL: основной route и representative scaled hit check для
  каждого backend. Не повторять весь state×backend×scale product.
- Размеры/scale bounded: все families при одном основном размере, minimum
  1024×768; representative affected layouts при 1920×1080 100%, 3840×2160 output
  200%, fractional 125%/150%. Virtual geometry честно маркировать; physical display
  timing и реальный OS IME из неё не следуют.
- Existing checks выбрать по фактическому diff; новые automated suites/harness
  не предписаны. Минимально необходимое расширение core tests согласовать заранее.
- Свести deviations и итог visual review в `docs/sv-stage-b2-acceptance.md`.
  Приёмка отражает реальный внешний вид; исторический B waiver не переписывается.

## Production seam и ограничения

Production `src/tomenet-sv`, existing peer/fixture seams, обычные local captures
и Markdown acceptance report. Новый runtime/test framework и canonical JSON
evidence/schema migration вне scope.

## Проверки и Definition of Done

Все reference mapping rows имеют результат и owner; существенные неперенесённые
элементы не скрыты как optional future work. Visual результат представлен и
решение о B2 recorded. B остаётся accepted; B2 не заявляет real server/gameplay,
не возобновляет снятые проверки B и не меняет canonical allocation.

Общие правила: минимальные SV-local changes; никакой отдельной реализации
поведения только для тестов. Дополнительные improvements фиксируются отдельно
в `docs/sv-improvements.md`, а не включаются автоматически в эту задачу.

Follow-up 2026-10-30: [экран выбора имени и штатный E_NEED_INFO при создании](../../sv-b2-creation-followup.md). Linux проверки/ревью завершены; пользовательская приёмка остаётся открытой.

Follow-up 2026-10-30: [единая сводка, initial preview10, рамки и birth hotkeys](../../sv-b2-summary-followup.md). Linux проверки/ревью завершены; пользовательская приёмка открыта.

Follow-up 2026-10-30: [credentials editing, sex Back, class grid и оформление](../../sv-b2-editing-followup.md). Linux checks/review PASS, пользовательская приёмка открыта; physical cause первоначального Backspace не доказана.

Follow-up 2026-10-30: [race/trait hotkeys/grid и подтверждённый native keyboard clock guard](../../sv-b2-race-trait-followup.md). Физический credentials Enter-сценарий пользователь подтвердил исправленным; общая B2 visual acceptance остаётся открытой.
