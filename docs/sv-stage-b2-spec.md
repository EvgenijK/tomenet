# Этап B2 — native интерфейс входа по веб-макету

Статус: in progress, 2026-10-28. Основание: прямой запрос пользователя
спланировать B2 после обнаруженного расхождения принятого B с HTML UX prototype.
B2-001 реализован ([evidence](sv-b2-001-evidence.md)), B2-002 реализован
([evidence](sv-b2-002-evidence.md)); B2-003…006 остаются planned.

## Результат и место в плане

Игрок видит в production `tomenet-sv` интерфейс подключения, аккаунта,
выбора/создания персонажа по зафиксированному login-макету, а MOTD — по своему
отдельно утверждённому прототипу. Переносятся композиция, графика, типографика,
палитра, элементы управления, состояния выбора/ошибки/ожидания и предусмотренное
макетом адаптивное поведение. Текущий экран из строк с фиксированными координатами
не является финальным результатом B2.

Порядок работ: **B (accepted) → B2 → C001–C064**. B2 — дополнительный
presentation checkpoint существующего B scope с нулём новых capability IDs.
Это не новый block allocation в `capabilities/stages.json`: A8/B54, owners B,
162 obligations и распределение C001–C064 сохраняются. Название B2 используется
в документах и implementation tickets; валидаторы canonical stage membership
не требуют регистрации нового acceptanceStage ради этого presentation follow-up.

B принят 2026-10-28 по [решению пользователя](sv-stage-b-acceptance.md#решение-пользователя--2026-10-28)
без ожидания оставшихся проверок. B2 не переоткрывает B и не возобновляет
снятые provider, IME или общие human-review gates. Приёмка изменённого B2 UI
проверяет его собственный внешний вид и работу элементов управления.

## Источники истины

- Внешний вид: зафиксированный `login.html` основного HTML UX prototype,
  итоговый вариант D; точные source fingerprints и screen mapping —
  [reference](tasks/stage-b2/reference.md).
- MOTD: [утверждённый вариант C](../.scratch/single-window-sdl3-client/issues/28-prototype-the-server-defined-motd-screen.md#answer),
  whole-window, сохранение server-defined композиции, центрирование и maximum
  uniform fit с 24 logical units отступа.
- Семантика и доступность действий: принятые production paths B, behavior baseline
  и canonical B owners. HTML fixture responses, статические персонажи и числовые
  параметры не заменяют server-authoritative данные.
- [CONTEXT.md](../CONTEXT.md), [архитектура SV](sv-architecture.md), ADR-0001…0006
  и [правило изоляции](../AGENTS.md) сохраняются. Подтверждённые HTML visuals
  нормативны; completeness mock-функциональности не является native gate.

Нельзя объявить HTML-контроль production-ready только потому, что он нарисован.
Для каждого visible control определяются production action, состояние
available/disabled/unavailable и обратная связь. Несуществующие outcome/backend
не добавляются молча ради совпадения с демонстрационными данными.

## Граница реализации

В scope входят существующие pregame surfaces, их navigation и feedback;
подробный screen/state список фиксирует reference. Presentation resources
(включая ASCII-art оформление и анимацию), шрифтовые роли, layout и hit testing
входят в те же задачи, что первый production consumer. Они не откладываются
в C006/C007 под видом общей font/rendering работы.

Real-server integration, post-MOTD startup, gameplay/HUD/map, settings UI,
character reorder, DNA persistence и полноценный Guide остаются в своих C
этапах. Guide entry B2 сохраняет `The guide is in development` и возврат
к действительному caller. Chronicle не создаёт обязательный live-news backend.

Работа по возможности ограничивается `src/client/sv/` и необходимыми SV resources.
Без общей UI-библиотеки, legacy/common рефакторинга, нового protocol path или
дублирования игрового поведения ради UI. Local presentation primitives допустимы.
Производные views читают session/interaction state; UI передаёт semantic actions.
Одни вычисленные logical rectangles используются rendering и hit testing.
Контекст ввода, draft, server slot IDs и session generation сохраняются при
resize/focus, смене renderer и возвращении с дочерней surface.

Presentation UTF-8 и wire field bytes — разные контракты. Новые текстовые и
шрифтовые примитивы не расширяют допустимые отправляемые account/name/password
bytes. Masked field не выдаёт raw secret в public view или evidence. Явный Show
использует только private render-only доступ, не retained render cache;
при уходе reveal сбрасывается, captures остаются masked.

## План работ

Implementation DAG, owners deliverables и Definition of Done находятся в
[индексе B2](tasks/stage-b2/README.md). Для каждой screen family есть явный
implementation ticket и ссылка на web selectors/reference, а final gate
проверяет перенос дизайна отдельно от успешного protocol flow.

## Приёмка

Текущая обязательная платформа — Linux, software и OpenGL, согласно
[правилу платформ](agents/platform-validation.md). Windows/MinGW/Wine deferred.

Принимается native production executable с controlled peer, без отдельной
preview scene как единственного результата. Web reference и native captures
сопоставляются при одинаковых logical размерах и состояниях. Сравниваются
композиция, размеры/пропорции панелей, typography roles, цвета, graphics,
selection/focus, scrolling и disabled/error/wait feedback. Pixel-perfect
сравнение browser и SDL не требуется; заметно другая композиция или отсутствие
основных элементов макета не оправдывается проходящими core tests.

Минимальные текущие проверки и формат visual evidence описаны в final gate.
Переиспользуются существующие B production-path сценарии и только необходимые
core regression checks затронутой логики. Новый harness, expanded suites,
exhaustive Cartesian matrix и stress/soak этим планом не вводятся. Расширение
проверок согласовывается с пользователем по AGENTS.md.

B2 не завершён, если протокол работает, а внешний вид остаётся прежним текстовым
экраном. Итог представляется как сопоставление reference/native screens со
списком известных отклонений, а не только перечень прошедших тестов.
