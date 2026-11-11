# C011 — baseline, production seams и вопросы

Сверка 2026-11-11: три агента независимо проверили canonical scope,
preferences/controls и startup Lua/readiness; root свёл задачи и проверил
значимые baseline/API источники. Graphify использован до поиска исходников;
граф дал навигацию, актуальные dirty sources проверены непосредственно.
Это planning audit, не runtime evidence и не пользовательская приёмка.

## Объём и ownership

Authority: `docs/capabilities/stages.json`, `native-coverage.json`, manifest и
reconciliation. C011 — session preparation, 5 outcomes, 18 obligations.
Точные записи сохранены без изменения в [coverage.json](coverage.json).
001 принимает profile (6), 003 — startup transfer/failure (3+3), 004 — identity
(3), 005 — redraw (3). 002 — integration C008 без новых IDs, 006 — evidence.
Все canonical implementation/evidence statuses остаются pending.

| Production/baseline источник | Контракт C011 |
|---|---|
| `login/native-run.c`, `login/flow.c`, `settings/preferences.c` | Global preferences остаются app-scoped, character clone не копирует global effects; оба источника должны дожить до payload. Inspection не consume; `clear_effects` стирает все виды и action queue |
| `login/options.h`, `login/flow.h` | Outcome — copied observations, READY — startup milestone; destroy пока закрывает connection. Требуется явный preparation owner/lifetime API |
| `protocol/login.c`, `protocol/login.h` | END ограничивает startup stream; `sv_login_take_remaining` удерживает coalesced live bytes. Учитываются также wire unread tail, output и offset частичной отправки |
| `app.c`, `input/native-input.c`, `ui/native-controls.c` | App generation, login generation и native epoch сейчас различны; выбрать явный mapping/единое session generation, сохранить effective flush options и однократность controls |
| `transfer/startup.c`, `transfer/transfer.c`, `lua/reload.c` | C010 уже делает real flags→begin и каждый END→completion/aggregate. Attachment destroy закрывает Lua runtime/Guide; payload нужен раньше |
| `nclient.c:Receive_char_info` (2812) | Пять version branches, accepted identity и старые privilege/mode bits; не draft/list. First-load preferences/INS/keymap не повторяются при каждом view |
| `c-init.c:initialize_player_ins_files` (362) | Первый успешный INS character→class→trait→race→global, без merge; baseline conversion и порядок rules |
| `settings/audio.[ch]`, `native-run.c`, `protocol/login.c` | Initial audio production уже существует: requested/effective snapshot, optional device/pack degradation и Send_audio; не новый backend |
| `c-cmd.c:cmd_redraw` (9080), `nclient.c:Send_redraw` (7956) | Ctrl-R → PKT_REDRAW + mode 0 и invalidation huge bars; projection rebuild не исполняет команды/preferences/Lua заново |

## Readiness и границы

Хранить раздельно early init, session begin/open, transfer result, runtime reload,
применимый Guide result, историю failures и текущую пригодность ресурсов.
Принятое решение 2026-11-10 сохраняется: last END пытается reload и после failed
close; успешный reload не превращает transfer ERR в успех. `NOT_READY` Guide,
если rebuild не требовался, не failure. Безусловное закрытие приложения при
любой optional ошибке или игнорирование всех ошибок не являются baseline.

001 задаёт общий явный payload, 002 — transport/controls ownership, 003 —
startup results/runtime, 004 — identity. Payload передаёт owned resources либо
явно удержанный owner, version/generation и occurrence identity. Preparation
inspection не повторяет effects; cancellation не переносит character work в retry.
Process-sticky INIT flags и shared files имеют application lifetime.

Вход существующим персонажем требует отдельного внимания: текущий
`sv_login_start_play` ограничен creating-character. 003 реализует existing-character preparation caller с общим serializer
для existing/committed birth; actual enter-game принимает C014.
PLAY acknowledgement не заменяет startup END и readiness.

C014 принимает actual socket→live adoption, close/relog; C013 — live map;
C012 — полный HUD. C052/C053 принимают actual macro layers/runtime, C016 и
соответствующие owners — effects delivery; C021 — autoinscription на update,
C059 — полный INS management; C055/C056 — live audio/configuration.
Отсутствующий late consumer не становится успешным no-op или unknown baseline
packet. C011 готовит применимые данные/интерфейсы, а combined flows остаются
обязательными своим owners с pending evidence. Это не обратные prerequisites C011.

## Вопросы и проверки

Новых блокирующих продуктовых вопросов три агента не обнаружили. Реализация
выбирает компактный payload C API, mapping поколений, ack/cancel occurrences и
baseline readiness classification. Если потребуется изменение baseline или
новая политика продолжения после ошибки, вынести конкретный вопрос пользователю.

Сейчас выполнены только анализ, подготовка задач и статическая сверка документов.
План проверок: необходимый минимум Linux core через production preparation,
существующий managed peer и focused identity/redraw paths. Не создавать новый
harness, general fault framework, broad suite или полную visual matrix;
расширение проверок согласовывается заранее. Windows/MinGW/Wine deferred.
При реализации evidence различает actual result, pending и неприменимость;
принятые B/C002–C010 не переоткрываются автоматически.

## Повторный критический review задач — 2026-11-11

По повторному запросу пользователя три агента проверили полноту и выполнимость
плана. Декомпозиция 001–006 сохранена; новые ticket IDs не нужны. Уточнения:

- 001: INS/audio/profile projections — обязательные наблюдаемые результаты,
  а не только дополнительные пункты сверки двух preference owners.
  Ранний selected-name OPT/options до PLAY отделён от authoritative identity
  и INS после END; payload различает готовность этих участков без цикла ожидания.
- 003: existing-character PLAY preparation обязателен; readiness проверяет END,
  active transfers, текущую пригодность runtime и владение pending output.
  История initial failure не стирается успешным восстановлением runtime.
- 004: native caller должен реально доставить accepted identity в state/view;
  bounded retention и явная неполная готовность лучше silent drop/byte scan.
  Одного обещания будущего consumer C014 для собственного результата недостаточно.
  Identity не обязана идти первой после END: необходимые preceding decoders
  входят в минимальный ingress C011. Пока их нет, 004 остаётся незавершённой;
  managed peer должен проверить допустимый packet перед identity и INS/hook.
- 005: executor использует существующий command slot; Ctrl-R действует в command
  context. После consuming take команда удерживается при backpressure;
  user redraw не содержит recovery PKT_UNKNOWN prefix.
- 006: нынешние first-session peer cases не достигают полной PLAY/END границы.
  Нужен минимальный continuation того же peer, без нового simulator/live loop.
  Собственная работа C011 не закрывается pending; для каждого obligation
  checked subflow и pending remainder указываются раздельно.

Новых продуктовых вопросов эти уточнения не требуют. Это review и правка
planning документов; production code и runtime tests в этом проходе не менялись.

## Реализация и финальная сверка — 2026-11-11

План выполнен в Linux preparation/core scope: [evidence](../../sv-c011-evidence.md).
Добавлены owned preparation/resources, startup INS, identity/model/readout,
redraw executor и необходимые version-aware prefix framers. Exact complete
unapplied frames передаются FIFO до raw tail; applied occurrences не replay.
Post-END FILE использует тот же C010 registry и ordered reply queue.

Шесть native managed-peer случаев и production build прошли на финальных
sources; root сверил hashes. Независимые reviews закрыли выявленные defects,
включая premature input, pending-byte counts, partial INS duplicates, pacing,
preparation move guards, prefix/FILE порядок и lifetime map resources.
Это не пользовательская приёмка и не PASS непроверенных поздних lifecycle flows.


## Дополнительные проверки собственного scope — 2026-11-11

По дополнительному запросу пользователя agents закрыли native PAUSE/thin,
initial audio enabled/fallback/missing/no-device и полный native birth/readout.
11 managed cases и production build прошли на финальном source snapshot;
actual isolated server 4.9.3.0.2.0 подтвердил PLAY, native MOTD acknowledgement,
accepted `Born`, runtime readiness и complete payload. 666 startup frames перед
identity удержаны FIFO без применения inventory/audio/unique consumers.

Actual-server evidence исправило ошибочное прежнее требование standalone early
OPTIONS: ранний character OPT snapshot нужен до PLAY, сетевые options передаются
внутри PLAY; отдельный pre-PLAY packet сервер этой фазы не обрабатывает.
Историческое C002 evidence не переписано. Thin counter теперь переживает decoder
WAITING внутри bounded ingress batch. Printable PAUSE получает временную native
text readiness; post-MOTD readout показывает accepted identity, завершённый birth
не перекрывает его. Redraw context и приоритет failure status сохранены.

Ограниченный Linux visual review пройден; полный live loop C014 и late consumers
не реализованы. Canonical 5/18 allocation/statuses и отсутствие пользовательской
приёмки сохраняются. Точные cases, hashes, ограничения и история intermediate
failures — в общем evidence; новых blocker текущего preparation scope не осталось.


## Приёмка — 2026-11-11

C011 и задачи 001–006 приняты пользователем в проверенном preparation/core scope.
[Решение](../../sv-stage-c011-acceptance.md). Ранее отмеченное отсутствие приёмки
относится к истории проверок; allocation и canonical runtime statuses сохранены.
