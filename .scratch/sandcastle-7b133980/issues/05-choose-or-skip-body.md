# 05: Выбрать или пропустить body

**Type:** implementation

**Tracker role:** Wayfinder planning/execution history for canonical [SV-B-023](../../../docs/tasks/stage-b/SV-B-023-birth-choices.md); this child does not own Stage B capability IDs.

**What to build:** Игрок выбирает legal body modification, когда она доступна для текущего server setup и draft, либо проходит дальше без фиктивного owner; Backspace возвращает к trait или сразу к race, если trait был недоступен.

**Blocked by:** 04: Выбрать или пропустить trait.

**Status:** resolved

**Assignee:** codex/sandcastle-7b133980-wave-5-1

Labels: enhancement, ready-for-agent

**Production seam for TDD:** body flow вызывается через публичный `sv_endpoint_run(SvEndpointOptions)` и production birth interaction/input routing. Seam принят P1-AC07, P1-AC16, P1-AC19 и P1-AC28; проверки используют `sv-core`.

- [x] Body step получает legal choices из authoritative birth setup/flags, отклоняет illegal/stale choice и сохраняет stable highlighted identity для keyboard, mouse, random и доступного DNA input. [Contract: P1-AC07, P1-AC10, P1-AC11, P1-AC26; checks: `sv-core`]
- [x] Confirm и Q/Ctrl-Q идут через production input router, а unavailable body автоматически пропускается без назначения owner. [Contract: P1-AC07, P1-AC11, P1-AC12, P1-AC19, P1-AC28; checks: `sv-core`]
- [x] Backspace из body возвращает trait owner, если тот существовал, иначе race owner; ни confirm, ни skip, ни Backspace не отправляют partial/completed character result. [Contract: P1-AC02, P1-AC16, P1-AC19, P1-AC29; checks: `sv-core`]
- [x] Body behavior реализован и проверен на production SV path без test-only choice engine или ненужного shared extraction. [Contract: P2-AC01, P2-AC03, P2-AC04, P2-AC05; checks: `controller-review`]

## Answer

Реализован production flow выбора body внутри SV. Production interaction строит
stable body identities `Normal body` и `Fruit bat`, хранит исходную identity в
draft и проверяет актуальную class/race/trait совместимость перед каждым select
и confirm. Keyboard, mouse, random и доступный DNA input используют общий
production input router; illegal и stale DNA input не меняют highlight/draft.

Известные baseline birth setups с authoritative `creation_flags` 0/1 получают
body choices. Неизвестный server setup не получает фиктивного body owner:
interaction сразу назначает следующий production owner `SV_BIRTH_STATS`, оставляя
body draft пустым. Confirm также передаёт ownership в stats, но ничего не
отправляет. Q/Ctrl-Q завершают creation через тот же router.

Backspace из доступного body очищает body draft и подтверждение trait и
возвращает `SV_BIRTH_TRAIT`; если trait был dummy/unavailable и пропущен, owner
возвращается сразу в `SV_BIRTH_RACE`. Stable body highlight сохраняется. Managed
peer после name packet проверяет закрытие production socket без дополнительных
bytes, поэтому confirm, skip и Backspace не отправляют partial/completed character
result.

Production seam: публичный `sv_endpoint_run(SvEndpointOptions)` → decoded
`SvContactSetup`/authoritative creation flags → `SV_PREGAME_BIRTH` → `SvBirth` →
`sv_native_birth_event()`. Tests используют этот production interaction/input
path; отдельного test-only choice engine нет.

TDD (`python3 -B tests/sv_name_editor_submit_checks.py`):

- RED legal/navigation slice: exit 1 на compile — public endpoint outcome ещё не
  содержал body identities/list/highlight/draft/owner contracts.
- GREEN legal/navigation slice: exit 0 — list `[Normal, Fruit bat]`, illegal
  reject, keyboard/mouse navigation и confirm в stats прошли.
- RED random slice: exit 1 — body random оставался в body owner.
- GREEN random slice: exit 0 — `*` подтвердил только legal body identity.
- RED DNA slice: exit 1 — legal body DNA не подтверждалась.
- GREEN DNA slice: exit 0 — legal Fruit bat DNA подтверждена; stale ID и missing
  DNA сохранили owner, highlight и пустой draft.
- RED Backspace slice: exit 1 — trait draft оставался confirmed после возврата.
- GREEN Backspace slice: exit 0 — возврат к trait очистил его draft.
- RED skipped-trait predecessor slice: exit 1 — body list не подготавливался
  после автоматического trait skip.
- GREEN skipped-trait predecessor slice: exit 0 — body Backspace вернул race.
- RED unavailable-body slice: exit 1 — неизвестный authoritative setup назначал
  body owner.
- GREEN unavailable-body slice: exit 0 — body пропущен прямо в stats без draft.
- Mouse double-confirm и Q/Ctrl-Q coverage через тот же seam — exit 0.

Focused verification:

- `python3 -B tests/sv_name_editor_submit_checks.py` — PASS.
- `python3 -B tests/sv_name_rejection_recovery_checks.py` — PASS.
- `python3 -B tests/sv_endpoint_checks.py` — PASS.
- `python3 -B tests/sv_ordinary_create_cancel_checks.py` — PASS.
- `git diff --check` — PASS.

Полный build/test/review workflow не запускался. Legacy/shared gameplay code не
менялся; возможностей улучшения вне текущего scope не обнаружено.
