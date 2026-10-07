# 24: Нормализовать point-buy DNA по каждому stat

**Type:** implementation

**Tracker role:** Wayfinder planning/execution history for canonical [SV-B-023](../../../docs/tasks/stage-b/SV-B-023-birth-choices.md); this child does not own Stage B capability IDs.

**What to build:** На production stats owner point-buy DNA применяет каждое значение по baseline: in-range stat сохраняется, out-of-range stat заменяется minimum, после чего staged или automatic reincarnation продолжает birth независимо от общего budget и числа maximum values.

**Blocked by:** 07: Выбрать stats (completed); 19: Продолжить automatic DNA choices через birth flow (completed).

**Status:** resolved

**Assignee:** codex/sandcastle-7b133980-wave-15-3

Labels: bug, ready-for-agent

**Finding ID:** `D-ae73325c7f5522dd` (`birth.stats-dna-normalization-semantics`).

**Production seam for TDD:** публичный `sv_endpoint_run(SvEndpointOptions)` получает DNA stat vector и authoritative creation setup; реальные `#`/`%` SDL events проходят native birth adapter и production stats interaction, а outcome наблюдает committed stats, owner and automatic continuation. Seam принят P1-AC02, P1-AC08, P1-AC10, P1-AC26 и P1-AC28; проверка — `sv-core`.

- [x] Point-buy DNA normalizes each stat independently: every value within canonical min/max is copied and every out-of-range value becomes canonical minimum; whole-vector budget or multiple maxima do not reject baseline DNA application. [Contract: P1-AC02, P1-AC08, P1-AC10, P1-AC26, P2-AC04; checks: `sv-core`, `controller-review`]
- [x] Successful staged `#` commits the normalized vector and advances from stats to mode with coherent remaining/confirmation state; successful automatic `%` performs the same normalization and continues through the existing production owner graph. [Contract: P1-AC02, P1-AC08, P1-AC10, P1-AC26, P1-AC28; checks: `sv-core`]
- [x] Production-path regressions cover mixed valid/out-of-range values, an over-budget all-in-range vector and multiple maximum values against canonical expected stats in both compile configurations. [Contract: P1-AC08, P1-AC10, P1-AC19, P1-AC26, P2-AC05; checks: `sv-build`, `sv-core`]
- [x] Normalization changes only the current session-scoped draft: DNA persistence and `birth.restore-dna`/`birth.save-dna` remain excluded, and no partial/completed character result is sent. [Contract: P1-AC02, P1-AC26, P1-AC27, P1-AC29; checks: `sv-core`, `controller-review`]
- [x] Исправление и tests остаются по возможности SV-local и вызывают production interaction вместо отдельной test-only stats implementation. [Contract: P2-AC01, P2-AC02, P2-AC04, P2-AC05, P2-AC09; checks: `controller-review`]

**Deferred check:** `P1-birth-runtime-evidence` / P1-AC30 остаётся deferred и не считается пройденным этим implementation ticket; evidence собирается только на full acceptance для exact accepted contract digest и HEAD.

## Answer

Production seam: публичный `sv_endpoint_run(SvEndpointOptions)` получает
authoritative creation setup и DNA stat vector. Реальные SDL `#` и `%` проходят
через `sv_native_birth_event()` и production `SvBirth`; outcome наблюдает owner,
committed stats, remaining/confirmation state и automatic continuation. Managed
peer после единственного разрешённого name packet требует EOF, поэтому сценарии
также проверяют отсутствие partial/completed-character send.

Point-buy DNA теперь повторяет baseline `choose_stat_order()`: каждое значение
8..17 копируется независимо, а значение ниже 8 или выше 17 заменяется на 8.
Полученный вектор сразу становится session-scoped stats draft и committed stats;
его общий budget и число значений 17 не являются причиной отказа. Staged `#`
переходит к mode с пересчитанным `stats_remaining` и сброшенным confirmation
pending. Automatic `%` применяет тот же production command и продолжает
существующий owner graph до ready. Missing DNA по-прежнему безопасно остаётся на
stats, выключает automatic continuation и не меняет draft.

TDD red → green через согласованный production seam:

- `SV_BIRTH_CASE=stats-point-dna-illegal python3 -B tests/sv_name_editor_submit_checks.py`
  — **RED**, exit 1: production interaction отвергал весь смешанный DNA vector и
  оставлял owner `SV_BIRTH_STATS`, поэтому assertion ожидаемого mode падал.
- После минимального SV-local изменения та же команда — **GREEN**, exit 0 для
  default и `SV_CLASS_BEFORE_RACE=0` binaries; committed vector равен
  `{8, 14, 12, 10, 9, 8}` для входа `{18, 14, 12, 10, 9, 7}`.
- Дополнительные focused cases
  `stats-point-dna-over-budget`, `stats-point-dna-multiple-max` и
  `auto-dna-stats-normalized` — **GREEN**, exit 0 каждый в обеих compile
  configurations. Они фиксируют canonical vectors, owner continuation,
  remaining/confirmation state и automatic continuation.

Focused verification, 2026-10-07:

- `python3 -B tests/sv_name_editor_submit_checks.py` — **PASS**, exit 0; полная
  production endpoint birth regression matrix в обеих compile configurations,
  собранная с ASan/UBSan.
- `git diff --check` — **PASS**, exit 0.

Полный orchestrator build/test/review workflow не запускался. Изменения
ограничены SV production interaction, его production-path tests и этим тикетом;
legacy/shared gameplay code, DNA persistence, `birth.restore-dna` и
`birth.save-dna` не менялись. Новых improvement opportunities вне текущего scope
не обнаружено. `P1-birth-runtime-evidence` остаётся **deferred; not run** и будет
собран только на full acceptance для exact accepted contract digest и HEAD.
