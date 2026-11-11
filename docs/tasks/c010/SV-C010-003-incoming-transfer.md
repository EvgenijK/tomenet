# SV-C010-003 — FILE_INIT/DATA/END и владение входящей передачей

Статус: accepted — Linux startup/core, 2026-11-11.
Labels: enhancement, ready-for-agent
Owned IDs: `capability.transfer.init`, `capability.transfer.data`, `capability.transfer.end`.
Зависимости реализации core: C003 resource-overlay/transient-owner и framing/interface 002.
Полное завершение END: связка 001 с принятым Lua core C009; это closure результата,
а не prerequisite создания owner/API для 001/004.

## Основа и работа

Нормативны [план](README.md), [planning snapshot](coverage.json),
`Receive_file` (`src/client/nclient.c:477`) и `local_file_init/write/close`,
`get_xfers_num`, `kill_xfers` в `src/common/files.c`. Сохраняются 12
`obligation.transfer.<init|data|end>.*`; END принимает полный outcome только
вместе с [001](SV-C010-001-lua-reload-integration.md).

1. Создать SV-local transfer owner с реальными ID/state/file ownership и явными
   результатами, подключённый к production decoder. Изолировать SV от общего
   `fdata`; существующий legacy/common код менять только при необходимости задачи.
   Сохранить baseline mapping/validation, temporary creation/open и допустимое
   обращение с ID, включая неизвестные/повторные IDs; не вводить новый allow-list.
2. INIT принимает destination и temporary ownership до DATA, failed open/validation
   не объявляет success. C002 `-u` отклоняет все INIT, не только Lua filenames.
   Успешный INIT немедленно передаёт occurrence в 001, до следующего packet.
3. DATA сохраняет исходные unsigned length/ID rules, порядок chunks и точные bytes.
   Полный payload проверяется до file effects; incomplete input откатывается без
   частичной записи. Не смешивать missing transfer, incomplete payload и disk failure.
   В baseline `local_file_write == -2` после полного DATA означает diagnostic и
   return 0 без ACK/ERR: сохранить этот wire outcome с явным semantic failure,
   не возвращать consumed DATA как NEED_MORE и не записывать его автоматически
   повторно. Malformed length и unknown-ID DATA требуют bounded, framing-safe
   validation, без unsafe fixed-buffer поведения legacy. Если безопасное продолжение
   невозможно, вернуть явную session failure по ADR-0006, без нового неоговорённого
   retry/ERR wire protocol. Изменение recovery — отдельное решение, не скрытый перенос.
4. END закрывает/публикует actual file и освобождает соответствующий transfer по
   baseline, с точными ACK/ERR, EACCES/прочими failure outcomes. Legacy publication
   — copy в destination через `wb`, не atomic replace; не обещать несуществующий
   rollback или сохранность старого файла после любой ошибки публикации.
   Применить C003 ownership без изменения file semantics.
   Для полного INIT/DATA/END commit и reply соблюдать output contract 002:
   backpressure не повторяет temp creation/write/close или Lua effects и не
   превращает уже обработанное событие в incomplete packet.
5. Передать 001 close result и число всех активных transfers, включая sender,
   до ACK/ERR. Последний END вызывает Lua reload и после failed close по решению
   2026-11-10; ERR остаётся ERR. Сохранить раздельные transfer/runtime/Guide results
   и накопленный startup failure. Каждый complete END синхронно потребляется до
   следующего, включая chained END; новый wire END с тем же fnum не подавляется
   deduplication. Подробный порядок effects закреплён в 001.
6. Дать production startup/lifecycle owner явные begin/close/generation interfaces:
   stale input не меняет files/runtime и не отправляет replies новой сессии;
   отмена освобождает собственные handles/temp files, не удаляет общие resources.
   Полный live close/relog consumer остаётся C014, preparation/readiness — C011.

## Минимальная проверка и завершение

Через production ingress в 005 подтвердить actual owned paths/content и emitted
bytes для INIT→DATA→END, wrong/zero ID, failed open/write/close, split/chained packets,
cancellation и stale generation. Совместить это с 001: несколько transfers,
last-END reload, failed last publication, sticky flags и independent results.
Проверять текущие Linux branches, без нового harness/broad suite; расширение
проверок заранее согласовать. C011/C014 consumers не получают PASS от core API.
Завершение: три outcomes и все 12 obligations traceable, без test-only поведения,
ложной публикации или потери причины failure.

## Результат реализации — 2026-11-10

Реализован `transfer/transfer.[ch]`: registry/temp ownership, INIT/DATA/END, baseline wb publication, generation/cleanup и synchronous Lua completion. C003 export prompts не применяются; legacy/common не изменены.

[Evidence, команды и ограничения](../../sv-c010-evidence.md).
Canonical statuses/ownership сохранены; pending flows не получают PASS.

## Comments

### 2026-11-11 — приёмка пользователя

Пользователь подтвердил: «Этап принимаю». Задача принята в границах
[решения о приёмке C010](../../sv-stage-c010-acceptance.md). История реализации,
evidence и непроверенные условия сохраняются; canonical statuses не меняются.
