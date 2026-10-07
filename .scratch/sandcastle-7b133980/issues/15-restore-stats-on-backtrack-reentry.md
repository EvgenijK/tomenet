# 15: Восстановить stats при возврате Backspace

**Type:** implementation

**What to build:** После Backspace из mode или другого допустимого возврата к stats игрок получает снова пригодное к редактированию stats interaction: может заменить частичный или уже завершённый ordered-stat выбор, подтвердить новый legal результат и снова перейти вперёд без stale consumption или сетевой отправки.

**Blocked by:** 09: Завершить или откатить creation без send (completed).

**Status:** resolved

**Assignee:** codex/sandcastle-7b133980-wave-12-2

Labels: bug, ready-for-agent

**Finding ID:** `D-a034cf7e7cd5e676` (`birth.stats-backtrack-reentry-state`).

**Production seam for TDD:** публичный `sv_endpoint_run(SvEndpointOptions)` ведёт production birth flow до stats/mode и возвращает Backspace через native SDL adapter и `sv_birth_command()`; outcome наблюдает legal stats, order/allocation, highlight, confirmation flags, owner и draft, а socket boundary — отсутствие send. Seam принят P1-AC08, P1-AC17, P1-AC18 и P1-AC28; проверка — `sv-core`.

- [x] Возврат к ordered stats восстанавливает согласованные legal choices, count, order progress, stable highlight и confirmation state как для частично набранного, так и для полностью подтверждённого порядка; ранее consumed список не блокирует новый confirm. [Contract: P1-AC08, P1-AC10, P1-AC17; checks: `sv-core`]
- [x] Re-entry определяет и проверяет явную baseline-семантику ранее committed stats draft: новый выбор заменяет его только после legal production confirmation, а illegal/stale input сохраняет согласованный draft и owner. [Contract: P1-AC02, P1-AC08, P2-AC04; checks: `sv-core`, `controller-review`]
- [x] Production-path scenarios проходят Backspace после partial и completed ordered stats, делают новый keyboard/mouse выбор и снова достигают mode в обеих compile configurations. [Contract: P1-AC11, P1-AC17, P1-AC18, P1-AC19, P1-AC28; checks: `sv-build`, `sv-core`]
- [x] Тот же re-entry проверен через dedicated PvP Maia owner path с сохранением его принятого owner graph; Backspace/reconfirm не отправляют partial или completed packet. [Contract: P1-AC02, P1-AC18, P1-AC20, P1-AC29; checks: `sv-core`]
- [x] Исправление и tests остаются в SV production path и не вводят отдельную test-only stats implementation. [Contract: P2-AC01, P2-AC04, P2-AC05; checks: `controller-review`]

**Deferred check:** `P1-birth-runtime-evidence` / P1-AC30 остаётся deferred и не считается пройденным этим implementation ticket; evidence собирается только на full acceptance для exact accepted contract digest и HEAD.

## Comments

- 2026-10-07: Claimed by `codex/sandcastle-7b133980-wave-12-2`; TDD seam is the ticket-approved public `sv_endpoint_run(SvEndpointOptions)` production birth flow through the native SDL adapter and `sv_birth_command()`.

## Answer

Исправление выполнено в SV production interaction. Ordered/point-buy edit state
теперь перезапускается с полным legal list, нулевым progress и стабильным legal
highlight при возврате из mode, при Backspace из partial stats и при post-mode
PvP Maia trait return. Ранее подтверждённый stats draft хранится отдельно от
повторно редактируемого буфера и заменяется только полным legal confirmation;
до него outcome продолжает публиковать committed draft.

Согласованный public seam: `sv_endpoint_run(SvEndpointOptions)`. Controlled peer
передаёт authoritative setup, native SDL keyboard/mouse events проходят через
`sv_native_birth_event()` и `sv_birth_command()`, а `SvEndpointOutcome` наблюдает
owner, legal list, progress, highlight, flags и draft. После единственного
разрешённого name packet peer требует EOF, поэтому любой partial/completed birth
send приводит к падению сценария. Test-only stats implementation не добавлена.

TDD red → green, 2026-10-07:

- `SV_BIRTH_CASE=stats-order-reenter-completed python3 -B tests/sv_name_editor_submit_checks.py` — **RED**, exit 1: после mode Backspace consumed list оставлял owner в stats; после минимального restart — **GREEN**, exit 0 в обеих compile configurations.
- `SV_BIRTH_CASE=stats-order-reenter-partial python3 -B tests/sv_name_editor_submit_checks.py` — **RED**, exit 1: stale partial prefix сохранялся в новом порядке; после restart на stats Backspace — **GREEN**, exit 0 в обеих configurations.
- `SV_BIRTH_CASE=stats-order-reenter-pvp-maia python3 -B tests/sv_name_editor_submit_checks.py` — **RED**, exit 1: post-mode Maia return не мог повторно пройти stats; после restart на accepted trait-owner transition — **GREEN**, exit 0 в обеих configurations.
- `SV_BIRTH_CASE=stats-order-reenter-reject python3 -B tests/sv_name_editor_submit_checks.py` — **RED**, exit 1: outcome преждевременно публиковал partial edit вместо committed draft; после разделения committed draft и edit state — **GREEN**, exit 0 в обеих configurations.

Финальная focused проверка запустила четыре новых сценария и regression-сценарии
`mode-back`, `mode-pvp-maia-back` через тот же production seam: все шесть —
**PASS**, exit 0, default и `SV_CLASS_BEFORE_RACE=0`, ASan/UBSan.
`git diff --check` — **PASS**, exit 0. Полные `sv-build`, `sv-core` и controller review не
запускались: их выполняет orchestrator после сборки tickets.

`P1-birth-runtime-evidence` / P1-AC30 остаётся **deferred; not run** до full
acceptance на exact accepted contract digest и HEAD, как требует исходный owner.
