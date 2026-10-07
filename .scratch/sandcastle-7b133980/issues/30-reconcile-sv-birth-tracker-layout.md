# 30: Привести SV birth decomposition к canonical tracker layout

**Type:** repair

**Tracker role:** Wayfinder planning/execution history for canonical [SV-B-023](../../../docs/tasks/stage-b/SV-B-023-birth-choices.md); this child does not own Stage B capability IDs.

**What to build:** История decomposition SV-B-023 становится однозначной частью локального tracker: Sandcastle effort получает полный Wayfinder map и decision index, child records больше не выглядят как второй набор canonical Stage B implementation owners, а triage metadata использует отдельные status и labels. Содержимое resolved Answers, finding history и authoritative ownership исходного SV-B-023 сохраняются.

**Blocked by:** 26: Verify focused production coverage (completed).

**Status:** resolved

**Assignee:** Codex

Labels: bug, ready-for-agent

**Finding ID:** `D-ce5b76002eb6c0a2` (`sv-task-tracker.noncanonical-scratch-implementation-tickets`).

- [x] Effort map ссылается на originating SV-B-023, immutable contract digest, batch manifests и каждый существующий child ticket; resolved children перечислены в decision history, а открытые repair tickets образуют видимый frontier по своим blocking edges. [Contract: P2-AC10, P2-AC12; checks: `controller-review`]
- [x] Records 01–26 и появившиеся в effort repair records классифицированы как Wayfinder planning/execution history, не заявляют отдельное ownership Stage B capability IDs и не дублируются новыми implementation tickets; authoritative SV-B-023 и stage index остаются на canonical месте. [Contract: P1-AC01, P1-AC23, P1-AC26, P1-AC29, P2-AC10; checks: `controller-review`]
- [x] Каждый triaged child record имеет отдельную category label и ровно одну из пяти standard state labels; в частности focused records 13, 20 и 26 получают недостающую state label без подмены `Status:`. [Contract: P2-AC10, P2-AC11; checks: `controller-review`]
- [x] Existing status, assignee, Answer, verification result и finding linkage сохраняются; tracker-only repair не меняет production code, accepted behavior или claims о выполненных/deferred checks. [Contract: P1-AC30, P2-AC04, P2-AC05, P2-AC10; checks: `controller-review`]

**Deferred check:** `P1-birth-runtime-evidence` / P1-AC30 остаётся deferred; tracker reconciliation только сохраняет это состояние и не заявляет runtime evidence выполненным.

## Answer

Создан canonical Wayfinder index
`.scratch/sandcastle-7b133980/map.md`. Он ссылается на originating
`docs/tasks/stage-b/SV-B-023-birth-choices.md`, Stage B index, immutable
acceptance contract с digest
`690a04cefb7fd0e1615dce46d1a538e1dcbd856232af1a1373be93f987095c82`,
все четыре batch manifests и каждый child record 01–30. Все resolved records
перечислены в `Decisions so far`; после resolution record 30 открытых children
нет, а его blocking edge к resolved record 26 сохранён во frontier history.

В records 01–30 добавлена явная классификация `Wayfinder
planning/execution history` со ссылкой на canonical SV-B-023 и декларацией,
что child не владеет Stage B capability IDs. Исходные Type, Status, Assignee,
Answer, verification result, finding linkage и deferred-check claims records
01–29 сохранены. Новые implementation tickets в `docs/tasks/` не создавались;
canonical SV-B-023 и Stage B index не редактировались. Focused records 13, 20
и 26 получили недостающую standard state label `ready-for-agent` рядом с
существующей category `enhancement`; их `Status: resolved` не менялся.

Production seam для этого tracker-only repair отсутствует: production behavior,
SV code и tests не менялись, поэтому behavioral red → green TDD loop и новые
tests неприменимы. Принятый production seam исторических birth records
`sv_endpoint_run(SvEndpointOptions)` не заменялся и не вызывался для проверки
Markdown ledger. Навык TDD тем самым не привёл к созданию test-only seam.

Focused verification:

- до изменения `python3 -B - <<'PY'` (inline structural tracker audit) —
  **RED**, exit 1: отсутствовал `map.md`, во всех 30 children отсутствовала
  Wayfinder-history classification, а records 13, 20 и 26 не имели state label;
- после изменения тот же `python3 -B - <<'PY'` audit — **GREEN**, exit 0:
  `issues=30 resolved=30 errors=0`; проверены canonical/digest/batch links,
  единственность child links, decision/frontier placement, tracker role,
  triage roles, link targets и immutable digest;
- `git diff --check` — **pass**;
- scope/preservation checks — **pass**: изменены только Wayfinder map и child
  metadata/record 30; `src/`, `tests/`, canonical SV-B-023, Stage B index и
  `acceptance-contract.json` не изменены; прежние Answers records 01–29
  сохранены byte-for-byte после исключения добавленной role metadata и трёх
  требуемых state labels.

Полный build/test/review workflow не запускался по handoff orchestrator.
`P1-birth-runtime-evidence` / P1-AC30 остаётся **deferred; not run** у
исходных producer/verifier owners. Accepted behavior и прежние claims о
выполненных или deferred checks не менялись. Новых improvement opportunities
для `docs/sv-improvements.md` в scope tracker repair не обнаружено.
