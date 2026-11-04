# Sandcastle task 0241e0da

Specification: docs/tasks/stage-b/SV-B-024-birth-dna.md
Base: cc904d55839993c3cc779b8bd9089e2402910adb
Verified code HEAD: ccb7e7ae89832fb064fc53db90b3537bec0456c8

## Originating request

# SV-B-024 — Подтверждение создания персонажа

Статус: specified; реализация и runtime evidence pending.

## Пользовательский результат

После всех обязательных birth steps клиент ровно один раз отправляет полный
versioned creation/play handshake и ждёт authoritative acknowledgement.

## Зависимости и граница

Зависит от [SV-B-023](SV-B-023-birth-choices.md). Историческое имя файла
сохранено для ссылок; DNA load/save в этом тикете больше нет и относится к C.

Handshake использует production defaults/identity bytes, требуемые полным
baseline outcome. Это не принимает profile/resource workflows, real-server
integration, `session.enter-game` или первый gameplay surface — они остаются C.

## Единственная первичная ответственность

<!-- owned-capabilities:start -->
| ID | Полный результат baseline / policy | Canonical sources |
|---|---|---|
| `capability.birth.complete` | Only after all required creation steps and dedicated mode adjustments submit final identity, stats, options, geometry and graphics/resource handshake and wait for play acknowledgement. | `source.baseline.session-play`, `source.baseline.session-dna`, `source.baseline.session-setup`, `source.policy.session` |
<!-- owned-capabilities:end -->

## Production SV проверки

1. Exact bytes/version gates для identity, stats, options, geometry, graphics,
   font/resource fields и dedicated adjustments через production serializer.
2. До завершения всех legal steps send отсутствует; double confirm, resize,
   focus и interleaved packets не дублируют final request.
3. Acknowledgement/rejection/disconnect/partial/malformed reply возвращают
   правильный owner; stale generation никогда не открывает следующий surface.
4. Guide placeholder obligation выполняется через actual caller, не через Guide
   implementation или test-only callback; после acknowledgement peer-driven
   MOTD ведёт в live-session handoff, не в gameplay surface.

## Definition of Done

Controlled peer видит точные bytes и authoritative acknowledgement. Успех
заканчивается на границе startup screen flow; gameplay presentation не требуется.
Сквозная проверка — [SV-B-025](SV-B-025-entry-complete.md).


Ticket batches: 3
Build attempts: 5
Test attempts: 3
Code review rounds: 3

Parallelism: 3
Assembled waves: 9

## Completed tickets

- .scratch/sandcastle-0241e0da/issues/01-complete-handshake.md
- .scratch/sandcastle-0241e0da/issues/02-versioned-handshake.md
- .scratch/sandcastle-0241e0da/issues/03-dedicated-completion.md
- .scratch/sandcastle-0241e0da/issues/04-single-submission.md
- .scratch/sandcastle-0241e0da/issues/05-authoritative-reply.md
- .scratch/sandcastle-0241e0da/issues/06-guide-motd-handoff.md
- .scratch/sandcastle-0241e0da/issues/07-runtime-audit.md
- .scratch/sandcastle-0241e0da/issues/08-focused-test-coverage.md
- .scratch/sandcastle-0241e0da/issues/10-accurate-ack-transcript.md
- .scratch/sandcastle-0241e0da/issues/11-restore-child-types.md
- .scratch/sandcastle-0241e0da/issues/12-index-resolved-decisions.md
- .scratch/sandcastle-0241e0da/issues/13-complete-runtime-input-inventory.md
- .scratch/sandcastle-0241e0da/issues/14-focused-test-coverage.md

## Integrated deliveries with controller-owned acceptance

- .scratch/sandcastle-0241e0da/issues/09-final-head-runtime-evidence.md: delivery integrated; historical worker status retained. Run acceptance is established only by the final verdict below.

## Deferred checks (not completed)


## Automatic recovery decisions

- repair at parallel-work: Repair the existing assembly pipeline: ticket 07 is blocked on orchestrator-owned acceptance gates, not an absent external prerequisite. Its worker commit 7bcaf3a502883c5867bd4695a3bb2bb7f0b00c92 contains the collector and 108 passing focused sessions, but remains unintegrated; evidence targets base 3e0135bfbd31510afefb93589ae20ee562d49768. Preserve completed tickets 01–06. Assemble the existing wave-4 worker commit while retaining pending acceptance, then run required sv-build (`make -s -C src -f makefile.sv tomenet-sv`) and sv-core (`bash -s -- core < .sandcastle/checks.sh`). Obtain independent controller review and rerun `python3 tests/sv_birth_runtime_evidence.py <fresh-empty-directory>` at the exact final integrated HEAD, binding the verdict to immutable digest 0ba2cedc9474a7f17f847c0bdb89d04023b0a9edefa8611f7528564eeb4f83ef. Route any fixes through the existing worker branch and assembly. Restore completion only after all required gates pass; refresh final-HEAD evidence after subsequent commits. Keep P1-ac12/P1-entry-evidence deferred to SV-B-025 and outside repair batches; preserve SV-B-023 acceptance ownership and stage-C boundaries. Do not reset or increase existing limits. Record: .scratch/sandcastle-0241e0da/recovery/1.md
- repair at parallel-work: Repair the existing wave-6 worker/assembly pipeline. Tickets 01–08 are resolved and integrated at 72fac69a58b7875f1d812980312acaa63573d4c9; worker commit 4bb9c17479807fc95ae44ee317500297eab7b9df remains unintegrated. Ticket 09 is blocked on orchestrator-owned build/core and independent controller acceptance, not an absent external implementation owner. Its reported 108-session captures and author readback cannot replace those gates, and the referenced external evidence directories are absent here. Preserve completed workers and assemble the existing wave-6 commit with acceptance pending. Recover or reproduce the required reviewed-HEAD evidence for 9a98594669cba32bef9d98236cf8f0902a27eba0. Route any fixes or ticket-resolution commits through the existing worker branch and assembly. After all integration/resolution commits, run make -s -C src -f makefile.sv tomenet-sv and bash -s -- core < .sandcastle/checks.sh using the configured Python environment. Run python3 tests/sv_birth_runtime_evidence.py <fresh-empty-directory> at that exact final integrated HEAD, retaining all 108 sessions without filters. Obtain independent controller integrity and semantic review, including exact handshake bytes, reply ownership, actual Guide caller and ack-gated MOTD/handoff; raw successful reply is 02 03 7a. Retain captures, actual build/core results, verdict and acceptance receipt in durable controller-accessible storage outside Git, bound to that HEAD and immutable digest 0ba2cedc9474a7f17f847c0bdb89d04023b0a9edefa8611f7528564eeb4f83ef. Restore completion only when every required gate passes; any later commit requires refreshed final evidence. Keep P1-ac12/P1-entry-evidence deferred to SV-B-025 and excluded from repair batches; preserve SV-B-023 ownership and stage-C boundaries. Do not increase or reset quota, cycle or recovery limits. Record: .scratch/sandcastle-0241e0da/recovery/2.md
- stop at parallel-work: Recovery cannot safely advance because of a controller/worker deadlock. Integration remains at 72fac69a58b7875f1d812980312acaa63573d4c9 with tickets 01–08 integrated; worker HEAD 48edfae35ce46996dd8c5337dacf6ecd5afe90b5 preserves 4bb9c17479807fc95ae44ee317500297eab7b9df but ticket 09 remains claimed. Its required acceptance depends on post-assembly controller gates, while parallel.mjs assembles only completed workers and workflow.mjs prohibits workers from running the full workflow. Parallel-work repair guidance reaches the blocked worker, so repeating the previous assembly instructions cannot reach the controller gates. Reported author captures do not establish independent acceptance, and their external evidence directory is absent here. Restore a supported controller handoff that separates committed worker delivery from pending acceptance, without falsely resolving ticket 09 or closing D-9496281ca15e0741. Preserve existing commits and route any runtime repair through worker branches and assembly. Then retain reviewed-HEAD evidence and run the required SV build, configured core checks, all 108 runtime sessions and independent semantic audit at the exact final assembled HEAD, with durable receipts bound to digest 0ba2cedc9474a7f17f847c0bdb89d04023b0a9edefa8611f7528564eeb4f83ef. Keep SV-B-025 entry evidence deferred and excluded from repair batches. Do not increase or reset quota, cycle or recovery limits. Record: .scratch/sandcastle-0241e0da/recovery/3.md
- repair at assemble: Repair the existing wave-6 assembly handoff. The controller receipt exists in .sandcastle/runs/0241e0da/state.json and isDeliveryReady returns true, but workflow.mjs omits receipt metadata from the assembly prompt. The assembly agent therefore cannot verify the permitted blocked-delivery merge. This is an internal handoff defect, not an absent external dependency. Supply the saved receipt through assembly-scoped guidance: validated revision 48edfae35ce46996dd8c5337dacf6ecd5afe90b5, receipt digest 30e7bba450d7da2c0cf7a9ce73a3d0480c923abb70e0abec56a373b90e44fa35, ticket digest f0b6d5819fa0d4f8d4144cd5d7e961356fc16c4d9ffe0db604cd0d718a6dd513. Ticket and change digests independently match Git. This authorizes delivery integration only; controller acceptance remains pending. Have the existing assembly agent merge that exact worker revision into codex/sandcastle-dev-0241e0da, currently 167262a9033e1cd2d7ffb64723570dea056ffa4e, preserving completed tickets 01–08 and maintenance ancestry. Preserve ticket 09's validated content, claimed status and open finding D-9496281ca15e0741. Verify ancestry, unchanged ticket digest, focused checks and a clean integration tree. Then restore completion only after required sv-build, configured sv-core, fresh unfiltered 108-session production capture and independent integrity/semantic review pass at the frozen final assembled HEAD. Recover or reproduce required reviewed-HEAD evidence for 9a98594669cba32bef9d98236cf8f0902a27eba0. Retain controller-accessible durable artifacts and verdicts bound to exact HEAD, contract digest 0ba2cedc9474a7f17f847c0bdb89d04023b0a9edefa8611f7528564eeb4f83ef and binding digest 5a41006942bc7809efc7664b481a64b4655a288011932139d33e00f87f1cbc60. Author capture reports do not establish acceptance. Route any code corrections through existing worker branches and assembly. Keep SV-B-025 entry evidence deferred and excluded from repair batches; preserve SV-B-023 ownership and stage-C boundaries. Do not increase or reset quota, cycle or recovery limits. Record: .scratch/sandcastle-0241e0da/recovery/4.md
- repair at plan-validate: Repair the existing ticket-planning pipeline for plan-validate checkpoint plan-validate:0:8:2:1:1. Integration HEAD f44af041838131f6461c04e63f7211c1c918fed9 already contains wave-6 revision 48edfae35ce46996dd8c5337dacf6ecd5afe90b5; preserve tickets 01–08 and ticket 09's delivered work with acceptance pending. The rejection is deterministic: batch-2.json declares only three birth-runtime inputPaths, while the collector's inventory omits consumed src/common/pack.h, src/common/md5.h and runtime fonts. font.c loads lib/xtra/font/CascadiaMono-Regular.ttf or fallback lib/xtra/font/16x24x.pcf, and endpoint-run.c sends the effective basename in the handshake. Rehashing the existing 142 fingerprints cannot establish unchanged omitted inputs. This is an internal manifest/collector defect, not an absent external owner. Have the existing planner enumerate individual committed producer dependencies: all six runners, included harness files, fixtures, compiled sources, transitive repository headers and runtime resources. Preserve independently derived baseline-oracle provenance and controller tool/runtime identity. Add a focused worker delivery prerequisite and link birth-runtime.producedBy to it; its worker must repair the collector's source inventory and before/after fingerprints through a worker branch and assembly. Merely expanding inputPaths is insufficient. Use separate Worker delivery criteria and Controller acceptance obligations so delivery does not wait for final controller gates. Restore progress only after independent review approves the corrected binding. After all deliveries are assembled, require the immutable SV build, configured core checks, fresh unfiltered 108-session production capture including no-send cases, independent integrity/semantic review and fresh final audit at the exact final integrated HEAD and contract digest 0ba2cedc9474a7f17f847c0bdb89d04023b0a9edefa8611f7528564eeb4f83ef. Retain durable external receipts; preserve historical evidence under its original revisions. Keep P1-ac12/P1-entry-evidence deferred to SV-B-025 and excluded from repair batches, preserve SV-B-023 ownership and stage-C boundaries, and do not increase or reset quota, cycle or recovery limits. Record: .scratch/sandcastle-0241e0da/recovery/5.md
- repair at plan-validate: Repair the existing planning pipeline at checkpoint plan-validate:0:8:2:1:1. Integration HEAD e78c3a5367c77327d9f41285f4fa887ebf54f45e preserves worker revision 48edfae35ce46996dd8c5337dacf6ecd5afe90b5. Tickets .scratch/sandcastle-0241e0da/issues/11-restore-child-types.md and .scratch/sandcastle-0241e0da/issues/12-index-resolved-decisions.md lack canonical Tracker role metadata. An in-memory probe of the production completion validator rejects both simulated deliveries; adding the required field makes both pass. Have the existing planner add this exact metadata to both tickets: 'Tracker role: Wayfinder planning/execution history for canonical [SV-B-024](../../../docs/tasks/stage-b/SV-B-024-birth-dna.md); this child does not own Stage B capability IDs.' Preserve their implementation types, separate Labels/Status fields, parent, history and worker/controller responsibility split. Restore progress only after independent review approves the corrected binding; the committed approval for the rejected binding does not supersede this incident. Preserve completed tickets 01–08 and ticket 09's delivered work with acceptance pending. Execute tickets 10–13 through worker branches and assembly, including ticket 13's collector repair before capture. Retain all 22 criteria and mandatory final integrated-HEAD SV build, configured core checks, fresh unfiltered 108-session production capture, independent integrity/semantic review and fresh final audit, with durable receipts bound to exact HEAD and contract digest 0ba2cedc9474a7f17f847c0bdb89d04023b0a9edefa8611f7528564eeb4f83ef. Keep P1-entry-evidence/P1-ac12 deferred to SV-B-025 and excluded from repair batches; preserve SV-B-023 ownership and stage-C boundaries. This is an internal planning defect, not an absent external dependency. Do not increase or reset quota, cycle or recovery limits; this is attempt 2 at the saved checkpoint. Record: .scratch/sandcastle-0241e0da/recovery/6.md

## Reviews

### Round 1

Reviewed HEAD: 9a98594669cba32bef9d98236cf8f0902a27eba0
Summary: Reviewed cc904d55839993c3cc779b8bd9089e2402910adb...9a98594669cba32bef9d98236cf8f0902a27eba0 with Standards and Spec agents in parallel. No concrete production-code defect or scope violation found. One deduplicated mandatory evidence blocker remains. Exact-HEAD build/core passed; full acceptance is not established. No files edited.

- medium: .scratch/sandcastle-0241e0da/evidence/08/RESULTS.json:3 — Mandatory fresh controlled-peer production evidence is unavailable at reviewed HEAD. Historical captures and unchanged source fingerprints support implementation correctness but do not fulfil the required final runtime gate. Fix: Collect and independently verify fresh controlled-peer handshake, acknowledgement and Guide/MOTD evidence at HEAD 9a98594669cba32bef9d98236cf8f0902a27eba0 and the immutable contract digest; retain the exact-HEAD receipt before closure.

### Round 2

Reviewed HEAD: cbbfbd38751b53bb0be78e5ba014ea0e313245b2
Summary: Reviewed cc904d55839993c3cc779b8bd9089e2402910adb...cbbfbd38751b53bb0be78e5ba014ea0e313245b2 with parallel Standards and Spec axes and an independent final artifact audit. Production behavior and current build/core evidence pass. Closure remains blocked by an inaccurate runtime transcript and mandatory tracker omissions. No files changed.

- medium: tests/sv/guide-motd.c:209 — The current controlled-peer transcript reports authoritative acknowledgement bytes 02 03 01, although the peer sends 02 03 7a. Correct binary captures establish actual behavior but do not make the false runtime statement accurate. Fix: Derive printed acknowledgement bytes from the actual reply array and collect fresh sealed evidence at the resulting final HEAD. Preserve previous captures as historical evidence.
- low: .scratch/sandcastle-0241e0da/issues/01-complete-handshake.md:7 — Wayfinder children 01–06 omit the required Type field. Fix: Add the appropriate Type metadata to each affected child while preserving status and history.
- low: .scratch/sandcastle-0241e0da/map.md:18 — Resolved children 01, 02 and 04 have Answers but no decision summaries under Decisions so far. Fix: Add short factual summaries linking to each resolved child's Answer.

### Round 3

Reviewed HEAD: ccb7e7ae89832fb064fc53db90b3537bec0456c8
Summary: Reviewed cc904d55839993c3cc779b8bd9089e2402910adb...ccb7e7ae89832fb064fc53db90b3537bec0456c8. Parallel Standards and Spec reviews found no blocking defects. Independent final audit supports implementation acceptance for contract 0ba2cedc9474a7f17f847c0bdb89d04023b0a9edefa8611f7528564eeb4f83ef. Exact-HEAD build/core receipts pass; sealed runtime attempt d93fcb6e-e864-4986-8318-093e1c6aa9c7 contains 108 production-endpoint cases. Verified 162 source and 604 artifact fingerprints, then independently checked packet contents, replies, production callers and rendered startup frames. Deferred acceptance retains its owners. No files edited.

- No findings

## Acceptance

Verdict: ready; scope: implementation
Contract SHA256: 0ba2cedc9474a7f17f847c0bdb89d04023b0a9edefa8611f7528564eeb4f83ef
Contract: .scratch/sandcastle-0241e0da/acceptance-contract.json
Audited code HEAD: ccb7e7ae89832fb064fc53db90b3537bec0456c8
Fresh final audits: 1/2

Coverage: 21 passed, 0 failed, 1 pending.

## Criteria and coverage

- P1-ac1: passed — 1. After all required creation steps and dedicated mode adjustments, submit the complete versioned creation/play handshake exactly once and wait for authoritative play acknowledgement.. Source: docs/tasks/stage-b/SV-B-024-birth-dna.md. Evidence: sv_birth_committed_play requires every required commitment, READY ownership and the current generation. The production endpoint invokes sv_login_start_play, which queues the complete handshake and changes SELECTED to WAIT_PLAY, preventing repeat submission. Controlled-peer waiting, partial and acknowledgement cases confirm the transitions.
- P1-ac2: passed — 2. Use production defaults and identity bytes required by the baseline outcome.. Source: docs/tasks/stage-b/SV-B-024-birth-dna.md. Evidence: Independently reconstructed all 199 default option bytes from legacy option_info and shipped options.prf: production defaults match, including 88 enabled options. Captures contain the committed identity/stat bytes, disabled audio/graphics, actual CascadiaMono-Regular.ttf resource, 66×22 geometry and complete zero redefinition tables.
- P1-ac3: passed — 3. Verify exact bytes and version gates for final identity, stats, options, geometry, graphics, font/resource fields and dedicated adjustments through the production SV serializer. Controller review and fresh final audit must establish production-path evidence; baseline runner success alone does not establish this evidence.. Source: docs/tasks/stage-b/SV-B-024-birth-dna.md. Evidence: Compared production serializer gates directly with baseline nclient.c:Net_start. Independently reconstructed and compared all 94 captured handshakes with zero differences, including 40 version probes, dedicated adjustments, both birth compile orders and point-buy stats. Strict versus inclusive gates, option counts, table counts and character widths agree.
- P1-ac4: passed — 4. Send no final request before all legal creation steps complete; double confirm, resize, focus and interleaved packets must not duplicate the final request.. Source: docs/tasks/stage-b/SV-B-024-birth-dna.md. Evidence: The sealed submission suite has 18 passing cases. Its production endpoint peer checks no final bytes before completion, cancel/quit at all seven owners, backtracking, stale input, held/double confirmation, resize/focus events, interleaved ping and delayed acknowledgement with keepalive. SELECTED-to-WAIT_PLAY gating prevents duplicate final requests.
- P1-ac5: passed — 5. Acknowledgement, rejection, disconnect, partial replies and malformed replies return control to the correct owner; stale generations never open the next surface.. Source: docs/tasks/stage-b/SV-B-024-birth-dna.md. Evidence: Inspected the production incremental decoder, endpoint failure handling and generation guards. The 17 sealed reply cases establish successful split replies, partial waiting, rejection, malformed replies, truncated EOF, disconnect, peer quit, acknowledgement with trailing packets, rejection with trailing success and retired-generation input/reply isolation.
- P1-ac6: passed — 6. Fulfil the Guide placeholder obligation through the actual production caller, without substituting Guide implementation or a test-only callback. Controller review and fresh final audit verify this production seam.. Source: docs/tasks/stage-b/SV-B-024-birth-dna.md. Evidence: Guide gestures pass through sv_native_birth_event and the actual endpoint placeholder caller. The guide-motd suite observes real TTF rendering and SDL presentation while forwarding both unchanged. Both compile orders verify owner coverage and preserved drafts; inspected PNG output displays 'The guide is in development'.
- P1-ac7: passed — 7. After authoritative acknowledgement, peer-driven MOTD leads to live-session handoff without opening a gameplay surface.. Source: docs/tasks/stage-b/SV-B-024-birth-dna.md. Evidence: Production PLAY_ACKNOWLEDGED handling publishes peer-provided MOTD, then LIVE_HANDOFF after acknowledgement by the user or the configured skip. Sealed Guide cases prohibit MOTD/handoff presentation before authoritative acknowledgement. Inspected MOTD and handoff PNGs show the peer text and disabled gameplay boundary.
- P1-ac8: passed — 8. Provide controlled-peer runtime evidence showing the exact handshake bytes and authoritative acknowledgement. Implementation readiness or baseline check success alone does not satisfy this full-acceptance evidence requirement; the controller verifies it during review and fresh final audit.. Source: docs/tasks/stage-b/SV-B-024-birth-dna.md. Evidence: Independently verified receipt digest 2d99ba0c51653c39ab8c6f7d1b935cf7a726a921d7f44f245752c0174a73bbce, all 604 receipt artifact hashes and all 162 repository inputs against this HEAD. The sealed full-108 manifest contains six passing controlled-peer suites using sv_endpoint_run. Binary replies contain 02 03 7a and agree with the successful Guide acknowledgement transcripts; incomplete replies do not claim acknowledgement.
- P1-ac9: passed — 9. Preserve the dependency on SV-B-023 birth choices and the historical ticket filename for references; DNA load/save remains owned by stage C and is outside this implementation.. Source: docs/tasks/stage-b/SV-B-024-birth-dna.md. Evidence: The authoritative SV-B-024-birth-dna.md filename and SV-B-023 dependency remain unchanged. The implementation consumes the existing birth interaction and adds no DNA load/save workflow.
- P1-ac10: passed — 10. Keep profile/resource workflows, real-server integration, session.enter-game and the first gameplay surface outside SV-B-024 acceptance; these remain stage C responsibilities. Handshake resource fields remain required.. Source: docs/tasks/stage-b/SV-B-024-birth-dna.md. Evidence: The production diff adds creation/play serialization and startup transitions without adding profile/resource workflows, real-server integration, session.enter-game or gameplay presentation. Required graphics/font/resource handshake fields are present and byte-verified.
- P1-ac11: passed — 11. End successful SV-B-024 acceptance at the startup screen flow boundary; gameplay presentation is not required.. Source: docs/tasks/stage-b/SV-B-024-birth-dna.md. Evidence: Success is reported at SV_PREGAME_LIVE_HANDOFF. Inspected runtime output and rendered handoff capture explicitly stop at startup completion with gameplay input, map and HUD inactive.
- P1-ac12: pending — 12. Preserve SV-B-025 ownership of the end-to-end entry verification. Its deferred evidence is not passing evidence for this checkpoint.. Source: docs/tasks/stage-b/SV-B-024-birth-dna.md. Evidence: Deferred owner: SV-B-025. Its authoritative ticket retains end-to-end native entry verification. The sealed manifest marks P1-entry-evidence deferred; SDL dummy/software captures do not establish downstream executable, physical-input or platform acceptance.; pending at SV-B-025: The assigned source places end-to-end verification in the downstream SV-B-025 ticket; this checkpoint covers SV-B-024 birth completion.
- P2-1: passed — 1. Minimize changes to the existing client and shared files when implementing this SV task.. Source: AGENTS.md. Evidence: All ten changed production source files are under src/client/sv. No existing legacy client or src/common production file changed.
- P2-2: passed — 2. Place implementation and adaptation in SV where possible. Permit duplication that preserves isolation and avoids a large dependency set; extract shared code only when needed for this task, rather than solely to eliminate duplication.. Source: AGENTS.md. Evidence: Serialization, defaults, birth adaptation and startup state changes remain SV-local. No shared extraction or additional legacy dependency set was introduced.
- P2-3: passed — 3. Preserve the protocol and gameplay semantics of the behavior baseline during the transfer. Require the immutable SV build and core checks for implementation readiness; assess baseline preservation separately through controller review and fresh final audit.. Source: AGENTS.md. Evidence: Independent baseline comparison confirms handshake semantics, dedicated identity adjustments, fatal play failure ownership and startup boundaries. Exact-HEAD controller sv-build and sv-core logs report success; the core runner includes all six focused birth suites. These logs were inspected, not rerun in this read-only audit.
- P2-4: passed — 4. Tests must invoke the production SV path. A local SV implementation is permitted; a separate implementation of behavior used only by tests is prohibited. Verify the production seam through controller review and fresh final audit.. Source: AGENTS.md. Evidence: Focused executables compile and invoke production sv_endpoint_run, native input, birth interaction, login serializer/decoder and rendering. Controlled peers supply protocol stimuli and independent byte oracles. Font and rendering observers forward real functions without substituting creation behavior.
- P2-5: passed — 5. Record any discovered opportunities to improve legacy code, shared modules, or SV organization in docs/sv-improvements.md, separately from implementing the current feature.. Source: AGENTS.md. Evidence: docs/sv-improvements.md separately records controlled-peer worker lifetime diagnostics, the misleading compatibility-table comment and legacy acknowledgement read robustness.
- P2-6: passed — 6. Each improvement entry must identify the problem, affected code, proposal, status, and necessary checks. Recording an improvement does not bring it into this task's implementation scope.. Source: AGENTS.md. Evidence: Each added improvement entry identifies its problem, affected code, proposal, separate proposed status and required checks. The corresponding legacy/shared improvements were not implemented in this production diff.
- P2-7: passed — 7. Do not treat intentional duplication for isolation as a defect or an obligation to consolidate later. Apply the repository isolation rule with priority over older project-document requirements for a shared SV and legacy implementation.. Source: AGENTS.md. Evidence: SV-local defaults and protocol serialization preserve isolation. Intentional duplication was assessed against behavior and dependency boundaries, not treated as a consolidation defect.
- P2-8: passed — 8. When creating, triaging, or reviewing tasks, use local Markdown: Wayfinder tasks in .scratch/ and SV implementation tickets in docs/tasks/, following the referenced issue-tracker path and status rules without expanding this task's authoritative obligations.. Source: AGENTS.md. Evidence: Authoritative implementation tickets remain under docs/tasks/stage-b; execution children remain under .scratch/sandcastle-0241e0da/issues. Inspected child metadata retains local Type, Status, Assignee and tracker ownership. Generated children were not used to alter the supplied contract.
- P2-9: passed — 9. When performing triage, use the five standard roles and a separate Labels: field, with role correspondence clarified by docs/agents/triage-labels.md.. Source: AGENTS.md. Evidence: Inspected triage metadata uses a separate Labels: field with documented category/state labels, including enhancement or bug and ready-for-agent or ready-for-human. Labels remain separate from Wayfinder resolution status.
- P2-10: passed — 10. Before working with domain terminology or architecture, read the root CONTEXT.md and task-relevant ADRs; use the referenced domain documentation to clarify the single-context layout without introducing obligations outside the fixed authoritative sources.. Source: AGENTS.md. Evidence: Read root CONTEXT.md, docs/agents/domain.md and relevant ADRs on interaction/UI separation, main-thread ownership, explicit connections, session generations and error handling before evaluating the architecture. The implementation preserves authoritative server results and generation-scoped startup ownership.

## Controller checks

- sv-build: passed; /home/svechnik/Projects/tomenet_modern_client/.sandcastle/runs/0241e0da/check-sv-build-5-2.json
- sv-core: passed; /home/svechnik/Projects/tomenet_modern_client/.sandcastle/runs/0241e0da/check-sv-core-5-3.json
- controller-review: passed; Controller-owned review and fresh final-audit gate at the exact contract digest and HEAD
- P1-entry-evidence: not run; Deferred downstream end-to-end verification owned by SV-B-025; does not constitute passing evidence for this checkpoint.

## Residual backlog (open, not fixed)


## Defect history

- D-9496281ca15e0741: resolved; integrated repair cycles: 1/3; Mandatory fresh controlled-peer production evidence is unavailable at reviewed HEAD. Historical captures and unchanged source fingerprints support implementation correctness but do not fulfil the required final runtime gate.
- D-86f98a5241b62943: resolved; integrated repair cycles: 1/3; The current controlled-peer transcript reports authoritative acknowledgement bytes 02 03 01, although the peer sends 02 03 7a. Correct binary captures establish actual behavior but do not make the false runtime statement accurate.
- D-df28653d4c8d0985: resolved; integrated repair cycles: 1/3; Wayfinder children 01–06 omit the required Type field.
- D-e6e9e23d4a29cac2: resolved; integrated repair cycles: 1/3; Resolved children 01, 02 and 04 have Answers but no decision summaries under Decisions so far.

## Preserved behavior

- Complete baseline Net_start wire order and strict/inclusive version gates.
- Production option defaults, committed identity/stat bytes, resource fields, map geometry and complete visual redefinition tables.
- Dedicated IDDC variants, skipped-trait normalization and post-mode PvP Maia trait/stat adjustments.
- No final request before all legal birth steps complete; exactly one final request per completed creation.
- Incremental authoritative acknowledgement assembly with continued keepalive, ping and server-flag handling.
- Fatal play rejection/disconnect ownership without accidental name/login retry or stale-generation surface transitions.
- Actual Guide placeholder callers preserve birth drafts and return to their owners.
- Peer-driven MOTD ends at startup live-session handoff with gameplay inactive.
- Legacy/shared production isolation and stage C ownership of excluded workflows.


Headless gates do not establish full ticket acceptance. Later caller integrations, display-backed human review and platform checks remain pending unless separately evidenced.
