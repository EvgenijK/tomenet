# 04: Выбрать или пропустить trait

**Type:** implementation

**What to build:** После race игрок выбирает legal compatible trait из server setup, а когда trait unavailable или единственный dummy choice не требует выбора, production interaction сразу назначает следующего реального owner; Backspace никогда не останавливается на пропущенном шаге.

**Blocked by:** 03: Выбрать race с class compatibility.

**Status:** resolved

**Assignee:** codex/sandcastle-7b133980-wave-4-1

Labels: enhancement, ready-for-agent

**Production seam for TDD:** trait flow проходит через публичный `sv_endpoint_run(SvEndpointOptions)` и authoritative setup, декодированный production contact path. Seam принят P1-AC05, P1-AC14, P1-AC19 и P1-AC28; проверки используют `sv-core`.

- [x] Доступный trait step показывает legal server-provided identities и сохраняет stable highlight при illegal, keyboard, mouse, random и доступном DNA input; compatibility учитывает выбранные race/class. [Contract: P1-AC05, P1-AC10, P1-AC11, P1-AC26; checks: `sv-core`]
- [x] Confirm и Q/Ctrl-Q проходят через production interaction/input router, не отправляют partial/completed result и сохраняют baseline choice semantics. [Contract: P1-AC02, P1-AC05, P1-AC11, P1-AC12, P1-AC28, P1-AC29; checks: `sv-core`]
- [x] Backspace из доступного trait возвращает race owner; unavailable/dummy trait автоматически пропускается в обоих направлениях и никогда не получает ownership. [Contract: P1-AC14, P1-AC19; checks: `sv-core`]
- [x] Trait rules остаются SV-local, а tests вызывают production interaction state с реальным server setup. [Contract: P2-AC01, P2-AC04, P2-AC05; checks: `controller-review`]

## Answer

Реализован production flow выбора trait внутри SV. После legal race confirm
interaction строит список stable server trait IDs по authoritative `choice` bits;
дополнительно сохраняется уже подтверждённая совместимость пары class/race.
ID `0` остаётся baseline dummy identity, а выбираемые traits начинаются с ID `1`.
Keyboard и mouse работают по видимым legal позициям, но draft хранит исходный
server ID. Illegal/stale input не меняет highlight или draft.

Legal confirm, mouse double-click, random и доступный DNA input подтверждают trait
и передают owner в `SV_BIRTH_BODY`. Q/Ctrl-Q завершают creation через production
router. Backspace из доступного trait возвращает `SV_BIRTH_RACE`, сохраняя race
draft и stable trait highlight. Setup без traits, с единственным dummy trait или
без compatible real trait сразу назначает `SV_BIRTH_BODY`; обратный Backspace
проходит такой пропущенный step сразу к race. Managed peer проверяет закрытие
production socket без bytes после name packet, поэтому ни один из переходов не
отправляет partial/completed character result.

Production seam: публичный `sv_endpoint_run(SvEndpointOptions)` → decoded
`SvContactSetup` production contact path → `SV_PREGAME_BIRTH` → `SvBirth` →
`sv_native_birth_event()`. Отдельной test-only реализации choice behavior нет.

TDD (`python3 -B tests/sv_name_editor_submit_checks.py`):

- RED legal/navigation slice: exit 1 на compile — публичный production outcome
  ещё не содержал trait list/highlight/draft/owner contracts.
- GREEN legal/navigation slice: exit 0 — server IDs `[1, 2]`, illegal reject,
  keyboard/mouse navigation и confirm в `SV_BIRTH_BODY` прошли.
- RED random slice: exit 1 на compile — отсутствовал trait random outcome.
- GREEN random slice: exit 0 — `*` подтвердил только один из legal IDs `1`/`2`.
- RED DNA slice: exit 1 на compile — endpoint options/outcome ещё не принимали
  trait DNA.
- GREEN DNA slice: exit 0 — legal ID `2` подтверждён; incompatible ID `3`, stale
  ID `99` и missing DNA сохранили owner, highlight и draft.
- RED skip slice: exit 1 на compile — отсутствовал production skip outcome.
- GREEN skip slice: exit 0 — dummy-only и unavailable setup прошли прямо в body,
  а обратный Backspace вернул race, не назначая trait ownership.
- Backspace, Q/Ctrl-Q и mouse double-confirm coverage через тот же seam — exit 0.

Focused verification:

- `python3 -B tests/sv_name_editor_submit_checks.py` — PASS.
- `python3 -B tests/sv_name_rejection_recovery_checks.py` — PASS.
- `python3 -B tests/sv_endpoint_checks.py` — PASS.
- `python3 -B tests/sv_ordinary_create_cancel_checks.py` — PASS.
- `git diff --check` — PASS.

Полный build/test/review workflow не запускался. Legacy/shared gameplay code не
менялся; возможностей улучшения вне текущего scope не обнаружено.
