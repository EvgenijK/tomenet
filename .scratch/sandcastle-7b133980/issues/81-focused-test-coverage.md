# 81: Verify focused production coverage

Type: implementation
Tracker role: Wayfinder planning/execution history for canonical [SV-B-023](../../../docs/tasks/stage-b/SV-B-023-birth-choices.md); this child does not own Stage B capability IDs.
Status: resolved
Assignee: codex/sandcastle-7b133980-wave-58-1
Labels: enhancement

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
- 2026-10-09: No other open or claimed effort child remained. Resolved ticket 79
  is the latest production behavior change and resolved ticket 80 already verified
  its five new scenarios. The commits after that verification contain only
  Wayfinder and improvement-register changes; no additional production or test
  delta required another focused scenario. No test-first loop is claimed for this
  coverage-only verification.

## Answer

Проверен текущий assembled production path после records 79 и 80. Coverage gap
не найден: пять default-order scenarios из record 79 уже наблюдают полный
non-hidden server-visible class list отдельно от legal subset, stable identities,
отказ incompatible keyboard/mouse selection, legal confirm, navigation, random,
bounded viewport и interleaved events. Соседние production scenarios сохраняют
alternate order, hidden identity, Backspace/DNA и session-lifecycle границы.
Поэтому новые tests и изменение behavior не требовались; отдельная test-only
реализация не добавлена и red -> green loop не заявлен.

Production seam: публичный `sv_endpoint_run(SvEndpointOptions)`. Controlled peer
передаёт authoritative contact/login/birth setup; настоящие SDL keyboard, mouse,
wheel, focus и resize events проходят native adapters и public birth
interaction/input-router boundary. Production scene и hit testing читают тот же
`SvBirthView`; `SvEndpointOutcome` наблюдает visible/legal identities, owner,
highlight и draft. Peer разрешает только предусмотренный name handoff и ожидает
EOF, поэтому partial либо completed birth send провалил бы scenario.

Focused verification на pre-resolution HEAD `db153c1cbc` (финальный commit меняет
только этот Wayfinder record и его map link), 2026-10-09:

```sh
for birth_case in \
  default-visible-class-letter \
  default-visible-class-keyboard-confirm \
  default-visible-class-mouse \
  default-visible-class-viewport \
  default-visible-class-random \
  alternate-visible-class-letter \
  alternate-visible-class-mouse \
  alternate-visible-class-viewport \
  alternate-random \
  legacy-hidden-dna-death-knight \
  class-back \
  class-dna \
  class-dna-incompatible \
  session-interleave \
  session-stale-input \
  session-disconnect
do
  SV_BIRTH_CASE="$birth_case" \
    PATH=/opt/sv-venv/bin:/usr/local/bin:/usr/bin:/bin \
    python3 -B tests/sv_name_editor_submit_checks.py
done
```

Результат: **PASS**, 16/16 selected modes, loop exit 0. Runner собирал
production sources с `-Wall -Wextra -Werror`, ASan и UBSan; применимые cases
выполнялись в обеих compile configurations. Compiler, sanitizer и assertion
failures не было, запрещённых outbound birth bytes не наблюдалось. Повторяющиеся
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
