# 85: Сохранить ordinary capacity при optional dedicated flags

**Type:** repair

**Tracker role:** Wayfinder planning/execution history for canonical [SV-B-023](../../../docs/tasks/stage-b/SV-B-023-birth-choices.md); this child does not own Stage B capability IDs.

**What to build:** Игрок с доступным ordinary slot может начать обычное создание персонажа при любом сочетании advertised dedicated IDDC/PvP feature flags. Encoded dedicated limits резервируют место только для действительно поддерживаемых dedicated modes: ordinary capacity остаётся равной decoded ordinary limit с учётом общего client cap, а effective total capacity увеличивается только на supported dedicated capacity.

**Blocked by:** None (can start immediately).

**Status:** ready-for-agent

Labels: bug, ready-for-agent

**Finding ID:** `D-d037649996d38bb2` (`sv.creation-availability.reserves-unsupported-dedicated-slots`), one coherent repair of SV character-creation availability accounting and its production-boundary regressions.

**Production seam for TDD:** публичный `sv_endpoint_run(SvEndpointOptions)` получает authoritative contact limits, dedicated feature flags и roster от controlled peer; реальные keyboard/mouse create actions проходят production login interaction/input router, а `SvEndpointOutcome` и peer наблюдают character-selection state, вход в name/birth flow и outbound packets. Этот seam принят для production-path проверки prerequisite integration и interaction routing по P1-AC01, P1-AC02, P1-AC28 и P2-AC05; проверки — `sv-core`, `sv-build` и `controller-review`.

- [ ] RED production-endpoint matrix с encoded limits 12 ordinary / 2 IDDC / 1 PvP воспроизводит потерю ordinary slots для четырёх сочетаний feature flags: neither, IDDC-only, PvP-only и both; GREEN подтверждает ordinary create на последнем свободном ordinary slot и отклоняет следующий slot на точной ordinary boundary в каждом сочетании. [Contract: P1-AC01, P1-AC02, P2-AC04, P2-AC05; check: `sv-core`]
- [ ] Effective total capacity равна ordinary capacity плюс только advertised dedicated capacity (12, 14, 13 и 15 для указанной matrix, пока не срабатывает общий cap); existing или absent dedicated slots не вычитаются из ordinary limit, а IDDC/PvP creation отдельно соблюдает собственную supported-mode boundary. [Contract: P1-AC01, P2-AC04; checks: `sv-core`, `controller-review`]
- [ ] Keyboard и mouse ordinary create actions на разрешённой boundary достигают существующего production name/birth handoff без fixed-response или test-only availability implementation; запрещённая boundary оставляет character-selection state неизменным и не отправляет name, birth, partial либо completed-character packet. [Contract: P1-AC01, P1-AC02, P1-AC11, P1-AC28, P2-AC05; check: `sv-core`]
- [ ] Regression coverage сохраняет normalized `creation_flags` 0/1 и fail-fast policy для значений больше 1, оба compile orders и существующие dedicated IDDC/PvP entry paths; final completed-character send остаётся вне SV-B-023. [Contract: P1-AC19, P1-AC20, P1-AC29, P1-AC31; checks: `sv-build`, `sv-core`]
- [ ] Repair остаётся SV-local по возможности, не меняет legacy gameplay implementation и не расширяет canonical SV-B-023 scope; обнаруженные unrelated improvement opportunities записываются отдельно с требуемыми полями. [Contract: P2-AC01, P2-AC02, P2-AC03, P2-AC06, P2-AC07, P2-AC08, P2-AC09; check: `controller-review`]

**Deferred check:** `P1-birth-runtime-evidence` / P1-AC30 остаётся deferred у named full-acceptance producer/verifier и не считается пройденным этим repair; evidence собирается только на exact accepted contract digest `15de367676b8c4c23c1ba787bb60b4e976455f27926f4b7688e17fc75204dbe6` и implementation HEAD.
