# SV-C004-003 — startup и character preference layers

Статус: **accepted**, 2026-11-09, в составе завершённого по решению пользователя C004.
Labels: enhancement
Зависимости: SV-C004-001/002, production settings/file core C002/C003.
Owned IDs: `capability.preferences.bootstrap`.
Контракт: [spec](../../sv-stage-c004-spec.md), [источники и load order](source-contract.md), [exact rows](coverage.json).

1. Подключить единый preference owner в `login/native-run.c`; lifetime завершать
   в общем cleanup. Передавать только необходимые references через login options.
2. Соблюдать policy order: defaults/shipped bootstrap → own global OPT → system
   preferences → selected effective map font/graphics preferences → own system
   OPT → own character OPT pre-send → macro orchestration позднего C052.
3. Не допустить legacy U/user/global.opt/options.prf base; сохранить shipped
   options fallback и отдельный S/options.prf entrypoint, без повторного исполнения
   include/action при наличии entrypoint в pref.prf. Никакого class OPT autoload.
4. Зафиксировать system-source selection. Предлагаемый SV-local fallback —
   совместимый B/user/pref-sdl3.prf, когда U/B pref-sv.prf отсутствует; не менять
   глобальный ANGBAND_SYS или shared legacy файл. Сохранить trigger bytes как
   данные, не считать input recognition доказанным. Unsupported trigger consumer
   не должен молча выглядеть работающей binding.
5. Сохранить `flow.c` character overlay до первой отправки options. Retry/смена
   персонажа начинается со свежего base, не с предыдущего overlay. Локальный
   layout остаётся источником wire big_map, shared PRF не создаёт Term topology.
6. Resource-stage hook стоит до global-sv.opt. До C006/C007 нет выдуманного map
   consumer: text font не подменяет map font; enabled mapping/reload evidence pending.
7. Одноразовые effects имеют явного получателя и owner. Session actions привязаны
   к актуальному поколению/attempt; старые actions не перетекают в retry, redraw
   не повторяет эффекты. Startup actions не запрещать новым blanket gate: если
   runtime ещё отсутствует, это pending обязательство, а не изменённая семантика.
8. Arcade special load: сохранить различие automatic/manual и исходный gate;
   actual server applicability и filename selection документировать до включения,
   не добавлять tail-load ко всем явным запросам и не реализовывать C052 reloads.

Минимум проверок: тот же U/S/B fixture с competing layer values; существующий
production login/options packet путь подтверждает character-before-send и fresh
retry. Проверить cleanup/effect ownership и отсутствие CFG/OPT/source writes.
Один короткий Linux startup observation подтверждает actual assembly при
необходимости; полная B/B2 visual/input matrix не требуется.

Готовность core: current startup реально вызывает новый loader, existing option
packet semantics сохранены. Отсутствующие handoff/map/macro consumers и C/D action
regressions фактически ещё не проверены; дальнейшие work/checks закреплены
в следующих этапах и не блокируют принятое решение о завершении C004.

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
