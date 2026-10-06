# SV-B-025 — M2: account/character creation screen flow

Статус: specified; нулевой milestone, runtime evidence pending.

## Пользовательский результат

Один production executable проходит оба обязательных пути Stage B:

- existing account → existing character selection;
- new account или existing account → new slot/name → birth choices → final
  acknowledgement.

## Зависимости и граница

Зависит от [SV-B-020](SV-B-020-first-session.md),
[SV-B-021](SV-B-021-account-manage.md) и
[SV-B-024](SV-B-024-birth-dna.md). Milestone не владеет capability IDs.

Успех означает полностью рабочие native startup screens через production
UI/model/input/protocol path и controlled peer. Он не означает real-server
compatibility, startup FILE/Lua/profile, `session.enter-game` или первый gameplay
screen; эти результаты принимаются в C.

## Единственная первичная ответственность

<!-- owned-capabilities:start -->
<!-- Нет: это integration milestone без собственного capability ownership. -->
<!-- owned-capabilities:end -->

## Production SV проверки

1. Existing и creation routes с exact wire bytes, все legal version/build
   branches и ни одного terminal fallback.
2. Cancel/back/retry/rejection/disconnect на каждом surface восстанавливают
   правильный parent, focus, draft/selection и session generation.
3. Physical keyboard/input method и мышь, resize/minimize/restore, split/chained
   packets, keepalive/ping и provider completion не меняют семантику.
4. Final acknowledgement и peer-driven MOTD оставляют клиент в live-session
   handoff с disabled gameplay input и не требуют map/HUD.

Evidence возвращается каждому implementation owner; milestone не принимает
capability по одному happy-path screenshot.
