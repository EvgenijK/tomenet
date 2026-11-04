# 87: Verify focused production coverage

Type: implementation
Tracker role: Wayfinder planning/execution history for canonical [SV-B-023](../../../docs/tasks/stage-b/SV-B-023-birth-choices.md); this child does not own Stage B capability IDs.
Status: resolved
Assignee: codex/sandcastle-7b133980-wave-65-1
Labels: enhancement, ready-for-agent

Test the completed SV change through production paths. Inspect the active tickets and add focused missing tests if needed. Run the relevant focused checks. Keep all explicit deferred acceptance checks pending. The orchestrator runs the full headless gate after assembly.

- [x] Relevant production paths are exercised and results recorded under Answer.
- [x] Any new tests are committed, with no test-only behavior or claims of unrun acceptance.

## Comments

- 2026-10-09: Claimed before coverage inspection on
  `codex/sandcastle-7b133980-wave-65-1`. The accepted public production seam is
  `sv_endpoint_run(SvEndpointOptions)`: authoritative contact/login setup enters
  through the endpoint, real SDL gestures pass through native adapters and the
  production login/birth interaction/input router, and `SvEndpointOutcome` plus
  the controlled peer observe character-selection state, ownership, draft and
  prohibited sends. This focused seam does not replace deferred native runtime
  evidence P1-AC30.
- 2026-10-09: Open/claimed-record inspection found this ticket as the sole
  unresolved effort child. Resolved tickets 85 and 86 are the production and
  test deltas after focused record 84. Their endpoint scenarios already cover
  the repaired capacity matrix and authoritative unavailable-body sequencing;
  assembled controls cover supported stat modes, available body, dedicated
  creation and unsupported-mode fail-fast in both compile orders. No missing
  focused test or production defect was found, so this coverage-only ticket
  does not claim a red -> green loop.

## Answer

Проверен assembled production path после tickets 85 и 86 на exact
pre-resolution HEAD `f5c8df892ae783e06061567a9416bb02e0f44e14`.

Production seam: публичный `sv_endpoint_run(SvEndpointOptions)`. Controlled
peer передаёт authoritative contact limits, advertised dedicated flags, roster,
server version, supported creation flags и complete birth setup через реальный
protocol path. Keyboard/mouse SDL events проходят native adapters и production
login/birth interaction/input router. `SvEndpointOutcome` наблюдает phase,
name/birth entry, owner, server-provided lists, highlights и draft; peer требует
EOF после отмены/quit, поэтому неожиданный name, birth, partial или completed
packet приводит к провалу сценария. Отдельной test-only реализации поведения
нет.

Инспекция existing focused coverage не выявила пробела. Capacity matrix
проверяет 18 production endpoint scenarios: исходные ordinary/exclusive
controls, available/full ordinary boundary для всех сочетаний neither,
IDDC-only, PvP-only и both, а также отдельные available/full IDDC/PvP
boundaries. В ней используются и keyboard, и mouse create actions. Empty-body
keyboard (`creation_flags == 0`) и mouse (`creation_flags == 1`) scenarios
получают authoritative pre-separate-body setup, обходят недоступный body owner,
делают Backspace к ближайшему доступному predecessor и завершаются без send в
обоих class/race compile orders. Available-body, оба stat modes, dedicated entry
и unsupported ordinary/exclusive controls исключают подмену соседней семантики.
Новые tests и production code не требовались; test-first цикл для уже
реализованного поведения не заявляется.

Focused verification:

```sh
PATH=/opt/sv-venv/bin:/usr/local/bin:/usr/bin:/bin \
  python3 -B tests/sv_ordinary_create_cancel_checks.py

for birth_case in \
  body-unavailable-keyboard body-unavailable-mouse \
  first-run-body-reject non-first-run-body-manual \
  stats-order-navigate stats-point-navigate mode-dedicated \
  unsupported-creation-ordinary unsupported-creation-exclusive
do
  SV_BIRTH_CASE="$birth_case" \
    PATH=/opt/sv-venv/bin:/usr/local/bin:/usr/bin:/bin \
    python3 -B tests/sv_name_editor_submit_checks.py
done
git diff --check
```

Результат: **PASS**. Capacity runner выполнил 18 production endpoint
scenarios. Девять selected birth modes выполнились в default и alternate
compile configurations (18 endpoint executions); loop exit 0. Оба runner
собрали production sources с `-Wall -Wextra -Werror`, ASan и UBSan; compiler,
sanitizer и assertions прошли. Повторяющиеся declared-font fallback и SDL
`Leaked thread` diagnostics соответствовали существующему focused-runner output
и не сопровождались failure. `git diff --check` — **PASS**.

Full `sv-build`, unfiltered `sv-core`, controller review и full acceptance не
запускались и не объявляются пройденными: их выполняет orchestrator после
assembly. Legacy/shared и production behavior files не изменялись; новых
unrelated improvement opportunities не обнаружено.
`P1-birth-runtime-evidence` / P1-AC30 для accepted contract digest
`15de367676b8c4c23c1ba787bb60b4e976455f27926f4b7688e17fc75204dbe6`
остаётся **deferred; not run** у исходного named producer/verifier ownership.
