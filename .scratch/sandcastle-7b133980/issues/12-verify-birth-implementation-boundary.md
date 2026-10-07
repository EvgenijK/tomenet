# 12: Проверить implementation boundary SV-B-023

**Type:** verification

**Tracker role:** Wayfinder planning/execution history for canonical [SV-B-023](../../../docs/tasks/stage-b/SV-B-023-birth-choices.md); this child does not own Stage B capability IDs.

**What to build:** Готовая реализация SV-B-023 передаётся на implementation review с воспроизводимыми build/core результатами для обеих compile configurations, картой production birth seams и честно deferred full-acceptance runtime evidence; ни DNA persistence, ни final send, ни Stage G Guide capability не объявляются частью результата.

**Blocked by:** 09: Завершить или откатить creation без send; 10: Сохранить owner при session events; 11: Вернуться из birth Guide placeholder.

**Status:** resolved

**Assignee:** codex/sandcastle-7b133980-wave-10-1

Labels: enhancement, ready-for-agent

- [x] `sv-build` проходит для default и alternate class/race compile configurations, а `sv-core` покрывает production endpoint/input/state paths всех sex/race/trait/class/body/stats/mode slices. [Contract: P1-AC02, P1-AC03, P1-AC04, P1-AC05, P1-AC06, P1-AC07, P1-AC08, P1-AC09, P1-AC19, P1-AC28; checks: `sv-build`, `sv-core`]
- [x] Focused production evidence связывает legal/illegal/random/DNA, stable highlighting, keyboard/mouse/confirm/quit, backtrack owner graph, dedicated paths, lifecycle suppression и Guide placeholder с публичным endpoint seam; test-only behavior и fixed-response wizard отсутствуют. [Contract: P1-AC10, P1-AC11, P1-AC12, P1-AC13, P1-AC14, P1-AC15, P1-AC16, P1-AC17, P1-AC18, P1-AC20, P1-AC21, P1-AC22, P1-AC24, P1-AC25, P1-AC28; checks: `sv-core`, `controller-review`]
- [x] Review подтверждает prerequisite integration с SV-B-022 и сохраняет границы: Guide capability остаётся Stage G, DNA persistence — Stage C, `birth.restore-dna`/`birth.save-dna` не требуются, final completed-character send остаётся SV-B-024. [Contract: P1-AC01, P1-AC23, P1-AC26, P1-AC27, P1-AC29; checks: `controller-review`]
- [x] Full native runtime evidence `P1-birth-runtime-evidence` записано как deferred, not run, с исходной acceptance-phase причиной и владельцами; ticket не утверждает, что evidence принято, и не меняет parent task. [Contract: P1-AC30; check: `P1-birth-runtime-evidence`]
- [x] Diff review подтверждает минимальные legacy/shared изменения, SV-local placement, допустимость изолирующего дублирования, baseline protocol/gameplay semantics и отсутствие shared extraction только ради deduplication. [Contract: P2-AC01, P2-AC02, P2-AC03, P2-AC04, P2-AC05, P2-AC09; checks: `controller-review`]
- [x] Любые найденные улучшения legacy/shared/SV вынесены отдельно с problem, affected code, proposal, status и required checks; запись не расширяет текущий scope и intentional duplication не считается дефектом сама по себе. [Contract: P2-AC06, P2-AC07, P2-AC08; checks: `controller-review`]
- [x] Task/evidence artifacts соблюдают local Markdown path/status/Labels rules и используют glossary/ADR vocabulary после чтения root context и relevant ADR. [Contract: P2-AC10, P2-AC11, P2-AC12; checks: `controller-review`]

## Answer

Implementation boundary проверена на assembled implementation HEAD
`db4e1fcb79` при неизменном acceptance contract digest
`690a04cefb7fd0e1615dce46d1a538e1dcbd856232af1a1373be93f987095c82`.
Зависимости 09–11 имеют `Status: resolved` и присутствуют в этом HEAD. Вход из
SV-B-022 остаётся production name handoff: только server-confirmed creation
переводит `SvPregame` в `SV_PREGAME_BIRTH`; parent task не изменялся.

### Карта production seams

| Обязательство | Public production seam и наблюдение |
|---|---|
| Server-provided sex/class/race/trait/body/stats/mode, legal/illegal/random/DNA и stable highlight | `sv_endpoint_run(SvEndpointOptions)` принимает настоящий decoded `SvContactSetup` и server flags, создаёт session-owned `SvBirth`, строит `SvBirthView` и публикует итог через `SvEndpointOutcome`. Managed peer задаёт authoritative lists/masks; assertions используют независимые IDs и ожидаемые owner/draft values. |
| Keyboard, mouse, confirm, quit и Backspace owner graph | Native SDL events проходят через `sv_native_birth_event()` в единый `sv_birth_command()` production interaction path. Default и `SV_CLASS_BEFORE_RACE=0` binaries используют тот же input router; unavailable trait/body никогда не становятся owner. |
| Dedicated IDDC/PvP, post-mode PvP Maia return и ready draft | Те же endpoint/input/state seams получают dedicated/server flags; outcome наблюдает owner, choices и готовность draft. Controlled production connection после допустимого name request обязана дать EOF без дополнительных bytes, поэтому ни transition, ни ready draft не являются final send. |
| Network/focus/resize, disconnect и stale generation | Production endpoint loop обслуживает network и SDL events в одной session generation. Outcome наблюдает owner/selection/draft и active/closed state; controlled outbound connection подтверждает отсутствие post-close/stale sends. |
| Birth Guide placeholder | `?` проходит через тот же native birth route, endpoint scene показывает ровно `The guide is in development`, а keyboard/mouse close возвращает существующий birth caller. Outcome наблюдает open/close counts, owner mask и byte-for-byte draft preservation; peer наблюдает отсутствие network bytes. |

`tests/sv_name_editor_submit_checks.py` собирает две конфигурации непосредственно
из production SV sources и запускает controlled-peer matrix через публичный
`sv_endpoint_run(SvEndpointOptions)`. Suite зарегистрирован в
`.sandcastle/checks.sh` как часть `sv-core`; отдельного choice engine,
test-only behavior или fixed-response wizard нет.

Это focused verification уже реализованного поведения. Новых tests или
production changes не потребовалось, поэтому red → green и test-first claim не
делаются. Согласованный seam — public endpoint с production SDL input,
interaction state и outbound socket boundary.

### Focused results, 2026-10-07

- `python3 -B tests/sv_name_editor_submit_checks.py` — **PASS**, exit 0:
  default и alternate compile configurations; все семь choice slices,
  legal/illegal/random/DNA, keyboard/mouse/confirm/quit, owner/backtrack/skip
  matrix, dedicated IDDC/PvP, lifecycle suppression и все reachable placeholder
  entries.
- `make -s -C src -f makefile.sv tomenet-sv` — **PASS**, exit 0; build record
  `.sv-build/linux/698df40fdb19ac12a861/build.txt`.
- `make -s -C src -f makefile.sv SV_CLASS_BEFORE_RACE=0 tomenet-sv` — **PASS**,
  exit 0; build record `.sv-build/linux/4ffa490428e54d3e4b15/build.txt`.
- `git diff --check` — **PASS**, exit 0.

Полный `sv-core` и controller review workflow не запускались: fresh assembled
gates выполняет orchestrator после объединения tickets. Локальный focused result
не объявляется full native runtime acceptance.

### Scope и diff review

Diff `b656431b5b..db4e1fcb79` от планирования SV-B-023 не меняет
`src/client/c-birth.c`, другие legacy client sources, `src/common` или server
production code. Реализация размещена в `src/client/sv/{session,input,ui}` и
public endpoint assembly; build adaptation ограничена `src/makefile.sv`, а
coverage — production-path tests и регистрацией runner. Shared extraction ради
deduplication не выполнялась; SV-local implementation и изолирующее дублирование
соответствуют правилу изоляции и сохраняют protocol/gameplay semantics behavior
baseline.

DNA values используются только как входы текущего draft с требуемой random/DNA
семантикой. `birth.restore-dna` и `birth.save-dna` отсутствуют; persistence
остаётся Stage C. Placeholder проверяет сохранность birth caller, но Guide
capability остаётся Stage G. `SV_BIRTH_READY` не сериализует completed character;
final completed-character send остаётся SV-B-024.

Повторившиеся diagnostics `Leaked thread` и obsolete common `atol` declaration
уже отдельно записаны в `docs/sv-improvements.md` с problem, affected code,
proposal, status и required checks. Они не расширяют этот ticket. Новых
improvement opportunities при review не обнаружено; intentional duplication
ради SV isolation дефектом не объявлялось.

### Deferred full acceptance

| Check | Status | Причина | Producer / verifier owners |
|---|---|---|---|
| `P1-birth-runtime-evidence` | **deferred; not run** | The contract completion scope is implementation. This native runtime evidence is collected during full acceptance after the implementation is available; the deferral is caused by the acceptance phase boundary, not by the absence of a repository runner. | SV-B-023 implementation assignee produces the evidence; the Sandcastle controller full-acceptance verifier validates it. |

Ни один scenario-indexed native runtime artifact не объявлен выполненным или
принятым. На full acceptance producer должен получить evidence для exact accepted
contract digest и exact тогдашнего HEAD, а controller — проверить его на тех же
digest/HEAD.
