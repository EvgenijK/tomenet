# 84: Verify focused production coverage

Type: implementation
Tracker role: Wayfinder planning/execution history for canonical [SV-B-023](../../../docs/tasks/stage-b/SV-B-023-birth-choices.md); this child does not own Stage B capability IDs.
Status: resolved
Assignee: codex/sandcastle-7b133980-wave-63-1
Labels: enhancement, ready-for-agent

Test the completed SV change through production paths. Inspect the active tickets and add focused missing tests if needed. Run the relevant focused checks. Keep all explicit deferred acceptance checks pending. The orchestrator runs the full headless gate after assembly.

- [x] Relevant production paths are exercised and results recorded under Answer.
- [x] Any new tests are committed, with no test-only behavior or claims of unrun acceptance.

## Comments

- 2026-10-09: Claimed before coverage inspection on
  `codex/sandcastle-7b133980-wave-63-1`. The accepted public production seam is
  `sv_endpoint_run(SvEndpointOptions)`: authoritative contact/login setup enters
  through the endpoint, real SDL gestures pass through native adapters and the
  production login/birth interaction/input router, and `SvEndpointOutcome` plus
  the controlled peer observe character-selection state, ownership, draft and
  prohibited sends. This focused seam does not replace deferred native runtime
  evidence P1-AC30.
- 2026-10-09: Open/claimed-record inspection found this ticket as the sole
  unresolved effort child. Resolved ticket 83 is the only production-test delta
  after focused record 82. Its focused scenarios already cover unsupported and
  supported creation modes plus separate unavailable-step controls in both
  compile configurations. No missing focused test or production defect was
  found, so this coverage-only ticket does not claim a red -> green loop.

## Answer

Проверен assembled production path после ticket 83. Его unsupported ordinary и
exclusive cases передают через controlled peer нормализованные
`creation_flags > 1`, запускают реальные keyboard/mouse create actions и
подтверждают точный текст `Unsupported character creation mode`, неизменный
character-selection state, отсутствие name/birth interaction, нетронутый draft
и отсутствие name, birth, partial и completed-character packets. Supported
controls отдельно проводят `creation_flags == 0` и `creation_flags == 1` через
ordinary и dedicated creation. `trait-unavailable-skip` и
`first-run-body-reject` сохраняют server-provided unavailable conditions и не
подменяют их unsupported setup. Все selected modes входят в default и alternate
runner inventories.

Production seam: публичный `sv_endpoint_run(SvEndpointOptions)`. Production
contact decoder удаляет только service bit `0x02`; native SDL adapters передают
gestures в production login/birth interaction и input router. Endpoint outcome
наблюдает phase/roster/selection, name evidence, birth owner/lists/highlights и
draft, а controlled peer после create action требует EOF и тем самым запрещает
любой неожиданный outbound packet. Отдельной test-only реализации поведения нет.

Coverage gap после инспекции не найден, поэтому новые tests и production code не
добавлялись. Это focused verification уже реализованного поведения; test-first
цикл не заявляется.

Focused verification на exact pre-resolution HEAD
`b93b4fd9db90ac3712fc9a3d06856ed6997e4880`, 2026-10-09:

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

Результат: **PASS**, `FOCUSED_CASES_PASS=8`, loop exit 0; каждый selected mode
выполнился в обеих compile configurations (16 production endpoint executions).
Runner собрал production sources с `-Wall -Wextra -Werror`, ASan и UBSan;
compiler, sanitizer и assertions прошли. Повторяющиеся declared-font fallback и
SDL `Leaked thread` diagnostics соответствуют известному focused-runner output и
не сопровождались failure. `git diff --check` — **PASS**.

Full `sv-build`, unfiltered `sv-core`, controller review и full acceptance не
запускались и не объявляются пройденными: их выполняет orchestrator после
assembly. Legacy/shared и production behavior files не изменялись; новых
unrelated improvement opportunities не обнаружено.
`P1-birth-runtime-evidence` / P1-AC30 для accepted contract digest
`15de367676b8c4c23c1ba787bb60b4e976455f27926f4b7688e17fc75204dbe6`
остаётся **deferred; not run** у исходного named producer/verifier ownership.
