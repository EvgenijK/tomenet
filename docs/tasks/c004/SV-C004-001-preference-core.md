# SV-C004-001 — production PRF interpreter и данные

Статус: **accepted**, 2026-11-09, в составе завершённого по решению пользователя C004.
Labels: enhancement
Зависимости: принятый production core C002/C003.
Owned IDs: `capability.preferences.load`.
Контракт: [spec](../../sv-stage-c004-spec.md), [source contract](source-contract.md), [exact rows](coverage.json).

1. Сверить record table/gates с текущим baseline. Сохранить distinctions active,
   accepted inert, unsupported и meaningful data с отсутствующим consumer.
2. Развить `settings/options.[ch]` через один SV-local preference owner
   (`settings/preferences.[ch]` — предлагаемый путь). Сохранить wrappers,
   defaults/slots/exact aliases; дать production option application seam.
3. Реализовать grammar, include dispatch в позиции строки, action A state,
   mappings, A/P/H/C/D definitions, raw S record и W compatibility disposition.
   Не использовать single-byte `SvInputBindings` как полное PRF macro storage.
4. Владеть payload bytes/source provenance; `text_to_ascii` semantics перенести
   узко локально. Macro-only фильтр идёт после `%`; trigger/body gates точные.
5. `#:` — одноразовый message effect, `!/?` — baseline prepend action effect с
   marker 29 и execution-state contract. Связать существующего получателя явно;
   если consumer отсутствует, сохранять factual pending, не выдавать enqueue за
   исполнение и не посылать bytes в login/password text field.
6. Очередь и данные ограничены с явной диагностикой переполнения, без скрытой
   потери/усечения action. Не вводить произвольные меньшие лимиты вместо baseline
   MACRO_MAXLEN/очередных ограничений; значимые limits зафиксировать в evidence.

Минимальный core case через public production API: mixed records с include,
exact alias/data bytes, A state через include/последующий load, macro types/delete,
W без Term, # escaping/chat marker, два !/? prepend effects и gates. Совместить
кейсы в существующем core entrypoint; full macro gameplay suite не добавлять.

Готовность core: production owner используется загрузчиками, direct core checks
проверяют ту же реализацию. `.result/.allocation-checks` остаются частично pending
до реальных callers/consumers, `.failure-lifecycle` уточняется вместе с 002/003.
Не заявлять full load outcome PASS по одному parser/store.

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
