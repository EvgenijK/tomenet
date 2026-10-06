# Narrow stage B to the native pregame flow

Type: decision
Status: resolved
Assignee: codex
Blocked by: 29, 36

## Question

Где теперь заканчивается этап B, если его цель — минимально необходимые рабочие
экраны входа, а не первая живая игровая сессия? Какие части прежнего B не должны
подтягиваться зависимостями `session.enter-game`, и чем честно проверяется B без
реального сервера?

## Comments

### Confirmed by the user — 2026-10-05

Пользователь оставил в B только необходимые части подключения, аккаунта и
аутентификации, управления и создания персонажа, pregame-lifecycle, сетевого
протокола, пользовательского ввода и защищённого хранения credentials.

Подтверждена формулировка результата:

> Полностью рабочий native flow экранов подключения, аутентификации, выбора и
> создания персонажа, использующий production UI/model/input/protocol path и
> проверяемый через управляемый protocol peer. Реальная серверная интеграция и
> первый игровой экран относятся к C.

Остальной прежний объём B переносится в C. Это не означает перенос в C всего
позднего D/E/G: меняется назначение только тех outcomes, которые до этого были
распределены в B.

## Answer

Этап B заканчивается после полностью нативного pregame-flow:

1. выбор endpoint и подключение production transport к управляемому protocol peer;
2. account creation/login и обработка server-shaped success/rejection;
3. получение, отображение и управление списком персонажей;
4. выбор существующего персонажа либо полный native flow создания персонажа;
5. peer-driven MOTD и переход в явное состояние ожидания следующей, уже
   live-session, фазы;
6. cancel/back/retry/disconnect/error paths, field limits/encoding, generation
   cleanup и защищённое хранение credentials, необходимые этим экранам.

Управляемый peer заменяет только внешнюю сторону протокола. Он не подменяет
production decoder, session model, interaction state, input router, serializer,
transport или UI. Он должен уметь воспроизводить нормативные ответы, отказы,
фрагментацию и разрыв соединения, чтобы проверять реальный production path без
доступности общего TomeNET-сервера.

B не заявляет совместимость с реальным сервером и не принимает первый игровой
экран. Post-MOTD startup, live-server negotiation, profile/preferences/macros, FILE
transfer, Lua reload, game resources, map, HUD/status, messages/chat, gameplay,
death/quit/reconnect и прочие прежние B outcomes переходят в C. Они не становятся
предпосылками B только потому, что прежний `session.enter-game` объединял pregame
и live-session части. Если текущая атомарность registry не позволяет выразить
эту границу, allocation должен оставить составной outcome в C либо разделить его
на самостоятельно проверяемые outcomes; завышать B ради старого dependency edge
нельзя.

Приёмка B доказывает рабочие native screens и production protocol path через
управляемый peer. Реальный TomeNET server round trip, его version/build branches,
startup side effects и первый авторитетный map/HUD state впервые обязательны в C.
Эти два вида evidence нельзя выдавать друг за друга.

Последующая canonical reallocation материализовала решение как
**A8/B54/C615/D162/E64/F1/G21**. В B находятся connection5, account11,
character8, birth15, session2, network5, input4 и credentials4. Этот ticket
фиксирует пользовательскую границу; registry, reconciliation и implementation
tasks обновляются отдельными изменениями.

Решение не меняет границы модулей из ADR-0001…0006 и не вводит новый доменный
термин: `managed protocol peer` — тестовый внешний участник, а не часть продуктовой
модели. Поэтому новый ADR и изменение `CONTEXT.md` не требуются.
