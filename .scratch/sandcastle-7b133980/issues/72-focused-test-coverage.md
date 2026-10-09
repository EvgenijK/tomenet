# 72: Verify focused production coverage

Type: implementation
Status: resolved
Assignee: codex/sandcastle-7b133980-wave-49-1
Labels: enhancement

Test the completed SV change through production paths. Inspect the active tickets and add focused missing tests if needed. Run the relevant focused checks. Keep all explicit deferred acceptance checks pending. The orchestrator runs the full headless gate after assembly.

- [x] Relevant production paths are exercised and results recorded under Answer.
- [x] Any new tests are committed, with no test-only behavior or claims of unrun acceptance.

## Comments

- 2026-10-09: Claimed on `codex/sandcastle-7b133980-wave-49-1` before
  coverage inspection. The accepted public production seam is
  `sv_endpoint_run(SvEndpointOptions)`: one authoritative loaded DNA outcome
  and server setup enter through the controlled peer, real SDL `%`, `#`,
  manual confirm, Backspace and quit events pass through the native adapter and
  production birth interaction/input router, and `SvEndpointOutcome` plus EOF
  at the peer observe state and prohibited sends. This focused seam does not
  replace deferred native runtime evidence P1-AC30.
- 2026-10-09: Active-record inspection found ticket 71 as the only production
  delta since focused record 70. Its existing endpoint cases covered early
  invalidation followed by manual advance or Backspace/re-entry. Missing
  focused coverage was added for invalidation at the final mode owner followed
  by Backspace and repeated staged/automatic DNA attempts. This is coverage of
  already implemented behavior; no production defect or red -> green behavior
  repair was found or claimed.

## Answer

Проверен active record 71 и его SV-local общий lifecycle валидности загруженного
birth DNA outcome. Существующие scenarios уже доказывали ранний missing/illegal
class/race component, необратимую invalidation, manual advance и re-entry.
Добавлен focused production endpoint scenario
`auto-dna-ordered-shared-mode-backtrack`: fully legal sex/class/race/trait/body
и ordered stats доходят до отсутствующего обязательного mode DNA, после чего
Backspace возвращает в stats, а повторные native `#` и `%` не восстанавливают
DNA availability и не меняют owner, highlighted stat или committed draft.
Mode остаётся неподтверждённым. Controlled peer принимает только разрешённый
name packet и требует EOF, поэтому partial/completed birth send провалил бы
scenario.

Production seam: публичный `sv_endpoint_run(SvEndpointOptions)`. Authoritative
setup и loaded DNA поступают через controlled peer; реальные SDL events проходят
native birth adapter и публичную production birth-interaction/input-router
границу. `SvEndpointOutcome` наблюдает общую DNA availability, owner, stable
highlight, committed draft и mode state. Scenario включён в default и
`SV_CLASS_BEFORE_RACE=0` inventories. Test-only behavior path и изменения
legacy/shared production files не добавлены.

Это coverage-only ticket для уже реализованного поведения. При написании нового
assertion первоначальные ожидания были приведены к существующей production
outcome semantics: после подтверждения outcome публикует committed stats, а
Backspace сохраняет флаг подтверждённого draft. Это не выявило и не потребовало
production behavior fix; red -> green TDD-цикл не заявляется.

Focused verification, 2026-10-09:

```sh
for birth_case in \
  auto-dna-shared-manual-advance \
  auto-dna-shared-backtrack-reentry \
  auto-dna-ordered-shared-mode-backtrack \
  auto-dna-full \
  auto-dna-ordered-keyboard \
  auto-dna-skip-trait-body \
  auto-dna-dedicated-iddc \
  auto-dna-dedicated-pvp-maia \
  auto-dna-incompatible-class \
  auto-dna-ordered-mode-missing
do
  SV_BIRTH_CASE="$birth_case" \
    python3 -B tests/sv_name_editor_submit_checks.py
done
```

Результат: **PASS**, общий exit 0. Все десять cases прошли в default и alternate
compile configurations. Проверены early и late shared invalidation, manual
advance, Backspace/re-entry, valid aggregate, ordered stats suspension/resume,
unavailable trait/body skipping, dedicated IDDC/PvP и post-mode PvP Maia,
existing incompatible-class и missing-mode controls. Runner при каждом запуске
собрал production endpoint с `-Wall -Wextra -Werror`, ASan и UBSan; compiler,
sanitizer и assertion failures в итоговой matrix не было. Declared-font fallback
и SDL `Leaked thread` diagnostics совпали с известным focused-runner output и не
сопровождались failure. `git diff --check` — **PASS**.

Полные `sv-build`, `sv-core`, controller review и общий build/test/review workflow
не запускались: их выполняет orchestrator после assembly. Новых побочных
improvement opportunities не обнаружено. `P1-birth-runtime-evidence` / P1-AC30
для accepted contract digest
`690a04cefb7fd0e1615dce46d1a538e1dcbd856232af1a1373be93f987095c82`
остаётся **deferred; not run** у named full-acceptance producer/verifier и этим
ticket не объявляется выполненным.
