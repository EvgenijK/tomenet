# 06: Actual Guide caller и peer-driven MOTD до live-session handoff

**What to build:** Игрок возвращается из временного Guide placeholder к тому же birth caller, завершает creation и после authoritative play acknowledgement проходит peer-driven MOTD до live-session handoff на границе startup screen flow.

**Blocked by:** 01: Полный creation/play handshake и ожидание acknowledgement.

Status: resolved
Assignee: codex/sandcastle-0241e0da-wave-3-2

Labels: enhancement, ready-for-agent

**Finding IDs:** None — initial decomposition; ledger repair findings не предоставлены.

**Origin:** SV-B-024, capability.birth.complete; immutable contract `0ba2cedc9474a7f17f847c0bdb89d04023b0a9edefa8611f7528564eeb4f83ef`.

**Production seam:** Public SV production seam для TDD: `sv_endpoint_run(SvEndpointOptions)` с controlled peer и native SDL input. Public `sv_birth_command` через birth interaction используется только для дополнительных state checks; он не заменяет endpoint evidence final send. Связь: P1-ac1/P1-ac3/P2-4 → sv-core и controller-review.

Существующий endpoint seam доступен; прямого публичного final-play serializer API пока нет. Реализовать требуемый production path внутри SV и вызывать его через endpoint; не придумывать test-only интерфейс. Если endpoint не позволяет проверить обязательный критерий, зафиксировать seam/evidence blocker, а не заменять тестируемое поведение.

## Acceptance criteria

- [x] **P1-ac6, P2-4:** actual native production caller каждого обязательного достижимого Guide entry показывает ровно `The guide is in development`, возвращает тот же owner и сохраняет selection/draft; после возврата игрок реально завершает birth. Guide implementation и test-only callback не заменяют caller.
- [x] **P1-ac1, P1-ac7:** задержанный play ack не открывает post-birth MOTD/handoff; после authoritative ack используются peer-provided MOTD и baseline continue/skip policy, финальное состояние — live-session handoff. Existing-character startup behavior сохраняется.
- [x] **P1-ac7, P1-ac11:** actual native surface/outcome и controlled-peer transcript подтверждают порядок ack → MOTD → handoff; gameplay surface и session.enter-game не открываются.
- [x] **P1-ac6, P1-ac8:** предоставить воспроизводимое evidence actual caller и native MOTD/handoff для controller review; headless callback assertions не считать доказательством native caller.

## Границы и проверки

Каждый implementation срез сохраняет protocol/game semantics behavior baseline и SV isolation (P2-1–P2-4, P2-7), проходит sv-build и настроенный sv-core; controller-review и fresh final audit отдельно устанавливают acceptance evidence. Начинать с failing проверки public production seam, затем реализовать production поведение.

SV-B-023 — сохранённая upstream dependency, уже присутствующий production foundation, но его full acceptance/native evidence pending; он не включён в manifest как выдуманный completed dependency. При обнаружении реального отсутствующего prerequisite остановить зависимую реализацию и записать blocker.

P1-ac9–P1-ac11: DNA persistence, profile/resource workflows, real-server integration, session.enter-game и gameplay presentation не входят в этот срез. Требуемые handshake resource bytes входят. P1-ac12 / P1-entry-evidence deferred в SV-B-025, не passed. Обнаруженные улучшения записывать отдельно по P2-5/P2-6; намеренная изоляционная duplication сама по себе не finding.

Публикация тикета не означает, что implementation, runtime checks или controller audit уже выполнены. Работа по frontier начинается с 01; blocked children доступны после завершения перечисленных prerequisites.

## Comments

2026-10-10: Claimed on the assigned isolated branch; initial status was ready-for-agent. Dependency 01 is resolved and its production final handshake/ack path is present. Agreed seam: `sv_endpoint_run(SvEndpointOptions)` with controlled peer and native SDL input, as explicitly authorized in the ticket.

## Answer

Implemented on `codex/sandcastle-0241e0da-wave-3-2`; status history:
ready-for-agent → claimed 2026-10-10 → resolved after focused verification.
No other branch was edited or merged. Dependency 01 is present and retained;
SV-B-023 native/full acceptance is still pending at its original owner.

**Production seam:** explicitly agreed public `sv_endpoint_run(SvEndpointOptions)`
with controlled TCP peer and native SDL input. Production Guide callers already
existed: no replacement Guide or callback was introduced. Endpoint coverage now
returns from every reachable ordinary Guide caller (class, race, trait, body,
point-buy stats and mode), preserves the same owner/selection/draft, and actually
finishes birth, proving the final identity/stats through the peer's full-byte oracle.
Sex and stat-order Guide no-ops are retained. Both class/race compile orders pass.

**Changed behavior:** authoritative creation/play acknowledgement transfers
ownership to the existing peer MOTD interaction. Pending or partial replies
cannot enter it. The transition clears prior keys and applies existing skip-MOTD
policy; a subsequent native key (including letters) continues to live-session
handoff. Success is reported at that startup boundary. Existing-character
selection/MOTD behavior remains intact. All production edits are in SV; no legacy
or shared source edits. Earlier handshake-only tests explicitly skip MOTD and
expect the new live-handoff destination after acknowledgement.

**Red/green:** `python3 tests/sv_guide_motd_checks.py` initially failed after the
full authoritative reply because no MOTD frame was presented. The initial fixture
had a truncated MOTD string, corrected during the loop; setup errors are not
claimed as behavioral red. The corrected final test was also replayed against
unchanged production parent HEAD with `SV_GUIDE_ORDERS=1 SV_GUIDE_CASES=ack
python3 tests/sv_guide_motd_checks.py`: same no-MOTD assertion failed (see red.log).
After the minimal SV implementation, `python3 tests/sv_guide_motd_checks.py` passed:
10 cases, both compile orders, delayed ack, partial ack, continue, skip and
point-buy Guide followed by real completion. ASan/UBSan and compiler warnings
are enabled. The point-buy fixture changes only baseline committed stats.

**Focused regressions passed:**

- `python3 tests/sv_first_session_checks.py` (existing-character startup, MOTD,
  skip, disconnect and retry).
- `python3 tests/sv_birth_complete_checks.py` (full bytes, pending/partial/ack).
- `SV_DEDICATED_CASES=iddc,pvp-maia python3 tests/sv_birth_dedicated_checks.py`
  (both birth-choice orders).
- `SV_SUBMISSION_CASE=ack,retry python3 tests/sv_birth_single_submission_checks.py`.

**Evidence:** [frames, peer bytes, acknowledgement and transcripts](../evidence/06/README.md).
Reproduce: `SV_GUIDE_EVIDENCE=.scratch/sandcastle-0241e0da/evidence/06 python3
 tests/sv_guide_motd_checks.py`. The harness observes actual SDL/TTF rendering,
forwards real rendering calls, captures presented native surfaces, and validates
public endpoint outcomes and peer bytes. It does not substitute a Guide caller.
The environment uses SDL dummy/software; an interactive native display capture
can be reproduced with the documented backend override. Controller review/fresh
final audit remain responsible for accepting native evidence at exact HEAD.
Checkmarks above denote implementation/evidence availability, not completed
controller gates or whole-contract acceptance.

Registered the focused check in `.sandcastle/checks.sh` for orchestration.
As requested, the whole build/core/review workflow was not run. `sv-build`, full
configured `sv-core`, controller-review and fresh audit remain orchestrator-owned.
The upstream SDL worker teardown diagnostic observed in the existing-character
check is already recorded in `docs/sv-improvements.md`; no additional improvement
was included in this slice. Intentional test fixture reuse is not a consolidation
obligation.

DNA persistence, profile/resource workflows, real-server integration,
`session.enter-game` and gameplay presentation remain outside this slice. The
historical SV-B-024 ticket filename and resource handshake requirements remain.
P1-ac12 / P1-entry-evidence remain deferred to SV-B-025, not passed. Runtime audit
and deferred entry checks retain their originating owners.
