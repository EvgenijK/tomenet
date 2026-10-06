# SV-B-020 — M1: existing-character startup screen flow

Статус: specified; нулевой milestone, runtime evidence pending.

## Пользовательский результат

Один production executable проходит endpoint → contact → account/password →
overview/выбор существующего персонажа и peer-driven MOTD в предусмотренном
protocol порядке, используя native surfaces и управляемый protocol peer.

## Зависимости и граница

Зависит от [SV-B-006](SV-B-006-login.md), а транзитивно от SV-B-001, 002 и 005.
Milestone не владеет capability IDs и не заменяет acceptance своих producers.

Успешная граница — production startup flow получил предусмотренные peer
подтверждения и вошёл в явное состояние ожидания live-session handoff.
Profile/FILE/Lua, Net_start presentation, map, HP/HUD, messages, gameplay input
и real-server integration проверяются в C.

## Единственная первичная ответственность

<!-- owned-capabilities:start -->
<!-- Нет: это integration milestone без собственного capability ownership. -->
<!-- owned-capabilities:end -->

## Production SV проверки

1. Success path с точными bytes и screen transitions; нет fallback surface.
2. Cancel/quit/retry и provider missing/refused на каждом parent transition.
3. Split/chained packets, keepalive/ping, focus/resize, disconnect и новый
   generation не повторяют login/select и не применяют stale completion.
4. В конце flow нет gameplay surface или ложного `session.enter-game` claim.

Evidence возвращается исходным owners SV-B-001, 002, 005 и 006.
