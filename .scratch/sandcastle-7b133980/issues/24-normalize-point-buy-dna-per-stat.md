# 24: Нормализовать point-buy DNA по каждому stat

**Type:** implementation

**What to build:** На production stats owner point-buy DNA применяет каждое значение по baseline: in-range stat сохраняется, out-of-range stat заменяется minimum, после чего staged или automatic reincarnation продолжает birth независимо от общего budget и числа maximum values.

**Blocked by:** 07: Выбрать stats (completed); 19: Продолжить automatic DNA choices через birth flow (completed).

**Status:** ready-for-agent

**Assignee:** unassigned

Labels: bug, ready-for-agent

**Finding ID:** `D-ae73325c7f5522dd` (`birth.stats-dna-normalization-semantics`).

**Production seam for TDD:** публичный `sv_endpoint_run(SvEndpointOptions)` получает DNA stat vector и authoritative creation setup; реальные `#`/`%` SDL events проходят native birth adapter и production stats interaction, а outcome наблюдает committed stats, owner and automatic continuation. Seam принят P1-AC02, P1-AC08, P1-AC10, P1-AC26 и P1-AC28; проверка — `sv-core`.

- [ ] Point-buy DNA normalizes each stat independently: every value within canonical min/max is copied and every out-of-range value becomes canonical minimum; whole-vector budget or multiple maxima do not reject baseline DNA application. [Contract: P1-AC02, P1-AC08, P1-AC10, P1-AC26, P2-AC04; checks: `sv-core`, `controller-review`]
- [ ] Successful staged `#` commits the normalized vector and advances from stats to mode with coherent remaining/confirmation state; successful automatic `%` performs the same normalization and continues through the existing production owner graph. [Contract: P1-AC02, P1-AC08, P1-AC10, P1-AC26, P1-AC28; checks: `sv-core`]
- [ ] Production-path regressions cover mixed valid/out-of-range values, an over-budget all-in-range vector and multiple maximum values against canonical expected stats in both compile configurations. [Contract: P1-AC08, P1-AC10, P1-AC19, P1-AC26, P2-AC05; checks: `sv-build`, `sv-core`]
- [ ] Normalization changes only the current session-scoped draft: DNA persistence and `birth.restore-dna`/`birth.save-dna` remain excluded, and no partial/completed character result is sent. [Contract: P1-AC02, P1-AC26, P1-AC27, P1-AC29; checks: `sv-core`, `controller-review`]
- [ ] Исправление и tests остаются по возможности SV-local и вызывают production interaction вместо отдельной test-only stats implementation. [Contract: P2-AC01, P2-AC02, P2-AC04, P2-AC05, P2-AC09; checks: `controller-review`]

**Deferred check:** `P1-birth-runtime-evidence` / P1-AC30 остаётся deferred и не считается пройденным этим implementation ticket; evidence собирается только на full acceptance для exact accepted contract digest и HEAD.
