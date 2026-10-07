# 12: Проверить implementation boundary SV-B-023

**Type:** verification

**What to build:** Готовая реализация SV-B-023 передаётся на implementation review с воспроизводимыми build/core результатами для обеих compile configurations, картой production birth seams и честно deferred full-acceptance runtime evidence; ни DNA persistence, ни final send, ни Stage G Guide capability не объявляются частью результата.

**Blocked by:** 09: Завершить или откатить creation без send; 10: Сохранить owner при session events; 11: Вернуться из birth Guide placeholder.

**Status:** ready-for-agent

Labels: enhancement, ready-for-agent

- [ ] `sv-build` проходит для default и alternate class/race compile configurations, а `sv-core` покрывает production endpoint/input/state paths всех sex/race/trait/class/body/stats/mode slices. [Contract: P1-AC02, P1-AC03, P1-AC04, P1-AC05, P1-AC06, P1-AC07, P1-AC08, P1-AC09, P1-AC19, P1-AC28; checks: `sv-build`, `sv-core`]
- [ ] Focused production evidence связывает legal/illegal/random/DNA, stable highlighting, keyboard/mouse/confirm/quit, backtrack owner graph, dedicated paths, lifecycle suppression и Guide placeholder с публичным endpoint seam; test-only behavior и fixed-response wizard отсутствуют. [Contract: P1-AC10, P1-AC11, P1-AC12, P1-AC13, P1-AC14, P1-AC15, P1-AC16, P1-AC17, P1-AC18, P1-AC20, P1-AC21, P1-AC22, P1-AC24, P1-AC25, P1-AC28; checks: `sv-core`, `controller-review`]
- [ ] Review подтверждает prerequisite integration с SV-B-022 и сохраняет границы: Guide capability остаётся Stage G, DNA persistence — Stage C, `birth.restore-dna`/`birth.save-dna` не требуются, final completed-character send остаётся SV-B-024. [Contract: P1-AC01, P1-AC23, P1-AC26, P1-AC27, P1-AC29; checks: `controller-review`]
- [ ] Full native runtime evidence `P1-birth-runtime-evidence` записано как deferred, not run, с исходной acceptance-phase причиной и владельцами; ticket не утверждает, что evidence принято, и не меняет parent task. [Contract: P1-AC30; check: `P1-birth-runtime-evidence`]
- [ ] Diff review подтверждает минимальные legacy/shared изменения, SV-local placement, допустимость изолирующего дублирования, baseline protocol/gameplay semantics и отсутствие shared extraction только ради deduplication. [Contract: P2-AC01, P2-AC02, P2-AC03, P2-AC04, P2-AC05, P2-AC09; checks: `controller-review`]
- [ ] Любые найденные улучшения legacy/shared/SV вынесены отдельно с problem, affected code, proposal, status и required checks; запись не расширяет текущий scope и intentional duplication не считается дефектом сама по себе. [Contract: P2-AC06, P2-AC07, P2-AC08; checks: `controller-review`]
- [ ] Task/evidence artifacts соблюдают local Markdown path/status/Labels rules и используют glossary/ADR vocabulary после чтения root context и relevant ADR. [Contract: P2-AC10, P2-AC11, P2-AC12; checks: `controller-review`]
