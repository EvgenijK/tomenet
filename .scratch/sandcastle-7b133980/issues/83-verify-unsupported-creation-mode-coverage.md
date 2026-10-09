# 83: Проверить production coverage unsupported creation mode

**Type:** implementation

**Tracker role:** Wayfinder planning/execution history for canonical [SV-B-023](../../../docs/tasks/stage-b/SV-B-023-birth-choices.md); this child does not own Stage B capability IDs.

**What to build:** Подтвердить на собранном production-пути, что после удаления service contact bit `0x02` ordinary и dedicated/exclusive creation принимают поддерживаемые normalized `creation_flags` 0 и 1, а любое значение больше 1 немедленно и без побочных эффектов возвращает игроку точный текст `Unsupported character creation mode` на неизменённой character-selection surface. Focused coverage также сохраняет достижимость server-provided unavailable trait/body skips и не подменяет их unsupported setup.

**Blocked by:** None (can start immediately).

**Status:** resolved

**Assignee:** codex/sandcastle-7b133980-wave-62-1

Labels: enhancement, ready-for-agent

**Production seam for TDD:** публичный `sv_endpoint_run(SvEndpointOptions)`: authoritative contact/login setup входит через endpoint, реальные SDL gestures проходят native adapter и production login/birth interaction/input router, а endpoint outcome и controlled peer наблюдают character-selection state, name/birth ownership, draft и outbound sends. Если focused test обнаружит дефект, сначала зафиксировать RED через этот seam, затем внести минимальное SV-local production исправление и получить GREEN; отдельный test-only интерфейс или реализация поведения недопустимы. [Contract: P1-AC28, P1-AC31, P2-AC01, P2-AC05; checks: `sv-core`, `controller-review`]

- [x] Focused production scenarios для ordinary и dedicated/exclusive create в обоих compile orders передают normalized `creation_flags > 1` и подтверждают точный текст `Unsupported character creation mode`, полностью неизменённый character-selection state, отсутствие name/birth interaction и draft mutation, а также отсутствие name, birth, partial и completed-character packet. [Contract: P1-AC31; check: `sv-core`]
- [x] Контрольные production scenarios подтверждают, что normalized flags 0 и 1 по-прежнему открывают соответственно traditional и player-defined creation, а unavailable trait/body остаются отдельными server-provided условиями и корректно пропускаются в обоих compile orders. [Contract: P1-AC02, P1-AC19, P1-AC31; check: `sv-core`]
- [x] Keyboard и mouse create actions проходят один production input path; focused tests не вводят fixed-response wizard, test-only behavior или обход interaction state/input router. [Contract: P1-AC11, P1-AC28, P2-AC05; checks: `sv-core`, `controller-review`]
- [x] Relevant focused checks и `git diff --check` проходят, результаты и exact tested HEAD записаны в ticket Answer. Full `sv-build`, unfiltered `sv-core`, controller review и full acceptance не объявляются выполненными до их отдельных фактических запусков; `P1-birth-runtime-evidence` / P1-AC30 остаётся deferred с исходным producer/verifier ownership. [Contract: P1-AC30, P1-AC31; checks: `sv-build`, `sv-core`, `controller-review`, `P1-birth-runtime-evidence`]

## Comments

- 2026-10-09: Claimed on `codex/sandcastle-7b133980-wave-62-1` from exact
  recovery base `1b2b3b3058a1d14aa5b47b24741344f6f796c6d2`. Confirmed the
  ticket-approved public production seam `sv_endpoint_run(SvEndpointOptions)`
  before changing coverage.

## Answer

Production seam: публичный `sv_endpoint_run(SvEndpointOptions)`. Controlled
peer передаёт raw contact flags с service bit `0x02`; production contact decoder
удаляет только этот bit, после чего native SDL gestures проходят
`sv_native_name_event()` и production login/birth interaction/input router.
`SvEndpointOutcome` наблюдает character-selection phase/roster/selection,
name evidence, birth owner/lists/highlights/draft, а peer после create action
требует EOF и тем самым запрещает name, birth, partial и completed-character
packets.

Coverage теперь отделяет ровно два unsupported cases от прежних
`unknown-setup`/unavailable fixtures. Ordinary получает normalized flags `4`,
exclusive — `0x10`; оба в каждой compile configuration подтверждают exact text
`Unsupported character creation mode`, прежний roster из одного character,
пустую selection, отсутствие name interaction и полностью неинициализированный
birth draft/list state. Прежние unknown-mode scenarios, которые предполагали
birth behavior для `creation_flags > 1`, удалены из runnable inventory: после
P1-AC31 они не могут служить unavailable-step controls.

Поддерживаемые controls явно проверяют `creation_flags == 0` для ordinary и
dedicated traditional flow и `creation_flags == 1` для ordinary и нового
dedicated point-buy flow. Их реальные keyboard и mouse stats/mode gestures идут
через тот же production router. `trait-unavailable-skip` теперь действительно
выбирает race без legal server-provided traits и проверяет skip в обеих compile
configurations; `first-run-body-reject` отдельно сохраняет authoritative
unavailable Fruit-bat condition, единственный legal Normal body и отсутствие
mutation от отклонённого body input в обеих configurations.

Это coverage-only изменение: production code не менялся, поэтому test-first
behavior loop не заявляется. После отделения unsupported predicate прежний
`body-unavailable-skip` дал RED (SIGABRT на stale ожидании
`creation_flags == 0x10`), что подтвердило маскировку availability coverage
unsupported setup; obsolete future-mode cases были исключены, а отдельные
supported availability controls сохранены. Расширение
`trait-unavailable-skip` на alternate order сначала дало RED на owner assertion,
потому что fixture выбирал race с доступным trait; минимальная коррекция только
SDL gesture sequence дала GREEN через тот же endpoint seam.

Focused verification на exact tested code/test HEAD
`8248f23a64fc05d03b3116c2fc458e6e45bbcc20`:

```sh
for birth_case in \
  unsupported-creation-ordinary \
  unsupported-creation-exclusive \
  stats-order-navigate \
  stats-point-navigate \
  mode-dedicated \
  stats-point-dedicated \
  trait-unavailable-skip \
  first-run-body-reject
do
  SV_BIRTH_CASE="$birth_case" \
    PATH=/opt/sv-venv/bin:/usr/local/bin:/usr/bin:/bin \
    python3 -B tests/sv_name_editor_submit_checks.py
done
git diff --check
```

Результат: **PASS**, `FOCUSED_CASES_PASS=8`, loop exit 0; каждый выбранный case
выполнился во всех перечисленных runner inventory compile configurations.
Compiler (`-Wall -Wextra -Werror`), ASan/UBSan и assertions прошли. Повторяющиеся
font fallback и SDL `Leaked thread` diagnostics соответствуют известному
focused-runner output и не сопровождались failure. `git diff --check` — **PASS**.

Commit с code/test coverage: `8248f23a6` (`test(sv): verify unsupported
creation modes`). Legacy/shared и production behavior files не изменялись;
новых unrelated improvement opportunities не обнаружено. Full `sv-build`,
unfiltered `sv-core`, controller review и full acceptance не запускались и не
объявляются пройденными: их выполняет orchestrator после normal assembly.
`P1-birth-runtime-evidence` / P1-AC30 для accepted contract digest
`15de367676b8c4c23c1ba787bb60b4e976455f27926f4b7688e17fc75204dbe6`
остаётся **deferred; not run** у исходного named producer/verifier ownership.
