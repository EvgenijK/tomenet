# Sandcastle task f5fbf91a

Specification: docs/tasks/stage-b/SV-B-021-account-manage.md
Base: fec73b88ca5564e45ed3a89350133069b534577b
Verified code HEAD: 95aa667afd69350ddd01c764f26672d2de14bfb2

## Originating request

# SV-B-021 — Создание аккаунта

Статус: specified; реализация и runtime evidence pending.

## Пользовательский результат

Игрок создаёт новый аккаунт через те же native private fields и production
protocol path; только server response подтверждает создание и открывает overview.

## Зависимости и граница

Зависит от [SV-B-002](SV-B-002-contact.md),
[SV-B-005](SV-B-005-vault.md) и [SV-B-006](SV-B-006-login.md).
Смена пароля и account information перенесены в C.

## Единственная первичная ответственность

<!-- owned-capabilities:start -->
| ID | Полный результат baseline / policy | Canonical sources |
|---|---|---|
| `capability.account.create` | A valid unused account name and password follow server new-account handling; account flags/validation and resulting overview remain authoritative. | `source.baseline.session-credentials`, `source.protocol.session-login`, `source.baseline.session-login`, `source.policy.session` |
<!-- owned-capabilities:end -->

## Production SV проверки

1. New-account request exact bytes/version branches через production serializer;
   поля переиспользуют accepted input/private semantics без второй реализации.
2. Used/invalid name, bad password, account flag rejection, disconnect и retry
   сохраняют правильный parent и не показывают overview до server confirmation.
3. Successful authentication выполняет approved credential save policy; provider
   failure остаётся явным session-only состоянием.

## Definition of Done

Controlled peer покрывает success/rejection/cancel/error и fragmentation;
Linux/Windows native input и provider paths проверены. Результат включается в
[SV-B-025](SV-B-025-entry-complete.md).


Ticket batches: 5
Build attempts: 11
Test attempts: 6
Code review rounds: 3

Parallelism: 3
Assembled waves: 13

## Completed tickets

- .scratch/sandcastle-f5fbf91a/issues/01-create-unused-account-through-production-flow.md
- .scratch/sandcastle-f5fbf91a/issues/02-preserve-account-creation-failure-parents.md
- .scratch/sandcastle-f5fbf91a/issues/03-handle-credential-save-degradation.md
- .scratch/sandcastle-f5fbf91a/issues/04-package-implementation-evidence-and-deferred-runtime.md
- .scratch/sandcastle-f5fbf91a/issues/05-focused-test-coverage.md
- .scratch/sandcastle-f5fbf91a/issues/06-repair-sv-core-test-registry-after-stage-b-narrowing.md
- .scratch/sandcastle-f5fbf91a/issues/07-focused-test-coverage.md
- .scratch/sandcastle-f5fbf91a/issues/08-reconcile-account-create-acceptance-check.md
- .scratch/sandcastle-f5fbf91a/issues/09-verify-recovered-account-create-gates.md
- .scratch/sandcastle-f5fbf91a/issues/10-focused-test-coverage.md
- .scratch/sandcastle-f5fbf91a/issues/11-return-sv-b021-to-account-scope.md
- .scratch/sandcastle-f5fbf91a/issues/12-record-controlled-peer-acceptance.md
- .scratch/sandcastle-f5fbf91a/issues/13-restore-wayfinder-effort-index.md
- .scratch/sandcastle-f5fbf91a/issues/14-focused-test-coverage.md
- .scratch/sandcastle-f5fbf91a/issues/15-index-latest-resolved-wayfinder-child.md
- .scratch/sandcastle-f5fbf91a/issues/16-canonicalize-contact-rejection-reasons.md
- .scratch/sandcastle-f5fbf91a/issues/17-unify-pregame-terminal-transitions.md
- .scratch/sandcastle-f5fbf91a/issues/18-focused-test-coverage.md

## Deferred checks (not completed)


## Automatic recovery decisions

- stop at test-repair: The failure is deterministic infrastructure drift, not an SV-B-021 defect. Commit dc8de1f8b intentionally removed the macro/options/profile/settings implementations and tests, while the target checkout’s .sandcastle/checks.sh still invokes them. The integrated branch at 43932c85a already contains d824ba90b, which corrects that manifest and preserves all successful workers, but the active controller reads .sandcastle/checks.sh from its target root rather than sandbox.worktreePath, so worker/assembly repairs cannot affect the gate being executed. Retrying unchanged will fail again; restoring the removed later-stage flows or adding pass-through tests would widen scope and bypass required gates. Resume this saved checkpoint only after the controller is repaired to stream the checks script from the saved integration HEAD (or the target runner is equivalently corrected), then rerun the complete sv-build and sv-core gates without resetting limits. Record: .scratch/sandcastle-f5fbf91a/recovery/1.md
- retry at test-repair: The prior deterministic gate failure has been corrected without changing the completed SV-B-021 workers: all five worker commits remain integrated, d824ba90b supplies the scoped checks manifest that omits intentionally removed macro/options/profile/settings flows, and controller fix 338de1039 is now merged into the integration branch at 46939e404. The controller now reads .sandcastle/checks.sh from sandbox.worktreePath, with a regression test proving it does not use the stale target-root script. Retry the saved checkpoint, preserving successful workers and existing limits, and rerun the complete sv-build and sv-core acceptance gates. Restoration condition is that those gates execute the integration-tree manifest and pass; any resulting product failure must return to normal focused repair rather than being deferred or skipped. Record: .scratch/sandcastle-f5fbf91a/recovery/2.md
- repair at test-repair: The failure is deterministic acceptance-manifest drift, not a transient execution error or missing external dependency. Integration HEAD 8a79ec746 defines P1-account-create as `python3 -B tests/test_sv_account_create.py`, but that path does not exist anywhere in Git history. The branch already contains production-path coverage in `tests/sv_account_create_checks.py` and `tests/sv_account_failure_checks.py`, both registered by the corrected core gate and independently recorded as passing. Send the existing test-repair planner focused guidance to reconcile P1-account-create with those real production suites, preserving success, rejection, cancellation, error, fragmentation, credential-policy, and parent-state obligations; do not add a dummy/pass-through test or weaken/defer the gate. Preserve every completed worker and route any repository change through a worker branch and assembly. Recovery is complete only when the accepted contract/state digest references a runnable repository test, then P1-account-create, the full sv-build gate, and the complete sv-core gate all pass at the assembled HEAD. Linux/Windows native and SV-B-025 external acceptance remain explicitly pending under their existing owners. Record: .scratch/sandcastle-f5fbf91a/recovery/3.md
- retry at review: The saved review failed only its response contract: an Acceptance finding was emitted without referencing the applicable acceptance-contract criterion. This is a retryable reviewer-output failure, not evidence of a code defect, invalid planning manifest, failed gate, or absent external dependency. Integration HEAD 787688888 already contains the completed repair workers and focused verification, while the accepted contract at that revision provides explicit P1-* and P2-AC* criterion IDs. Rerun the saved review checkpoint at the unchanged integration HEAD, preserving all successful workers and existing limits. Recovery succeeds when the reviewer returns a schema-valid response in which every Acceptance finding cites one or more concrete contract criterion IDs; required build, test, and acceptance obligations must remain unchanged, and the existing Linux/Windows native and SV-B-025 deferrals must not be widened. Record: .scratch/sandcastle-f5fbf91a/recovery/4.md

## Reviews

### Round 1

Reviewed HEAD: 20fad908990b5b0d8180264eac5352dfbe9a7d76
Summary: Blocked by three deduplicated findings: unrelated Sandcastle framework work broadened the feature diff, the ticket incorrectly defers current controlled-peer acceptance, and Wayfinder metadata violates repository rules. All three controller checks passed at HEAD 20fad908990b5b0d8180264eac5352dfbe9a7d76. Native Linux, native Windows, and SV-B-025 integration acceptance remain pending with their declared owners.

- high: .sandcastle/contract-command-repair.mjs:1 — The account-creation change set implements generic contract-amendment, command-repair, runner-validation, schema, workflow, documentation, and test infrastructure unrelated to SV-B-021. Generated repair tickets cannot broaden the authoritative account feature scope, and these shared-framework changes make the committed diff substantially larger than the focused SV implementation. Fix: Remove or split the generic Sandcastle framework commits into their own authorized effort. Retain only the minimal account implementation, focused tests, evidence, and check registration required by SV-B-021.
- medium: docs/tasks/stage-b/SV-B-021-account-manage.md:3 — The implementation record says controlled-peer acceptance is deferred and later invents a deferred P1-controlled-peer-runtime checkpoint, although immutable P1-10 is current and mandatory and the controlled loopback production-path suites pass at this HEAD. This weakens and misstates the authoritative acceptance contract. Fix: Record P1-10 as current and passed using the controlled-peer production-path evidence; remove the invented controlled-peer deferral. Keep only Linux native, Windows native, and SV-B-025 integration acceptance pending.
- medium: .scratch/sandcastle-f5fbf91a/issues/01-create-unused-account-through-production-flow.md:1 — The Wayfinder effort does not follow the repository's required planning layout: it has no effort map, and seven child tickets omit the required Type field. Fix: Add .scratch/sandcastle-f5fbf91a/map.md with the required decisions index and add Type metadata near the top of issues 01, 02, 03, 04, 06, 08, and 09 while preserving their existing status and history.

### Round 2

Reviewed HEAD: 787688888c96e7ff897b47f2f290886399a3ec02
Summary: Review at 787688888c96e7ff897b47f2f290886399a3ec02: 26 criteria passed, P2-AC12 failed, and three explicitly deferred criteria remain pending. Both prior scope/evidence defects are resolved. Closure is blocked by the reopened Wayfinder-ledger defect; two additional low maintainability findings do not currently change verified behavior.

- medium: .scratch/sandcastle-f5fbf91a/map.md:26 — The previously repaired Wayfinder ledger is incomplete again: resolved child ticket 14 was added after the repair, but the effort map's Decisions so far index still ends at ticket 13. Fix: Add ticket 14 to the map's Decisions so far section and keep subsequent resolved children indexed without changing their existing status or history.
- low: src/client/sv/endpoint-run.c:97 — The new contact_rejection_reason function repeats the full rejection-code switch already present in contact_status, creating two production mappings that can drift and expose different displayed and persisted reasons. Fix: Keep one canonical rejection-code-to-reason mapping and decorate that result with retry/exit guidance only at the presentation boundary.
- low: src/client/sv/session/pregame.c:131 — sv_pregame_disconnect and the newly added sv_pregame_fail duplicate generation validation, terminal-state guards, reason copying, and revision handling; only the destination phase differs. Fix: Use one private terminal-transition helper parameterized by the destination phase, while retaining the two semantic public entry points.

### Round 3

Reviewed HEAD: 95aa667afd69350ddd01c764f26672d2de14bfb2
Summary: Implementation-scope review passed at HEAD 95aa667afd69350ddd01c764f26672d2de14bfb2. Both Standards and Spec axes found no current defect. All 27 current criteria pass; P1-11, P1-12, and P1-13 remain explicitly pending with their owners. Five registered defects are concretely resolved at this HEAD. No accepted residuals remain.

- No findings

## Acceptance

Verdict: ready; scope: implementation
Contract SHA256: 8c107ecd47aeac23a221871fcb80ad5c9f87846ba44b10a1d97a00a0d2bd82c0
Contract: .scratch/sandcastle-f5fbf91a/acceptance-contract.json
Audited code HEAD: 95aa667afd69350ddd01c764f26672d2de14bfb2
Fresh final audits: 1/2

Coverage: 27 passed, 0 failed, 3 pending.

## Criteria and coverage

- P1-1: passed — A valid unused account name and password follow the server's new-account handling; server-side account flags, validation, and resulting overview remain authoritative.. Source: docs/tasks/stage-b/SV-B-021-account-manage.md. Evidence: The existing contact/login path sends the supplied unused identity to the server and publishes the server-decoded flags and overview through SvPregame. The controlled-peer success case reaches an empty authenticated overview with server-provided creation/server flags.
- P1-2: passed — Account creation uses the established native private credential fields.. Source: docs/tasks/stage-b/SV-B-021-account-manage.md. Evidence: src/client/sv/endpoint-run.c calls the established enter_credentials path, which uses SvCredentialInput and the existing private text-field implementation. The success test drives that production path with SDL text/key events.
- P1-3: passed — Account creation uses the production protocol path, and only a confirming server response marks creation successful and opens the overview.. Source: docs/tasks/stage-b/SV-B-021-account-manage.md. Evidence: src/client/sv/protocol/contact.c and login.c provide the production serialization/decoding path. SvPregame becomes authenticated/OVERVIEW only after SvLogin receives the complete terminated server overview; fragmented success testing confirms no earlier publication.
- P1-4: passed — The production serializer emits the exact new-account request bytes for every supported protocol-version branch.. Source: docs/tasks/stage-b/SV-B-021-account-manage.md. Evidence: tests/sv/contact-negotiation.c compares complete contact and verify output for the pre-2 and protocol-2 branches. src/client/sv/protocol/login.c emits the exact PKT_LOGIN/NUL request, adding the six-byte identity only at the 4.9.2.1.0.2 threshold; login tests exercise both branches and atomic output sizing.
- P1-5: passed — The account fields reuse the accepted input and private-field semantics without introducing a second implementation of those semantics.. Source: docs/tasks/stage-b/SV-B-021-account-manage.md. Evidence: No account-specific editor was introduced. Account and password entry continue through input/credentials.c, native-endpoint.c, and text-field.c, retaining the accepted live-trim, private-field, limit, cancellation, and star-rejection semantics.
- P1-6: passed — Used or invalid account names, bad passwords, account-flag rejection, disconnect, and retry are handled by the production account-creation flow.. Source: docs/tasks/stage-b/SV-B-021-account-manage.md. Evidence: The production endpoint tests cover invalid-name contact rejection, bad password, generic server/contact rejection, used-account login rejection, malformed input, disconnect, and fresh-generation retry. Rejection reasons are derived from server status or PKT_QUIT text.
- P1-7: passed — Rejection, disconnect, and retry preserve the correct parent state and never expose the overview before server confirmation.. Source: docs/tasks/stage-b/SV-B-021-account-manage.md. Evidence: Pregame terminal transitions preserve the first terminal parent and reject late/stale updates. Failure tests assert FAILED versus DISCONNECTED parents, no authentication/overview data before confirmation, and a fresh generation on retry; credential tests verify password Escape returns to account before final cancellation.
- P1-8: passed — After successful authentication, credentials are saved only through the approved credential-save policy.. Source: docs/tasks/stage-b/SV-B-021-account-manage.md. Evidence: endpoint-run.c starts sv_vault_store only when state is READY and pregame.authenticated is true, using the approved server/port/account vault key and exact password bytes. The success peer verifies storage begins only after the final confirming overview byte.
- P1-9: passed — Credential-provider failure remains an explicit session-only state.. Source: docs/tasks/stage-b/SV-B-021-account-manage.md. Evidence: SvCredentialSaveState exposes SESSION_ONLY explicitly. Unavailable, locked, refused, invalid, error, start failure, and cancelled pending-save paths do not report saved state and retain an authenticated session-only outcome.
- P1-10: passed — A controlled-peer production-path test covers success, rejection, cancellation, error handling, and fragmented protocol input.. Source: docs/tasks/stage-b/SV-B-021-account-manage.md. Evidence: Controller evidence for P1-account-create passed at the audited head and contract digest. The two production endpoint suites cover controlled-peer success, contact/login rejection, credential cancellation, malformed/error input, disconnect, retry, and fragmented contact/login responses.
- P1-11: pending — The Linux native-input and credential-provider paths are verified in their native environment.. Source: docs/tasks/stage-b/SV-B-021-account-manage.md. Evidence: Explicitly deferred to SV-B-021. The supplied evidence is dummy/software headless execution, not Linux native-input and real credential-provider runtime acceptance.; pending at SV-B-021: Requires Linux native input and credential-provider runtime evidence that is unavailable at the implementation gate.
- P1-12: pending — The Windows native-input and credential-provider paths are verified in their native environment.. Source: docs/tasks/stage-b/SV-B-021-account-manage.md. Evidence: Explicitly deferred to SV-B-021. No Windows native-input or Credential Manager runtime evidence was supplied or inferred from Linux/headless checks.; pending at SV-B-021: Requires Windows native input and credential-provider runtime evidence that is unavailable at the implementation gate.
- P1-13: pending — The completed account-creation result is incorporated into SV-B-025 entry-completion acceptance.. Source: docs/tasks/stage-b/SV-B-021-account-manage.md. Evidence: Explicitly deferred to SV-B-025. This implementation stops at the account overview and does not claim downstream entry-completion acceptance.; pending at SV-B-025: This is downstream integration evidence owned by the later SV-B-025 entry-completion task.
- P1-14: passed — Account creation retains its declared dependency on SV-B-002 contact behavior.. Source: docs/tasks/stage-b/SV-B-021-account-manage.md. Evidence: The implementation retains the SV-B-002 production contact/socket/negotiation path; contact checks and the account controlled peer passed at the audited head.
- P1-15: passed — Account creation retains its declared dependencies on SV-B-005 vault behavior and SV-B-006 login behavior.. Source: docs/tasks/stage-b/SV-B-021-account-manage.md. Evidence: The flow continues to use the SV-B-005 vault API and SV-B-006 login serializer, interaction, decoder, view, and pregame model. Vault/login/core and focused account checks passed.
- P1-16: passed — Password changes and account-information functionality remain outside this implementation and deferred to stage C.. Source: docs/tasks/stage-b/SV-B-021-account-manage.md. Evidence: The production diff adds neither password-change nor account-information commands, packets, surfaces, or state.
- P2-AC1: passed — Changes to the existing client and shared files are minimized for the SV implementation.. Source: AGENTS.md. Evidence: Production edits are limited to five existing files under src/client/sv; no legacy client, common, or server production file changed.
- P2-AC2: passed — Implementation and adaptation are placed within SV wherever possible.. Source: AGENTS.md. Evidence: All implementation and adaptation changes reside under src/client/sv, with tests and documentation outside production source.
- P2-AC3: passed — Implementation may be duplicated when doing so preserves SV isolation and avoids importing a large dependency set.. Source: AGENTS.md. Evidence: The change retains isolated SV-owned protocol, pregame, endpoint, and credential interfaces without importing a legacy dependency graph.
- P2-AC4: passed — Shared code is extracted only when required by the current task, not solely to eliminate duplication.. Source: AGENTS.md. Evidence: No shared production module was extracted or modified merely to remove duplication.
- P2-AC5: passed — The port preserves the protocol and gameplay semantics of the behavior baseline.. Source: AGENTS.md. Evidence: Legacy contact, verify, initial-login, server-confirmed overview, rejection, and retry semantics were compared with the baseline. Exact branch tests and controlled peers exercise the corresponding SV serializers and decoders.
- P2-AC6: passed — Tests exercise the production SV path; a local SV implementation is permitted, but a separate behavior implementation used only by tests is not.. Source: AGENTS.md. Evidence: The focused suites invoke public sv_endpoint_run and compile the production endpoint, credential-input, contact/socket, login, pregame, view, and rendering sources. Provider doubles inject dependency outcomes while production vault behavior remains covered by the passing sv_vault checks; no separate account-flow implementation exists only in tests.
- P2-AC7: passed — Any opportunities discovered to improve legacy code, shared modules, or SV organization are recorded in docs/sv-improvements.md separately from implementation of the current feature.. Source: AGENTS.md. Evidence: Newly observed resolver-shutdown and common atol cleanup opportunities were recorded separately in docs/sv-improvements.md rather than implemented in this feature.
- P2-AC8: passed — Each recorded improvement identifies the problem and affected code.. Source: AGENTS.md. Evidence: Each newly added improvement entry states its problem and identifies the affected code.
- P2-AC9: passed — Each recorded improvement includes a proposal, status, and required checks; recording it does not add that improvement to the current task.. Source: AGENTS.md. Evidence: Each newly added improvement entry includes a proposal, separate/proposed status, and required checks, and explicitly keeps the work outside SV-B-021.
- P2-AC10: passed — Intentional duplication used to preserve isolation is not treated by itself as a defect or as an obligation to consolidate later.. Source: AGENTS.md. Evidence: SV-local implementation was assessed under the isolation exception; no finding was raised solely because behavior remains locally implemented.
- P2-AC11: passed — The repository SV isolation rule takes precedence over earlier project-document wording that requires a shared SV and legacy implementation.. Source: AGENTS.md. Evidence: The result follows the SV isolation rule: no legacy/shared production refactor was introduced despite comparable baseline behavior.
- P2-AC12: passed — If tasks are created, triaged, or reviewed during this work, local Markdown is used: Wayfinder items are kept in .scratch/, SV implementation tickets are kept in docs/tasks/, and the documented path and status rules are followed.. Source: AGENTS.md. Evidence: Wayfinder work is stored under .scratch/sandcastle-f5fbf91a with child Type/Status/Assignee metadata and an indexed map; the existing SV implementation ticket remains under docs/tasks/stage-b.
- P2-AC13: passed — If triage is performed during this work, it uses the five standard roles and a separate Labels field according to the documented role mapping.. Source: AGENTS.md. Evidence: Where triage metadata is present, category labels and the documented ready-for-agent role appear on a separate Labels field rather than replacing Wayfinder Status.
- P2-AC14: passed — Before working with domain terminology or architecture, the root CONTEXT.md, task-relevant ADRs, and the single-context layout rules are read and followed.. Source: AGENTS.md. Evidence: The implementation follows the repository glossary and relevant ADR boundaries: server-authoritative protocol decoding, explicit pregame state, generation-scoped terminal handling, production input separation, and SV-local module placement. No ADR conflict was found.

## Controller checks

- sv-build: passed; /home/svechnik/Projects/tomenet_modern_client/.sandcastle/runs/f5fbf91a/check-sv-build-11-5.json
- sv-core: passed; /home/svechnik/Projects/tomenet_modern_client/.sandcastle/runs/f5fbf91a/check-sv-core-11-6.json
- P1-account-create: passed; /home/svechnik/Projects/tomenet_modern_client/.sandcastle/runs/f5fbf91a/check-P1-account-create-11-6.json
- P1-linux-native: not run; Linux native-input and credential-provider runtime verification
- P1-windows-native: not run; Windows native-input and credential-provider runtime verification
- P1-entry-integration: not run; SV-B-025 downstream entry-completion integration evidence

## Residual backlog (open, not fixed)


## Defect history

- D-801cbe32143a46db: resolved; integrated repair cycles: 1/3; The account-creation change set implements generic contract-amendment, command-repair, runner-validation, schema, workflow, documentation, and test infrastructure unrelated to SV-B-021. Generated repair tickets cannot broaden the authoritative account feature scope, and these shared-framework changes make the committed diff substantially larger than the focused SV implementation.
- D-345b4357ca451a4d: resolved; integrated repair cycles: 1/3; The implementation record says controlled-peer acceptance is deferred and later invents a deferred P1-controlled-peer-runtime checkpoint, although immutable P1-10 is current and mandatory and the controlled loopback production-path suites pass at this HEAD. This weakens and misstates the authoritative acceptance contract.
- D-d6785374d0ab1f4c: resolved; integrated repair cycles: 2/3; The previously repaired Wayfinder ledger is incomplete again: resolved child ticket 14 was added after the repair, but the effort map's Decisions so far index still ends at ticket 13.
- D-72dcc7c122d0a059: resolved; integrated repair cycles: 1/3; The new contact_rejection_reason function repeats the full rejection-code switch already present in contact_status, creating two production mappings that can drift and expose different displayed and persisted reasons.
- D-49dc8d0dd64aef31: resolved; integrated repair cycles: 1/3; sv_pregame_disconnect and the newly added sv_pregame_fail duplicate generation validation, terminal-state guards, reason copying, and revision handling; only the destination phase differs.

## Preserved behavior

- Legacy client, common, and server production files remain unchanged.
- The server remains authoritative for account creation, validation, creation flags, server flags, and overview contents.
- Interactive account names retain the established 15-byte live-trim behavior.
- Passwords retain private-field handling, exact bytes, no history, and protocol-2 star rejection.
- No overview or credential save is exposed before a complete confirming server response.
- Rejection and disconnect preserve distinct terminal parents; retry starts a fresh generation and rejects stale results.
- Credential-provider failure remains an explicit unsaved session-only state without legacy plaintext fallback.
- Password changes and account-information functionality remain outside this implementation.


Headless gates do not establish full ticket acceptance. Later caller integrations, display-backed human review and platform checks remain pending unless separately evidenced.
