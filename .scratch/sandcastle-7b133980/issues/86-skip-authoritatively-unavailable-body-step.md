# 86: Пропустить authoritative unavailable body step

**Type:** repair

**Tracker role:** Wayfinder planning/execution history for canonical [SV-B-023](../../../docs/tasks/stage-b/SV-B-023-birth-choices.md); this child does not own Stage B capability IDs.

**What to build:** Когда authoritative supported creation setup не предоставляет ни одного legal body choice, игрок никогда не получает недоступного body owner: forward flow сразу переходит к следующему доступному owner, а Backspace обходит body в обоих class/race compile orders. Draft и соседние owner selections сохраняются, partial/completed packets не отправляются, а supported `creation_flags` остаются 0 или 1.

**Blocked by:** None (can start immediately).

**Status:** ready-for-agent

Labels: bug, ready-for-agent

**Finding ID:** `D-1563ec778f0a5c2f` (`sv.birth.body-step.never-unavailable`), one coherent repair of authoritative body-step availability, forward sequencing and every affected reverse transition.

**Production seam for TDD:** публичный `sv_endpoint_run(SvEndpointOptions)` получает authoritative server-provided birth setup через controlled peer и production setup/interaction state; реальные SDL keyboard/mouse gestures проходят native adapter и публичную birth-interaction/input-router boundary, а `SvEndpointOutcome` и peer наблюдают owner, `body_skipped`, legal body list, draft и отсутствие запрещённых outbound bytes. Seam принят P1-AC02, P1-AC15, P1-AC17, P1-AC19, P1-AC28 и P2-AC05; проверки — `sv-core`, `sv-build` и `controller-review`. Availability должна поступать через production setup state; отдельный test-only flag или behavior implementation недопустимы.

- [ ] RED production-endpoint case с supported normalized `creation_flags` 0 или 1 и authoritative empty legal-body set показывает прежнее назначение body owner; GREEN хранит complete-step availability в production setup/interaction state, выставляет `body_skipped`, оставляет legal body list пустым и не назначает body owner. [Contract: P1-AC02, P1-AC07, P1-AC19, P1-AC28, P1-AC31, P2-AC05; check: `sv-core`]
- [ ] В class-before-race order forward flow после предшествующего доступного owner сразу достигает stats, а Backspace из stats возвращается к ближайшему доступному trait/race owner, сохраняя draft/highlight и не отправляя partial либо completed-character packet. [Contract: P1-AC14, P1-AC16, P1-AC17, P1-AC19, P2-AC04; check: `sv-core`]
- [ ] В race-before-class order forward flow обходит empty body step и достигает class, а Backspace из class возвращается через правильного доступного predecessor без посещения body; отдельно покрыты сочетания available и skipped trait. [Contract: P1-AC14, P1-AC15, P1-AC16, P1-AC19, P2-AC04; checks: `sv-build`, `sv-core`]
- [ ] Keyboard и mouse production scenarios подтверждают одинаковое semantic sequencing; supported ordinary и dedicated IDDC/PvP flows, non-empty Normal/Fruit Bat body choices, random/DNA behavior, quit, disconnect/stale-generation guards и unsupported-mode fail-fast остаются без регрессии. [Contract: P1-AC02, P1-AC07, P1-AC11, P1-AC12, P1-AC20, P1-AC22, P1-AC26, P1-AC31; checks: `sv-build`, `sv-core`]
- [ ] Repair остаётся SV-local по возможности, использует production interaction state/input router и не добавляет fixed-response либо test-only behavior; unrelated legacy/shared/SV organization improvements записываются отдельно и не включаются в этот ticket. [Contract: P1-AC28, P2-AC01, P2-AC02, P2-AC03, P2-AC05, P2-AC06, P2-AC07, P2-AC08, P2-AC09; check: `controller-review`]

**Deferred check:** `P1-birth-runtime-evidence` / P1-AC30 остаётся deferred у named full-acceptance producer/verifier и не считается пройденным этим repair; evidence собирается только на exact accepted contract digest `15de367676b8c4c23c1ba787bb60b4e976455f27926f4b7688e17fc75204dbe6` и implementation HEAD.
