# 02: Выбрать класс в default compile order

**Type:** implementation

**Tracker role:** Wayfinder planning/execution history for canonical [SV-B-023](../../../docs/tasks/stage-b/SV-B-023-birth-choices.md); this child does not own Stage B capability IDs.

**What to build:** В production build с `CLASS_BEFORE_RACE` игрок после sex выбирает legal server-provided class, сохраняет устойчивую подсветку при keyboard/mouse/random/DNA input, подтверждает выбор для перехода к race либо возвращается Backspace к sex без сетевого результата.

**Blocked by:** 01: Войти в birth и выбрать пол.

**Status:** resolved

**Assignee:** codex/sandcastle-7b133980-wave-2-1

Labels: enhancement, ready-for-agent

**Production seam for TDD:** class flow проходит через публичный `sv_endpoint_run(SvEndpointOptions)` и его production birth interaction/input routing. Seam принят критериями P1-AC02, P1-AC06, P1-AC15 и P1-AC28; проверки используют `sv-core` и `sv-build`.

- [x] Default compile order назначает class owner сразу после sex и показывает только legal server-provided class identities; hidden/illegal/incompatible или stale identity нельзя подтвердить. [Contract: P1-AC06, P1-AC10, P1-AC19; checks: `sv-build`, `sv-core`]
- [x] Keyboard и mouse navigation, confirm, Q/Ctrl-Q, random и доступный DNA choice используют одну production selection; подсветка остаётся привязанной к identity, а не к случайному индексу. [Contract: P1-AC06, P1-AC10, P1-AC11, P1-AC12, P1-AC26, P1-AC28; checks: `sv-core`]
- [x] Legal confirm сохраняет class draft и передаёт ownership race step; Backspace возвращает sex owner и не создаёт partial result или completed play packet. [Contract: P1-AC02, P1-AC15, P1-AC29; checks: `sv-core`]
- [x] Class compatibility следует authoritative setup и behavior baseline, оставаясь в SV-local production code и production-path tests. [Contract: P1-AC06, P2-AC01, P2-AC04, P2-AC05; checks: `sv-core`, `controller-review`]

## Answer

Реализован default `CLASS_BEFORE_RACE` production flow внутри SV. После legal
sex confirm interaction назначает class owner и строит identity-map из
authoritative contact setup. В surface и input доступны только видимые classes,
для которых хотя бы одна server-provided race имеет соответствующий choice bit;
исходные server class IDs сохраняются при фильтрации. Hidden, несовместимый,
вышедший за диапазон и устаревший ID нельзя выбрать или подтвердить.

Keyboard, mouse, random и доступный class DNA проходят через единый `SvBirth`
selection и `sv_native_birth_event()`. Legal confirm сохраняет исходный class ID
в draft и передаёт owner `SV_BIRTH_RACE`. Q/Ctrl-Q завершают creation, а
Backspace из class возвращает owner `SV_BIRTH_SEX`, оставляя class highlight
привязанным к прежнему ID и не создавая class draft. Managed production peer
проверяет закрытие socket без дополнительных outbound bytes после name packet,
поэтому class navigation, confirm, reject, backtrack и quit не отправляют partial
result или completed-character packet.

Production seam: публичный `sv_endpoint_run(SvEndpointOptions)` → существующий
server-confirmed `SV_PREGAME_BIRTH` → `SvBirth` production interaction →
`sv_native_birth_event()`; отдельной test-only реализации поведения нет.

TDD (`python3 -B tests/sv_name_editor_submit_checks.py`):

- RED: exit 1 на compile — в публичных endpoint options/outcome и birth owner
  ещё отсутствовали class list/draft/DNA/RACE contracts.
- GREEN первого slice / RED следующего: sex regression, legal filtering,
  keyboard/mouse/illegal/confirm прошли; exit 1 на `class-random` assertion.
- GREEN random / RED DNA: random подтвердил один из legal server IDs; exit 1 на
  `class-dna` assertion.
- GREEN DNA / RED Backspace: legal DNA прошёл, hidden/incompatible/stale DNA
  сохранили прежний highlight; exit 1 на `class-back` owner assertion.
- GREEN: exit 0 для всех sex и class сценариев, включая Q/Ctrl-Q и отсутствие
  дополнительных outbound bytes.

Focused verification:

- `python3 -B tests/sv_name_editor_submit_checks.py` — PASS.
- `python3 -B tests/sv_name_rejection_recovery_checks.py` — PASS после
  сохранения принятого sex handoff для тестового пустого setup.
- `python3 -B tests/sv_endpoint_checks.py` — PASS.
- `make -s -C src -f makefile.sv tomenet-sv` — PASS (только существующие
  deprecated non-prototype warnings из common headers).
- `git diff --check` — PASS.

Полный build/test/review workflow не запускался. Legacy/shared gameplay code не
менялся; возможностей улучшения вне текущего scope не обнаружено.
