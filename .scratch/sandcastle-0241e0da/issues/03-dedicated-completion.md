# 03: Dedicated IDDC/PvP adjustments перед final handshake

**What to build:** Игрок завершает dedicated creation; сервер получает окончательные mode/identity/stats только после обязательных adjustments и дополнительных legal birth steps, затем клиент ждёт play acknowledgement.

**Blocked by:** 01: Полный creation/play handshake и ожидание acknowledgement.

Status: resolved
Assignee: codex/sandcastle-0241e0da-wave-2-2

Labels: enhancement, ready-for-agent

**Finding IDs:** None — initial decomposition; ledger repair findings не предоставлены.

**Origin:** SV-B-024, capability.birth.complete; immutable contract `0ba2cedc9474a7f17f847c0bdb89d04023b0a9edefa8611f7528564eeb4f83ef`.

**Production seam:** Public SV production seam для TDD: `sv_endpoint_run(SvEndpointOptions)` с controlled peer и native SDL input. Public `sv_birth_command` через birth interaction используется только для дополнительных state checks; он не заменяет endpoint evidence final send. Связь: P1-ac1/P1-ac3/P2-4 → sv-core и controller-review.

Существующий endpoint seam доступен; прямого публичного final-play serializer API пока нет. Реализовать требуемый production path внутри SV и вызывать его через endpoint; не придумывать test-only интерфейс. Если endpoint не позволяет проверить обязательный критерий, зафиксировать seam/evidence blocker, а не заменять тестируемое поведение.

## Acceptance criteria

- [x] **P1-ac1, P1-ac3, P1-ac4:** controlled peer получает точные итоговые bytes dedicated IDDC и PvP, включая baseline dedicated adjustments и обусловленные ими identity/stats/options; ordinary flow остаётся корректным.
- [x] **P1-ac1, P1-ac4:** post-mode PvP Maia trait return, skipped unavailable steps и оба поддерживаемых compile orders доходят до final send только после полного legal завершения; dedicated flags сами по себе не завершают birth.
- [x] **P1-ac3, P2-3, P2-4:** assertions сравнивают baseline outcome с реальным production draft/handshake; после полного ack остаётся startup owner, до ack — ожидание. SV-B-023 choice semantics не переписываются ради fixtures.

## Границы и проверки

Каждый implementation срез сохраняет protocol/game semantics behavior baseline и SV isolation (P2-1–P2-4, P2-7), проходит sv-build и настроенный sv-core; controller-review и fresh final audit отдельно устанавливают acceptance evidence. Начинать с failing проверки public production seam, затем реализовать production поведение.

SV-B-023 — сохранённая upstream dependency, уже присутствующий production foundation, но его full acceptance/native evidence pending; он не включён в manifest как выдуманный completed dependency. При обнаружении реального отсутствующего prerequisite остановить зависимую реализацию и записать blocker.

P1-ac9–P1-ac11: DNA persistence, profile/resource workflows, real-server integration, session.enter-game и gameplay presentation не входят в этот срез. Требуемые handshake resource bytes входят. P1-ac12 / P1-entry-evidence deferred в SV-B-025, не passed. Обнаруженные улучшения записывать отдельно по P2-5/P2-6; намеренная изоляционная duplication сама по себе не finding.

Публикация тикета не означает, что implementation, runtime checks или controller audit уже выполнены. Работа по frontier начинается с 01; blocked children доступны после завершения перечисленных prerequisites.

## Comments

2026-10-10: Claimed on the assigned isolated branch after verifying prerequisite 01 is resolved and its production endpoint serializer is present. Agreed TDD seam: `sv_endpoint_run(SvEndpointOptions)` with controlled peer and native SDL input; `sv_birth_command` only supplements state checks.

## Answer

Resolved on `codex/sandcastle-0241e0da-wave-2-2`. History: published
`ready-for-agent`; claimed 2026-10-10 after confirming 01 is resolved; resolved
following focused endpoint verification. No other branch was edited or merged.

**Production path:** the agreed seam is `sv_endpoint_run(SvEndpointOptions)`
with controlled TCP peer and native SDL input. SV-B-023 already implements
baseline dedicated modes and post-mode PvP Maia trait return. Slice 01 already
publishes the legal READY draft through `sv_birth_committed_play` and queues
production `sv_login_start_play` exactly once. The new endpoint checks establish
that these existing pieces implement dedicated completion together, including
final identity/stat adjustments. No production, legacy or common file changed;
SV-B-023 choices were not rewritten to accommodate fixtures.

**Test-first disposition:** inspected the existing production path, then added
and ran one focused dedicated IDDC endpoint check before considering production
changes. `python3 tests/sv_birth_dedicated_checks.py` passed for IDDC without any
production implementation change. The next vertical coverage slice,
`SV_DEDICATED_CASES=pvp-maia python3 tests/sv_birth_dedicated_checks.py`, also passed
without changes in both compile orders. No behavior defect was exposed, so no
behavior-changing implementation or red/green loop was needed. There is no
claimed behavioral red; manufacturing a failure or changing accepted SV-B-023
semantics merely to obtain red would misrepresent the existing implementation.
This result adds missing production endpoint evidence for implemented behavior.

**Focused verification:** `python3 tests/sv_birth_dedicated_checks.py` passed
all 20 cases: IDDC, IDDC Soloist, IDDC Hellish, IDDC with unavailable trait skipped,
and dedicated PvP Maia; each with waiting and full acknowledgement; each compiled
with `SV_CLASS_BEFORE_RACE=1` and `=0`. ASan/UBSan and strict warnings enabled.
Every peer compares the entire 16,266-byte production request with independent
baseline literals, including options, geometry, font/resource fields and all
visual tables. Literal oracle provenance and regeneration are recorded in
`tests/sv/fixtures/birth-dedicated-baseline.py`, building on the independent
ordinary baseline fixture, not captured SV output.

IDDC modes preserve male/fruit-bat identity and selected race/class/trait and
committed stats. Unavailable trait is skipped in the draft and becomes wire
trait 0. PvP Maia first reaches post-mode trait; BACKSPACE returns to stats;
recommitting reversed stat order and returning to trait still sends nothing;
only confirmed Corrupted (15) sends the final mode `0x390`, race 17, class 0 and
new order `5,4,3,2,1,0`. Dedicated flags, name acceptance, committed stats and
trait highlight alone do not complete birth. Peer assertions forbid early or
duplicate final requests, while resize/focus and ping remain serviced. Full ack
reaches `play-acknowledged`; absent ack stays `play-pending`. Both remain within
the startup flow; no gameplay surface opens.

**Regression:** `python3 tests/sv_birth_complete_checks.py` passed its ordinary
waiting/partial/full-ack cases. The new focused check is registered in
`.sandcastle/checks.sh`; the complete workflow was not run.

**Retained runtime evidence:** [captures and provenance](../evidence/03/README.md)
for IDDC and PvP Maia in both compile orders, waiting and acknowledged.
Reproduction passed:
`SV_DEDICATED_CASES=iddc,pvp-maia SV_DEDICATED_EVIDENCE=.scratch/sandcastle-0241e0da/evidence/03 python3 tests/sv_birth_dedicated_checks.py`.
The other focused cases use temporary captures by default.

**Ownership:** checkmarks indicate this current-baseline dedicated implementation
slice, not immutable-contract full acceptance. Version-gate layouts, including
old versions where the body owner is unavailable, remain at 02; this ticket
verifies unavailable trait skipping through the current endpoint. SV-B-023
native/full acceptance remains pending at its owner. Guide/MOTD and later
startup handoff, extended lifecycle and authoritative failure handling stay at
their original children. DNA persistence, profile/resource workflows,
real-server integration, `session.enter-game` and gameplay remain stage C.
P1-ac12/P1-entry-evidence remains deferred to SV-B-025, not passed.

No new legacy/shared/SV organization improvement was discovered; intentional
fixture reuse/duplication is not recorded as a defect. Per user instructions,
`sv-build`, the full configured `sv-core`, controller-review and fresh final
audit were not run or claimed passed: the orchestrator owns those gates.
