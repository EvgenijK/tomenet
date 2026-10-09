# 88: Сохранить legacy Fruit Bat outcome в mode step

**Type:** repair

**Tracker role:** Wayfinder planning/execution history for canonical [SV-B-023](../../../docs/tasks/stage-b/SV-B-023-birth-choices.md); this child does not own Stage B capability IDs.

**What to build:** Игрок на pre-4.4.1a server, где отдельного body step ещё нет, по-прежнему может завершить исторические Normal и Fruit Bat outcomes через baseline mode owner. Недоступный body step не получает ownership, но его отсутствие не удаляет Fruit Bat из legal creation outcomes: keyboard/mouse, random/DNA, legality, stable highlighting и Backspace сохраняют протокольную и игровую семантику без partial или completed-character send.

**Blocked by:** [86: Пропустить authoritative unavailable body step](86-skip-authoritatively-unavailable-body-step.md) (completed; establishes the version-aware absence of the separate body owner that this repair must preserve).

**Status:** ready-for-agent

Labels: bug, ready-for-agent

**Finding ID:** `D-3dd8c1894797fcf5` (`sv.birth.legacy-body-mode.drops-fruit-bat-outcome`), one coherent repair of the lost pre-4.4.1a Fruit Bat outcome and its production-path regressions.

**Production seam for TDD:** публичный `sv_endpoint_run(SvEndpointOptions)` получает authoritative pre-4.4.1a contact/setup от controlled peer; реальные SDL keyboard/mouse gestures проходят native adapter и публичную birth-interaction/input-router boundary, а `SvEndpointOutcome` и peer наблюдают owner, legal mode identities, highlight, draft/outcome и отсутствие запрещённых outbound bytes. Этот seam принят для production interaction, mode/body semantics и production-path tests по P1-AC02, P1-AC07, P1-AC09, P1-AC10, P1-AC11, P1-AC28 и P2-AC05; проверки — `sv-core`, `sv-build` и `controller-review`.

- [ ] RED production-endpoint scenarios для authoritative pre-4.4.1a setup доходят до mode и воспроизводят потерю Fruit Bat; GREEN предоставляет Normal и Fruit Bat как legal исторические outcomes у baseline mode owner, подтверждает каждый outcome в draft/result и никогда не назначает отсутствующий body owner. Eligibility и compatibility filters соответствуют behavior baseline, а unknown/illegal identity не продвигает interaction. [Contract: P1-AC02, P1-AC07, P1-AC09, P1-AC10, P1-AC19, P2-AC04, P2-AC05; check: `sv-core`]
- [ ] Keyboard и mouse navigation сохраняют stable highlighted identity при выборе Normal и Fruit Bat, confirm завершает выбранный legacy outcome, а random и применимый staged DNA проходят тот же production command path и дают только legal outcome; invalid DNA сохраняет принятую invalidation semantics. [Contract: P1-AC07, P1-AC09, P1-AC10, P1-AC11, P1-AC26, P1-AC27, P1-AC28, P2-AC05; check: `sv-core`]
- [ ] В обоих class/race compile orders Backspace из legacy mode возвращает к правильному ближайшему доступному owner, повторный вход сохраняет согласованные draft/highlight semantics, отдельный body owner остаётся skipped, а Backspace, quit и подтверждение mode не отправляют partial либо completed-character packet. [Contract: P1-AC12, P1-AC16, P1-AC18, P1-AC19, P1-AC29, P2-AC04; checks: `sv-build`, `sv-core`]
- [ ] Production-endpoint controls сохраняют modern separate Normal/Fruit Bat body step, supported traditional/point-buy stats, ordinary и dedicated IDDC/PvP flows, post-mode PvP Maia return, disconnect/stale-generation guards и unsupported-mode fail-fast без расширения ownership SV-B-023. [Contract: P1-AC02, P1-AC07, P1-AC08, P1-AC20, P1-AC22, P1-AC29, P1-AC31; checks: `sv-build`, `sv-core`, `controller-review`]
- [ ] Repair остаётся SV-local по возможности, использует production interaction state/input router и не добавляет fixed-response либо test-only behavior; unrelated legacy/shared/SV organization improvements записываются отдельно с требуемыми полями и не включаются в repair. [Contract: P1-AC28, P2-AC01, P2-AC02, P2-AC03, P2-AC05, P2-AC06, P2-AC07, P2-AC08, P2-AC09; check: `controller-review`]

**Deferred check:** `P1-birth-runtime-evidence` / P1-AC30 остаётся deferred у named full-acceptance producer/verifier и не считается пройденным этим repair; evidence собирается только на exact accepted contract digest `15de367676b8c4c23c1ba787bb60b4e976455f27926f4b7688e17fc75204dbe6` и implementation HEAD.
