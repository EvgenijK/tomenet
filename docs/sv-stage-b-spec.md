# Этап B — рабочие native pregame screens TomeNET SV

Статус: specified, граница подтверждена пользователем 2026-10-05. Части
endpoint/input, contact protocol, credentials и existing-character login уже
имеют production-реализацию прежнего широкого B, но требуют проверки и
доработки по этой суженной границе. Account creation, character creation и birth
flow ещё не реализованы; полная acceptance B остаётся pending. Canonical scope:
**54 active B outcomes**; прежний denominator этапа B устарел.

Спецификация фиксирует high-level scope и evidence boundary. Она не изменяет
canonical JSON, не объявляет native claims и не заменяет implementation tickets.

## Цель

Игрок проходит полностью нативный flow подключения, аккаунта, аутентификации,
выбора либо создания персонажа и peer-driven MOTD. Все экраны используют
production UI/model/input/transport/protocol path и проверяются через управляемый
protocol peer.

Этап заканчивается после выбора или создания персонажа, показа MOTD и перехода
в явное состояние ожидания live-session handoff. Реальная интеграция с TomeNET
server, post-MOTD startup и первый игровой экран относятся к функциональным
этапам после B, начиная с C001.

Нормативная граница: [decision 37](../.scratch/single-window-sdl3-client/issues/37-narrow-stage-b-to-pregame-flow.md#answer)
и [разбор границ](sv-stage-b-boundaries.md). Архитектурная основа —
[SV architecture](sv-architecture.md) и ADR-0001…0006. Правило изоляции SV из
[AGENTS.md](../AGENTS.md) имеет приоритет: production adaptation по возможности
остаётся в SV, а изменения legacy/common минимальны.

Текущее распределение: **A8/B54** заморожены; остальные **863 outcomes**
распределены по C001–C059 согласно
[функциональному плану](sv-functional-stage-plan.md).
B содержит только следующие семейства:

| Семейство | Outcomes B | Роль в результате |
|---|---:|---|
| connection | 5 | endpoint, contact и failure/cancel |
| account | 11 | account/password/auth и secret lifecycle |
| character | 8 | overview, existing/create slot, name и выход |
| birth | 15 | choices, backtracking, quit и complete |
| session | 2 | MOTD и disconnect |
| network | 5 | keepalive/ping, partial/malformed и server flags |
| input | 4 | physical keys, prompt navigation, text edit, confirm |
| credentials | 4 | lookup, Linux/Windows providers и private input |

## Включённый flow

### Подключение

- endpoint surface, локальная проверка адреса и выбранного порта;
- nonblocking connect/wait/error/retry/cancel;
- production transport и pregame negotiation с protocol peer;
- понятный возврат после отказа, disconnect или teardown;
- отсутствие старых session results после нового подключения.

### Аккаунт и аутентификация

- существующий account login и предусмотренный baseline account-creation flow;
- private password input без утечки в history, diagnostics или visible state;
- source-defined field-byte limits, encoding и protocol-version rules;
- server-shaped success/rejection и повторный ввод без потери допустимого draft;
- защищённое сохранение credentials в утверждённые моменты; при недоступности
  provider — session-only behavior без plaintext fallback.

### Управление персонажами

- получение и отображение character list от peer;
- стабильная slot identity и выбор существующего персонажа;
- создание ordinary/exclusive slot, ввод имени и отмена по baseline;
- empty/full/rejected list states, back/cancel/retry и disconnect;
- переход выбранного персонажа в MOTD/live-session handoff без карты или HUD.

### Создание персонажа

- имя и все server-authoritative этапы выбора, относящиеся к birth flow;
- доступные race/trait/class/body/stats/mode и их server-shaped ограничения;
- backtracking, cancel, rejection и повторный выбор с сохранением корректного
  parent/selection state;
- подтверждение создания peer и тот же MOTD/live-session handoff, что для
  существующего персонажа;
- Guide entry, если он присутствует на этих экранах, показывает только точную
  нативную заглушку `The guide is in development` и возвращает в реальный caller.

### MOTD и общая pregame-инфраструктура

- server-shaped MOTD surface и baseline close/continue behavior без перехода к
  карте или HUD;
- один input router и контекстные keyboard/mouse bindings этих экранов;
- production decoder, model, interaction state, serializer и UI;
- loading/error/disconnected states без terminal fallback;
- session generation, cancellation и очистка pending work;
- Secret Service на Linux и Credential Manager на Windows по принятой credential
  policy;
- минимальная text rendering/layout foundation из A, необходимая для читаемых
  экранов, без принятия отдельного gameplay-rendering scope.

## Управляемый protocol peer

Peer является контролируемой внешней стороной wire conversation. Он подключается
к тому же production transport/protocol path, который будет использовать реальный
сервер, и может детерминированно задавать:

- существующий и новый account flows;
- разные character lists и допустимые birth choices;
- MOTD content и продолжение до live-session handoff;
- success, rejection, retry и disconnect на каждом ожидании;
- packet fragmentation на границах полей и chained responses;
- поддерживаемые protocol/version ветви, относящиеся к pregame screens;
- malformed или неполный input для проверки отсутствия premature effects.

Fixtures могут задавать wire input и ожидаемый внешний результат, но не заменяют
production decoder/model/router/serializer/UI собственной реализацией поведения.
Headless rule checks дополняют, а не заменяют real-window interaction checks.

## Не входит в B

B не принимает и не обязан загружать ради старого `session.enter-game`:

- реальный TomeNET server round trip;
- post-MOTD `Net_start` и остальной live startup sequence;
- client profile, gameplay settings, preference/macro/INS processing;
- FILE transfer, Lua reload, DNA/game resources и first-launch import;
- карту, HUD/status, messages/chat или gameplay presentation model;
- map renderer, tiles, weather, lighting, audio и gameplay alerts;
- movement, targeting, items, stores, combat и другие игровые actions;
- gameplay quit/reconnect, death/tomb/final review;
- exports/screenshots/OS integrations и остальные прежние B integrations.

Конкретный outcome остаётся в B только если он самостоятельно проверяет
включённый pregame behavior. Общая utility или parser capability не считается
принятой для всех поздних callers. Составной `session.enter-game` и его
live-session prerequisites остаются в C либо уточняются отдельными атомарными
outcomes в canonical registry.

## Реализация в согласованных модулях

Следовать ADR о [разделении model/interactions/UI](adr/0001-separate-session-interactions-and-ui.md),
[главном потоке](adr/0002-main-thread-state-ownership.md),
[явных связях](adr/0003-explicit-module-connections.md),
[read views](adr/0004-revision-based-ui-views.md),
[session generations](adr/0005-session-scoped-work.md) и
[ошибках](adr/0006-errors-and-overload.md).

Application собирает lifecycle owners; transport/protocol владеет wire/version
правилами; session model публикует coherent pregame state; interaction владеет
pending choice и parent; UI читает state и передаёт смысловой ввод. Один input
полностью применяется до следующего. Ожидание пользователя или peer не блокирует
UI, network progress и teardown.

Пакет сначала полностью декодируется; incomplete packet не публикует state и не
вызывает reply. Outgoing packet валидируется и ставится целиком либо не ставится.
UI Unicode и wire bytes остаются разными представлениями. Cancel/Escape следует
конкретному caller contract, а не универсальному abort.

## Acceptance

B принимается только при выполнении всего применимого набора:

1. Existing-account → existing-character и account/character-creation branches
   проходят через production executable с управляемым peer.
2. Каждый экран, включая MOTD, реально отрисован в одном SDL window; keyboard
   navigation, private text input, mouse bindings где предусмотрены, resize/focus
   и возврат focus работают без terminal fallback.
3. Success/rejection/back/cancel/retry и disconnect проверены на каждом
   существенном ожидании; новая generation не принимает старый reply или task.
4. Split/incomplete/malformed packets и outgoing queue pressure не создают
   partial state, partial send или повторный side effect.
5. Field limits и encoding проверены на границах; secret не попадает в logs,
   history, diagnostics, screenshots или plaintext files.
6. Credential provider проверен на применимых Linux и Windows paths, включая
   provider unavailable и session-only fallback.
7. Managed-peer identity, scenario/version matrix, executable/configuration и
   platform fingerprints записаны в evidence; synthetic foundation A имеет
   актуальные regression results.
8. Stage report явно говорит, что live-server compatibility, post-MOTD startup и
   первый game screen не проверялись и не являются B claims.

Применимые submission budgets **20/50/200 ms** и короткие lifecycle/concurrency
checks сохраняются. Stress/soak, XHTML и global memory ceiling не добавляются.
Реальные Windows checks остаются обязательными там, где этого требует общий
platform acceptance contract; Wine не выдаётся за Windows acceptance.

## Выход и граница C

Успешный B даёт переиспользуемый production pregame path и детерминированную
protocol test boundary. Следующий этап подключает тот же path к реальному серверу,
выполняет post-MOTD startup/profile/FILE/Lua/resources и впервые показывает
авторитетные map/HUD data. Подробная high-level граница находится в
[спецификации C](sv-stage-c-spec.md).

Точные capability IDs, prerequisites и owners задаются финальным canonical
registry и stage allocation. B принимает только его 54 active outcomes; никакая
task-level зависимость не разрешает возвращать прежний live-session/gameplay scope.
