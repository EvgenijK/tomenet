# SV-C010-002 — FILE_CHECK, checksum и wire branches

Статус: accepted — Linux startup/core, 2026-11-11.
Labels: enhancement, ready-for-agent
Owned IDs: `capability.transfer.check`.
Зависимости реализации: принятые B protocol/transport и C003 resource-overlay/transient-owner; общий transfer interface с 003/004.

## Основа и работа

Нормативны [план этапа](README.md), [точный planning snapshot](coverage.json),
`Receive_file` в `src/client/nclient.c:477` и checksum/path functions в
`src/common/files.c`. Сохранить четыре `obligation.transfer.check.*` и все
canonical conditions/prerequisites; инженерный порядок их не меняет.

1. Добавить SV-local production `PKT_FILE` framing/dispatch, используя существующие
   protocol/transport owners. Header — command и unsigned 16-bit fnum; zero ID,
   неизвестный command и command 0 сохраняют соответствующее baseline поведение.
   Частичный header/filename/checksum/DATA не фиксирует effects или replies;
   полные chained packets обрабатываются последовательно, соседний пакет сохраняется.
   Подкоманды SUM/ACK/ERR передаются настоящему sender 004, incoming — 003.
   Явно подключить decoder к текущему startup ingress `protocol/login.c` через
   login flow; standalone decoder без actual dispatch не завершает задачу.
   Выбрать bounded buffering, вмещающий legal modern DATA frame 1030 bytes
   (4 header + 2 length + 1024 payload), либо эквивалентный staged parser без
   effects до полного frame. Текущие 1024 bytes live `SvProtocol` нельзя считать
   достаточной ёмкостью; его actual adoption остаётся C014.
2. CHECK сохраняет собственные baseline path rules и destination mapping. Решение Q40
   не допускает нового resource-only allow/deny-list и blanket rebase в SV profile.
   Сверить `client_user_path`, `local_file_check`/`local_file_check_new` и C003;
   не использовать account-secret storage как обычный transfer backend.
   INIT-only запрет leading `/`/substring `..` не распространять автоматически
   на CHECK или upload source: baseline checksum functions вызывают mapping/read
   напрямую. Не вводить общий transfer filename filter для разных операций.
3. Точный version gate — strict `> 4.6.1.1.0.1`: новая ветвь возвращает MD5 как
   четыре big-endian 32-bit words (16 digest bytes), старая — исходный broken
   Adler32 checksum. Не исправлять bug-compatible checksum при переносе.
   Сохранить результаты missing/open/read/invalid path и точные SUM bytes;
   CHECK не подменяется общим ACK/ERR из INIT/DATA/END.
   Missing/open failure даёт old checksum 0 или MD5 из 16 zero bytes; empty
   file — old checksum 1 или обычный MD5 empty digest. При read failure baseline
   посылает накопленный prefix checksum/digest: `Receive_file` игнорирует return
   flag checksum helper и всё равно отвечает SUM. Semantic read failure вернуть
   отдельно; не заменять эти wire outcomes на FILE_ERR или общий zero fallback.
4. Session generation проверяется до чтения/записи и replies; partial input и
   output-capacity retry не повторяют уже выполненный CHECK. Сохранить Linux
   compile/runtime branches исходного owner; Windows checks отложены.
5. Зафиксировать единый output/backpressure contract с `login/flow.c:pump_login`:
   сейчас любой `sv_login_receive != SV_OK` считается fatal. Разделить ожидание
   output capacity и protocol failure; после drain возобновлять buffered dispatch
   без нового socket read и без повторной подачи уже скопированных bytes.
   Пока output блокирует progress, pump прекращает новые reads; уже принятый
   input остаётся у своего владельца, incomplete/output wait/failure различаются.
   CHECK/SUM и INIT/DATA/END reply либо заранее резервируют место до effects,
   либо удерживают bounded pending reply/result после единственного commit.
   Socket short write продолжает тот же output tail; retry не повторяет чтение
   файла, publication, Lua reload или sender chunk. Реальное drain/resume
   подключение — часть C010, без ожидания C011/C014.

## Минимальная проверка и завершение

В общем минимальном production-core подходе 005 наблюдать exact SUM bytes по обе
стороны и на границе версии, baseline checksum/path outcomes, missing/invalid input,
split/chained framing и stale generation. Test-only parser/checksum/path модель
не является evidence. Отдельный harness, broad matrix и новые независимые suites
не вводить; расширение проверок заранее согласовать по AGENTS.
Завершение: CHECK работает через production protocol → transfer owner → filesystem
и реальный output, с явными failure results и trace всех четырёх obligations.

## Результат реализации — 2026-11-10

Реализованы SV-local CHECK/checksum/framing и actual FILE dispatch в `protocol/login.c`; output drain/resume интегрирован в `login/flow.c`. Strict version gate, operation-specific paths и bounded 1030-byte frame подтверждены core-проверкой.

[Evidence, команды и ограничения](../../sv-c010-evidence.md).
Canonical statuses/ownership сохранены; pending flows не получают PASS.

## Comments

### 2026-11-11 — приёмка пользователя

Пользователь подтвердил: «Этап принимаю». Задача принята в границах
[решения о приёмке C010](../../sv-stage-c010-acceptance.md). История реализации,
evidence и непроверенные условия сохраняются; canonical statuses не меняются.
