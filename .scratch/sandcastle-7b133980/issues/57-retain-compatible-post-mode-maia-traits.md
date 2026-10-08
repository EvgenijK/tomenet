# 57: Сохранить compatible traits в post-mode PvP Maia

**Type:** repair

**Tracker role:** Wayfinder planning/execution history for canonical [SV-B-023](../../../docs/tasks/stage-b/SV-B-023-birth-choices.md); this child does not own Stage B capability IDs.

**What to build:** После подтверждения ordinary или dedicated PvP mode для server-provided Maia production birth flow предлагает каждый trait, совместимый с Maia по authoritative server mask, и дополнительно допускает присутствующие специальные identities 14/15. Если 14/15 отсутствуют, но ordinary compatible traits остаются, игрок по-прежнему получает post-mode trait choice и может выбрать его keyboard, mouse, random или DNA, подтвердить либо вернуться Backspace без изменения уже проверенного owner graph и без partial/completed send.

**Blocked by:** 54: Снять post-mode Maia context перед глубоким Backspace (completed).

**Status:** resolved

**Assignee:** codex/sandcastle-7b133980-wave-38-1

Labels: bug, ready-for-agent

**Finding ID:** `D-11ec7c1d1ff9b668` (`birth-post-mode-maia-unavailable-step-unwind`), reopened recurrence after ticket 54: this sequential repair completes the same post-mode availability finding and does not create an unrelated second defect.

**Production seam for TDD:** публичный `sv_endpoint_run(SvEndpointOptions)` получает authoritative server-provided Maia race/trait setup и ordinary либо dedicated PvP intent; реальные keyboard, mouse, random, DNA, confirm и Backspace SDL events проходят native birth adapter и публичную production birth-interaction/input-router boundary. `SvEndpointOutcome` и controlled peer наблюдают owner, legal trait identities, stable highlight, draft и отсутствие запрещённых outbound bytes. Seam принят P1-AC02, P1-AC05, P1-AC09, P1-AC10, P1-AC11, P1-AC14, P1-AC18, P1-AC19, P1-AC20, P1-AC26 и P1-AC28; проверки — `sv-core` и `sv-build`.

- [x] Red production-path cases с Maia-compatible ordinary traits 1/2 и отсутствующими 14/15 воспроизводят ошибочный переход прямо в READY; green outcome сохраняет все compatible server-provided traits, добавляет 14/15 только когда они присутствуют и переходит в READY лишь при действительно пустом итоговом legal list. [Contract: P1-AC02, P1-AC05, P1-AC09, P1-AC10, P1-AC19, P1-AC20, P2-AC04, P2-AC05; checks: `sv-build`, `sv-core`]
- [x] Keyboard navigation, stable letter identity, mouse selection по отображённой identity, random, staged DNA и automatic DNA используют один post-mode legal list; illegal либо отсутствующая identity отклоняется без изменения owner, highlight или draft, а legal confirm завершает только local draft. [Contract: P1-AC02, P1-AC05, P1-AC10, P1-AC11, P1-AC26, P1-AC28, P2-AC04, P2-AC05; checks: `sv-core`]
- [x] Ordinary и production dedicated PvP paths проходят regression в обеих compile-time class/race order configurations; controls с присутствующими 14/15, без единого compatible trait и с non-Maia race сохраняют ранее проверенную semantics. [Contract: P1-AC05, P1-AC09, P1-AC10, P1-AC19, P1-AC20, P1-AC26, P2-AC04, P2-AC05; checks: `sv-build`, `sv-core`]
- [x] Backspace из post-mode trait возвращает к mode, а дальнейший Backspace использует восстановленный pre-mode ordinary trait state и существующий compile-order predecessor; Q/Ctrl-Q, disconnect и stale-generation input не отправляют partial/completed character result, final send остаётся у SV-B-024. [Contract: P1-AC02, P1-AC12, P1-AC14, P1-AC15, P1-AC18, P1-AC20, P1-AC22, P1-AC29; checks: `sv-core`, `controller-review`]
- [x] Repair и regressions остаются по возможности SV-local, проходят только через production interaction/input-router path и не добавляют test-only trait filtering; обнаруженные unrelated improvements записываются отдельно. [Contract: P1-AC28, P2-AC01, P2-AC02, P2-AC03, P2-AC05, P2-AC06, P2-AC07, P2-AC08, P2-AC09; checks: `controller-review`]

**Deferred check:** `P1-birth-runtime-evidence` / P1-AC30 остаётся deferred у named full-acceptance producer/verifier и не считается пройденным этим implementation repair; evidence собирается только на exact accepted contract digest `690a04cefb7fd0e1615dce46d1a538e1dcbd856232af1a1373be93f987095c82` и HEAD.

## Answer

Repair выполнен в SV-local production birth interaction. Post-mode PvP Maia
legal list теперь включает каждый server-provided trait, чей authoritative mask
содержит выбранную Maia race, и дополнительно присутствующие identities 14/15.
`prepare_traits()` по-прежнему переводит draft в READY только при действительно
пустом результате. Существующий owner graph и pre-mode snapshot не менялись.

Согласованный public seam: `sv_endpoint_run(SvEndpointOptions)`. Controlled peer
передаёт authoritative Maia race/trait setup и ordinary либо production
dedicated PvP intent. Настоящие SDL keyboard, mouse, random, staged DNA,
automatic DNA, confirm, Backspace, Q/Ctrl-Q, stale и disconnect inputs проходят
native adapter и public birth interaction/input-router boundary.
`SvEndpointOutcome` наблюдает owner, legal identities, stable highlight и draft;
peer допускает только предусмотренный name packet, поэтому partial/completed
birth send ломает scenario. Test-only trait filtering не добавлен.

TDD red → green, 2026-10-08:

- Red: `SV_BIRTH_CASE=mode-pvp-maia-no-post-traits python3 -B
  tests/sv_name_editor_submit_checks.py` — **FAIL**, exit 1: production flow
  перешёл в READY вместо post-mode trait owner при server-provided identities
  1/2 и отсутствующих 14/15.
- Green: тот же command после минимального изменения `legal_trait()` — **PASS**,
  exit 0 для default и `SV_CLASS_BEFORE_RACE=0` binaries.
- Focused green matrix тем же runner и `SV_BIRTH_CASE`: четыре ordinary/dedicated
  `*-no-post-traits` cases проверили stable illegal reject, mouse confirm, random
  и staged DNA; `unknown-setup-pvp-maia-no-post-traits-back` проверил keyboard
  navigation, Backspace и Ctrl-Q; `auto-dna-dedicated-pvp-maia-no-post-traits`
  проверил automatic DNA. Все — **PASS**, exit 0 в обеих compile configurations.
- Controls `mode-pvp-maia-empty-post-traits`, четыре `*-maia-trait-reenter`
  cases с итоговым list `[1,2,14,15]`, `mode-pvp-maia`,
  `auto-dna-dedicated-pvp-maia`, ordinary/dedicated `mode-pvp-race16-control`,
  `unknown-setup-pvp-maia-back-confirm` и
  `unknown-setup-pvp-maia-back-disconnect` — **PASS**, exit 0 в обеих compile
  configurations. `git diff --check` — **PASS**.

Focused runner компилировал production seam с `-Wall -Wextra -Werror`, ASan и
UBSan. Повторяющиеся declared-font fallback и SDL `Leaked thread` diagnostics
совпали с известным output; assertion, compiler и sanitizer failures не было.
Legacy/shared production files не менялись; unrelated improvements не
обнаружены. Полные `sv-build`, `sv-core`, controller review и общий workflow не
запускались: их выполняет orchestrator после assembly.
`P1-birth-runtime-evidence` / P1-AC30 остаётся **deferred; not run** до full
acceptance на exact accepted contract digest и HEAD.
