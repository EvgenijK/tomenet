# SV-C010-005 — минимальное Linux evidence C010

Статус: accepted — Linux startup/core, 2026-11-11.
Labels: enhancement, ready-for-agent
Owned IDs: 0 новых IDs.
Зависимости реализации: 001–004; production build/core path.

## Работа и критерии

1. Свести ровно пять `capability.transfer.*` и 20 obligations (result,
   failure-lifecycle, wire, allocation-checks у каждого) по [snapshot](coverage.json).
   Lua trace 001 не добавляет ownership C009 и не переоткрывает его приёмку.
2. Выполнить необходимый Linux build и самый необходимый минимум core checks,
   вызывающих production decoder, transfer owner, filesystem, sender и Lua consumer.
   Объединить связанные cases 001–004: version/checksum boundary, actual
   destination/source mapping и bytes; incoming partial/chained/failed writes;
   close→last reload→warning→metadata→ACK/ERR; upload ACK pacing и release;
   отмена и stale generation без file/runtime/reply effects новой сессии.
   В тех же связанных core cases наблюдать startup ingress и real drain/resume
   при backpressure без повторных file/Lua effects, legal 1030-byte DATA frame,
   sender EOF→END без новых incoming bytes. Direct call decoder в обход
   production startup dispatch/pump не подтверждает их подключение.
   Проверка через mock-only parser/sender/Lua aggregate не заменяет production.
3. Не добавлять новый общий harness, broad suite, peer, stress/soak или real-account
   runs. Расширение набора проверок заранее согласуется по AGENTS. Текущая
   обязательная платформа — [Linux](../../agents/platform-validation.md);
   Windows/MinGW/Wine deferred, без фиктивного PASS. Сохранять применимые Linux
   build/runtime branches и исходные protocol version gates.
4. В evidence записать revision/dirty state, точные команды/результаты, observed
   owned paths/content, emitted bytes, effect order, failure causes и pending
   limits. Отдельно отметить непроверенные actual C011 preparation/C014 live
   lifecycle/C041 menu; не представлять core как полный live/account run.
5. Сделать техническую сверку каждого obligation без автоматической смены
   canonical JSON/statuses и пользовательской приёмки. Evidence и completion
   tickets не принимают C011/C014/C041 заранее; core PASS не означает их PASS.
   Для каждого из 20 obligations указать подтверждённую часть, pending часть
   и неприменимые условия с объяснением. В частности, явно разобрать применимость
   generic read/write/replace failure, concurrent edits и unchanged unrelated
   owners к transfer publication; одного core happy path недостаточно для PASS
   всего failure-lifecycle obligation. Это разметка evidence, не разрешение
   расширить набор проверок без согласования.

Завершение: минимальные проверки подтверждают production поведение, все 20
obligations имеют конкретное evidence или честный pending outcome; при наличии
pending обязательного core flow core не объявляется технически готовым.
При pending частях obligations технический core PASS не означает полного
завершения/приёмки C010; границы результата и remaining flows указываются явно.

## Результат реализации — 2026-11-10

Минимальная Linux production core-проверка с ASan/UBSan и production build прошли. Для всех 20 obligations записаны подтверждённые и pending части; это не полный PASS obligations и не пользовательская приёмка этапа.

[Evidence, команды и ограничения](../../sv-c010-evidence.md).
Canonical statuses/ownership сохранены; pending flows не получают PASS.

## Comments

### 2026-11-10 — план двух дополнительных проверок

По запросу пользователя подготовлен [SV-C010-006](SV-C010-006-targeted-validation.md)
для sender output saturation и actual native socket/login flow. Это дополнение
к evidence, не переоткрытие завершённой 005. Реализация/запуск расширения пока
не выполнялись; прежние результаты и pending limits сохранены.

### 2026-11-10 — дополнение 006 завершено

После отдельного разрешения пользователя обе проверки 006 прошли:
[evidence](../../sv-c010-006-evidence.md). Статус/история 005 не переоткрываются;
новые результаты дополняют disposition sender output saturation и actual native
startup. Прочие pending limits сохранены.

### 2026-11-11 — приёмка пользователя

Пользователь подтвердил: «Этап принимаю». Задача принята в границах
[решения о приёмке C010](../../sv-stage-c010-acceptance.md). История реализации,
evidence и непроверенные условия сохраняются; canonical statuses не меняются.
