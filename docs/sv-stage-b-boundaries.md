# Границы этапа B — native pregame flow

Дата решения: 2026-10-05. Статус: подтверждено пользователем и материализовано
в canonical allocation; implementation tickets обновляются отдельно.

Нормативное решение: [Narrow stage B to the native pregame flow](../.scratch/single-window-sdl3-client/issues/37-narrow-stage-b-to-pregame-flow.md#answer).
Оно заменяет прежнее определение B как «живая сессия и игровой экран» в
[исходной последовательности](../.scratch/single-window-sdl3-client/issues/29-sequence-surface-migration-and-retire-terminal-fallback.md#answer).
Перенос Guide в G из [issue 36](../.scratch/single-window-sdl3-client/issues/36-defer-guide-to-stage-g.md#answer)
остаётся в силе.

## Новая граница

B реализует минимально необходимый, полностью рабочий native flow экранов:

- выбор endpoint и подключение;
- создание аккаунта и аутентификацию;
- список, выбор и предусмотренное pregame-управление персонажами;
- полный flow создания персонажа;
- peer-driven MOTD, ожидание, отказ, back/cancel/retry и disconnect;
- production UI/model/input/transport/protocol path;
- credentials и защищённое хранение, необходимые account flow.

Внешней стороной проверки служит управляемый protocol peer. Он говорит с
production transport/protocol path и задаёт нормативные ответы, отказы,
фрагментацию и разрывы соединения. Он не заменяет production decoder, model,
interaction state, input router, serializer или UI.

Конечное состояние B — выбранный либо созданный персонаж, показанный MOTD и
явный handoff к следующей фазе. Это ещё не `session.enter-game`, не живая игровая
сессия и не первый игровой экран.

## Что не входит в B

В C переходят все outcomes, которые ранее находились в B, но не нужны для
рабочих pregame screens, в том числе:

- real-server integration и доказательство совместимости с живым TomeNET server;
- post-MOTD startup и `session.enter-game`;
- profile/settings/preferences/macros, FILE transfer и Lua reload;
- game resources и gameplay-rendering prerequisites;
- первая авторитетная карта, HUD/status, сообщения и чат;
- movement, targeting, inventory/store и другие игровые действия;
- death, gameplay quit/reconnect, final review и другие post-entry transitions;
- прежние ранние части files/audio/import/config/rendering/platform families,
  если они не требуются непосредственно pregame screens.

Переносятся только прежние назначения B; уже отложенный состав D/E/G этим
решением автоматически не переезжает в C. Guide entry points до G продолжают
использовать утверждённую точную нативную заглушку.

## Правило зависимостей и atomic allocation

Старый `session.enter-game` объединял pregame с profile/FILE/Lua/resources и
первым игровым состоянием. Теперь этот составной outcome не может служить
основанием подтянуть такие зависимости обратно в B. Canonical reallocation
должен применить одно из двух правил:

1. оставить составной outcome и его prerequisites в C; либо
2. разделить его на самостоятельно проверяемые outcomes pregame handoff и
   live-session entry, если это соответствует capability policy.

Нельзя принимать часть составного outcome под его старым ID или объявлять
managed-peer evidence доказательством real-server behavior. Точные IDs и
prerequisites берутся только из финального canonical registry.

Финальное распределение active outcomes: **A8/B54/C615/D162/E64/F1/G21**.
B54 состоит из:

| Семейство | Outcomes B |
|---|---:|
| connection | 5 |
| account | 11 |
| character | 8 |
| birth | 15 |
| session | 2 |
| network | 5 |
| input | 4 |
| credentials | 4 |

## Честная граница evidence

| B доказывает | B не доказывает |
|---|---|
| Нативные pregame/MOTD screens и переходы без terminal fallback | Совместимость с конкретным живым сервером |
| Production decode/model/input/serialize/UI path | Post-MOTD startup side effects |
| Server-shaped success/rejection через managed peer | Реальные server version/build branches вне peer matrix |
| Packet fragmentation, disconnect и generation cleanup в pregame waits | Получение первого map/HUD state |
| Field-byte rules и защищённое хранение credentials | Игровые команды, post-entry quit/reconnect/death |

Real-server round trip и первый игровой экран являются обязательной приёмкой C.
Managed peer остаётся полезным детерминированным evidence и после этого, но не
заменяет live integration.

## Связь с архитектурой

ADR-0001…0006 сохраняются без изменений: session model, interaction, UI и
protocol разделены; mutable state принадлежит main thread; module connections,
revision views, session generation и error isolation остаются обязательными.
Изменена stage acceptance boundary, а не архитектура модулей.

Acceptance denominator B — 54 active outcomes из финального canonical registry.
Предыдущие числа и историческая таблица распределения больше не описывают текущую
границу и не должны использоваться как acceptance denominator.
