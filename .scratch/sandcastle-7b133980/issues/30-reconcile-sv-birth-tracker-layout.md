# 30: Привести SV birth decomposition к canonical tracker layout

**Type:** repair

**What to build:** История decomposition SV-B-023 становится однозначной частью локального tracker: Sandcastle effort получает полный Wayfinder map и decision index, child records больше не выглядят как второй набор canonical Stage B implementation owners, а triage metadata использует отдельные status и labels. Содержимое resolved Answers, finding history и authoritative ownership исходного SV-B-023 сохраняются.

**Blocked by:** 26: Verify focused production coverage (completed).

**Status:** unclaimed

**Assignee:** —

Labels: bug, ready-for-agent

**Finding ID:** `D-ce5b76002eb6c0a2` (`sv-task-tracker.noncanonical-scratch-implementation-tickets`).

- [ ] Effort map ссылается на originating SV-B-023, immutable contract digest, batch manifests и каждый существующий child ticket; resolved children перечислены в decision history, а открытые repair tickets образуют видимый frontier по своим blocking edges. [Contract: P2-AC10, P2-AC12; checks: `controller-review`]
- [ ] Records 01–26 и появившиеся в effort repair records классифицированы как Wayfinder planning/execution history, не заявляют отдельное ownership Stage B capability IDs и не дублируются новыми implementation tickets; authoritative SV-B-023 и stage index остаются на canonical месте. [Contract: P1-AC01, P1-AC23, P1-AC26, P1-AC29, P2-AC10; checks: `controller-review`]
- [ ] Каждый triaged child record имеет отдельную category label и ровно одну из пяти standard state labels; в частности focused records 13, 20 и 26 получают недостающую state label без подмены `Status:`. [Contract: P2-AC10, P2-AC11; checks: `controller-review`]
- [ ] Existing status, assignee, Answer, verification result и finding linkage сохраняются; tracker-only repair не меняет production code, accepted behavior или claims о выполненных/deferred checks. [Contract: P1-AC30, P2-AC04, P2-AC05, P2-AC10; checks: `controller-review`]

**Deferred check:** `P1-birth-runtime-evidence` / P1-AC30 остаётся deferred; tracker reconciliation только сохраняет это состояние и не заявляет runtime evidence выполненным.
