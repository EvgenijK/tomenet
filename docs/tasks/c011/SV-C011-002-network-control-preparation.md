# SV-C011-002 — подготовка live network control owner

Статус: **accepted** — Linux preparation/core, 2026-11-11, по решению пользователя.
Labels: enhancement
Owned IDs: 0 новых IDs; integration obligation принятого core C008.
Зависимости реализации: production C008, C011 session preparation/preferences.

## Основа и production путь

[C008 принят](../../sv-stage-c008-acceptance.md) в границах core. Эта работа
обязательна C011, не блокирует и не переоткрывает C008 автоматически.
[Evidence](../../sv-c008-evidence.md), [source contract](../c008/source-contract.md),
[CONTEXT](../../../CONTEXT.md), ADR
[0002](../../adr/0002-main-thread-state-ownership.md),
[0003](../../adr/0003-explicit-module-connections.md),
[0005](../../adr/0005-session-scoped-work.md),
[0006](../../adr/0006-errors-and-overload.md).

Использовать `app.[ch]`, `ui/native-controls.[ch]`, `input/native-input.[ch]`
и существующих owners `login/flow.c`, `protocol/login.c`; второй decoder или
control engine не создавать.

## Работа и минимальная Linux проверка

1. Подготовить live owner и явные bindings render submission/native input;
   передать effective `disable_flush`/`thin_down_flush` из profile/preferences.
2. Определить передачу socket, generation, version/setup, unread bytes и pending
   sends без потерь и replay. Login END сохраняет отдельный startup lifecycle;
   gameplay END/KEYPRESS остаются payload-free controls.
3. Передать C014 проверяемый handoff contract и ownership callbacks/resources.
   Подключаемый packet subset требует настоящих decoders; известный baseline
   packet не становится unknown из-за отсутствующего позднего consumer.
   Перенос всех поздних packet handlers не входит скрыто в C011.
4. Через production preparation path проверить effective options/bindings,
   coherent generation и однократную передачу unread/pending data, включая
   startup END, unread control bytes и неполный поддерживаемый packet с payload
   на границе handoff. Gameplay END/KEYPRESS — один байт без payload; сами эти
   controls нельзя дробить на partial frame. Actual live connection handoff
   проверяет C014; live map submission — C013. Не выдавать им PASS по snapshot.

Готовность: подготовленный production owner и применимые preparation checks,
явный contract для C013/C014 и ограничения отсутствующих recipients.
Trace — `obligation.network.*` C008; allocation и canonical statuses неизменны.
Достаточен минимальный core набор; расширение проверок согласовать по AGENTS.
Только Linux, Windows/MinGW/Wine deferred. Позднее evidence можно связать с C008
для трассировки, но его отсутствие не является backward gate.

## Уточнение production boundary — 2026-11-11

C010 уже останавливает startup ingress после END и предоставляет
`sv_login_take_remaining`; использовать этот seam. Удержать также
`attempt.wire.remaining`, tail `wire.sending` с offset sent и ещё не извлечённый
output `SvLogin`. Передача только unread bytes не переносит pending sends.
Move-once contract либо retention существующего owner не допускает дубликатов.

Login generation сейчас задаётся `SDL_GetTicksNS`, `SvApp` увеличивает собственное
поколение, native input имеет отдельный epoch. Выбрать единое generation либо
явный mapping; произвольное копирование числа не доказывает coherent lifetime.
Использовать effective character flush options до сериализации options внутри PLAY;
standalone pre-PLAY `PKT_OPTIONS` не отправляется.
Перед подключением stream C014 проверяет доступность настоящих consumers;
C011 не расширяет decoder всеми поздними игровыми packets.

## Comments

2026-11-11 — реализация и проверка по запросу пользователя. Подготовлен SvApp с effective flush options, явным login↔app generation mapping и свежим native epoch; SvNativeControls связан с renderer/input. Retained flow сохраняет socket, unread и partial-send offsets. Public transport/deferred drains соблюдают order/once; actual live loop C014 не реализовывался. Native PAUSE/timing не заявлены runtime PASS: проверены bindings, два FLUSH и обязательный core scope.

[Финальное evidence, команды и ограничения](../../sv-c011-evidence.md).
Пользовательская приёмка не заявлена; Windows проверки отложены.


2026-11-11 — follow-up по запросу пользователя закрыть оставшиеся проверки самого C011. Actual native PAUSE/thin pacing закрыты: два submitted PAUSE, stale rejection и fresh printable/Return acknowledgement, pending request не отвечен; text input выключен до teardown. Исправлен bounded ingress batch: 12 FLUSH → 10 paced / 2 unpaced. Native redraw ограничен post-MOTD preparation context.

Результаты и границы — [обновлённое evidence](../../sv-c011-evidence.md). Работы следующих этапов не выполнялись.


2026-11-11 — пользователь явно принял C011 и задачи 001–006 в проверенных границах Linux preparation/core. Задача завершена; [решение о приёмке](../../sv-stage-c011-acceptance.md). Исторические записи выше отражают состояние до приёмки; поздние obligations и canonical runtime statuses не повышаются автоматически.
