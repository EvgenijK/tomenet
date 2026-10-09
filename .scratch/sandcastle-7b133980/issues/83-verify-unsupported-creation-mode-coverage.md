# 83: Проверить production coverage unsupported creation mode

**Type:** implementation

**Tracker role:** Wayfinder planning/execution history for canonical [SV-B-023](../../../docs/tasks/stage-b/SV-B-023-birth-choices.md); this child does not own Stage B capability IDs.

**What to build:** Подтвердить на собранном production-пути, что после удаления service contact bit `0x02` ordinary и dedicated/exclusive creation принимают поддерживаемые normalized `creation_flags` 0 и 1, а любое значение больше 1 немедленно и без побочных эффектов возвращает игроку точный текст `Unsupported character creation mode` на неизменённой character-selection surface. Focused coverage также сохраняет достижимость server-provided unavailable trait/body skips и не подменяет их unsupported setup.

**Blocked by:** None (can start immediately).

**Status:** claimed

**Assignee:** codex/sandcastle-7b133980-wave-62-1

Labels: enhancement, ready-for-agent

**Production seam for TDD:** публичный `sv_endpoint_run(SvEndpointOptions)`: authoritative contact/login setup входит через endpoint, реальные SDL gestures проходят native adapter и production login/birth interaction/input router, а endpoint outcome и controlled peer наблюдают character-selection state, name/birth ownership, draft и outbound sends. Если focused test обнаружит дефект, сначала зафиксировать RED через этот seam, затем внести минимальное SV-local production исправление и получить GREEN; отдельный test-only интерфейс или реализация поведения недопустимы. [Contract: P1-AC28, P1-AC31, P2-AC01, P2-AC05; checks: `sv-core`, `controller-review`]

- [ ] Focused production scenarios для ordinary и dedicated/exclusive create в обоих compile orders передают normalized `creation_flags > 1` и подтверждают точный текст `Unsupported character creation mode`, полностью неизменённый character-selection state, отсутствие name/birth interaction и draft mutation, а также отсутствие name, birth, partial и completed-character packet. [Contract: P1-AC31; check: `sv-core`]
- [ ] Контрольные production scenarios подтверждают, что normalized flags 0 и 1 по-прежнему открывают соответственно traditional и player-defined creation, а unavailable trait/body остаются отдельными server-provided условиями и корректно пропускаются в обоих compile orders. [Contract: P1-AC02, P1-AC19, P1-AC31; check: `sv-core`]
- [ ] Keyboard и mouse create actions проходят один production input path; focused tests не вводят fixed-response wizard, test-only behavior или обход interaction state/input router. [Contract: P1-AC11, P1-AC28, P2-AC05; checks: `sv-core`, `controller-review`]
- [ ] Relevant focused checks и `git diff --check` проходят, результаты и exact tested HEAD записаны в ticket Answer. Full `sv-build`, unfiltered `sv-core`, controller review и full acceptance не объявляются выполненными до их отдельных фактических запусков; `P1-birth-runtime-evidence` / P1-AC30 остаётся deferred с исходным producer/verifier ownership. [Contract: P1-AC30, P1-AC31; checks: `sv-build`, `sv-core`, `controller-review`, `P1-birth-runtime-evidence`]
