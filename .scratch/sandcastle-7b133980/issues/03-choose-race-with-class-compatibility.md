# 03: Выбрать race с class compatibility

**Type:** implementation

**What to build:** В default compile order игрок выбирает legal server-provided race, причём class/race compatibility, random/DNA semantics, stable highlight, keyboard/mouse navigation, confirmation и возврат к class совпадают с behavior baseline и ничего не отправляют как завершённого персонажа.

**Blocked by:** 02: Выбрать класс в default compile order.

**Status:** resolved

**Assignee:** codex/sandcastle-7b133980-wave-3-1

Labels: enhancement, ready-for-agent

**Production seam for TDD:** race flow вызывается через публичный `sv_endpoint_run(SvEndpointOptions)` с server setup, уже используемым production contact/login conversation. Seam принят P1-AC02, P1-AC04, P1-AC13 и P1-AC28; проверки используют `sv-core`.

- [x] Race surface использует полный server-provided list и stable identities, отфильтровывает или отклоняет illegal/incompatible подтверждение согласно выбранному class и сохраняет прежний draft при отказе. [Contract: P1-AC04, P1-AC10; checks: `sv-core`]
- [x] Keyboard/mouse navigation, confirm, Q/Ctrl-Q, random и доступный DNA choice проходят через production input router и сохраняют highlighted identity. [Contract: P1-AC04, P1-AC10, P1-AC11, P1-AC12, P1-AC26, P1-AC28; checks: `sv-core`]
- [x] Legal confirm передаёт ownership следующему доступному birth step; Backspace возвращает class owner в default order и не отправляет partial или completed result. [Contract: P1-AC02, P1-AC13, P1-AC29; checks: `sv-core`]
- [x] Compatibility и choice semantics портированы без расширения legacy/shared implementation и проверяются через production SV state, а не отдельную test-only модель. [Contract: P2-AC01, P2-AC02, P2-AC04, P2-AC05; checks: `controller-review`]

## Answer

Реализован default `CLASS_BEFORE_RACE` production flow выбора race внутри SV.
После class confirm interaction строит compatibility-map по `choice` bits
authoritative contact setup, сохраняя исходные server race IDs. Native surface
показывает полный server-provided race list, несовместимые варианты отмечает как
недоступные, а keyboard/mouse navigation обходит только legal IDs. Direct,
mouse и DNA input с incompatible, отсутствующим или stale ID не меняют race
highlight/draft и не меняют owner.

Legal keyboard/mouse confirm сохраняет race ID и передаёт owner доступному
`SV_BIRTH_TRAIT`. Random выбирает только compatible server ID; доступный DNA
проходит ту же production legality check. Q/Ctrl-Q завершают creation, а
Backspace возвращает `SV_BIRTH_CLASS`, сохраняет class draft и стабильные
highlights, но очищает race draft. Managed production peer подтверждает закрытие
socket без bytes после name packet для navigation, reject, confirm, random, DNA,
Backspace и quit, поэтому partial или completed-character result не отправляется.

Production seam: публичный `sv_endpoint_run(SvEndpointOptions)` → существующий
server-confirmed `SV_PREGAME_BIRTH` → `SvBirth` production interaction →
`sv_native_birth_event()`; отдельной test-only модели поведения нет.

TDD (`python3 -B tests/sv_name_editor_submit_checks.py`):

- RED legal/compatibility slice: exit 1 на compile, потому что production endpoint
  ещё не имел race list/highlight/draft/owner contracts.
- GREEN legal/compatibility slice: exit 0; полный список из трёх server races,
  compatible stable IDs `[0, 2]`, keyboard/mouse reject и legal confirm прошли.
- RED random slice: exit 1 на assertion — owner оставался `SV_BIRTH_RACE`.
- GREEN random slice: exit 0; random подтвердил один из compatible IDs `0`/`2`.
- RED DNA slice: exit 1 на compile — production options ещё не принимали race DNA.
- GREEN DNA slice: exit 0; legal DNA подтвердил ID `2`, а incompatible/stale/missing
  DNA сохранил class draft, race highlight и owner.
- Backspace, Q и Ctrl-Q coverage через тот же seam — exit 0.

Focused verification:

- `python3 -B tests/sv_name_editor_submit_checks.py` — PASS.
- `python3 -B tests/sv_name_rejection_recovery_checks.py` — PASS.
- `python3 -B tests/sv_endpoint_checks.py` — PASS.
- `python3 -B tests/sv_ordinary_create_cancel_checks.py` — PASS.
- `git diff --check` — PASS.

Полный build/test/review workflow не запускался. Legacy/shared gameplay code не
менялся; возможностей улучшения вне текущего scope не обнаружено.
