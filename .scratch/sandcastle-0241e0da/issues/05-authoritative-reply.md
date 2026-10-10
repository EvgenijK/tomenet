# 05: Reply failures и stale generations возвращают правильный owner

**What to build:** Во время ожидания play игрок получает authoritative результат: success, rejection или disconnect возвращают управление соответствующему startup/lifecycle owner; partial reply ждёт продолжения, malformed и устаревший reply не открывают следующий surface.

**Blocked by:** 01: Полный creation/play handshake и ожидание acknowledgement.

Status: resolved
Assignee: codex/sandcastle-0241e0da-wave-3-1

Labels: enhancement, ready-for-agent

**Finding IDs:** None — initial decomposition; ledger repair findings не предоставлены.

**Origin:** SV-B-024, capability.birth.complete; immutable contract `0ba2cedc9474a7f17f847c0bdb89d04023b0a9edefa8611f7528564eeb4f83ef`.

**Production seam:** Public SV production seam для TDD: `sv_endpoint_run(SvEndpointOptions)` с controlled peer и native SDL input. Public `sv_birth_command` через birth interaction используется только для дополнительных state checks; он не заменяет endpoint evidence final send. Связь: P1-ac1/P1-ac3/P2-4 → sv-core и controller-review.

Существующий endpoint seam доступен; прямого публичного final-play serializer API пока нет. Реализовать требуемый production path внутри SV и вызывать его через endpoint; не придумывать test-only интерфейс. Если endpoint не позволяет проверить обязательный критерий, зафиксировать seam/evidence blocker, а не заменять тестируемое поведение.

## Acceptance criteria

- [x] **P1-ac5, P2-3:** independently derived baseline reply fixtures проверяют successful play acknowledgement, rejected play result, peer quit/disconnect и wrong packet/reply type; сообщения, retry/exit policy и destination owner соответствуют baseline. Character/name acceptance не подменяет play reply.
- [x] **P1-ac5:** split reply на каждой границе не меняет owner до получения полного пакета; malformed/truncated-at-disconnect reply обрабатывается корректным failure/disconnect owner без ложного success.
- [x] **P1-ac5, P2-4:** reply и input старого поколения после закрытия/retry не мутируют новую сессию, не открывают MOTD/handoff и не отправляют новый final request; fresh-generation successful reply работает.
- [x] **P1-ac5, P2-3:** interleaved разрешённые packets и trailing bytes после ack обрабатываются в baseline порядке через production decoder/lifecycle, без потери остатка и блокирующего чтения UI loop.

## Границы и проверки

Каждый implementation срез сохраняет protocol/game semantics behavior baseline и SV isolation (P2-1–P2-4, P2-7), проходит sv-build и настроенный sv-core; controller-review и fresh final audit отдельно устанавливают acceptance evidence. Начинать с failing проверки public production seam, затем реализовать production поведение.

SV-B-023 — сохранённая upstream dependency, уже присутствующий production foundation, но его full acceptance/native evidence pending; он не включён в manifest как выдуманный completed dependency. При обнаружении реального отсутствующего prerequisite остановить зависимую реализацию и записать blocker.

P1-ac9–P1-ac11: DNA persistence, profile/resource workflows, real-server integration, session.enter-game и gameplay presentation не входят в этот срез. Требуемые handshake resource bytes входят. P1-ac12 / P1-entry-evidence deferred в SV-B-025, не passed. Обнаруженные улучшения записывать отдельно по P2-5/P2-6; намеренная изоляционная duplication сама по себе не finding.

Публикация тикета не означает, что implementation, runtime checks или controller audit уже выполнены. Работа по frontier начинается с 01; blocked children доступны после завершения перечисленных prerequisites.

## Comments

2026-10-10: Claimed in the assigned isolated branch. Prerequisite 01 is resolved and its production handshake is present. Agreed seam: `sv_endpoint_run(SvEndpointOptions)` with controlled TCP peer and native SDL input.

## Answer

Implemented on `codex/sandcastle-0241e0da-wave-3-1`. History: initial
`ready-for-agent`, claimed 2026-10-10 after verifying resolved prerequisite 01,
then resolved after focused verification. No other branch was edited or merged.
Contract digest remains
`0ba2cedc9474a7f17f847c0bdb89d04023b0a9edefa8611f7528564eeb4f83ef`.

**Agreed production seam:** `sv_endpoint_run(SvEndpointOptions)` with a real
controlled TCP peer and native SDL gestures. Tests assert the public endpoint
outcome and actual socket bytes. They compare the entire 16,266-byte final
request with 01's independently derived baseline oracle; no test-only serializer,
decoder, lifecycle implementation or callback was introduced.

**Behavior and ownership:** full successful `PKT_REPLY, PKT_PLAY, PKT_SUCCESS`
returns to the acknowledged startup owner; either incomplete prefix stays in
play-pending. Rejection and malformed packets close the session through the
failure owner; peer quit and EOF use the disconnect owner. Peer-quit reasons
remain intact. Play rejection cannot start name recovery or retry authentication.
As in baseline `Net_start`, play failure has an exit policy, while pre-play name
rejection retains the existing retry policy. The SV lifecycle leaves the failure
visible and the application running until Escape/window close. Tests press R and
confirm that no replacement connection is opened after play failure.

The baseline user-facing rejection and wrong-reply reasons are retained. Wrong
packet diagnostics retain the packet type but omit legacy buffer-pointer/length
diagnostics, which have no stable meaning across incremental reads. EOF during
play reports `Error reading play reply`. Unterminated overlong quit packets use
the protocol failure reason rather than acknowledging play. Terminal decoders
do not accept a later successful reply.

Each retry creates its own transport, decoder and wire buffer. The endpoint
rejects native session gestures timestamped before that generation, including
stale text and confirm in the restored name editor. The retry fixture sends an
old successful reply both behind the rejected name response and to the retired
socket after the replacement has reached overview, injects stale SDL text/key
input, proves no request appears, then completes a fresh native birth and
successful acknowledgement. The public outcome proves distinct generations,
current-generation birth ownership and unchanged Carol identity. No MOTD or
handoff opens from stale input/replies.

Interleaved keepalive, SFLAGS and ping packets retain decoder order. A single
peer write contains packets before and after acknowledgement; both distinct
pongs arrive in order, trailing server flags are published, and there is no
duplicate final request. UI resize/focus remains serviced during partial waits.

**TDD red → green** (each command ran failing before its minimal production
change and passing after it):

- `python3 tests/sv_birth_reply_checks.py` (initial rejected-only test): red at
  `!outcome.name_rejected && !outcome.name_recovery_started`; green after
  separating final-play rejection from name recovery and preserving exit policy.
- `SV_REPLY_CASE=peer-quit python3 tests/sv_birth_reply_checks.py`: red at expected
  disconnect phase; green after the production peer-quit lifecycle result.
- `SV_REPLY_CASE=wrong-type python3 tests/sv_birth_reply_checks.py`: red at
  failure destination; green after malformed-play failure ownership and reason.
- `SV_REPLY_CASE=wrong-packet python3 tests/sv_birth_reply_checks.py`: red at
  baseline reason; green after preserving the wrong-packet play diagnostic.
- `SV_REPLY_CASE=disconnect python3 tests/sv_birth_reply_checks.py`: red at
  EOF reason; green after reporting the play-read failure with exit policy.
- `SV_REPLY_CASE=ack-trailing python3 tests/sv_birth_reply_checks.py`: behavioral
  red at final `server_flags` comparison; green after publishing decoded flags
  while pending/acknowledged. A wrong keepalive literal in initial fixture setup
  was corrected first; that setup failure is not claimed as behavioral red.
- `SV_REPLY_CASE=ack-retry python3 tests/sv_birth_reply_checks.py`: red at
  no-request observation after stale input; green after the generation guard.

**Focused verification:** final `python3 tests/sv_birth_reply_checks.py` passed
all 17 scenarios under ASan/UBSan and `-Wall -Wextra -Werror`: rejection, peer
quit, EOF, wrong reply type, wrong packet, success, success split at each of
two boundaries, either pending prefix, EOF at either prefix, interleaved/trailing
packets, retired-generation retry, rejected reply followed by success, malformed
quit, and successful ack followed by peer quit. Partial/truncated/terminal
coverage of already implemented behavior is coverage, not a claimed test-first
loop. After the final compatibility-only adjustment retaining the preexisting
`SV_INVALID` result for an already rejected login, the targeted
`SV_REPLY_CASE=rejected,peer-quit,ack-retry python3 tests/sv_birth_reply_checks.py`
passed again.

Focused regressions passed:

- `python3 tests/sv_login_checks.py`
- `SV_SUBMISSION_CASE=waiting,partial,ack,retry python3 tests/sv_birth_single_submission_checks.py`
- `SV_BIRTH_CASE=session-retry-generation python3 tests/sv_name_editor_submit_checks.py`

The last existing fixture prints the previously recorded SDL worker `Leaked
thread` diagnostic; its assertions pass. Its improvement remains separately
owned in `docs/sv-improvements.md`. A newly noticed legacy short-read robustness
opportunity is recorded there separately, with problem, affected code, proposal,
status and required checks. No legacy/shared protocol or client edits.

**Reproduction/evidence:** `tests/sv/birth-authoritative-reply.c` documents the
literal reply fixtures, independently derived from
`src/client/nclient.c:Net_start` and `Receive_reply` (packet IDs from
`src/common/pack.h`), not generated by the SV decoder.
`SV_BIRTH_EVIDENCE=/tmp/sv-reply-evidence python3 tests/sv_birth_reply_checks.py`
preserves each actual request capture and public phase trace. The normal runner
uses a temporary directory. The focused runner is registered in
`.sandcastle/checks.sh` for the orchestrator's core gate.

Checkmarks describe this implementation slice, not controller-owned acceptance.
Guide/MOTD/live handoff remain owned by 06; runtime evidence/fresh final audit
by 07/controller. SV-B-023 native/full acceptance remains pending; its existing
production dependency and the historical SV-B-024 ticket filename are preserved.
DNA persistence, profile/resource workflows, real-server integration,
`session.enter-game` and gameplay remain outside this slice. P1-ac12 /
P1-entry-evidence remains deferred to SV-B-025, not passed.

Per the requested division of work, the full `sv-build`, configured `sv-core`,
controller-review and fresh final audit were not run or claimed passed. The
orchestrator owns these gates after all tickets.
