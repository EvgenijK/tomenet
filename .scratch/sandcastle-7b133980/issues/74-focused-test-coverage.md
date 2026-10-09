# 74: Verify focused production coverage

Type: implementation
Status: resolved
Assignee: codex/sandcastle-7b133980-wave-51-1
Labels: enhancement

Test the completed SV change through production paths. Inspect the active tickets and add focused missing tests if needed. Run the relevant focused checks. Keep all explicit deferred acceptance checks pending. The orchestrator runs the full headless gate after assembly.

- [x] Relevant production paths are exercised and results recorded under Answer.
- [x] Any new tests are committed, with no test-only behavior or claims of unrun acceptance.

## Comments

- 2026-10-09: Claimed on `codex/sandcastle-7b133980-wave-51-1` before
  coverage inspection. The accepted public production seam is
  `sv_endpoint_run(SvEndpointOptions)`: authoritative setup and loaded DNA enter
  the endpoint, real SDL events pass through the native birth adapter and the
  production interaction/input router, and `SvEndpointOutcome` plus EOF at the
  controlled peer observe state and prohibited sends. This focused seam does
  not replace deferred native runtime evidence P1-AC30.
- 2026-10-09: Active-record inspection found ticket 73 as the only new
  implementation/test delta after focused record 72. Its existing endpoint
  scenarios cover the repaired aggregate-DNA success fixtures, retained reject
  controls, hidden-derived aggregate setup, mode-only case-sensitive binding
  repair and alternate-order mouse fixture. No missing focused branch or
  production defect was found, so no new test, behavior change or red -> green
  loop is claimed.

## Answer

Проверен active record 73. Шесть direct DNA success fixtures теперь начинают с
валидного loaded-DNA aggregate и подтверждают class, race, trait, body,
point-buy stats и mode через production interaction. Их missing/illegal
controls сохраняют отказ; hidden-derived success/mismatch/reject fixtures
сохраняют общий lifecycle валидности; point-buy variants проверяют owner-local
normalization. Отдельно существующие case-sensitive mode scenarios покрывают
lowercase `h`, отклонение неподходящего uppercase ввода и legal ordinary/
dedicated Hellish bindings, а `alternate-mouse` подтверждает правильную
server-visible class identity реальным SDL double-click. Нового пробела в
focused coverage не обнаружено, поэтому production и test files не менялись.

Production seam: публичный `sv_endpoint_run(SvEndpointOptions)`. Controlled
peer передаёт authoritative server setup и loaded DNA, реальные SDL keyboard,
DNA и mouse events проходят native birth adapter и production
birth-interaction/input-router boundary. `SvEndpointOutcome` наблюдает owner,
confirmed identity/value, highlighted selection, committed draft, DNA-used
marker и aggregate availability. Peer допускает только name handoff и требует
EOF, поэтому partial/completed birth send провалил бы сценарий. Отдельной
test-only реализации поведения нет; legacy/shared files не изменялись.

Focused verification, 2026-10-09:

```sh
for birth_case in \
  class-dna class-dna-incompatible \
  race-dna race-dna-missing \
  trait-dna trait-dna-incompatible \
  body-dna body-dna-missing \
  stats-point-dna stats-point-dna-missing \
  mode-dna mode-dna-missing \
  case-sensitive-mode-lower-h \
  case-sensitive-mode-uppercase-reject \
  case-sensitive-mode-ordinary-hellish \
  case-sensitive-mode-dedicated-hellish \
  hidden-derived-dna hidden-derived-dna-race-mismatch \
  hidden-derived-dna-missing hidden-derived-dna-incompatible \
  hidden-derived-dna-stale \
  stats-point-dna-over-budget stats-point-dna-illegal \
  stats-point-dna-multiple-max alternate-mouse
do
  SV_BIRTH_CASE="$birth_case" \
    PATH=/opt/sv-venv/bin:/usr/local/bin:/usr/bin:/bin \
    python3 -B tests/sv_name_editor_submit_checks.py
done
```

Результат: **PASS**, 25/25 cases, общий exit 0. Runner при каждом запуске собрал
обе production compile-order configurations с `-Wall -Wextra -Werror`, ASan и
UBSan; каждый scenario выполнился в обеих применимых inventory configurations.
Compiler, sanitizer и assertion failures не было. Повторяющийся SDL `Leaked
thread` diagnostic совпал с уже известным focused-runner output и не
сопровождался failure. Логи сохранены в
`/tmp/sandcastle-7b133980-ticket-74/`. `git diff --check` — **PASS**.

Это coverage-only ticket для уже реализованного поведения. Первый focused
production-seam запуск был green; дефект не обнаружен, поэтому red -> green
цикл и новые tests не заявляются. Полные `sv-build`, `sv-core`, controller
review и общий build/test/review workflow не запускались: их выполняет
orchestrator после assembly. Новых improvement opportunities не обнаружено.
`P1-birth-runtime-evidence` / P1-AC30 для accepted contract digest
`690a04cefb7fd0e1615dce46d1a538e1dcbd856232af1a1373be93f987095c82`
остаётся **deferred; not run** у named full-acceptance producer/verifier и этим
ticket не объявляется выполненным.
