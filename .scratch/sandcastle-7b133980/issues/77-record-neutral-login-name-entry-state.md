# 77: Зафиксировать нейтральное имя состояния ввода имени

**Type:** repair

**Tracker role:** Wayfinder planning/execution history for canonical [SV-B-023](../../../docs/tasks/stage-b/SV-B-023-birth-choices.md); this child does not own Stage B capability IDs.

**What to build:** В реестре SV improvements появляется одна самостоятельная запись о том, что production state `ordinary_name_active` используется и для ordinary, и для exclusive/dedicated name entry, поэтому его имя больше не отражает область действия. Запись предлагает будущую нейтральную замену, например `name_entry_active`, но не выполняет переименование, не меняет production behavior и не расширяет SV-B-023.

**Blocked by:** None (can start immediately).

**Status:** resolved

**Assignee:** codex/sandcastle-7b133980-wave-54-1

Labels: enhancement, ready-for-agent

**Finding ID:** `D-80a802fa22706ca7` (`sv-login.name-entry-active-misnamed-ordinary`), one coherent documentation repair; the proposed production-state rename remains separately scoped future work.

**Production seam:** Not applicable. This documentation-only repair must not rename the production field, change the public SV interaction/input-router path or create a test-only interface. Preservation of production behavior is checked by `controller-review`. [Contract: P1-AC28, P2-AC05]

- [x] The SV improvement register contains exactly one standalone record explaining that the name-entry active flag now covers both ordinary and exclusive/dedicated creation, so its ordinary-only name is misleading. [Contract: P2-AC06, P2-AC07; check: `controller-review`]
- [x] The record states Problem, Affected code, Proposal, Status and Required checks; it identifies the name-entry state declaration, both activation paths and its generic endpoint consumers, and proposes a neutral production name such as `name_entry_active`. [Contract: P2-AC06, P2-AC07, P2-AC12; check: `controller-review`]
- [x] The status marks the rename as separately proposed future work: this repair changes only the improvement record and Wayfinder history, without renaming code, altering ordinary or exclusive/dedicated creation, touching legacy/shared gameplay code, or changing any verified SV-B-023 behavior. [Contract: P1-AC02, P1-AC20, P1-AC28, P2-AC01, P2-AC03, P2-AC04, P2-AC08, P2-AC09; check: `controller-review`]
- [x] Required future checks cover a complete production reference migration, ordinary and exclusive/dedicated name-entry activation and exit, handoff into the existing birth interaction/input router, quit and prohibited-send behavior, `sv-build`, `sv-core` and `controller-review`; those future rename checks are recorded as required and are not claimed passed by this documentation repair. [Contract: P1-AC02, P1-AC12, P1-AC20, P1-AC22, P1-AC28, P2-AC04, P2-AC05, P2-AC07, P2-AC08; checks: `sv-build`, `sv-core`, `controller-review`]

**Deferred check:** `P1-birth-runtime-evidence` / P1-AC30 remains deferred to its named producer and verifier at full acceptance. This documentation repair neither performs nor claims that evidence.

## Comments

- 2026-10-09: Claimed on `codex/sandcastle-7b133980-wave-54-1`.

## Answer

В `docs/sv-improvements.md` добавлена ровно одна самостоятельная запись
`Give the shared login name-entry state a neutral name`. Она фиксирует, что
`SvLoginInteraction.ordinary_name_active` управляет единым name editor после
обоих production commands: `SV_LOGIN_COMMAND_CREATE_ORDINARY` и
`SV_LOGIN_COMMAND_CREATE_EXCLUSIVE`, поэтому ordinary-only имя больше не
соответствует области действия.

Запись перечисляет declaration в `input/login-interaction.h`, оба activation
path и общие edit/cancel/submit consumers в `input/login-interaction.c`, native
event router в `input/native-name.c`, а также contact-status и login-view
consumers в `endpoint-run.c`. Будущее production cleanup предлагает нейтральное
`name_entry_active` и полную миграцию production references при сохранении
`dedicated_creation`, mode flags, существующего birth interaction/input-router
handoff и protocol/gameplay semantics.

Статус явно оставляет rename отдельной будущей работой. Production, tests,
legacy/shared gameplay code и scope SV-B-023 этой правкой не изменены. Новые
behavior tests не добавлялись: согласованный production seam для изменения
поведения в этом documentation-only repair неприменим, а отдельный test-only
seam противоречил бы тикету и TDD skill. Навык TDD использован для соблюдения
этой границы; behavioral red/green цикл не заявляется.

Сфокусированная проверка документационной записи:

- до добавления записи `rg -n '^## Give the shared login name-entry state a
  neutral name$' docs/sv-improvements.md` — **RED**, exit 1, заголовок
  отсутствовал;
- после добавления структурная shell-проверка единственного заголовка, пяти
  обязательных полей, declaration/consumer paths, обоих activation commands,
  `name_entry_active`, scope constraints и future-check matrix — **GREEN**,
  exit 0: `heading_count=1; fields=5;
  production_references_and_future_checks=present`;
- `SV_BIRTH_CASE=mode-ordinary-ready python3 -B
  tests/sv_name_editor_submit_checks.py` — **PASS**, exit 0, production ordinary
  name handoff и birth flow в обеих compile configurations;
- `SV_BIRTH_CASE=exclusive-iddc-production python3 -B
  tests/sv_name_editor_submit_checks.py` — **PASS**, exit 0, production
  exclusive/dedicated name handoff и birth flow в обеих compile configurations;
- `git diff --check` — **PASS**, exit 0; изменены только improvement register и
  Wayfinder history этого тикета.

Полный build/test/review workflow не запускался: его выполняет orchestrator.
Полная migration, activation/exit, handoff, quit, prohibited-send проверки и
`sv-build`, `sv-core`, `controller-review` записаны как обязательные для будущего
rename и не заявляются пройденными этой документационной правкой.
`P1-birth-runtime-evidence` / P1-AC30 остаётся **deferred; not run** у named
producer/verifier на full acceptance.
