# 04: Одна final отправка при повторном вводе и interleaving

**What to build:** Игрок подтверждает завершённый birth один раз; последующие confirm, resize, focus и network events не создают второго handshake и не позволяют отправить неполный или устаревший draft.

**Blocked by:** 01: Полный creation/play handshake и ожидание acknowledgement.

Status: resolved
Assignee: codex/sandcastle-0241e0da-wave-2-3

Labels: enhancement, ready-for-agent

**Finding IDs:** None — initial decomposition; ledger repair findings не предоставлены.

**Origin:** SV-B-024, capability.birth.complete; immutable contract `0ba2cedc9474a7f17f847c0bdb89d04023b0a9edefa8611f7528564eeb4f83ef`.

**Production seam:** Public SV production seam для TDD: `sv_endpoint_run(SvEndpointOptions)` с controlled peer и native SDL input. Public `sv_birth_command` через birth interaction используется только для дополнительных state checks; он не заменяет endpoint evidence final send. Связь: P1-ac1/P1-ac3/P2-4 → sv-core и controller-review.

Существующий endpoint seam доступен; прямого публичного final-play serializer API пока нет. Реализовать требуемый production path внутри SV и вызывать его через endpoint; не придумывать test-only интерфейс. Если endpoint не позволяет проверить обязательный критерий, зафиксировать seam/evidence blocker, а не заменять тестируемое поведение.

## Acceptance criteria

- [x] **P1-ac1, P1-ac4:** controlled peer видит ровно один полный final request при double confirm, held/repeated confirm, resize, focus changes и interleaved разрешённых packets до и во время delayed acknowledgement; отличать final request от keepalive/pong traffic.
- [x] **P1-ac4:** на каждом незавершённом legal birth owner, после backtracking и при cancel/quit controlled peer не получает final request; проверять actual native input path, а не напрямую вызывать serializer.
- [x] **P1-ac5, P2-4:** после закрытия поколения поздний input не отправляет final bytes; новое поколение может завершить собственный birth один раз. Endpoint lifecycle и public birth interaction остаются владельцами состояния, UI rendering не инициирует повторный send.

## Границы и проверки

Каждый implementation срез сохраняет protocol/game semantics behavior baseline и SV isolation (P2-1–P2-4, P2-7), проходит sv-build и настроенный sv-core; controller-review и fresh final audit отдельно устанавливают acceptance evidence. Начинать с failing проверки public production seam, затем реализовать production поведение.

SV-B-023 — сохранённая upstream dependency, уже присутствующий production foundation, но его full acceptance/native evidence pending; он не включён в manifest как выдуманный completed dependency. При обнаружении реального отсутствующего prerequisite остановить зависимую реализацию и записать blocker.

P1-ac9–P1-ac11: DNA persistence, profile/resource workflows, real-server integration, session.enter-game и gameplay presentation не входят в этот срез. Требуемые handshake resource bytes входят. P1-ac12 / P1-entry-evidence deferred в SV-B-025, не passed. Обнаруженные улучшения записывать отдельно по P2-5/P2-6; намеренная изоляционная duplication сама по себе не finding.

Публикация тикета не означает, что implementation, runtime checks или controller audit уже выполнены. Работа по frontier начинается с 01; blocked children доступны после завершения перечисленных prerequisites.

## Comments

2026-10-10: Claimed on the assigned isolated branch. Prerequisite 01 is resolved and its production endpoint/serializer is present. Agreed seam from the ticket/contract: `sv_endpoint_run(SvEndpointOptions)` with controlled peer and native SDL input; public birth commands are supplementary state checks only. Full build/core/review gates remain orchestrator-owned.

## Answer

Implemented on `codex/sandcastle-0241e0da-wave-2-3`. History: ready-for-agent →
claimed 2026-10-10 → resolved after focused verification. Prerequisite 01 is
present/resolved; no other branch was edited or merged.

**Production seam:** the agreed `sv_endpoint_run(SvEndpointOptions)` with a
controlled TCP peer and native SDL input. All final-send observations cross the
production socket. No serializer-only test, test-only behavior implementation,
or additional production callback was introduced.

**Behavior change / TDD vertical slice:** endpoint teardown now closes its birth
interaction through public `sv_birth_close`, including native quit/window close,
pending acknowledgement, successful acknowledgement and retries. The public
outcome retains the last interaction owner for existing diagnostic checks while
reporting actual closed/inactive lifecycle flags. Existing production committed-
draft validation, login-state send guard and stale-input timestamp filtering
already provide single submission; they remain the production owners of those
rules. UI rendering does not submit the draft.

Red: `python3 tests/sv_birth_single_submission_checks.py` failed at
`outcome.birth_closed && !outcome.birth_active` after a complete 16,266-byte
request and native quit in the waiting case. The initial include/main-macro
compilation error was fixed before this behavioral red and is not TDD evidence.
Green: the same command passed waiting, partial and successful acknowledgement
cases after the minimal endpoint teardown change.

**Additional coverage of existing behavior (not claimed as separate TDD loops):**

- `python3 tests/sv_birth_single_submission_checks.py` passed all 18 native cases:
  waiting/partial/ack/retry and quit/cancel at sex, class, race, trait, body, stats,
  and mode. Quit cases observe each unfinished owner; cancel cases backtrack
  before native window close. Queued late confirmations produce no final bytes.
- Successful completion backtracks from mode, recommits stats through native
  input, then compares exactly one full request against the independent baseline
  fixture. Double confirm, 12 held/repeated confirmations, stale timestamped
  input, resize/focus changes and ping packets cannot create a duplicate. It also
  checks no early request before mode confirmation.
- The retry case disconnects the first generation at unfinished mode, injects
  late confirmations while it is closed, retries through native R, rejects stale
  input at the next birth and completes that generation once. Public outcome
  confirms distinct nonzero current/prior generations.
- After extending the acknowledgement delay beyond the production two-second
  keepalive interval,
  `SV_SUBMISSION_CASE=waiting,partial,ack,retry SV_BIRTH_EVIDENCE=.scratch/sandcastle-0241e0da/evidence/04 python3 tests/sv_birth_single_submission_checks.py`
  passed all four affected cases. Each explicitly consumes a one-byte keepalive
  separately from the 16,266-byte final request and exact ping/pong traffic.
  Peer keepalives also interleave before final submission and during pending ack.
  Partial acknowledgement remains pending; only the full reply advances phase.
- `python3 tests/sv_birth_complete_checks.py` passed the prerequisite 01's three
  production handshake cases.
- `SV_BIRTH_CASE=stats-order-confirm,mode-ordinary-ready python3 tests/sv_name_editor_submit_checks.py`
  passed the four existing cases across default/alternate class/race order builds.

The focused runner compiles production SV with ASan/UBSan and
`-Wall -Wextra -Werror`. It is registered in `.sandcastle/checks.sh` for the
orchestrator's configured core gate. [Evidence/provenance](../evidence/04/README.md)
contains full captures, authoritative reply fragments and native endpoint traces.
Normal invocations keep evidence in a temporary directory.

Implementation is SV-local (nine added endpoint lines); legacy/common code is
unchanged. The upstream SDL worker `Leaked thread` diagnostic is already recorded
in `docs/sv-improvements.md`, “Keep controlled-peer workers outside endpoint SDL
teardown”; that separate improvement was not implemented here. No additional
legacy/shared/SV organization improvement was discovered.

Checkmarks denote this implementation slice's focused evidence, not full contract
acceptance. `sv-build`, the complete configured `sv-core`, controller-review and
fresh final audit were not run or claimed passed; the orchestrator owns them.
Older version gates, dedicated outcomes, rejection/malformed reply ownership and
Guide/MOTD/live handoff remain with their originating tickets. SV-B-023 native/
full acceptance remains pending. DNA/profile/resource workflows, real-server
integration, session.enter-game and gameplay presentation remain outside this
slice. P1-ac12 / P1-entry-evidence remains deferred to SV-B-025, not passed.
