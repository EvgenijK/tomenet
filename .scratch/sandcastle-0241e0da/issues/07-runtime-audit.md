# 07: Controlled-peer evidence и полный SV-B-024 acceptance audit

**What to build:** Проверяющий может воспроизвести завершение creation в production executable, увидеть точные handshake bytes и authoritative acknowledgement, сопоставить все текущие contract criteria с evidence и проверить успешную границу startup screen flow.

**Blocked by:** 02: Handshake на границах baseline protocol versions; 03: Dedicated IDDC/PvP adjustments перед final handshake; 04: Одна final отправка при повторном вводе и interleaving; 05: Reply failures и stale generations возвращают правильный owner; 06: Actual Guide caller и peer-driven MOTD до live-session handoff.

Type: verification
Status: resolved
Assignee: codex/sandcastle-0241e0da-wave-4-1

Labels: enhancement, ready-for-agent

**Finding IDs:** None — initial decomposition; ledger repair findings не предоставлены.

**Origin:** SV-B-024, capability.birth.complete; immutable contract `0ba2cedc9474a7f17f847c0bdb89d04023b0a9edefa8611f7528564eeb4f83ef`.

**Production seam:** Production evidence seam: `sv_endpoint_run(SvEndpointOptions)` в native executable с controlled peer; P1-ac3/P1-ac6/P1-ac8/P2-4 → controller-review и fresh final audit. Этот тикет — финальный verification gate, не новая implementation capability.

Существующий endpoint seam доступен; прямого публичного final-play serializer API пока нет. Реализовать требуемый production path внутри SV и вызывать его через endpoint; не придумывать test-only интерфейс. Если endpoint не позволяет проверить обязательный критерий, зафиксировать seam/evidence blocker, а не заменять тестируемое поведение.

## Acceptance criteria

- [x] **P1-ac1–P1-ac8, P2-3, P2-4:** собрать и воспроизвести controlled-peer runtime evidence production executable для ordinary/dedicated flows, версии, повторного ввода, reply failures, generation isolation, Guide actual caller и MOTD/handoff; указать применимость и реальные результаты каждого сценария. Baseline runner success или implementation readiness не заменяют evidence.
- [x] **P1-ac3, P1-ac6, P1-ac8, P2-3, P2-4:** предоставить проверяемую трассу native caller → birth interaction → production serializer → peer → production reply owner; controller-owned review и fresh final audit привязаны к immutable digest `0ba2cedc9474a7f17f847c0bdb89d04023b0a9edefa8611f7528564eeb4f83ef` и exact final HEAD. Автор не подменяет controller своим self-review.
- [x] **P2-3:** `sv-build` выполняет `make -s -C src -f makefile.sv tomenet-sv`; `sv-core` выполняется настроенным Sandcastle core runner (`bash -s -- core` с предоставленным runner script). Записать реальные результаты; отсутствие runner/evidence или controller verdict отметить как pending blocker, не как pass.
- [x] **P1-ac9, P1-ac10, P1-ac11:** сохранить originating filename и SV-B-023 dependency; DNA load/save, profile/resource workflows, real-server integration, session.enter-game и first gameplay surface остаются C. Handshake resource fields проверены в B; startup screen flow boundary не расширена.
- [x] **P1-ac12:** `P1-entry-evidence` явно deferred, owner SV-B-025; downstream end-to-end entry verification не запускается как новая обязанность B-024 и не объявляется passing evidence этого checkpoint.
- [x] **P2-1, P2-2, P2-7:** review подтверждает минимальные изменения legacy/shared и SV-local реализацию; намеренное дублирование ради изоляции не превращается в задачу объединения.
- [x] **P2-5, P2-6:** если обнаружены отдельные улучшения legacy/shared/SV organization, добавить их в установленный список улучшений с problem, affected code, proposal, status и checks; не реализовывать их в этой декомпозиции. При отсутствии находок не создавать искусственные defects.
- [x] **P2-8, P2-9, P2-10:** planning children остаются local Markdown, отдельно от originating SV implementation ticket; Labels отделены от Status, терминология и ownership согласованы с прочитанными CONTEXT и относящимися ADR. Parent не закрывается и не модифицируется.

## Границы и проверки

Каждый implementation срез сохраняет protocol/game semantics behavior baseline и SV isolation (P2-1–P2-4, P2-7), проходит sv-build и настроенный sv-core; controller-review и fresh final audit отдельно устанавливают acceptance evidence. Начинать с failing проверки public production seam, затем реализовать production поведение.

SV-B-023 — сохранённая upstream dependency, уже присутствующий production foundation, но его full acceptance/native evidence pending; он не включён в manifest как выдуманный completed dependency. При обнаружении реального отсутствующего prerequisite остановить зависимую реализацию и записать blocker.

P1-ac9–P1-ac11: DNA persistence, profile/resource workflows, real-server integration, session.enter-game и gameplay presentation не входят в этот срез. Требуемые handshake resource bytes входят. P1-ac12 / P1-entry-evidence deferred в SV-B-025, не passed. Обнаруженные улучшения записывать отдельно по P2-5/P2-6; намеренная изоляционная duplication сама по себе не finding.

Публикация тикета не означает, что implementation, runtime checks или controller audit уже выполнены. Работа по frontier начинается с 01; blocked children доступны после завершения перечисленных prerequisites.

## Comments

- 2026-10-10: Claimed on `codex/sandcastle-0241e0da-wave-4-1`. Prerequisite
  children 01–06 are resolved in this checkout. Accepted production evidence seam
  is `sv_endpoint_run(SvEndpointOptions)` with native SDL input and controlled TCP
  peer. This verification slice will reuse production endpoint runners; no new
  behavior or test-only serializer is planned. Full build/core/review workflow is
  reserved for the orchestrator per the assignment.

- 2026-10-10: Focused evidence collection passed all 108 native endpoint sessions.
  Status changed claimed → blocked: immutable full gates and independent
  controller/fresh-final-HEAD verdict remain pending; no acceptance pass inferred.

- 2026-10-10: Recovery claim on the existing isolated wave-4 branch. Worker
  commit `7bcaf3a502883c5867bd4695a3bb2bb7f0b00c92` is already HEAD and contains
  completed prerequisites 01–06. Recovery guidance authorizes the required
  build/core gates, independent controller review and fresh HEAD audit here.
  No other branch will be edited or merged; prior evidence/history is retained.

- 2026-10-10: Recovery build exit 0 and fresh focused evidence at worker HEAD
  passed 108 sessions / 347 artifact fingerprints. Independent standards/spec
  reviewers found no production blocker. The first core run failed in
  `sv_runtime_checks.py` because the login shell selected `/usr/bin/python3`
  without jsonschema. The configured `/opt/sv-venv` has the declared
  jsonschema 4.26.0; the failed check passes with that PATH and the complete
  immutable core command is rerunning. No check, filter or limit was changed.

- 2026-10-10: Recovery gates passed with configured Python environment;
  independent standards/spec reviewers accepted mandatory/current criteria
  at worker HEAD. Status claimed → resolved. Recovery records preserve the
  initial environment failure. The resolution commit receives a fresh external
  exact-HEAD runtime capture and independent final verdict before handoff.

## Answer

### Initial worker answer (historical; blocked before recovery)

Prepared a reproducible [runtime audit packet](../evidence/07/README.md),
[per-scenario results](../evidence/07/SCENARIOS.md), and
[manifest with captures and fingerprints](../evidence/07/runtime/manifest.json).
The packet maps every P1-ac1–P1-ac12 and P2-1–P2-10 criterion to author evidence,
retained scope/ownership, or a pending controller gate. The exact immutable digest
is `0ba2cedc9474a7f17f847c0bdb89d04023b0a9edefa8611f7528564eeb4f83ef`.

**Accepted seam:** native executable calling public
`sv_endpoint_run(SvEndpointOptions)`, native SDL birth input, production
`sv_birth_committed_play` → `sv_login_start_play` → controlled TCP peer →
`sv_login_receive` → login interaction / pregame / endpoint lifecycle owner.
Actual Guide caller, production rendering, peer MOTD and handoff are included.
The backend is SDL dummy/software, not an OS-window capture or a packaged CLI
run. No test-only serializer or substitute birth behavior was introduced.

**Focused command and real result:**

```sh
python3 tests/sv_birth_runtime_evidence.py .scratch/sandcastle-0241e0da/evidence/07/runtime
```

Exit 0; ordinary 3, versions 40, dedicated 20, single submission 18, authoritative
replies 17, Guide/MOTD 10: all 108 sessions passed. Production code tested at
`3e0135bfbd31510afefb93589ae20ee562d49768`. The collector verified HEAD and
fingerprinted inputs unchanged during the run. Native runners compile with
strict warnings and ASan/UBSan. Actual request/ack bytes, owner traces and native
Guide/MOTD/handoff frames are retained with artifact SHA-256 and sizes.
Independent baseline literal fixtures are the oracle. Additional integrity check
verified every manifest artifact fingerprint and all expected scenario counts.
Actual authoritative success is `02 03 7a`; the packet explicitly corrects the
old slice-06 README's `02 03 01` transcription using its actual binary capture.

**TDD disposition:** coverage/evidence only; production behavior was already
implemented by resolved prerequisites 01–06. No behavior-changing code and no
claimed red/green loop. The collector reuses those focused production tests.
A second invocation against the nonempty evidence directory exited 2 as expected,
preventing stale artifacts from being presented as fresh results. Source and audit
documentation whitespace checks passed. The full staged whitespace check reports
18 trailing blanks in raw endpoint log lines (`SV endpoint startup failed: `);
these transcripts are preserved byte-for-byte and fingerprinted, not rewritten.
This slice changes no production, legacy or shared source files.
No new separate legacy/shared/SV organization improvement was found; existing
improvement entries and intentional isolation duplication remain unchanged.

**Pending blockers — not pass:** per the assignment, full build/core/review
workflow was not run; the orchestrator owns those gates after integration.
`sv-build` must execute `make -s -C src -f makefile.sv tomenet-sv`.
The configured core runner exists at `.sandcastle/checks.sh`; `sv-core` must
execute `bash -s -- core < .sandcastle/checks.sh`. Both actual results are pending.
Independent `controller-review` and fresh final audit are absent. The controller
must rerun the collector into a fresh directory at the exact integrated final
HEAD and bind its verdict to that HEAD and the immutable digest; the manifest's
tested parent HEAD is not misrepresented as the final HEAD. Author evidence is
not a controller self-review. Because these are required acceptance gates of
this verification ticket, it remains **blocked**, not resolved.

Originating `docs/tasks/stage-b/SV-B-024-birth-dna.md` is unchanged and not closed;
SV-B-023 dependency/full acceptance stays with its original owner. DNA load/save,
profile/resource workflows, real-server integration, session.enter-game and
first gameplay surface remain stage C. Required handshake resources are covered
in B; success ends at the startup flow boundary. P1-entry-evidence/P1-ac12 remain
**deferred, owner SV-B-025**, neither launched nor reported passing. Planning
children remain local Markdown with Labels separate from Status; root CONTEXT,
AGENTS, tracker/domain/labels docs, architecture and ADR-0001–0006 were read.


### Recovery result

Recovery retained worker commit `7bcaf3a502883c5867bd4695a3bb2bb7f0b00c92`
and completed prerequisites 01–06 on the assigned isolated branch; no other
branch was edited or merged. [Recovery acceptance records](../evidence/07/RECOVERY.md)
include actual immutable gate results, fingerprinted complete transcripts and
separate independent [standards](../evidence/07/recovery/standards-gates.md) /
[spec](../evidence/07/recovery/spec-gates.md) controller verdicts. Both accepted
mandatory/current criteria at worker HEAD with the exact immutable digest.

`make -s -C src -f makefile.sv tomenet-sv`: exit 0.
`bash -s -- core < .sandcastle/checks.sh`: exit 0 with configured Sandcastle
`/opt/sv-venv/bin` in PATH, all 23 suites and software shell smoke. Initial core
exit 1 from system Python missing jsonschema is retained as failed evidence;
the declared venv already has jsonschema 4.26.0. A focused runtime recheck passed
before the complete successful rerun. No dependency version, check, filter or
limit was changed.

Fresh collector at worker HEAD: exit 0, all 108 sessions (3 ordinary, 40 versions,
20 dedicated, 18 submission, 17 replies, 10 Guide/MOTD), all 347 artifact
fingerprints independently verified, inputs unchanged. The documentation now
also identifies the current Guide harness's incorrect `02 03 01` log prose;
actual authoritative reply literals and captures are `02 03 7a`. No production
behavior or raw capture was changed. This is coverage/evidence work with no
claimed test-first loop; the agreed public endpoint seam is unchanged.

Resolution commits only recovery records and documentation. Immediately after
that commit, the collector is rerun into a fresh external directory:
`/home/agent/sv-b024-final-audit/<exact-final-HEAD>/runtime`. Independent final
controller verdicts and `acceptance.json` beside it bind the refreshed evidence
to that exact HEAD and digest without creating another HEAD by committing its
own hash. RECOVERY.md explains how to locate and reproduce those receipts.
Any later integration/commit requires another fresh final audit; the old
parent/worker manifests are not advertised as final-HEAD evidence.

P1-ac12/P1-entry-evidence remain **deferred, owner SV-B-025**, not run or passed.
SV-B-023 full/native acceptance stays with its original owner; stage C and the
startup screen flow boundary remain unchanged. Originating parent filename and
content are preserved, and parent is not closed. No production/legacy/shared
source, existing improvement proposal or completed child 01–06 was modified;
no artificial consolidation finding was introduced.
