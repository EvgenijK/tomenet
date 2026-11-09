# SV-C004-002 — includes, resolved owners и ошибки

Статус: **accepted**, 2026-11-09, в составе завершённого по решению пользователя C004.
Labels: enhancement
Зависимости: SV-C004-001; тот же parser/state, отдельная реализация не создаётся.
Owned IDs: `capability.preferences.include-failure`.
Контракт: [spec](../../sv-stage-c004-spec.md), [источники](source-contract.md), [exact rows](coverage.json).

1. Различить optional top-level, explicit/manual load и обязательный nested
   include. Missing optional не выдаёт ложную ошибку; missing selected/include
   не маскируется clean success, как сейчас в ограниченном C002 loader.
2. Использовать C003 file owner API. Хранить logical source role, physical
   resolved owner/path и include origin; fallback path не переименовывает
   логическую роль, но factual source identity должна быть точной.
3. Собственные option entrypoints разрешать в S, shipped bootstrap — в B,
   explicit shared macro/resources — U/B. Includes сохраняют provenance;
   суффикс PRF не меняет владельца. Absolute/custom explicit sources сохраняют
   согласованное значение; не навязывать им basename-only правило automatic load.
4. Сохранить существующий bounded stack/cycle и owner-escape protection, выявлять
   canonical-path cycles. Missing/invalid/unreadable/cycle дают диагностику;
   далее читать валидные строки normal load без rollback прежних effects.
5. Вернуть clean/partial/unavailable/fatal вместе с contextual diagnostics.
   Не повторять legacy aggregate 0 как отсутствие ошибок. Не обещать snapshot
   isolation при concurrent external file edits, которого file API не даёт.
6. Не писать `.tmp`, не комментировать исходник через `#ERROR:` и не autosave OPT.
   Retained compatible source records оставить future Save/import owner.

Минимальные cases входят в mixed fixture 001: S/U/B с различимыми значениями,
shared→bundled fallback, missing optional против missing include/manual,
malformed/cycle и valid tail, доступный failure path и неизменность source bytes.
Не строить general filesystem fault framework и Save/replace matrix.

C051 отдельно подключает parse/stage без исполнения и linked-group all-or-skip
при missing/cyclic/out-of-source includes. Этот flow здесь не реализуется,
не становится not-applicable и не принимается по normal-load continuation.
Готовность core: реальные owners/results доступны current production callers,
include-failure obligations имеют core evidence и явный pending C051 disposition.

## Результат реализации

Current production core этой задачи реализован и прошёл минимальные Linux проверки;
полное outcome evidence для отсутствующих consumers остаётся pending.
[Evidence и точные ограничения](../../sv-c004-evidence.md),
[сверка готовности](../../sv-stage-c004-acceptance.md). Canonical статусы не повышены.

## Решение пользователя и дальнейшая работа — 2026-11-09

Задача завершена и принята вместе с C004. Remaining integrations/checks относятся
к [самостоятельным планам следующих этапов](README.md#самостоятельные-обязательства-следующих-этапов).
Они не являются gate завершения этой задачи, не переоткрывают её автоматически;
найденные там проблемы исправляются в рамках этапа-владельца. Фактический
pending evidence не переписан как PASS. Возврат evidence в C004 — optional trace,
не prerequisite уже состоявшегося завершения.
