# 01: Полный creation/play handshake и ожидание acknowledgement

**What to build:** После завершения ordinary birth на текущей baseline версии игрок отправляет полный handshake и остаётся в startup screen flow до authoritative play acknowledgement. Это первый сквозной срез: native confirm → legal committed birth draft → production SV serializer → socket → controlled peer → acknowledgement → состояние ожидания следующего startup owner.

**Blocked by:** None (can start immediately).

Status: resolved
Assignee: codex/sandcastle-0241e0da-wave-1-1

Labels: enhancement, ready-for-agent

**Finding IDs:** None — initial decomposition; ledger repair findings не предоставлены.

**Origin:** SV-B-024, capability.birth.complete; immutable contract `0ba2cedc9474a7f17f847c0bdb89d04023b0a9edefa8611f7528564eeb4f83ef`.

**Production seam:** Public SV production seam для TDD: `sv_endpoint_run(SvEndpointOptions)` с controlled peer и native SDL input. Public `sv_birth_command` через birth interaction используется только для дополнительных state checks; он не заменяет endpoint evidence final send. Связь: P1-ac1/P1-ac3/P2-4 → sv-core и controller-review.

Существующий endpoint seam доступен; прямого публичного final-play serializer API пока нет. Реализовать требуемый production path внутри SV и вызывать его через endpoint; не придумывать test-only интерфейс. Если endpoint не позволяет проверить обязательный критерий, зафиксировать seam/evidence blocker, а не заменять тестируемое поведение.

## Acceptance criteria

- [x] **P1-ac1, P1-ac4:** native production flow, использующий SV-B-023 choices, отправляет final request только после всех legal steps; незавершённый draft не сериализуется. Successful name/login status не принимается за play acknowledgement.
- [x] **P1-ac2, P1-ac3:** controlled peer сравнивает весь handshake текущей baseline версии с независимо выведенными baseline bytes: итоговые identity/mode/body/class/race/trait, шесть committed stats, production option defaults, geometry, audio/graphics flags, font/resource strings и все требуемые visual identity/redefinition tables. Нулевые или сокращённые fixtures не заменяют production defaults.
- [x] **P1-ac1, P1-ac5:** задержанный ответ сохраняет ожидание и обслуживание UI/network; только полный authoritative successful play reply завершает ожидание. Gameplay surface не открывается.
- [x] **P1-ac3, P1-ac8, P2-4:** failing controlled-peer test становится passing через тот же serializer, который вызывается native executable; сохранить захваченные bytes и ack как evidence этого среза, не объявляя full acceptance.
- [x] **P1-ac9:** сохранить зависимость от SV-B-023 и историческое имя originating ticket; его pending native/full acceptance не объявлять завершённым.

## Границы и проверки

Каждый implementation срез сохраняет protocol/game semantics behavior baseline и SV isolation (P2-1–P2-4, P2-7), проходит sv-build и настроенный sv-core; controller-review и fresh final audit отдельно устанавливают acceptance evidence. Начинать с failing проверки public production seam, затем реализовать production поведение.

SV-B-023 — сохранённая upstream dependency, уже присутствующий production foundation, но его full acceptance/native evidence pending; он не включён в manifest как выдуманный completed dependency. При обнаружении реального отсутствующего prerequisite остановить зависимую реализацию и записать blocker.

P1-ac9–P1-ac11: DNA persistence, profile/resource workflows, real-server integration, session.enter-game и gameplay presentation не входят в этот срез. Требуемые handshake resource bytes входят. P1-ac12 / P1-entry-evidence deferred в SV-B-025, не passed. Обнаруженные улучшения записывать отдельно по P2-5/P2-6; намеренная изоляционная duplication сама по себе не finding.

Публикация тикета не означает, что implementation, runtime checks или controller audit уже выполнены. Работа по frontier начинается с 01; blocked children доступны после завершения перечисленных prerequisites.

## Comments

2026-10-09: Claimed for implementation on the isolated assigned branch. Agreed TDD seam: `sv_endpoint_run(SvEndpointOptions)` with controlled TCP peer and native SDL input; final-send evidence must cross this production boundary.

## Answer

Implemented the ordinary current-baseline (4.9.4.0.0.0) creation/play slice on
`codex/sandcastle-0241e0da-wave-1-1`. History: initially `ready-for-agent`, claimed
2026-10-09, resolved after focused red/green verification. No other branch was
edited or merged.

**Agreed production seam:** `sv_endpoint_run(SvEndpointOptions)` with native SDL
input and a controlled TCP peer. This is the same endpoint called by the native
executable. `sv_birth_committed_play` publishes only a current, active, legally
committed READY draft through the birth interaction boundary; the endpoint queues
it once through production `sv_login_start_play`, then services UI/network while
awaiting the complete authoritative `PKT_REPLY, PKT_PLAY, PKT_SUCCESS` reply.
Successful name status does not acknowledge play. Partial replies remain pending;
a complete successful reply reaches `SV_PREGAME_PLAY_ACKNOWLEDGED`, waiting for
its next startup owner. No gameplay surface opens.

**TDD red:** `python3 tests/sv_birth_complete_checks.py` failed before production
implementation at `poll(&final_ready, 1, 800) == 1`: legal native birth completion
produced no final request. Initial fixture setup was corrected before recording
this behavioral red; those setup failures are not claimed as the TDD red.

**TDD green:** the same command passed after implementation, with ASan/UBSan and
`-Wall -Wextra -Werror`, for delayed/no acknowledgement, split incomplete reply,
and completed successful reply. Every case compares all 16,266 captured bytes
with an independent baseline-derived literal oracle. It checks no early request
at name acceptance or committed-stats-only, continued ping service, UI resize/
focus events, duplicate confirmations, selected identity and committed stats,
and the final public endpoint phase. Only the complete reply advances it.

**Focused regression:**
`SV_BIRTH_CASE=stats-order-confirm,mode-ordinary-ready python3 tests/sv_name_editor_submit_checks.py`
passed for the existing default and alternate class/race order builds. These
focused upstream checks do not complete SV-B-023 native/full acceptance. Their
existing SDL worker teardown diagnostic is recorded separately in
`docs/sv-improvements.md`; legacy/shared code and those fixtures were not changed.

**Evidence:** [captures and provenance](../evidence/01/README.md), including full
request bytes, actual acknowledgement bytes and endpoint phase traces. Reproduce:
`SV_BIRTH_EVIDENCE=.scratch/sandcastle-0241e0da/evidence/01 python3 tests/sv_birth_complete_checks.py`.
Normal checks use a temporary evidence directory. The focused check is registered
in `.sandcastle/checks.sh` for the orchestrator's `sv-core` gate.

The production defaults include all 199 options and all four complete visual
redefinition tables. With the effective shipped Cascadia text font, zero visual
entries are the independently derived baseline server-identity sentinel; they
are not omitted tables or a zero fixture replacing production defaults. Audio/
graphics flags report the native executable's disabled outcome; geometry is the
66x22 map viewport, independent of UI window pixels. Resource bytes are included.
Implementation and adaptation are entirely SV-local; no legacy/common edits.

**Scope/remaining owners:** checkmarks above mean this ticket's implementation
slice, not full immutable-contract acceptance. The dependency on
[SV-B-023](../../../docs/tasks/stage-b/SV-B-023-birth-choices.md) remains, with its
native/full acceptance pending. The originating historical filename
`docs/tasks/stage-b/SV-B-024-birth-dna.md` is preserved. Older protocol gates remain
owned by 02 (the current serializer explicitly rejects older layouts); dedicated
completion by 03; extended submission lifecycle by 04; rejection/disconnect/
malformed/stale reply ownership by 05; Guide/MOTD/live handoff by 06; runtime audit
by 07/controller. DNA persistence, profile/resource workflows, real-server
integration, `session.enter-game` and gameplay remain stage C. P1-ac12 /
P1-entry-evidence remains deferred to SV-B-025, not passed.

Per the requested division of work, `sv-build`, the complete configured `sv-core`,
controller-review and fresh final audit were not run and are not claimed passed;
the orchestrator runs those gates after all tickets.
