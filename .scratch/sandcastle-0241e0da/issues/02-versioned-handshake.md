# 02: Handshake на границах baseline protocol versions

**What to build:** Игрок завершает ordinary creation с теми же значениями, а controlled peer каждой применимой baseline версии получает ровно тот формат полного handshake, который эта версия поддерживает, и подтверждает play.

**Blocked by:** 01: Полный creation/play handshake и ожидание acknowledgement.

Type: implementation
Status: resolved
Assignee: codex/sandcastle-0241e0da-wave-2-1

Labels: enhancement, ready-for-agent

**Finding IDs:** None — initial decomposition; ledger repair findings не предоставлены.

**Origin:** SV-B-024, capability.birth.complete; immutable contract `0ba2cedc9474a7f17f847c0bdb89d04023b0a9edefa8611f7528564eeb4f83ef`.

**Production seam:** Public SV production seam для TDD: `sv_endpoint_run(SvEndpointOptions)` с controlled peer и native SDL input. Public `sv_birth_command` через birth interaction используется только для дополнительных state checks; он не заменяет endpoint evidence final send. Связь: P1-ac1/P1-ac3/P2-4 → sv-core и controller-review.

Существующий endpoint seam доступен; прямого публичного final-play serializer API пока нет. Реализовать требуемый production path внутри SV и вызывать его через endpoint; не придумывать test-only интерфейс. Если endpoint не позволяет проверить обязательный критерий, зафиксировать seam/evidence blocker, а не заменять тестируемое поведение.

## Acceptance criteria

- [x] **P1-ac2, P1-ac3, P2-3:** составить и проверить version-gate matrix из canonical baseline session-play/session-setup: strict newer-than и inclusive at-least границы; identity/trait/audio/graphics/font fields, option counts, geometry, character widths и размеры visual redefinition tables. Для каждой различимой границы проверить соседние версии и саму границу.
- [x] **P1-ac1, P1-ac3:** для каждого формата controlled peer получает полный exact-byte handshake после legal birth, возвращает authoritative acknowledgement; production defaults и identity bytes сохраняются при различной длине payload.
- [x] **P1-ac3, P2-4:** byte oracle независим от serializer; фактические bytes получены через production endpoint, без отдельной test-only сериализации поведения клиента. Неприменимые версии явно обосновать исходным baseline, не считать проверенными.

## Границы и проверки

Каждый implementation срез сохраняет protocol/game semantics behavior baseline и SV isolation (P2-1–P2-4, P2-7), проходит sv-build и настроенный sv-core; controller-review и fresh final audit отдельно устанавливают acceptance evidence. Начинать с failing проверки public production seam, затем реализовать production поведение.

SV-B-023 — сохранённая upstream dependency, уже присутствующий production foundation, но его full acceptance/native evidence pending; он не включён в manifest как выдуманный completed dependency. При обнаружении реального отсутствующего prerequisite остановить зависимую реализацию и записать blocker.

P1-ac9–P1-ac11: DNA persistence, profile/resource workflows, real-server integration, session.enter-game и gameplay presentation не входят в этот срез. Требуемые handshake resource bytes входят. P1-ac12 / P1-entry-evidence deferred в SV-B-025, не passed. Обнаруженные улучшения записывать отдельно по P2-5/P2-6; намеренная изоляционная duplication сама по себе не finding.

Публикация тикета не означает, что implementation, runtime checks или controller audit уже выполнены. Работа по frontier начинается с 01; blocked children доступны после завершения перечисленных prerequisites.

## Comments

2026-10-10: Claimed on the assigned isolated branch after confirming ticket 01 is resolved and its production foundation is present. Agreed seam: `sv_endpoint_run(SvEndpointOptions)` with native SDL input and a controlled TCP peer. Preserve the existing history; full gates and audits remain orchestrator-owned.

## Answer

Implemented the versioned ordinary creation/play handshake on the assigned
isolated branch `codex/sandcastle-0241e0da-wave-2-1`. History: ready-for-agent →
claimed 2026-10-10 after verifying resolved prerequisite 01 and its merged
production foundation → resolved after focused verification. No other agent
branch was edited or merged. The supplied effort has no `map.md`; this answer
and the evidence link remain at the existing authoritative ticket location.

**Production seam:** the pre-agreed `sv_endpoint_run(SvEndpointOptions)` with
native SDL input and a controlled TCP peer. Every example goes through legal
birth, committed production defaults, the existing endpoint caller and SV-local
`sv_login_start_play`, then a real socket. No new serializer API, test-only client
behavior, absent caller, shared extraction, or legacy/common implementation was
introduced. The peer supplies version-correct setup/overview packets; it does
not serialize expected play behavior. Public endpoint outcomes supplement,
but do not replace, the captured final-request bytes.

**Baseline matrix:** [matrix, provenance and captures](../evidence/02/README.md).
The canonical manifest sources `source.baseline.session-play` / `session-setup`
at revision `0ef6b362cc7676805fdd0becff680dac50fe83c1` are `Net_start` and
`Net_setup` in `src/client/nclient.c`; the local file's SHA-256 matches the
manifest. Each distinguishable gate has below/at/above controlled versions:
class setup recommendations; setup trait count and final identity/trait/audio;
geometry; 96/128/154/199 options; K/R expansion; inclusive u32 character width;
inclusive graphics/font/resource fields; and strict feature-table expansion.
Overview mode/location and inclusive initial-login address are also probed to
establish the complete version-correct startup route. Unknown visual count is
always 127. The two older feature-limit arms both use 256 in the baseline and
therefore do not add a separate feature-table boundary; K/R do change there.

The explicit independent worked-example oracle in
`tests/sv_birth_version_checks.py` reads no SV serializer or captured bytes. It
uses literal identity/stats/geometry, explicit per-version layout counts, and
the independently baseline-derived option literal from slice 01. All required
visual tables are present, with the text-font server-identity sentinel described
by their baseline provenance. Production identity/mode/body/class/race and six
committed ranks remain identical; versions without trait support legally skip
that creation step and omit its wire field. Defaults are the appropriate prefix
of the full 199 production options. Resource strings appear only where supported.
No distinguishable Net_start/Net_setup gate is excluded as inapplicable.
Unlisted versions sharing a layout, historical releases and real servers are not
claimed individually runtime-verified. Compile-time BREAK_GRAPHICS/GCU variants
are outside the accepted native SDL resource outcome, not a version exemption.

**TDD vertical slices (same agreed endpoint seam):**

1. `python3 tests/sv_birth_version_checks.py` with the initial single
   `4.9.1.2.0.2` example: **red**, the current-only production guard returned
   invalid after legal birth and the peer failed to receive a complete request
   (`exact(socket, captured_play, expected_size)`). After enabling the older
   resource layout and its canonical 256-feature gate: **green**, 14,986 exact
   bytes and `play-acknowledged`.
2. `SV_BIRTH_VERSION=4.8.1.2.0.0 python3 tests/sv_birth_version_checks.py`:
   **red**, full request byte comparison failed because 199 options were sent
   where baseline requires 154. After the option-count gate: **green**, 14,941
   exact bytes and authoritative acknowledgement.
3. `SV_BIRTH_VERSION=4.4.3.1.0.0 python3 tests/sv_birth_version_checks.py`:
   **red**, the remaining minimum-version guard rejected legal completion.
   After implementing earlier baseline identity/option/geometry/table/character
   layouts: **green**, 5,233 exact bytes and authoritative acknowledgement.
   A supplementary state assertion was corrected from wire trait zero to the
   existing public `SV_BIRTH_TRAIT_NONE` for the legitimately skipped trait;
   that fixture correction is not a separate behavioral red.

**Focused final verification:**

- `SV_BIRTH_EVIDENCE=.scratch/sandcastle-0241e0da/evidence/02 python3 tests/sv_birth_version_checks.py`
  passed all **40 endpoint sessions**, covering 13 boundary triplets plus current
  4.9.4.0.0.0, with ASan/UBSan and `-Wall -Wextra -Werror`. Requests range from
  5,233 to 16,266 bytes. Every complete capture matches its independent oracle;
  every peer sends `02 03 7a`; every endpoint reaches `play-acknowledged`.
  Early sends, trailing/duplicate sends, and interleaved ping service are checked.
- `python3 tests/sv_birth_complete_checks.py` passed the existing current-baseline
  waiting, partial reply and successful reply cases, each preserving all 16,266
  expected bytes. Only the complete authoritative reply advances the phase.
- `git diff --check` passed. The focused matrix check is registered in
  `.sandcastle/checks.sh` for the orchestrator's configured core gate.

**Separate improvement:** the shared `defines.h` feature compatibility comment
names a different threshold from canonical `Net_start`; problem, affected code,
proposal, status and future checks are recorded in `docs/sv-improvements.md`.
The shared comment and executable baseline were left unchanged. Intentional SV
isolation is not recorded as a defect or a consolidation obligation.

Checkmarks mean completion of this ticket's implementation slice, not full
immutable-contract acceptance. Per the requested division of work, the full
`sv-build`, configured `sv-core`, controller-review and fresh final audit were
not run or claimed passed. Those gates remain orchestrator/controller-owned.
SV-B-023 native/full acceptance stays pending; the dependency and originating
historical `SV-B-024-birth-dna.md` filename are preserved. Dedicated completion,
extended lifecycle/failure ownership, Guide/MOTD/live handoff and runtime audit
remain with their originating tickets. DNA persistence, profile/resource
workflows, real-server integration, session.enter-game and gameplay presentation
remain stage C; P1-ac12 / P1-entry-evidence remains deferred to SV-B-025, not passed.
