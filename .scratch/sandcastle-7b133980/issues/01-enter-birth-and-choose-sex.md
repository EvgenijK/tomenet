# 01: Войти в birth и выбрать пол

**Type:** implementation

**Tracker role:** Wayfinder planning/execution history for canonical [SV-B-023](../../../docs/tasks/stage-b/SV-B-023-birth-choices.md); this child does not own Stage B capability IDs.

**What to build:** После подтверждённого сервером создания имени игрок остаётся в production pregame session, видит native sex choice из authoritative birth setup, меняет устойчиво подсвеченный вариант клавиатурой или мышью, использует допустимый random/DNA choice и подтверждает legal выбор без отправки partial или completed character result.

**Blocked by:** None (can start immediately).

**Status:** resolved

**Assignee:** codex/sandcastle-7b133980-wave-1-1

Labels: enhancement, ready-for-agent

**Production seam for TDD:** полный пользовательский путь вызывается через публичный `sv_endpoint_run(SvEndpointOptions)` и существующий server-confirmed handoff в `SV_PREGAME_BIRTH`. Seam принят критериями P1-AC01, P1-AC02, P1-AC03 и P1-AC28; проверки идут через `sv-core`, а отсутствие этого production handoff блокирует ticket вместо добавления test-only entry point.

- [x] Existing SV-B-022 ordinary-name flow действительно достигает birth только после server acceptance, сохраняет generation и authoritative setup; ticket не реализует заново character management и останавливается как blocked, если этот prerequisite production handoff отсутствует. [Contract: P1-AC01, P1-AC28; checks: `sv-core`, `controller-review`]
- [x] Native birth interaction показывает legal server-provided sex values с устойчивыми identities; illegal/stale value не меняет draft и owner, а random и доступный DNA choice сохраняют baseline semantics без чтения или записи DNA persistence. [Contract: P1-AC03, P1-AC10, P1-AC26, P1-AC27; checks: `sv-core`, `controller-review`]
- [x] Keyboard и mouse navigation меняют одну production interaction selection, confirmation переводит owner к следующему шагу, а Q/Ctrl-Q на sex использует baseline quit/retry outcome. [Contract: P1-AC03, P1-AC11, P1-AC12, P1-AC28; checks: `sv-core`]
- [x] Вход, navigation, random/DNA, confirm и quit не создают partial result и не отправляют completed play packet; final completed-character send остаётся у SV-B-024. [Contract: P1-AC02, P1-AC12, P1-AC29; checks: `sv-core`, `controller-review`]
- [x] Реализация и production-path tests остаются по возможности внутри SV, сохраняют behavior baseline и не выделяют shared код только ради устранения допустимого изолирующего дублирования. [Contract: P2-AC01, P2-AC02, P2-AC03, P2-AC04, P2-AC05, P2-AC09; checks: `controller-review`]

## Answer

Реализован production birth interaction для выбора пола внутри SV. После
server-confirmed ordinary-name handoff `sv_endpoint_run(SvEndpointOptions)`
остаётся в `SV_PREGAME_BIRTH`, сохраняет generation и borrowed authoritative
contact setup, показывает native Male/Female choice и маршрутизирует keyboard и
mouse в единое состояние с устойчивыми `SvBirthSex` identities. Legal confirm
передаёт ownership `SV_BIRTH_CLASS`; random и переданный production caller-ом
доступный DNA seed проходят те же legality checks без DNA persistence. Illegal
choice и недоступный DNA не меняют highlight/draft/owner. Q и Ctrl-Q завершают
creation. Ни один sex action не ставит данные в login output; managed peer
проверяет закрытие без дополнительных outbound bytes, поэтому partial и final
character packets не отправляются.

Production seam: публичный `sv_endpoint_run(SvEndpointOptions)` → существующий
server-confirmed `SV_PREGAME_BIRTH` → `SvBirth` production interaction →
`sv_native_birth_event()`.

TDD:

- RED: `python3 -B tests/sv_name_editor_submit_checks.py` — exit 1, production
  endpoint outcome не имел birth state (`birth_entered`, owner, sex и navigation).
- GREEN: та же команда — exit 0 для keyboard/mouse/illegal, random, DNA,
  unavailable DNA, Q и Ctrl-Q сценариев через managed production socket.
- RED→GREEN mouse-confirm slice: та же команда сначала завершилась assertion
  failure на double-click confirm, затем PASS после маршрутизации double click
  через тот же production selection и confirm command.

Focused verification:

- `python3 -B tests/sv_name_editor_submit_checks.py` — PASS.
- `python3 -B tests/sv_name_rejection_recovery_checks.py` — PASS.
- `python3 -B tests/sv_endpoint_checks.py` — PASS.
- `make -s -C src -f makefile.sv tomenet-sv` — PASS (только существующие
  deprecated non-prototype warnings из common headers).

Полный build/test/review workflow не запускался; новый focused check добавлен в
`sv-core` inventory для orchestrator gate. Legacy/shared gameplay code не менялся;
возможностей улучшения вне текущего scope не обнаружено.
