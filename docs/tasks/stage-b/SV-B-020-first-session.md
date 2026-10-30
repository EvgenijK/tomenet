# SV-B-020 — M1: existing-character startup screen flow

Проверки платформ: [текущее правило](../../agents/platform-validation.md); Windows/MinGW/Wine отложены и не блокируют текущую приёмку.

Статус: accepted — принято пользователем 2026-10-28 вместе с этапом B.
Оставшиеся проверки сняты с текущего gate прямым решением пользователя;
они не объявлены выполненными. [Решение о приёмке](../../sv-stage-b-acceptance.md#решение-пользователя--2026-10-28).

Исторический статус до решения пользователя: production M1 path реализован и проверен на Linux с managed peer;
Linux runtime evidence и cumulative acceptance исходных owners остаются pending;
Windows build/runtime validation отложена пользователем и не блокирует приёмку.

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

## Реализация 2026-10-06

- `tomenet-sv` проходит native endpoint и private credential input, contact/setup,
  server-confirmed overview, выбор существующего персонажа, peer MOTD и остаётся
  в явной model/UI phase `live-session-handoff` без запуска gameplay/profile/FILE/Lua.
- SV-local pregame presentation model владеет generation/revision, overview,
  выбранным персонажем, MOTD, failure/disconnect и handoff phase; stale generation
  не может изменить новую модель.
- Login protocol path принимает fragmentation/chaining, keepalive и ping echo во
  время pregame waits. Ошибка показывает retry/exit owner; `R` создаёт новую
  connection generation, `Q` выходит из overview, disconnect не создаёт handoff.
- `tests/sv_first_session_checks.py` поднимает TCP peer и вызывает production
  UI/model/input/transport/protocol code для success, disconnect, quit и
  rejection→retry→success; отдельный executable smoke остаётся в
  `tests/sv_login_live_checks.py`.

Milestone не меняет capability ownership и не является Windows или real-server
evidence. Platform-specific Secret Service/Credential Manager acceptance и полный
evidence matrix остаются у SV-B-001/002/005/006.


2026-10-28: интеграционные проверки M2 и исправления input lifecycle возвращены
owner в [отчёте B-025](SV-B-025-entry-complete.md#реализация-и-проверки--2026-10-28).
Это дополнительное Linux evidence; pending Linux native acceptance
и capability ownership не изменены. Windows build/runtime validation отложена
пользователем и не блокирует текущую приёмку.


## Cumulative Linux review — 2026-10-28

[Сводный B-075 отчёт](../../sv-stage-b-acceptance.md) учитывает свежие
production-path проверки owners, actual Secret Service и cumulative A
регрессии. Milestone остаётся pending до закрытия актуальных Linux
input/provider и per-obligation evidence требований; Windows deferred.
