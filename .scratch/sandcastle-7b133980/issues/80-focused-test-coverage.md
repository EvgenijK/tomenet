# 80: Verify focused production coverage

Type: implementation
Tracker role: Wayfinder planning/execution history for canonical [SV-B-023](../../../docs/tasks/stage-b/SV-B-023-birth-choices.md); this child does not own Stage B capability IDs.
Status: resolved
Assignee: codex/sandcastle-7b133980-wave-57-1
Labels: enhancement, ready-for-agent

Test the completed SV change through production paths. Inspect the active tickets and add focused missing tests if needed. Run the relevant focused checks. Keep all explicit deferred acceptance checks pending. The orchestrator runs the full headless gate after assembly.

- [x] Relevant production paths are exercised and results recorded under Answer.
- [x] Any new tests are committed, with no test-only behavior or claims of unrun acceptance.

## Comments

- 2026-10-09: Claimed before coverage inspection. The accepted public production
  seam is `sv_endpoint_run(SvEndpointOptions)`: authoritative server setup enters
  the endpoint, real SDL events pass through the native adapters and production
  birth interaction/input router, and `SvEndpointOutcome` plus the controlled
  peer observe visible/legal identities, owner, highlight, draft and prohibited
  sends. This focused seam does not replace deferred native runtime evidence
  P1-AC30.
- 2026-10-09: Active-record inspection found resolved ticket 79 as the sole
  production delta after focused record 78. Its five new scenarios already cover
  the repaired default-order class presentation and input mapping through the
  accepted production seam. Nearby existing scenarios cover the alternate order,
  hidden identities, Backspace, DNA and session lifecycle. No missing focused
  test or production defect was found, so no new test, behavior change or
  red -> green loop is claimed.

## Answer

Проверен active record 79 и его preservation boundary. Исправление в
`prepare_classes()` сохраняет полный non-hidden server-visible class list в
default class-before-race flow и отдельно формирует legal subset. Уже добавленные
в ticket 79 production scenarios наблюдают incompatible row без сдвига stable
identity, отказ direct keyboard/mouse выбора, legal confirm, navigation, bounded
viewport, random и interleaved focus/resize/network events. Coverage gap после
инспекции не найден, поэтому этот coverage-only ticket не добавляет tests или
production behavior и не заявляет test-first loop.

Production seam: публичный `sv_endpoint_run(SvEndpointOptions)`. Controlled peer
передаёт authoritative contact/login/birth setup; настоящие SDL keyboard, mouse,
wheel, focus и resize events проходят native adapters и public birth
interaction/input-router boundary. Production scene и hit testing читают тот же
`SvBirthView`; `SvEndpointOutcome` наблюдает visible/legal identities, owner,
highlight и draft. Peer разрешает только предусмотренный name handoff и ожидает
EOF, поэтому partial либо completed birth send провалил бы scenario. Отдельной
test-only реализации behavior нет.

Focused verification, 2026-10-09:

```sh
for birth_case in \
  default-visible-class-letter \
  default-visible-class-keyboard-confirm \
  default-visible-class-mouse \
  default-visible-class-viewport \
  default-visible-class-random
do
  SV_BIRTH_CASE="$birth_case" \
    PATH=/opt/sv-venv/bin:/usr/local/bin:/usr/bin:/bin \
    python3 -B tests/sv_name_editor_submit_checks.py
done

for birth_case in \
  alternate-visible-class-letter alternate-visible-class-mouse \
  alternate-visible-class-viewport alternate-random \
  legacy-hidden-dna-death-knight class-back class-dna \
  class-dna-incompatible session-interleave session-stale-input \
  session-disconnect
do
  SV_BIRTH_CASE="$birth_case" \
    PATH=/opt/sv-venv/bin:/usr/local/bin:/usr/bin:/bin \
    python3 -B tests/sv_name_editor_submit_checks.py
done
```

Результат: **PASS**, 16/16 selected modes, оба loop exit 0. Runner каждый раз
собрал default и alternate production binaries с `-Wall -Wextra -Werror`, ASan
и UBSan; selected modes выполнились в применимых compile configurations.
Проверены repaired default visible-list keyboard/mouse/viewport/random paths,
сохранённые alternate-order mappings, legacy hidden-class exclusion и derived
DNA в обеих configurations, class Backspace/DNA rejection, event interleaving,
stale-generation и disconnect suppression. Compiler, sanitizer и assertion
failures не было; запрещённых outbound birth bytes не наблюдалось. Повторяющиеся
declared-font fallback и SDL `Leaked thread` diagnostics соответствуют известному
focused-runner output и не сопровождались failure.

Полные `sv-build`, unfiltered `sv-core`, controller review и общий
build/test/review workflow не запускались: их выполняет orchestrator после
assembly. Legacy/shared и production behavior files не изменялись; новых
improvement opportunities не обнаружено. `P1-birth-runtime-evidence` / P1-AC30
для accepted contract digest
`690a04cefb7fd0e1615dce46d1a538e1dcbd856232af1a1373be93f987095c82`
остаётся **deferred; not run** у named full-acceptance producer/verifier и этим
ticket не объявляется выполненным.
