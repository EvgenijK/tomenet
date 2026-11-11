# C011 — подготовка сессии

Статус: **accepted** — Linux preparation/core, 2026-11-11.
C011 и задачи 001–006 завершены и приняты; [решение](../../sv-stage-c011-acceptance.md).
Задачи 001–006 реализованы в preparation/core scope и проверены;
[финальное evidence](../../sv-c011-evidence.md). Три агента проверили canonical allocation, production seams
и вопросы; результаты сведены в [source contract](source-contract.md).

C011 подготавливает профиль и ресурсы сессии перед игрой и передаёт явный
preparation payload. В canonical allocation **5 outcomes и 18 obligations**:

| Outcome | Задача | Obligations |
|---|---|---:|
| `session.load-profile-input` | 001 | 6 |
| `session.transfer-startup-files` | 003 | 3 |
| `session.startup-file-failure` | 003 | 3 |
| `session.read-identity` | 004 | 3 |
| `session.redraw` | 005 | 3 |

Точные IDs, prerequisites, conditions и pending statuses сохранены в
[planning snapshot](coverage.json); это не runtime evidence.

| Задача | Результат |
|---|---|
| [SV-C011-001](SV-C011-001-preference-adoption.md) | Profile preparation, preferences/effects, startup INS/audio projections и общий payload |
| [SV-C011-002](SV-C011-002-network-control-preparation.md) | Effective controls, generation и unread/output ownership contract |
| [SV-C011-003](SV-C011-003-lua-startup-preparation.md) | Startup transfer/Lua results, readiness и сохранение runtime до handoff |
| [SV-C011-004](SV-C011-004-server-identity.md) | Подтверждённая сервером identity, version branches и native readout |
| [SV-C011-005](SV-C011-005-redraw.md) | Redraw action, request bytes и projection invalidation |
| [SV-C011-006](SV-C011-006-evidence.md) | Минимальное Linux evidence и сверка всех 18 obligations |

Рекомендуемый порядок: общий payload и ранний профиль 001, связанные 002/003,
authoritative identity 004 и identity-dependent часть 001, redraw 005,
сводка 006. Selected-name OPT загружается до PLAY; effective options передаются
внутри PLAY без отдельного pre-PLAY `PKT_OPTIONS`. INS и поздние
identity-dependent hooks используют 004. Порядок файлов не требует использовать
birth draft вместо accepted identity или ждать identity перед ранним PLAY.
Подготовленный контракт можно завершить без actual socket→live adoption C014.
Startup milestone/PLAY acknowledgement не равны ни readiness, ни enter-game.

Текущая основа — принятые C002–C010. C010 уже реализует FILE owner,
real flags→Lua begin и сохранение aggregate; повторять их не требуется.
C014 отвечает за live adoption/close/relog, C013 — live map, C012 — полный HUD.
Actual macro layers/runtime, effect recipients, autoinscription update и live
аудио остаются C052/C053, C016 и связанным owners, C021/C059, C055/C056.
Их combined evidence остаётся pending и не становится обратным gate C011.
Все canonical obligations сохраняются: локальный core результат не означает
PASS непроверенного позднего flow или всего canonical obligation.
Собственные результаты 001–005 обязательны для технического завершения C011;
их нельзя закрыть общим `pending`. Минимум указан в [006](SV-C011-006-evidence.md).

Новых блокирующих вопросов пользователю анализ не выявил. Точные C API,
соответствие generation и ack/cancel pending effects выбираются при реализации
по baseline и ADR. Изменение baseline либо расширение проверок требует отдельного
согласования. Только необходимый минимум Linux core checks production-пути;
Windows/MinGW/Wine deferred. Новый harness, broad suite и полная visual matrix
не входят в план. Правила [AGENTS](../../../AGENTS.md) имеют приоритет.

## Подключение controls C008

При создании actual login → live session handoff подключить тот же production
control owner [C008](../c008/README.md): ordered pause/flush submission, свежий
native input epoch и options pacing. Передать generation, unread bytes и pending
sends без повторного исполнения controls. Synthetic/native core evidence C008
не доказывает handoff. Полный набор игровых packet consumers должен быть готов
до подачи live stream: отсутствие позднего decoder не означает unknown packet
по behavior baseline. Canonical allocation C008 и его integration gaps сохраняются;
C008 принят в границах core. Обязательная работа закреплена в
[SV-C011-002](SV-C011-002-network-control-preparation.md); она не блокирует и
не переоткрывает C008 автоматически. Actual handoff проверяет C014, live map —
C013. Это не переоткрывает B.


## Подключение Lua startup C009

Обязательная работа закреплена в
[SV-C011-003](SV-C011-003-lua-startup-preparation.md). При полном планировании C011
подключить [C009 production runtime и reload](../c009/README.md):
передать реальные server feature flags до open/c-init, использовать отдельные
transfer result и Lua load result перед dependent startup/handoff. Done flag
не означает успешную загрузку. Ошибка сохранения не должна превращаться в
успешное обновление от одного лишь успешного reload. По решению пользователя
2026-11-10 сохраняется baseline: последнее END вызывает попытку reload даже при
ошибке сохранения; результаты учитываются отдельно.
[Решение C009](../c009/source-contract.md).
Actual FILE_END/publication/ACK consumer — C010; прямой core API test C009
не доказывает actual preparation C011. Session teardown/generation остаётся C014.
Этот раздел фиксирует integration obligation; полный план приведён выше,
приёмка C011 не заявлена.

C009 и SV-C009-001–003 завершены и приняты пользователем 2026-11-10 в текущих
границах core. Реализация и проверки 003 обязательны C011, не блокируют и не
переоткрывают C009 автоматически. Canonical allocation/conditions/prerequisites
и registry statuses сохраняются; непроверенный preparation flow не получает PASS.
Backlink evidence в C009 служит трассировке, не является gate его завершения.

## Реализованный production путь — 2026-11-11

`login/preparation.[ch]` предоставляет read-only inspection, move-once owner,
occurrence ack, public transport/FILE continuation и ordered deferred drains.
Payload удерживает preferences/history/Lua/app и реальные native resources,
full requested/effective references и owned MapFont definitions. Existing PLAY
и committed birth используют один serializer. Startup prefix включает необходимые
social/skill/rendering/map RLE packets; они сохраняются без приёмки поздних
capabilities. Post-END FILE проходит через прежний C010 owner с настоящими replies.

Root подтвердил production Linux build и все 11 focused native cases:
исходные success/repair/transfer failure/QUIT/cancel/retry, native controls,
полный birth и три initial audio variants. Дополнительный isolated actual-server
run завершил birth → PLAY → MOTD acknowledgement → видимую accepted identity
и complete preparation; 666 preceding frames удержаны без применения поздних
capabilities. Actual native PAUSE/thin pacing проверены; полный live loop и late
combined flows не заявлены. Disposition всех 18 obligations, exact commands,
hashes и артефакты приведены в evidence; canonical allocation/statuses неизменны.

## Закрытие оставшихся проверок — 2026-11-11

По дополнительному запросу пользователя проверены собственные gaps C011.
Исправлены thin batch boundary, printable PAUSE readiness, преждевременный
standalone OPTIONS, missing mandatory prefix framing и перекрытие identity
завершённым birth/stub. Options загружаются рано и передаются внутри PLAY.
Redraw ограничен post-MOTD preparation context; failure status сохраняет приоритет.
Локальный настоящий сервер и ограниченный X11/software visual review прошли.
Новых блокирующих findings текущего preparation scope нет; пользовательская
приёмка не заявлена. Работа следующих этапов не выполнялась.


## Приёмка — 2026-11-11

Пользователь принял C011 и задачи 001–006. Предыдущие разделы сохраняют историю
планирования и проверок до принятия; текущий статус — accepted.
[Решение и сохранённые границы](../../sv-stage-c011-acceptance.md).
