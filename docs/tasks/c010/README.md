# C010 — интеграция Lua reload при передаче файлов

Статус: planned, 2026-11-10. Это обязательное дополнение для будущего полного
плана C010, не исчерпывающая декомпозиция его пяти canonical outcomes.
Canonical allocation, prerequisites и statuses сохраняются.

C009 принят пользователем 2026-11-10 в текущих границах Linux core.
[Связка SV-C010-001](SV-C010-001-lua-reload-integration.md) — самостоятельная
обязательная работа C010; она не блокирует и не переоткрывает C009 автоматически.
Pending flows не получают PASS, backlink evidence служит только трассировке.

| Задача | Результат | Canonical ownership |
|---|---|---|
| [SV-C010-001](SV-C010-001-lua-reload-integration.md) | Actual accepted INIT/END → Lua owner, отдельные results и ACK/ERR | 0 новых IDs; Lua остаётся C009 |


## Обязательная интеграция C009

Использовать [production Lua owner C009](../c009/README.md) и
[completion contract 002](../c009/SV-C009-002-reload-lifecycle.md), без отдельной
реализации Lua reload в transfer decoder. Самодостаточный объём, API, baseline,
минимальные Linux checks и критерии завершения закреплены в
[SV-C010-001](SV-C010-001-lua-reload-integration.md). При полном планировании C010
сохранить эту связку с владельцем `capability.transfer.end`; evidence к трём
`obligation.lua.reload.*` служит трассировке принятого core.

C010 владеет CHECK/INIT/DATA/END/upload, version/checksum branches, temporary
ownership, публикацией и ACK/ERR. Передать C009 результат close, актуальное число
transfers, session generation. Сразу после успешного INIT передать accepted-INIT occurrence
в application Lua owner: updated_audio/guide отмечаются по baseline substring
имени `audio.lua`/`guide.lua`. Rejected INIT и `-u` их не меняют. Признаки sticky
в течение процесса: subsequent DATA/END failure, disconnect до END и новая session
их не сбрасывают. Generation проверяется до фиксации; передавать признаки только
из текущего session batch при END недостаточно.

После последнего END при нулевом числе transfers вызвать reload текущих файлов,
в том числе при неуспешном сохранении: так решил пользователь 2026-11-10.
Сохранить последовательность close/publication → reload → audio warning →
Guide metadata → ACK/ERR. Ошибка Guide getter не подавляет предшествующий warning. Ошибка передачи и результат reload независимы: ERR не становится ACK
от успешного чтения старых файлов, успешная передача не скрывает Lua failure.
Передать оба результата C011, без ложной startup readiness. Результат каждого
END синхронно потребить до обработки следующего END, включая chained packets:
completion slot — последний результат, а не очередь. Сохранять отказы в startup
aggregate, чтобы последующий successful END не маскировал предыдущую ошибку.

`-u` C002 отвергает все FILE_INIT как baseline, а не только `.lua`; он не запрещает
обычное чтение имеющихся локальных scripts. Частичный packet не даёт эффектов;
полные события сохраняют порядок, stale generation не вызывает file/runtime
эффектов. Не подавлять реальный новый wire END фиктивной deduplication по fnum;
отличать его от повторного внутреннего потребления того же события.

## Завершение интеграции

В существующих минимальных Linux core checks C010 наблюдать actual decoder →
close → reload/metadata/warning → ACK/ERR: несколько файлов, последний END,
failed last close, partial/chained input и stale generation, без повторных
эффектов от redraw/повторного внутреннего consumption. Прямой C009 API test не
доказывает эти flows. Дополнение не вводит новый harness или broad suite;
расширение проверок согласовать по AGENTS. Windows/MinGW/Wine deferred.
Реализация, evidence и приёмка C010 не заявлены.
