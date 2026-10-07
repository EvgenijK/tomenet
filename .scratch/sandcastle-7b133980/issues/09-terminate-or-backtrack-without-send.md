# 09: Завершить или откатить creation без send

**Type:** implementation

**What to build:** На каждом достижимом birth owner Q и Ctrl-Q завершают creation через baseline quit/retry handling, а Backspace проходит по точному owner graph; ни cancel, ни backtracking, ни готовый draft не отправляют partial или completed character packet.

**Blocked by:** 08: Выбрать mode и завершить draft.

**Status:** resolved

**Assignee:** codex/sandcastle-7b133980-wave-9-1

Labels: enhancement, ready-for-agent

**Production seam for TDD:** полная cancel/backtrack matrix выполняется через публичный `sv_endpoint_run(SvEndpointOptions)` и наблюдаемую outbound production connection. Seam принят P1-AC02, P1-AC12–P1-AC20, P1-AC28 и P1-AC29; проверки используют `sv-core`.

- [x] Q и Ctrl-Q на sex/race/trait/class/body/stats/mode вызывают baseline quit/retry parent outcome, не подтверждают draft и не отправляют partial или completed result. [Contract: P1-AC02, P1-AC11, P1-AC12; checks: `sv-core`]
- [x] Backspace matrix покрывает race, trait, class, body, stats и mode в обоих compile orders, включая unavailable-step skipping и post-mode PvP Maia trait return. [Contract: P1-AC13, P1-AC14, P1-AC15, P1-AC16, P1-AC17, P1-AC18, P1-AC19, P1-AC20; checks: `sv-core`]
- [x] Outbound trace после каждого cancel/backtrack/ready-draft case не содержит partial character data или completed play packet; ticket не принимает ownership final send и не добавляет `birth.restore-dna`/`birth.save-dna`. [Contract: P1-AC02, P1-AC26, P1-AC27, P1-AC29; checks: `sv-core`, `controller-review`]
- [x] Вся matrix проходит через production interaction state/input router, без fixed-response wizard или test-only behavior implementation. [Contract: P1-AC28, P2-AC04, P2-AC05; checks: `sv-core`, `controller-review`]

## Answer

Cancel/backtrack matrix завершена через production SV birth interaction. Q и
Ctrl-Q проверены на sex/race/trait/class/body/stats/mode в default и
`SV_CLASS_BEFORE_RACE=0` compile configurations. Каждый case проходит через
native SDL event, `sv_native_birth_event()` и `sv_birth_command()`, возвращает
успешный parent endpoint outcome с `birth_quit` и сохраняет текущего owner без
подтверждения его выбора.

Backspace сохраняет уже реализованный owner graph для race, trait, class, body,
stats и mode, включая пропуск недоступных trait/body. Исправлена отличавшаяся от
baseline ветвь: Backspace из обязательного post-mode PvP Maia trait теперь
возвращает к stats owner, как legacy `goto cstats`, а не к race. Сценарий проходит
в обоих compile orders.

Production peer после единственного допустимого name request читает outbound
production connection до EOF и требует ноль последующих bytes в каждом cancel,
backtrack и ready-draft case. Поэтому partial character data и completed play
packet не отправляются. Final send остаётся у SV-B-024; `birth.restore-dna` и
`birth.save-dna` не добавлялись.

Production seam: публичный `sv_endpoint_run(SvEndpointOptions)` → decoded
`SvContactSetup`/server flags → `SV_PREGAME_BIRTH` → `SvBirth` →
`sv_native_birth_event()`, с outbound-наблюдением на controlled production
connection. Отдельной test-only реализации поведения нет.

TDD, post-mode PvP Maia Backspace slice:

- RED: `SV_BIRTH_CASE=mode-pvp-maia-back python3 -B tests/sv_name_editor_submit_checks.py`
  — exit 1, assertion ожидал `SV_BIRTH_STATS`, production outcome оставался на
  другом owner.
- GREEN: та же команда — exit 0 для default и alternate compile configurations;
  outbound после name request пуст.
- Q/Ctrl-Q matrix добавляет focused coverage уже существовавшего production
  поведения, поэтому для неё отдельный test-first fix не заявляется.

Focused verification:

- `python3 -B tests/sv_name_editor_submit_checks.py` — PASS (обе compile
  configurations, полный birth regression set и cancel/backtrack/no-send matrix).
- `SV_BIRTH_CASE=quit-class python3 -B tests/sv_name_editor_submit_checks.py` —
  PASS в обеих compile configurations.
- `SV_BIRTH_CASE=ctrl-quit-mode python3 -B tests/sv_name_editor_submit_checks.py`
  — PASS в обеих compile configurations.
- `python3 -B tests/sv_endpoint_checks.py` — PASS.
- `python3 -B tests/sv_name_rejection_recovery_checks.py` — PASS.
- `python3 -B tests/sv_ordinary_create_cancel_checks.py` — PASS.
- `git diff --check` — PASS.

Полный build/test/review workflow не запускался. Legacy/shared gameplay code не
менялся; возможностей улучшения вне текущего scope не обнаружено.
