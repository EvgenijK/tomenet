# SV-C010-001 — подключение принятого Lua core C009 к FILE_INIT/END

Статус: accepted — Linux startup/core, 2026-11-11.
Labels: enhancement, ready-for-agent
Owned IDs: 0 новых IDs; integration obligation принятого C009 внутри C010.
Зависимости реализации: production transfer decoder/publication C010,
C003 resource overlay/transient owner, actual generation текущего startup owner
и согласованный preparation interface с C011. Завершение C011 не является
новым prerequisite C010: caller C010 предоставляет реальные decoded feature
inputs, C011 позднее принимает полный preparation/handoff flow.

## Решение и граница

Пользователь 2026-11-10 принял C009 в текущих границах Linux core, поручив
закрепить оставшиеся интеграции в C010/C011/C014. Эта работа обязательна C010;
её отсутствие, доработки и проверки не блокируют и не переоткрывают C009
автоматически. [Решение о приёмке](../../sv-stage-c009-acceptance.md),
[core evidence](../../sv-c009-evidence.md), [контракт и baseline](../c009/source-contract.md).
Pending actual transfer flows остаются непроверенными; прямой core test не даёт
им PASS. Backlink evidence к C009 служит трассировке, а не обратному gate приёмки.

Это задача связки с Lua, не полный план пяти canonical outcomes C010:
`capability.transfer.check`, `.init`, `.data`, `.end`, `.upload`.
Основной caller trace — `capability.transfer.init` и `capability.transfer.end`;
у каждого сохраняются `obligation.transfer.<init|end>.result`, `.failure-lifecycle`,
`.wire`, `.allocation-checks`. Canonical prerequisite `.end` включает
`capability.files.resource-overlay`, `capability.files.transient-owner`,
`capability.lua.reload`. Lua ownership остаётся у C009; новые IDs и переназначение
allocation не нужны. CHECK/checksum version branches и upload остаются полному
плану C010, эта задача не принимает их заранее.

## Production работа

1. Использовать существующий application owner
   [`SvLuaReload`](../../../src/client/sv/lua/reload.h). В C010 минимальный
   application/startup adapter передаёт существующий owner из `login/native-run.c`
   в текущую login attempt через явный borrowed attachment. Вызвать
   `sv_lua_reload_begin(owner, generation, features)` с реальными version-aware
   `sv_login_flags`/`sv_login_version` до accepted INIT/END; fixture/default features
   не заменяют decoded данные. Begin исполняется один раз на нужный generation,
   не на каждый poll/redraw или backpressure retry. C011 позднее принимает полный
   preparation/readiness/handoff flow и не является prerequisite этой связки.
   Результат begin сохраняется отдельно от early init и последующих reload;
   `-u` поступает из настоящего invocation C002, не из test-only настройки.
   Attachment отзывается и generation закрывается до teardown attempt/owner;
   owner переживает retry attempt, сохраняя process-sticky flags.
   Не создавать Lua runtime или reload implementation в decoder. Main thread
   последовательно доставляет полные occurrences; diagnostic callback не reenter.
2. После успешного FILE_INIT/destination preparation немедленно вызвать
   `sv_lua_reload_file_init(owner, generation, filename, true)`, до следующего
   packet. Только актуальное generation меняет process-sticky flags;
   baseline matching — substring `audio.lua`/`guide.lua`, не basename equality.
   Rejected INIT и `-u` flags не меняют. DATA/END failure и disconnect до END
   их не сбрасывают: следующий session использует тот же application owner.
   Нельзя восстановить flags только из текущего session batch на END.
3. `-u` из C002 отвергает все FILE_INIT как baseline, а не только Lua filenames.
   Он не запрещает чтение уже существующих local scripts. Сохранить destination
   validation, temporary ownership и ответы transfer; не вводить новый allow-list.
4. Для каждого нового полного END сначала выполнить close/publication и получить
   transfer result и число всех активных transfers после close, как baseline
   `get_xfers_num`, не только Lua файлов или текущего входящего batch. Затем вызвать
   `sv_lua_reload_end(owner, generation, remaining, transfer_success)`.
   Порядок: close/publication → last reload → audio restart warning → Guide
   metadata → ACK/ERR. Пока remaining ненулевой, reload отсутствует. Ошибка
   Guide getter не подавляет уже предшествующий audio warning. Audio backend и
   effective live mappings не перезапускаются.
5. При последнем END с remaining=0 вызвать reload даже после failed close:
   пользователь выбрал baseline поведение 2026-11-10. Читается фактическое
   содержимое диска, которое может быть старым/частичным; reload тоже может
   ошибиться. ACK/ERR определяется transfer result: успешное чтение не превращает
   failed transfer в ACK, transfer success не скрывает runtime/metadata failure.
6. Синхронно `sv_lua_reload_take` каждого END и потребить copied `SvLuaCompletion`
   до dispatch следующего END, включая chained input. Slot содержит последний
   результат, а не очередь. Передать C011 отдельные transfer/runtime/Guide results;
   удерживать отказы в startup aggregate, чтобы следующий успешный END не затёр
   прежнюю ошибку. У не-last END `runtime.status`/`guide_status == SV_LUA_NOT_READY`
   означает отсутствие попытки, а Guide status может быть NOT_READY и на last END,
   если metadata rebuild не требовался либо runtime failure его предотвратил.
   Учитывать `reload_attempted`, actual runtime result и выполненный metadata шаг;
   отсутствие необязательной попытки само по себе не объявлять новой Lua failure.
   Aggregate принадлежит actual startup/transfer owner C010, доступен будущему
   C011 через явный copied result/interface, не renderer или test-only структуре.
   Outcome inspection не потребляет completion повторно и не теряет ранний failure.
   Новый generation начинает новый aggregate; close сохраняет copied итог для
   получателя, освобождая session-owned данные, без переноса failures старой
   attempt в новую. Sticky audio/Guide flags остаются у application Lua owner.
7. Частичный packet не фиксирует accepted INIT/END, запись файлов или Lua effects.
   Полные occurrences сохраняют порядок. Stale/inactive generation проверяется
   до file effects и обращения к Lua owner; false от API не превращать в success.
   Redraw/resize/повторное внутреннее чтение completion не повторяет reload.
   Новый реальный wire END, в том числе с повторным fnum, не подавлять фиктивной
   deduplication по fnum. Actual session close/relog исполняет C014; shared files
   не удаляются как часть Lua cleanup.

## Минимальная Linux проверка и завершение

В необходимой core проверке production decoder C010 наблюдать реальные
file publication, Lua data и emitted ACK/ERR bytes: несколько transfers → один
last reload; split/chained input без частичных/повторных effects; failed last close
с reload и сохранённым ERR; разделение runtime/Guide failure и transfer success;
accepted INIT → disconnect до END → новый session last END со sticky warning/
metadata; rejected/`-u` INIT и stale generation без flags/file/runtime effects.
Синхронное потребление chained END и сохранение предыдущего failure должны
проходить через настоящий consumer, не отдельную test-only модель aggregate.
Совмещать связанные cases в существующем минимальном C010 подходе; не вводить
новый harness, peer, broad suite, stress/soak или real-account runs. Расширение
проверок заранее согласовать по AGENTS. Windows/MinGW/Wine deferred по
[текущему правилу платформ](../../agents/platform-validation.md).

В evidence C010 записать revision/dirty state, команды, exact bytes, observed
paths, порядок effects, результаты и pending limits. Trace к перечисленным
`obligation.transfer.init.*`/`obligation.transfer.end.*` и применимым
`obligation.lua.reload.result`, `.failure-lifecycle`, `.allocation-checks`.
Остальные flows C010 и actual C011 readiness/C014 teardown не получают PASS.
Canonical JSON/statuses не менять автоматически. Готовность этой задачи —
production transfer caller использует принятый Lua owner и подтверждены указанные
связки; полный этап C010 по этой одной задаче завершённым не считается.

## Результат реализации — 2026-11-10

Подключён production `transfer/startup` adapter к login attempt: decoded flags → begin, accepted INIT и каждый END → Lua owner, synchronous completion и copied aggregate. Borrowed owner закрывается до application teardown; C011/C014 consumers остаются своими этапами.

[Evidence, команды и ограничения](../../sv-c010-evidence.md).
Canonical statuses/ownership сохранены; pending flows не получают PASS.

## Comments

### 2026-11-11 — приёмка пользователя

Пользователь подтвердил: «Этап принимаю». Задача принята в границах
[решения о приёмке C010](../../sv-stage-c010-acceptance.md). История реализации,
evidence и непроверенные условия сохраняются; canonical statuses не меняются.
