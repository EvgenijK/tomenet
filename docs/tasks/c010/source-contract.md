# C010 — baseline и границы переноса

Сверка 2026-11-10: три агента исследовали canonical scope, production protocol
и декомпозицию задач; root проверил значимые исходники. Graphify использован
до поиска исходников. Документ фиксирует план, не implementation/evidence PASS.

## Нормативный объём

`capabilities/stages.json`: C010, transfer, пять outcomes:
`capability.transfer.check`, `.init`, `.data`, `.end`, `.upload`.
У каждого четыре obligations: `result`, `failure-lifecycle`, `wire`,
`allocation-checks` — всего 20. Исходные statuses сохраняются.
Все зависят от C003 `files.resource-overlay` и `files.transient-owner`;
END дополнительно от принятого C009 `lua.reload`.

## Источники и обязательная семантика

| Источник | Контракт |
|---|---|
| `src/client/nclient.c:477–647` | FILE dispatcher: INIT/DATA/END/CHECK/SUM/ACK/ERR; zero ID отвергается, partial frame ждёт продолжения |
| `nclient.c:597–637` | Строго newer-than `4.6.1.1.0.1`: MD5 как четыре network uint32; иначе исторический checksum |
| `nclient.c:650–699` | DATA body и отправка CHECK/INIT/DATA/END, исходные slot/length/bytes |
| `src/common/files.c:128–157` | Original client_user_path mapping; на Linux fallback `ANGBAND_DIR + fname`, без blanket переноса в shared user root |
| `files.c:159–334` | ACK/ERR и checksum negotiation sender; source open, переход к INIT или завершение при совпадении |
| `files.c:337–389` | kill_xfers, ACK-driven do_xfers и число всех transfer records, включая исходящие |
| `files.c:412–472` | INIT temporary owner и DATA write; INIT отвергает leading `/` и любое вхождение `..` |
| `files.c:476–519` | END открывает destination `wb` и копирует temporary; record снимается и при failed close |
| `files.c:524+` | Исторически broken Adler-32 сохраняется как wire algorithm, не заменяется стандартным Adler-32 |
| `src/client/c-cmd.c:10369+` | cmd_script_upload: отмена, filename prompt с limit 30, chunksize 256/1024 по тому же version gate |
| `src/client/sv/lua/reload.[ch]` | Существующий production Lua owner C009; accepted INIT, generation, last END и copied completion |
| `src/client/sv/files/files.[ch]` | C003 resource/transient owners; export publication API не является готовой заменой server transfer |

Q40 [settings policy](../../capabilities/settings-policy.md) уже сохраняет
original validation/mapping: без нового resource-only allow-list и без blanket
rebase в S. Generic `sv_file_path` имеет другие правила, поэтому его нельзя
просто подставить вместо transfer validation. Export replace consent/conflict
и atomic rename из C003 не переносить в серверные обновления автоматически;
baseline не обещает сохранность старого destination после ошибки копирования.
Изолированный transfer-specific SV owner допустим, legacy/common refactor не нужен.

Validation различается по операциям: запрет leading `/` и любого `..` применяется
в baseline INIT. CHECK (`nclient.c:593–609`) и upload source
(`files.c:260–271,312–323`) вызывают mapping/read напрямую. Не распространять
INIT-only фильтр на них общим helper. Missing/open failure checksum — old 0 или
16 zero MD5 bytes; empty file — old 1 или MD5 empty digest. При read failure
отправляется checksum/digest прочитанного prefix; return flag helper не меняет
SUM на ERR. Semantic error и эти baseline wire bytes учитываются отдельно.

## Полнота пакета и ошибки

Неполный пакет не изменяет files/state и не выдаёт ответ. Полные chained events
обрабатываются последовательно, каждый со своими effects/completion.
Full DATA write failure — другой случай: `nclient.c:533–544` печатает ошибку
и возвращает 0 без ACK/ERR после удаления transfer из `local_file_write`.
Не превращать это в ожидание дополнительных bytes, повторную запись или
придуманный ACK. SV должен различать semantic disk failure и incomplete frame;
изменение wire recovery требует отдельного решения. Не переносить небезопасные
buffer accesses или утечки ресурсов как необходимую protocol семантику.
DATA length проверяется до копирования в bounded storage; unknown ID не оставляет
его payload для разбора как следующий packet. Если malformed frame нельзя безопасно
потребить с сохранением границ, вернуть явную protocol failure владельцу сессии
по ADR-0006; это не NEED_MORE и не повод отправлять выдуманный wire ответ.

Upload — полный flow `remote_update → CHECK/SUM → INIT → ACK-driven DATA → END
→ final ACK/ERR → cleanup`, включая checksum match без отправки файла.
ACK/ERR framing принятого B не подтверждает sender flow C010.
При EOF `do_xfers` ставит DONE и сохраняет READY; следующий pump pass посылает
END без нового ACK. Production loop (`c-init.c:3179`) вызывает pump отдельно
от incoming ACK. SV 004 должен дать такой progress и при отсутствии новых
incoming bytes; output capacity retry удерживает тот же chunk без повторного fread.

## Actual startup ingress и output

Текущий `protocol/login.c:sv_login_receive` хранит принятые bytes и может вернуть
`SV_BACKPRESSURE`; `login/flow.c:pump_login` пока считает любой не-OK fatal.
Задача 002 подключает FILE dispatch и настоящее drain/resume: при output wait
прекратить новые reads, сохранить input и после drain продолжить разбор без
повторного append. Reserve ответа до effects либо pending committed reply
исключает повторные INIT/DATA/END file/Lua effects. Socket short writes
продолжают исходный output tail. Sender 004 использует тот же output contract.

Legal modern DATA frame — 1030 bytes: header 4 + length 2 + payload 1024.
Текущий live `SvProtocol` buffer 1024 bytes недостаточен для целого frame.
Startup decoder выбирает достаточный bounded buffer или staged parsing с commit
полного frame; actual live adoption остаётся C014, не отдельный decoder для него.

## Lua и следующие consumers

[Решение пользователя о failed last END](../c009/source-contract.md): reload
текущих файлов выполняется при нуле всех transfers даже после failed close.
Фраза canonical END «does not falsely reload» не отменяет это решение:
transfer failure остаётся ERR, runtime/Guide results независимы и не создают
ложную readiness. Порядок close/publication → reload → audio warning → Guide
metadata → ACK/ERR. Accepted INIT немедленно фиксирует process-sticky flags;
disconnect и новый session их не сбрасывают. Подробности — в
[SV-C010-001](SV-C010-001-lua-reload-integration.md).

C010 предоставляет реальные decoded feature inputs и generation для Lua owner,
синхронно потребляет каждый END completion и удерживает предыдущие failures.
C011 позднее принимает полный preparation/readiness flow, C014 — actual
handoff/teardown/relog; их завершение не prerequisite C010.
Принятые B/C003/C009 автоматически не переоткрываются.

`login/native-run.c:778–784` сейчас создаёт application Lua owner только локально,
без attachment к login flow. Минимальное borrowed attachment, begin из реальных
decoded flags/version и close текущей attempt выполняет 001 в C010. Begin result
и aggregate имеют production владельца, reset по generation и copied итог для
C011; begin не повторяется при poll/redraw/retry output. Полный C011 readiness
и C014 handoff не требуются для подключения current startup caller C010.

## Вопросы и проверки

Новых блокирующих продуктовых вопросов анализ не выявил. Точные SV interfaces,
transfer limits и подключение dispatcher/sender выбираются при реализации по
этому контракту; изменение baseline или расширение набора проверок согласуется
отдельно. Только необходимый минимум core checks через production-путь на Linux;
Windows/MinGW/Wine отложены. Реализация C010 этим анализом не заявлена.

Повторный независимый анализ тремя агентами 2026-11-10 уточнил указанные startup,
backpressure, checksum/path и sender EOF contracts. Достаточно правок 001–005;
новых tickets и новых canonical IDs не требуется. Evidence 005 различает
подтверждённые, pending и неприменимые части каждого obligation; core subset
не превращает целый obligation или весь этап в PASS автоматически.

## Actual реализация — 2026-11-10

Исходная сверка выше сохраняется как baseline planning audit. Реализованы
`transfer/transfer.[ch]`, `transfer/startup.[ch]`, attachment/dispatch/tick в login
и native factory. Существующие C003 read-only bundled IO и export publication
не подменяют server wb publication: transfer пишет validated destination напрямую.
ENOENT CHECK — штатный zero SUM request обновления, не startup failure; реальные
open/read errors учитываются отдельно при сохранении baseline SUM.
[Linux evidence, observed results и pending limits](../../sv-c010-evidence.md).
