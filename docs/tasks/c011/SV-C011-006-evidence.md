# SV-C011-006 — минимальное Linux evidence и сверка объёма

Статус: **accepted** — Linux preparation/core, 2026-11-11, по решению пользователя.
Labels: enhancement
Owned IDs: 0; сверка пяти C011 outcomes и 18 obligations.
Зависимости: результаты 001–005; это evidence задача, не отдельный test framework.

## Работа

1. Использовать [coverage.json](coverage.json) как planning snapshot неизменённой
   canonical allocation. Для каждого obligation записать production путь,
   фактическое наблюдение, команду/артефакт и границу результата: checked,
   pending либо неприменимо с причиной. Snapshot сам по себе не evidence PASS.
   Pending собственной работы C011 блокирует техническое завершение этапа;
   перечень из 18 pending строк не является выполнением этой задачи. Если checked
   только core subflow, в той же строке перечислить pending remainder и его owner;
   не ставить checked целому lifecycle obligation по частичному результату.
2. Объединить необходимые связанные cases 001–005 в существующем Linux core
   подходе и managed peer: profile/effects/identity, preparation results и
   ownership, controls/unread/output boundary, redraw. Вызывать production SV;
   отдельная реализация семантики для тестов недопустима. Не дублировать уже
   принятое evidence C002–C010: проверить actual C011 caller и его результат.
   Существующий `tests/sv/first-session.c` пока не проходит нужную границу:
   `PEER_C010` выходит на overview, `PEER_SUCCESS` завершает existing selection
   после MOTD; они не дают полный PLAY → REPLY/PLAY → END и accepted identity.
   Минимально продолжить тот же managed peer до production preparation API,
   проверить PLAY/report bytes, завершение startup, сохранённый unread хвост
   и lifetime payload. Это необходимая core проверка C011, не новый live loop
   или отдельный simulator. Старый C010 PASS её не заменяет.
3. Записать revision/dirty state, применённые команды, bytes/order/generation,
   lifetime payload при success/failure/cancel/retry и наблюдаемые диагностики.
   Различать login startup milestone, preparation readiness и actual enter-game.
   Неподтверждённый live adoption/teardown/relogin C014 не выдавать за PASS.
4. Для profile отдельно отразить startup INS и audio projections, macro/input
   hooks и отсутствующих поздних consumers. Для redraw — реально проверенные
   projections и pending C012/C013. Ссылки на поздние задачи — trace обязательств,
   а не доказательство исполнения. Canonical descriptions/statuses не сужать
   ради завершения core; технический итог и пользовательская приёмка различаются.
5. Обновить индекс этапа фактическим результатом и перечислить оставшиеся
   вопросы/ограничения. Возможные улучшения вне scope записывать отдельно в
   [sv-improvements](../../sv-improvements.md), не включать в реализацию молча.

По [AGENTS](../../../AGENTS.md) разрешён только необходимый минимум core checks;
расширение проверок предварительно согласуется. Windows/MinGW/Wine отложены и
не блокируют текущую Linux приёмку. Подготовка этого плана не запускает тесты,
не заявляет implementation PASS и не принимает этап от имени пользователя.

## Обязательный технический результат

| Задача | Что должно быть реализовано и подтверждено production evidence |
|---|---|
| 001 | Effective profile/history/keymap данные, оба preference owners и pending occurrences; выбранный INS с ordered rules/disposition; actual initial audio requested/effective snapshot; явный payload/lifetime |
| 002 | Effective control options/bindings, generation mapping и move-once unread/pending output contract без replay |
| 003 | Реальный startup completion, раздельные results и текущая readiness; runtime остаётся работоспособным у payload owner после завершения login preparation |
| 004 | Native production приём accepted identity, semantic state, readable view и передача в payload; одного прямого вызова decoder недостаточно |
| 005 | Production command-context redraw executor, точные bytes, retry без потери/дубликата и rebuild реально имеющихся projections |

Эти результаты обязательны; их отсутствие не оформляется как поздний consumer.
Pending допустим только для конкретной части obligation с названным владельцем
и ссылкой на его задачу: actual adoption/close/relog C014, full HUD/map C012/C013,
macro layers/runtime C052/C053, effect delivery C016 и соответствующие recipients,
INS application C021/management C059, live audio C055/C056. Неприменимость
обосновывается version/build gate или действующим platform rule, а не отсутствием
реализации. Техническое завершение этого preparation scope не меняет canonical
statuses и не означает принятия непроверенных end-to-end portions.

## Comments

2026-11-11 — реализация и проверка по запросу пользователя. Финальный root прогон всех шести native core cases и production Linux build прошли. Свёрены exact source snapshots и disposition всех 18 obligations; partial core не повышает canonical statuses. Independent reviews закрыты без оставшихся блокирующих findings.

[Финальное evidence, команды и ограничения](../../sv-c011-evidence.md).
Пользовательская приёмка не заявлена; Windows проверки отложены.


2026-11-11 — follow-up по запросу пользователя закрыть оставшиеся проверки самого C011. Закрыты оставшиеся собственные проверки: root final 11 cases + Linux production build PASS, actual isolated-server client exit 0 и limited visual review PASS. 152 hashes совпадают во всех 11 receipts; 149 production hashes actual-server run совпадают с теми же sources. Canonical allocation/statuses неизменны; пользовательская приёмка не заявлена.

Результаты и границы — [обновлённое evidence](../../sv-c011-evidence.md). Работы следующих этапов не выполнялись.


2026-11-11 — пользователь явно принял C011 и задачи 001–006 в проверенных границах Linux preparation/core. Задача завершена; [решение о приёмке](../../sv-stage-c011-acceptance.md). Исторические записи выше отражают состояние до приёмки; поздние obligations и canonical runtime statuses не повышаются автоматически.
