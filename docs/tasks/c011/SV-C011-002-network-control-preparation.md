# SV-C011-002 — подготовка live network control owner

Статус: planned, 2026-11-09.
Labels: enhancement, ready-for-agent
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
   partial control packet на границе handoff. Actual live connection handoff
   проверяет C014; live map submission — C013. Не выдавать им PASS по snapshot.

Готовность: подготовленный production owner и применимые preparation checks,
явный contract для C013/C014 и ограничения отсутствующих recipients.
Trace — `obligation.network.*` C008; allocation и canonical statuses неизменны.
Достаточен минимальный core набор; расширение проверок согласовать по AGENTS.
Только Linux, Windows/MinGW/Wine deferred. Позднее evidence можно связать с C008
для трассировки, но его отсутствие не является backward gate.
