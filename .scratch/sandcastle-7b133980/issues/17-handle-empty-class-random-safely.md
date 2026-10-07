# 17: Безопасно обработать random при пустом class set

**Type:** implementation

**Tracker role:** Wayfinder planning/execution history for canonical [SV-B-023](../../../docs/tasks/stage-b/SV-B-023-birth-choices.md); this child does not own Stage B capability IDs.

**What to build:** Если authoritative birth setup не даёт ни одного viable visible class/race path, random class input завершается предсказуемым production error/ignored outcome без arithmetic fault, изменения draft или сетевой отправки; lifecycle получает смысловой результат для безопасного продолжения или закрытия.

**Blocked by:** 02: Выбрать класс в default compile order (completed).

**Status:** resolved

**Assignee:** codex/sandcastle-7b133980-wave-13-1

Labels: bug, ready-for-agent

**Finding ID:** `D-c69d3e18378a47ab` (`birth.empty-class-random-divide-by-zero`).

**Production seam for TDD:** публичный `sv_endpoint_run(SvEndpointOptions)` получает empty/wholly-incompatible authoritative setup и подаёт `*` через native SDL birth adapter и production interaction; endpoint outcome и process/sanitizer result наблюдают semantic error, lifecycle и отсутствие send. Seam принят P1-AC06, P1-AC10 и P1-AC28; проверки — `sv-core` и `controller-review`.

- [x] Empty или wholly incompatible server class/race setup не приводит к modulo-by-zero, crash или undefined behavior при random class command. [Contract: P1-AC02, P1-AC06, P1-AC10, P2-AC04; checks: `sv-core`]
- [x] Production interaction отклоняет `*` без изменения highlight/draft либо сообщает lifecycle смысловую ошибку и безопасно закрывает interaction; выбранный outcome согласован с ADR error boundary и проверяется явно. [Contract: P1-AC06, P1-AC10, P1-AC28, P2-AC12; checks: `sv-core`, `controller-review`]
- [x] Malformed/empty-list regression проходит через настоящий decoded setup, native router и `sv_endpoint_run`, а не через test-only random helper; default и alternate builds сохраняют эквивалентную безопасную семантику. [Contract: P1-AC19, P1-AC28, P2-AC05; checks: `sv-build`, `sv-core`]
- [x] Reject/error outcome не отправляет partial/completed character data и не меняет ownership final send. [Contract: P1-AC02, P1-AC29; checks: `sv-core`, `controller-review`]
- [x] Изменение остаётся локальным для SV; возможное более широкое hardening legacy/shared code записывается отдельно и не расширяет repair. [Contract: P2-AC01, P2-AC03, P2-AC06, P2-AC07, P2-AC08; checks: `controller-review`]

**Deferred check:** `P1-birth-runtime-evidence` / P1-AC30 остаётся deferred и не считается пройденным этим implementation ticket; evidence собирается только на full acceptance для exact accepted contract digest и HEAD.

## Comments

- 2026-10-07: Claimed by `codex/sandcastle-7b133980-wave-13-1`; TDD seam is the ticket-approved public `sv_endpoint_run(SvEndpointOptions)` production birth flow through decoded setup and the native SDL adapter.

## Answer

Пустой authoritative class/race setup теперь даёт явный восстановимый
`SV_BIRTH_INPUT_IGNORED`: random class/race command выставляет
`illegal_ignored`, но не меняет owner, highlight, draft или random-confirmation
flags. Lifecycle продолжает interaction и затем безопасно обрабатывает Q. Это
соответствует ADR-0006: interaction возвращает смысловой reject, не разрывая
сессию и не скрывая повреждение состояния.

Production seam: публичный `sv_endpoint_run(SvEndpointOptions)` получает
декодированный `SvContactSetup` с нулевыми race/class/trait counts; `*` проходит
через native SDL adapter и production interaction. `SvEndpointOutcome` наблюдает
нулевые legal lists, неизменённые `NONE` highlight/draft identities, текущего
owner и `illegal_ignored`. Controlled peer после разрешённого name packet требует
EOF, поэтому любой partial/completed birth send ломает scenario. Test-only random
helper или отдельная модель поведения не добавлялись.

TDD red → green, 2026-10-07:

- `SV_BIRTH_CASE=class-random-empty python3 -B tests/sv_name_editor_submit_checks.py`
  — **RED**, exit 1: UBSan сообщил `division by zero` в `class_command`, ASan
  завершил default binary по FPE; **GREEN**, exit 0 после empty-set reject в
  default и `SV_CLASS_BEFORE_RACE=0` binaries под ASan/UBSan.

Focused verification:

- `SV_BIRTH_CASE=class-random-empty python3 -B tests/sv_name_editor_submit_checks.py`
  — **PASS**, exit 0, default и alternate compile configurations.
- `SV_BIRTH_CASE=class-random`, `SV_BIRTH_CASE=race-random` и
  `SV_BIRTH_CASE=alternate-random` с тем же runner — **PASS**, exit 0.
- `git diff --check` — **PASS**, exit 0.

Полные `sv-build`, `sv-core`, controller review и deferred
`P1-birth-runtime-evidence` не запускались: их выполняет orchestrator на exact
accepted contract digest и HEAD. Изменены только SV birth interaction и его
production-path tests; legacy/shared code не менялся. Новых improvement
opportunities не обнаружено.
