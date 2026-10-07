# 07: Выбрать stats

**Type:** implementation

**What to build:** После последнего class/race/body owner игрок завершает baseline stats choice через production interaction: legal server-compatible distribution/order, random/DNA choice, stable highlight, keyboard/mouse confirmation и Backspace к реальному предыдущему owner без сетевого результата.

**Blocked by:** 06: Сохранить alternate class/race compile order.

**Status:** resolved

**Assignee:** codex/sandcastle-7b133980-wave-7-1

Labels: enhancement, ready-for-agent

**Production seam for TDD:** stats flow вызывается через публичный `sv_endpoint_run(SvEndpointOptions)` и production birth interaction/input router в обеих compile configurations. Seam принят P1-AC08, P1-AC17, P1-AC19 и P1-AC28; проверки используют `sv-core`.

- [x] Stats surface предоставляет legal server-compatible choices для текущего draft, отклоняет illegal/over-budget/stale confirmation и сохраняет stable highlighted identity или allocation. [Contract: P1-AC08, P1-AC10; checks: `sv-core`]
- [x] Keyboard/mouse navigation, confirm, Q/Ctrl-Q, random и доступный DNA choice используют production router и сохраняют baseline semantics. [Contract: P1-AC08, P1-AC10, P1-AC11, P1-AC12, P1-AC26, P1-AC28; checks: `sv-core`]
- [x] Backspace возвращает body или ближайший реально доступный predecessor в активном compile order; Backspace и confirmation не отправляют partial/completed result. [Contract: P1-AC02, P1-AC17, P1-AC19, P1-AC29; checks: `sv-core`]
- [x] Stats behavior и tests остаются production SV-local и сохраняют protocol/gameplay baseline. [Contract: P2-AC01, P2-AC04, P2-AC05; checks: `controller-review`]

## Answer

Stats реализованы внутри production SV birth interaction. Для authoritative
`creation_flags == 0` interaction ведёт server-compatible permutation шести
stat identities: уже выбранные identities исключаются из legal list, highlight
остаётся устойчивым, а `*` случайно завершает только оставшуюся часть порядка.
Для `creation_flags == 1` interaction ведёт point-buy allocation с baseline
диапазоном 8..17, tiered стоимостью, 30 исходными очками, максимум одним значением
17 и нижней границей с учётом authoritative race/class adjustments. Неизвестный
setup не получает фиктивных legal stats.

Keyboard, mouse и semantic commands проходят через `sv_native_birth_event()` и
`sv_birth_command()`. Point-buy с оставшимися очками сохраняет baseline warning:
первый confirm оставляет stats owner, второй подтверждает намерение; изменение
selection/allocation сбрасывает pending confirm. Legal point-buy DNA применяется
атомарно, а missing, out-of-range, multiple-max и over-budget DNA сохраняют
highlight/allocation и owner. Traditional order поддерживает random; Q и Ctrl-Q
завершают creation через общий router.

После подтверждения stats ownership передаётся следующему production owner
`SV_BIRTH_MODE`, но completed/partial character result не отправляется. Backspace
возвращает default flow в body либо ближайший реально доступный trait/race, а
alternate flow — в class. Production peer после единственного name packet
проверяет закрытие socket без дополнительных bytes во всех confirm, Backspace и
quit сценариях.

Production seam: публичный `sv_endpoint_run(SvEndpointOptions)` → decoded
`SvContactSetup`/creation flags → `SV_PREGAME_BIRTH` → `SvBirth` →
`sv_native_birth_event()`. Default и `SV_CLASS_BEFORE_RACE=0` binaries используют
тот же interaction/input path; отдельной test-only реализации stats нет.

TDD (`python3 -B tests/sv_name_editor_submit_checks.py`):

- RED order slice: exit 1 на compile — public endpoint outcome не предоставлял
  stats list/highlight/order state. GREEN: exit 0 для legal list, stable
  keyboard/mouse highlight и partial order в обеих compile configurations.
- RED point-buy slice: exit 1 на compile — public outcome не предоставлял
  allocation mode/remaining points. GREEN: exit 0 для tiered adjustment и
  stable allocation/navigation.
- RED DNA slice: exit 1 на compile — `SvEndpointOptions` не принимал stats DNA.
  GREEN: exit 0 для legal atomic DNA completion через production router.
- RED under-budget confirm slice: exit 1 на compile — interaction не предоставлял
  pending confirmation state. GREEN: exit 0 для двухфазного keyboard/mouse
  confirmation с оставшимися очками.
- Итоговый GREEN: exit 0; дополнительно покрыты full order, random permutation,
  Q/Ctrl-Q, Backspace matrix для stats, unavailable predecessors, authoritative
  lower bound, missing/out-of-range/multiple-max/over-budget DNA и отсутствие
  post-name network bytes в default и alternate binaries.

Focused verification:

- `python3 -B tests/sv_name_editor_submit_checks.py` — PASS.
- `python3 -B tests/sv_endpoint_checks.py` — PASS.
- `python3 -B tests/sv_name_rejection_recovery_checks.py` — PASS.
- `python3 -B tests/sv_ordinary_create_cancel_checks.py` — PASS.
- `git diff --check` — PASS.

Полный build/test/review workflow не запускался. Legacy/shared gameplay code не
менялся; возможностей улучшения вне текущего scope не обнаружено.
