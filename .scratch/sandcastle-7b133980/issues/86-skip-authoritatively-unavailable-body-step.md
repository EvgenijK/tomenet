# 86: Пропустить authoritative unavailable body step

**Type:** repair

**Tracker role:** Wayfinder planning/execution history for canonical [SV-B-023](../../../docs/tasks/stage-b/SV-B-023-birth-choices.md); this child does not own Stage B capability IDs.

**What to build:** Когда authoritative supported creation setup не предоставляет ни одного legal body choice, игрок никогда не получает недоступного body owner: forward flow сразу переходит к следующему доступному owner, а Backspace обходит body в обоих class/race compile orders. Draft и соседние owner selections сохраняются, partial/completed packets не отправляются, а supported `creation_flags` остаются 0 или 1.

**Blocked by:** None (can start immediately).

**Status:** resolved

**Assignee:** codex/sandcastle-7b133980-wave-64-2

Labels: bug, ready-for-agent

**Finding ID:** `D-1563ec778f0a5c2f` (`sv.birth.body-step.never-unavailable`), one coherent repair of authoritative body-step availability, forward sequencing and every affected reverse transition.

**Production seam for TDD:** публичный `sv_endpoint_run(SvEndpointOptions)` получает authoritative server-provided birth setup через controlled peer и production setup/interaction state; реальные SDL keyboard/mouse gestures проходят native adapter и публичную birth-interaction/input-router boundary, а `SvEndpointOutcome` и peer наблюдают owner, `body_skipped`, legal body list, draft и отсутствие запрещённых outbound bytes. Seam принят P1-AC02, P1-AC15, P1-AC17, P1-AC19, P1-AC28 и P2-AC05; проверки — `sv-core`, `sv-build` и `controller-review`. Availability должна поступать через production setup state; отдельный test-only flag или behavior implementation недопустимы.

- [x] RED production-endpoint case с supported normalized `creation_flags` 0 или 1 и authoritative empty legal-body set показывает прежнее назначение body owner; GREEN хранит complete-step availability в production setup/interaction state, выставляет `body_skipped`, оставляет legal body list пустым и не назначает body owner. [Contract: P1-AC02, P1-AC07, P1-AC19, P1-AC28, P1-AC31, P2-AC05; check: `sv-core`]
- [x] В class-before-race order forward flow после предшествующего доступного owner сразу достигает stats, а Backspace из stats возвращается к ближайшему доступному trait/race owner, сохраняя draft/highlight и не отправляя partial либо completed-character packet. [Contract: P1-AC14, P1-AC16, P1-AC17, P1-AC19, P2-AC04; check: `sv-core`]
- [x] В race-before-class order forward flow обходит empty body step и достигает class, а Backspace из class возвращается через правильного доступного predecessor без посещения body; отдельно покрыты сочетания available и skipped trait. [Contract: P1-AC14, P1-AC15, P1-AC16, P1-AC19, P2-AC04; checks: `sv-build`, `sv-core`]
- [x] Keyboard и mouse production scenarios подтверждают одинаковое semantic sequencing; supported ordinary и dedicated IDDC/PvP flows, non-empty Normal/Fruit Bat body choices, random/DNA behavior, quit, disconnect/stale-generation guards и unsupported-mode fail-fast остаются без регрессии. [Contract: P1-AC02, P1-AC07, P1-AC11, P1-AC12, P1-AC20, P1-AC22, P1-AC26, P1-AC31; checks: `sv-build`, `sv-core`]
- [x] Repair остаётся SV-local по возможности, использует production interaction state/input router и не добавляет fixed-response либо test-only behavior; unrelated legacy/shared/SV organization improvements записываются отдельно и не включаются в этот ticket. [Contract: P1-AC28, P2-AC01, P2-AC02, P2-AC03, P2-AC05, P2-AC06, P2-AC07, P2-AC08, P2-AC09; check: `controller-review`]

**Deferred check:** `P1-birth-runtime-evidence` / P1-AC30 остаётся deferred у named full-acceptance producer/verifier и не считается пройденным этим repair; evidence собирается только на exact accepted contract digest `15de367676b8c4c23c1ba787bb60b4e976455f27926f4b7688e17fc75204dbe6` и implementation HEAD.

## Comments

- 2026-10-09: Claimed on `codex/sandcastle-7b133980-wave-64-2` and
  confirmed the ticket-approved public production seam
  `sv_endpoint_run(SvEndpointOptions)` before adding coverage.

## Answer

Production contact decoding now records separate-body-step availability in the
SV-local `SvContactSetup`. Servers before 4.4.1a expose Fruit Bat as part of the
mode step and therefore provide no separate legal body choices; supported
normalized `creation_flags` remain independently 0 or 1. `SvBirth` consumes
this production setup state. An unavailable step publishes an empty legal body
list, clears body highlight/draft, sets `body_skipped`, and transfers ownership
directly to stats in class-before-race order or class in race-before-class
order. Existing stats/class Backspace logic then selects the nearest real
trait/race predecessor without visiting body. Available modern setup behavior
continues to derive Normal/Fruit Bat legality, first-run filtering, random and
DNA semantics as before.

Production seam: public `sv_endpoint_run(SvEndpointOptions)`. The controlled
peer supplies the contact version, supported creation flags and complete setup;
real SDL keyboard/mouse events pass through the native adapter and public birth
interaction/input-router boundary. `SvEndpointOutcome` observes owner, legal
lists, highlights and draft. The peer requires EOF after quit, so any birth,
partial or completed-character outbound bytes fail the scenario. No endpoint
option, fixed-response path or test-only behavior implementation was added.

TDD RED on the pre-implementation tree:

```sh
SV_BIRTH_CASE=body-unavailable-keyboard \
  PATH=/opt/sv-venv/bin:/usr/local/bin:/usr/bin:/bin \
  python3 -B tests/sv_name_editor_submit_checks.py
```

Result: **RED**, exit 1/SIGABRT at the production outcome assertion
`outcome.birth_body_skipped`; the supported setup incorrectly synthesized a
body list and owner.

After the minimal production change, the same command was **GREEN**, exit 0,
in both compile orders. The corresponding `body-unavailable-mouse` point-buy
slice (`creation_flags == 1`) was also **GREEN**, exit 0, in both orders. The
default build proved forward entry into stats before Backspace; the alternate
build proved forward entry into class before Backspace. Both finished at the
preserved race draft/highlight with no body owner or outbound result.

Focused verification on implementation commit `3e3cec354`:

```sh
for birth_case in \
  body-unavailable-keyboard body-unavailable-mouse \
  first-run-body-reject non-first-run-body-manual body-random body-dna \
  body-quit body-back body-back-no-trait trait-unavailable-skip stats-back \
  alternate-body-back alternate-class-back \
  mode-dedicated exclusive-pvp-maia-production \
  session-disconnect session-stale-input \
  unsupported-creation-ordinary unsupported-creation-exclusive
do
  SV_BIRTH_CASE="$birth_case" \
    PATH=/opt/sv-venv/bin:/usr/local/bin:/usr/bin:/bin \
    python3 -B tests/sv_name_editor_submit_checks.py
done
git diff --check
```

Result: **PASS**, 19 selected cases in every compile configuration registered
for those cases; both empty-body slices explicitly ran in both class/race
orders. Compiler `-Wall -Wextra -Werror`, ASan/UBSan and assertions passed.
Available and skipped trait predecessors are covered by `body-back`,
`body-back-no-trait`, `alternate-body-back` and the empty-body slices; modern
Normal/Fruit Bat, random/DNA, quit, ordinary/dedicated IDDC/PvP,
disconnect/stale-generation and unsupported-mode controls all remained green.
The recurring declared-font fallback and SDL `Leaked thread` diagnostics were
the existing focused-runner output and did not accompany a failure.
`git diff --check` passed.

The implementation is confined to SV production setup/interaction code and its
production-endpoint tests; legacy/shared gameplay code is unchanged. No
unrelated improvement opportunity was found. Per orchestration instructions,
the full `sv-build`, unfiltered `sv-core`, controller review and full acceptance
were not run or claimed. `P1-birth-runtime-evidence` / P1-AC30 remains
**deferred; not run** with its original named producer/verifier ownership and
accepted contract digest.
