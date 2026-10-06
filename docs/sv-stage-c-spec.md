# Этап C — real-server session и первый игровой экран TomeNET SV

> Исторический snapshot, superseded 2026-10-06. Broad stage C заменён этапами
> C001–C064 из [функционального плана](sv-functional-stage-plan.md); A/B не
> изменены. Этот документ больше не задаёт canonical allocation.

Статус: historical high-level scope, 2026-10-05. Исторический scope:
**615 outcomes широкого C**. Текущее распределение задают C001–C064.

## Цель

C соединяет принятый в B production pregame path с реальными TomeNET servers и
доводит его от MOTD handoff до первой авторитетной игровой сцены. После этого C
принимает прежний игровой/session scope B и ранее запланированные game actions C.

Это следствие подтверждённой [границы B](../.scratch/single-window-sdl3-client/issues/37-narrow-stage-b-to-pregame-flow.md#answer),
а не ослабление live-server acceptance: managed peer остаётся для детерминированных
protocol branches, но не заменяет реальный server round trip.

## Первый live milestone

Минимальный сквозной результат C:

> real TomeNET server → account/auth → выбор либо создание персонажа → MOTD →
> post-MOTD startup/profile/FILE/Lua/resources → первый авторитетный map + HUD state.

Порядок определяется behavior/protocol sources, а не строкой выше. Existing и
new-character branches используют тот же production transition; send не выдаётся
за server confirmation, а первый map/HUD state — за сам факт входа.

## Перенесённый из B scope

В C приходят все прежние B outcomes за пределами рабочих pregame screens:

- live-server negotiation и version/build behavior;
- post-MOTD startup lifecycle и `session.enter-game`;
- profile/settings/preferences/macros/INS, FILE transfer, Lua и game resources,
  если они являются реальными prerequisites live session;
- map, HUD/status, messages/chat и gameplay rendering;
- basic movement и прочие ранее назначенные B game actions;
- gameplay disconnect/reconnect/quit, death и session-end flows;
- прежние B slices files/audio/import/config/rendering/platform families;
- реальные integration checks поздних callers, которые старый B пытался принять
  вместе с первым игровым экраном.

Перенос касается только прежнего allocation B. Отдельно отложенные outcomes D,
E и G сохраняют свои этапы, если canonical dependency review не докажет их
необходимость конкретному accepted C flow. Guide остаётся в G; до него действует
точная нативная заглушка.

## Собственный game-action scope C

Сохраняется ранее утверждённое назначение C: items/inventory/equipment,
targeting/look/directions, spells/skills/ghost/mimic/runes/stances/techniques,
ordinary stores и связанные confirmations/requests. Должны сохраняться keysets,
user macro load/play/waits, mouse intents, cancellation/retry, slot identity и
return to the actual parent.

Конкретное объединение перенесённого и прежнего C scope выполняется в canonical
allocation и stage tasks. Broad family label не доказывает все callers.

## Evidence boundary

C требует одновременно:

- реальные server round trips для existing и creation entry branches;
- deterministic peer cases для protocol versions, rejection, fragmentation и
  условий, которые нельзя стабильно вызвать на одном живом сервере;
- первый authoritative map/HUD state и real-window rendering;
- startup side effects через production owners, а не test-only implementations;
- gameplay/action success, cancel, retry, disconnect и generation teardown;
- Linux/Windows, renderer и optional-build evidence согласно общему acceptance
  contract и фактически затронутым platform paths;
- regressions всего принятого B pregame flow.

Недоступность real server или test accounts является execution/evidence blocker C,
но не основанием выдать managed-peer B за live-server acceptance.

## Неопределённые здесь детали

Историческое распределение active outcomes было
**A8/B54/C615/D162/E64/F1/G21**. Оно сохранено здесь только как snapshot и не
задаёт текущий denominator или ownership. Актуальные IDs, counts и порядок
берутся из `docs/capabilities/stages.json` и `native-coverage.json`.
