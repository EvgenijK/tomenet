# SV-C011-005 — запрос redraw и восстановление projections

Статус: **accepted** — Linux preparation/core, 2026-11-11, по решению пользователя.
Labels: enhancement
Owned ID: `capability.session.redraw` (3 obligations).

## Production путь и работа

1. Подключить executor redraw к существующему production command path:
   `app.c:local_command` уже классифицирует Ctrl-R (18), `command_pending`
   удерживает команду, `sv_app_take_command` извлекает её. Второй input router
   не нужен. Сохранить command-context binding Ctrl-R по
   `c-cmd.c:cmd_redraw` и `nclient.c:Send_redraw`.
   Wire request — ровно `PKT_REDRAW` + mode byte 0; существующий recovery
   prefix `PKT_UNKNOWN` к пользовательскому request не добавлять. Local expose/resize/repaint
   не должен автоматически посылать игровой запрос.
   Ctrl-R не является глобальным shortcut: в text prompt baseline использует
   его для поиска по истории (`c-util.c:askfor_aux`). Credential/editor/modal
   input не должен попадать в gameplay redraw executor.
2. Инвалидировать применимые presentation/render caches и восстановить текущие
   projections из authoritative session state. Baseline сбрасывает cached huge
   hp/sp/mp/stamina bars; SV использует собственную invalidation/revision,
   без `Term` или копирования старого raster как источника данных.
3. Не повторять preferences, Lua open/reload, передачи файлов, отправленные
   commands и startup effects. Backpressure удерживает одну action occurrence
   без повторного request; stale generation не отправляет redraw новой сессии.
   `sv_app_take_command` очищает pending slot до отправки: после take executor
   обязан удержать команду до успешного queue/явной отмены либо использовать
   минимальный peek/commit contract. Не терять её при BACKPRESSURE и не принимать
   новую occurrence за retry уже поставленного request. Invalidation выполняется
   один раз для принятой occurrence.
4. Явно связать существующие render/model owners. Полные HUD C012, карта C013
   и поздние surfaces остаются их этапами: дать им обязательный contract
   invalidation и отметить pending combined evidence. Успешный no-op callback
   отсутствующего consumer не доказывает восстановление его projections.

## Необходимый минимум Linux проверки

Через production action/input path проверить точные request bytes, однократную
отправку, stale generation и rebuild имеющегося state при interleaved update.
Проверить, что Ctrl-R в активном text input context не выдаёт `PKT_REDRAW`;
реализация полного history-search UI остаётся владельцу соответствующего prompt.
Повторный local repaint/resize не добавляет packets и не повторяет effects.
Использовать существующий core подход, без нового visual matrix/harness.

Готовность: работающий action/serializer и восстановление доступных projections;
result/lifecycle/allocation-checks имеют раздельный trace. Отсутствующие поздние
consumers перечислены явно, их evidence не получает PASS. Общие правила —
[план](README.md), [source contract](source-contract.md). Только Linux сейчас.

## Comments

2026-11-11 — реализация и проверка по запросу пользователя. Existing command slot исполняет user redraw, атомарно ставит PKT_REDRAW+0, удерживает occurrence при output backpressure и публикует coherent view/redraw revision. Quantity context и stale generation не посылают redraw. Full поздние HUD/map projections остаются C012/C013.

[Финальное evidence, команды и ограничения](../../sv-c011-evidence.md).
Пользовательская приёмка не заявлена; Windows проверки отложены.


2026-11-11 — follow-up по запросу пользователя закрыть оставшиеся проверки самого C011. Усилена проверка coherent redraw при interleaved HP/message: первая submission сохраняет предыдущий целостный view, следующая показывает обновлённый; message delivery не потребляется redraw. Native Ctrl-R gate учитывает post-MOTD preparation context; full HUD/map/live resize остаются поздним owners.

Результаты и границы — [обновлённое evidence](../../sv-c011-evidence.md). Работы следующих этапов не выполнялись.


2026-11-11 — пользователь явно принял C011 и задачи 001–006 в проверенных границах Linux preparation/core. Задача завершена; [решение о приёмке](../../sv-stage-c011-acceptance.md). Исторические записи выше отражают состояние до приёмки; поздние obligations и canonical runtime statuses не повышаются автоматически.
