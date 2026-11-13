# SV-C053-001 — actual macro recognition и load-time execution

Статус: planned, 2026-11-09.
Labels: enhancement
Owned IDs: `capability.input.macro-match`, `capability.input.macro-wait`,
`capability.input.macro-xwait` (existing C053 allocation).
Зависимости: production C004 `SvPreferences`, C052 definitions/layers и
C011/C014 live session lifecycle; конкретные action stages владеют gameplay result.

## Проблема и production путь

C004 уже декодирует macros/actions и сохраняет разрешённые `!/?` в prepend queue,
но `login/native-run.c` сейчас только сообщает aggregate pending counts;
`login/flow.c` хранит generation-owned character effects до cleanup. Полного
пользовательского исполнения пока нет. `src/client/sv/input/input.[ch]` имеет
ограниченную однобайтовую action и не заменяет этот runtime.

C004 завершён и принят решением пользователя 2026-11-09; оставшаяся реализация и
проверки теперь обязательны в C053. Найденные при интеграции дефекты исправляются
здесь, без повторного открытия C004; evidence backlink туда необязателен.
Canonical ID allocation/conditions/obligations сохраняются. Исторические
`preferences.*` ID C004 не становятся ID C053.

Baseline: `src/client/c-util.c:macro_maybe`, `macro_ready`,
`inkey_aux`/`inkey`, `sync_sleep`/`sync_xsleep`; `c-files.c:135 text_to_ascii`
и `1625 !/?`; `z-term.c:3924` queue prepend. Перед реализацией пересверить
actual input contexts, store/message gates и control-sentinel branches.

## Реализация

1. Подключить actual input router к definitions и effects существующего
   `SvPreferences`. Полный trigger и action — opaque byte sequences; не сужать
   их до single-byte action и не пропускать через Unicode text editor/decoder.
   Physical SDL3 trigger encoding проверяется против сохранённых PRF bytes,
   включая shipped pref-sdl3 defaults. Не создавать второй PRF parser.
2. Подключить macro-derived command bytes к тому же effective keymap/core
   interface C005, учитывая normal/roguelike, raw bypass и caller context.
   Проверка этого позднего route принадлежит C053, а не readiness C005.
   Реализовать longest-trigger matching, unmatched-byte pushback и policy
   normal/hybrid/command macros в действительном caller context, включая
   message/store gates и `macros_in_stores`. Completion/control sentinels не
   передаются как обычные игровые keypresses. Macro action не запускает повторное
   распознавание самой себя вопреки baseline branches.
3. Доставлять разрешённые load-time `!/?` в тот же command stream. Получать state
   через согласованный C011/C014 handoff: global startup intents не теряются
   из-за clone(false), character intents не исполняются до актуальной session
   readiness и не переносятся на чужой generation. Не вводить blanket startup
   запрет, новый `in_game` gate или запись action в credential/text field.
4. Сохранить runtime preconditions: parse_macro=true, sleep semaphore=false,
   macro_missing_item=0, abort_prompt=false. Marker29 завершает macro; исходные
   completion/after-macro/control ветви и caller-specific abort/missing-item
   semantics сохраняются. `?` уже отсекается bodymonster context C052; `!`
   остаётся разрешённым. Macro-only filter не порождает ни эти actions, ни `#`.
5. Pending queue является prepend: загрузка action1, action2 даёт
   action2,29,action1,29,previous bytes; bytes внутри action не переставляются.
   Runtime потребляет очередь/effects один раз: inspection/redraw/observer read
   не повторяет действия. Не исполнять action и по effect list, и по queue.
6. Реализовать WAIT96 (`\w`, две цифры, 100ms units) и XWAIT30 (`\W`, четыре
   цифры, 100ms units) в main-loop state machine с обслуживанием сети/timers/UI.
   Обычный wait завершает duration/semaphore/confirm по исходным gates;
   произвольную interactive cancellation ему не добавлять. Extended wait
   сохраняет baseline fresh Escape discard, Space resume/restore old queue,
   прочие fresh keys сохраняются. Закрытие session всегда прекращает её commands
   по ADR-0005; это отличается от игровой wait cancellation.
7. Использовать уже применённый parser effective encoded prefix1023 и полный
   retained raw record. LIMIT/partial не разрешает runtime выполнить raw tail;
   decoded NUL завершает реально используемый C-string payload. Bound/overflow
   сообщает failure без silent action loss. Source files не переписываются.

## Минимальная Linux проверка и готовность

Переиспользовать existing input/session production test entrypoint. Компактно
проверить longest trigger и pushback, normal/hybrid/command context gates,
точные physical bytes, marker28/29 handling и фактическое выполнение загруженных
`!/?` через реальный command dispatcher. Два load-time action подтверждают
prepend order на observable command/protocol result, не только queue snapshot.

На том же production wait path с управляемым временем проверить WAIT96 duration
и source semaphore/confirm completion, XWAIT30 Escape/Space/other-fresh-key
поведение и восстановление queued input. Interleaved network/timers, focus/resize,
completion, caller abort/missing item и generation teardown не должны повторять
commands, блокировать UI или отправлять stale bytes. Не создавать отдельную
тестовую реализацию ожидания или fake callback, выдающий execution PASS.

Проверить macro-derived → C005 keymap route по actual command/direction result
и применимым protocol bytes, включая normal/roguelike и bypass. C005 уже
доказывает свой production module; эта поздняя проверка выполняется C053
без переоткрытия C005.

Нужен actual caller regression конкретного игрового action, включая обязательный
поздний execution/action regression для C052 `.allocation-checks`: сопоставить
реальные C052 layer/form-derived definitions и !/? effects с исполняемым command
result. C052 принимает свою decoded-update orchestration без ожидания C053;
эта задача закрывает именно runtime часть, без переоткрытия C052. Успешный dispatcher
helper не принимает результат чужого gameplay stage. Сопоставить реальные bytes,
context/cancel и конечное состояние caller; отсутствующий caller остаётся
обязательным пунктом реализации соответствующего владельца. C052 отдельно
проверяет layer/reload orchestration, C014 — handoff/delivery lifecycle;
согласовать интеграционный сценарий без дублирования suites.

Готовность: production input consumer распознаёт PRF definitions и один раз
исполняет разрешённые actions; обе wait ветви и completion/cancel доказаны на
реальных callers. Девять canonical obligations получают factual evidence C053;
теоретическая очередь, counters или source audit не закрывают результат.
Только Linux; расширение проверок согласовать, Windows/MinGW/Wine deferred.
Этот документ не объявляет runtime реализованным или C053 принятым.

## Интеграция принятого C008 — решение 2026-11-09

[C008 и задачи 001–004 приняты](../../sv-stage-c008-acceptance.md) в текущих
границах core. Эта задача C053 обязательно завершает настоящую интеграцию
confirmation owner, а не новый FIFO или тестовую модель macro wait.

Production seam: `sv_app_begin_confirmation` перед WAIT/XWAIT, ordered
`sv_app_take_confirmation` для исходного unsigned command byte и
`sv_app_end_confirmation` при завершении/отмене. Сверить конкретные baseline
wait gates в `src/client/c-util.c`; при входе старое подтверждение не
переиспользуется. Использовать тот же generation и native control owner C008.

На настоящем macro dispatcher/wait path проверить split/ordered CONFIRM и
наблюдаемое продолжение ожидания/команды, unsolicited/wrong-owner/stale
confirmation, completion/cancel/close/relog без replay. Пока wait активен,
применимые pause/flush и network updates обслуживаются; свежий pause ack не
попадает в action stream, repaint не повторяет command. API/queue-only check
не заменяет caller evidence. Согласовать один минимальный сценарий с уже
запланированными WAIT/XWAIT checks этой задачи, без отдельной широкой suite.

Trace: `obligation.network.confirm.result`, `.lifecycle`, `.allocation-checks`
и применимые pause/flush lifecycle obligations из C008; canonical allocation
остаётся у C008. Работа и исправления обязательны C053, не блокируют и не
переоткрывают принятый C008 автоматически. Evidence backlink служит трассировке,
а не gate C008. Linux обязателен, Windows/MinGW/Wine deferred; расширение проверок
согласуется по AGENTS. C053 этим решением не принят и остаётся planned.
